/*
**	macflipliquid3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 2, 2017. 
**
**	Permission is hereby granted, free of charge, to any person obtaining a copy of
**	this software and associated documentation files (the "Software"), to deal in
**	the Software without restriction, including without limitation the rights to use,
**	copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
**	Software, and to permit persons to whom the Software is furnished to do so,
**	subject to the following conditions:
**
**	The above copyright notice and this permission notice shall be included in all copies
**	or substantial portions of the Software.
**
**	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
**	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
**	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
**	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
**	CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
**	OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//
#include "macflipliquid3.h"
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/shared_bitarray3.h>
#include <shiokaze/array/macarray_extrapolator3.h>
#include <shiokaze/array/macarray_interpolator3.h>
#include <shiokaze/array/array_upsampler3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/filesystem.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
macflipliquid3::macflipliquid3 () {
	//
	m_param.PICFLIP = 0.95;
	m_highres_particlerasterizer.set_name("Highresolution Particle Rasterizer for FLIP","HighresRasterizer");
}
//
void macflipliquid3::configure( configuration &config ) {
	//
	config.get_double("PICFLIP",m_param.PICFLIP,"PICFLIP blending factor");
	assert( m_param.PICFLIP >= 0.0 && m_param.PICFLIP <= 1.0 );
	//
	macliquid3::configure(config);
	//
	m_double_shape = 2 * m_shape;
	m_half_dx = 0.5 * m_dx;
	//
	config.set_default_double("HighresRasterizer.RadiusFactor",1.0);
	config.set_default_double("HighresRasterizer.WeightFactor",2.0);
	config.set_default_unsigned("HighresRasterizer.NeighborLookUpCells",2);
	//
	m_highres_particlerasterizer->set_environment("shape",&m_double_shape);
	m_highres_particlerasterizer->set_environment("dx",&m_half_dx);
	//
	m_highres_macsurfacetracker->set_environment("shape",&m_double_shape);
	m_highres_macsurfacetracker->set_environment("dx",&m_half_dx);
}
//
void macflipliquid3::post_initialize () {
	//
	macliquid3::post_initialize();
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( ">>> Started FLIP initialization\n" );
	//
	extend_both();
	m_flip->seed(m_fluid,[&](const vec3d &p){ return interpolate_solid(p); },m_velocity);
	//
	console::dump( "<<< Initialization finished. Took %s\n", timer.stock("initialization").c_str());
}
//
void macflipliquid3::idle() {
	//
	scoped_timer timer(this);
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity),m_dx);
	double CFL = m_timestepper->get_current_CFL();
	unsigned step = m_timestepper->get_step_count();
	timer.tick(); console::dump( ">>> %s step started (dt=%.2e,CFL=%.2f)...\n", dt, CFL, console::nth(step).c_str());
	//
	shared_macarray3<float> face_density(m_shape);
	shared_macarray3<float> save_velocity(m_shape);
	shared_macarray3<float> momentum(m_shape);
	shared_macarray3<float> mass(m_shape);
	//
	// Update fluid levelset
	m_flip->update([&](const vec3d &p){ return interpolate_solid(p); },m_fluid);
	//
	// Advect fluid levelset
	m_macsurfacetracker->advect(m_fluid,m_solid,m_velocity,dt);
	//
	// Advect FLIP particles
	m_flip->advect(
		[&](const vec3d &p){ return interpolate_solid(p); },
		[&](const vec3d &p){ return interpolate_velocity(p); },
		m_timestepper->get_current_time(),dt);
	//
	// Grid velocity advection
	m_macadvection->advect_vector(m_velocity,m_velocity,m_fluid,dt);
	//
	// Mark bullet particles
	m_flip->mark_bullet(
		[&](const vec3d &p){ return interpolate_fluid(p); },
		[&](const vec3d &p){ return interpolate_velocity(p); },
		m_timestepper->get_current_time()
	);
	//
	// Correct positions
	m_flip->correct([&](const vec3d &p){ return interpolate_fluid(p); });
	//
	// Reseed particles
	m_flip->seed(m_fluid,
		[&](const vec3d &p){ return interpolate_solid(p); },
		m_velocity
	);
	//
	// Splat momentum and mass of FLIP particles onto grids
	m_flip->splat(momentum(),mass());
	//
	// Compute face mass
	timer.tick(); console::dump( "Computing face mass..." );
	m_macutility->compute_face_density(m_solid,m_fluid,face_density());
	console::dump( "Done. Took %s\n", timer.stock("compute_face_mass").c_str());
	//
	// Compute the combined grid velocity
	timer.tick(); console::dump( "Computing combined grid velocity..." );
	//
	shared_macarray3<float> overwritten_velocity(m_shape);
	overwritten_velocity->activate_as(mass());
	overwritten_velocity->parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn ) {
		double m = mass()[dim](i,j,k);
		double grid_mass = std::max(0.0,face_density()[dim](i,j,k)-m);
		it.set((grid_mass*m_velocity[dim](i,j,k)+momentum()[dim](i,j,k)) / (grid_mass+m));
	});
	//
	// Velocity overwrite
	overwritten_velocity->const_serial_actives([&](int dim, int i, int j, int k, auto &it) {
		m_velocity[dim].set(i,j,k,it());
	});
	console::dump( "Done. Took %s\n", timer.stock("compute_combined_velocity").c_str());
	//
	// Save the current velocity
	save_velocity->copy(m_velocity);
	//
	// Add external force
	inject_external_force(m_velocity,dt);
	//
	// Set volume correction
	set_volume_correction(m_macproject.get());
	//
	// Project
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
	//
	// Extend both the level set and velocity
	extend_both();
	//
	// Update FLIP momentum
	m_flip->update(save_velocity(),m_velocity,dt,(macliquid3::m_param).gravity,m_param.PICFLIP);
	//
	console::dump( "<<< %s step done. Took %s\n", console::nth(step).c_str(), timer.stock("simstep").c_str());
	//
	// Export mesh
	export_mesh();
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void macflipliquid3::do_export_mesh( unsigned frame ) const {
	//
	scoped_timer timer(this);
	//
	timer.tick(); console::dump( "Computing high-resolution levelset..." );
	//
	shared_array3<float> doubled_fluid(m_double_shape.cell(),1.0);
	shared_array3<float> doubled_solid(m_double_shape.nodal(),1.0);
	//
	array_upsampler3::upsample_to_double_cell<float>(m_fluid,m_dx,doubled_fluid());
	array_upsampler3::upsample_to_double_nodal<float>(m_solid,m_dx,doubled_solid());
	//
	shared_bitarray3 mask(m_double_shape);
	shared_array3<float> sizing_array(m_shape);
	//
	std::vector<particlerasterizer3_interface::Particle3> points, ballistic_points;
	std::vector<macflip3_interface::particle3> particles = m_flip->get_particles();
	for( int n=0; n<particles.size(); ++n ) {
		//
		particlerasterizer3_interface::Particle3 point;
		point.p = particles[n].p;
		point.r = particles[n].r;
		double levelset_value = interpolate_fluid(particles[n].p);
		//
		if( levelset_value < 0.5*m_dx ) {
			points.push_back(point);
			mask().set(mask->shape().clamp(point.p/m_half_dx));
		} else {
			if( particles[n].bullet ) ballistic_points.push_back(point);
		}
		//
		vec3i pi = m_shape.find_cell(point.p/m_dx);
		sizing_array->set(pi,std::max((float)particles[n].sizing_value,sizing_array()(pi)));
	}
	//
	mask().dilate(4);
	doubled_fluid->activate_as(mask());
	//
	shared_array3<float> particle_levelset(m_double_shape,0.125*m_dx);
	m_highres_particlerasterizer->build_levelset(particle_levelset(),mask(),points);
	//
	doubled_fluid->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
		double rate = array_interpolator3::interpolate(sizing_array(),0.5*vec3d(i,j,k));
		double f = it(), p = particle_levelset()(i,j,k);
		it.set( rate * std::min(f,p) + (1.0-rate) * f );
	});
	//
	console::dump( "Done. Took %s\n", timer.stock("generate_highres_mesh").c_str());
	//
	auto vertex_color_func = [&](const vec3d &p) { return p; };
	auto uv_coordinate_func = [&](const vec3d &p) { return vec2d(p[0],0.0); };
	//
	timer.tick(); console::dump( "Generating mesh..." );
	m_highres_macsurfacetracker->export_fluid_mesh(m_export_path,frame,doubled_solid(),doubled_fluid(),vertex_color_func,uv_coordinate_func);
	console::dump( "Done. Took %s\n", timer.stock("export_highres_mesh").c_str());
	//
	std::string particle_path = console::format_str("%s/%d_particles.dat",m_export_path.c_str(),frame);
	timer.tick(); console::dump( "Writing ballistic particles..." );
	FILE *fp = fopen(particle_path.c_str(),"wb");
	size_t size = ballistic_points.size();
	fwrite(&size,1,sizeof(unsigned),fp);
	for( size_t n=0; n<size; ++n ) {
		float position[3] = { (float)ballistic_points[n].p.v[0],
							  (float)ballistic_points[n].p.v[1],
							  (float)ballistic_points[n].p.v[2] };
		float radius = ballistic_points[n].r;
		fwrite(position,3,sizeof(float),fp);
		fwrite(&radius,1,sizeof(float),fp);
	}
	fclose(fp);
	console::dump( "Done. Size=%d. Took %s\n", size, timer.stock("write_ballistic").c_str());
	//
	do_export_solid_mesh();
}
//
void macflipliquid3::render_mesh( unsigned frame ) const {
	//
	scoped_timer timer(this);
	global_timer::pause();
	//
	assert(console::get_root_path().size());
	//
	std::string mitsuba_path = console::get_root_path() + "/flipliquid_mitsuba";
	std::string copy_from_path = filesystem::find_resource_path("flipliquid","mitsuba");
	if( ! filesystem::is_exist(mitsuba_path)) {
		if( filesystem::is_exist(copy_from_path)) {
			console::run( "cp -r %s %s", copy_from_path.c_str(), mitsuba_path.c_str());
		} else {
			console::dump( "Could not lcoate mitsuba files (%s).\n", copy_from_path.c_str());
			exit(0);
		}
	}
	//
	std::string render_command = console::format_str("cd %s; python render.py %d mesh %g %g %g %d %g %g %g %g %g %g",
				mitsuba_path.c_str(),
				frame,
				0.5, 0.5, 1.0,
				(macliquid3::m_param).render_sample_count,
				(macliquid3::m_param).target[0], (macliquid3::m_param).target[1], (macliquid3::m_param).target[2],
				(macliquid3::m_param).origin[0], (macliquid3::m_param).origin[1], (macliquid3::m_param).origin[2]);
	//
	console::dump("Running command: %s\n", render_command.c_str());
	console::system(render_command.c_str());
	//
	if( (macliquid3::m_param).render_transparent ) {
		//
		std::string render_command = console::format_str("cd %s; python render.py %d transparent %g %g %g %d %g %g %g %g %g %g",
				mitsuba_path.c_str(),
				frame,
				0.5, 0.5, 1.0,
				(macliquid3::m_param).render_transparent_sample_count,
				(macliquid3::m_param).target[0], (macliquid3::m_param).target[1], (macliquid3::m_param).target[2],
				(macliquid3::m_param).origin[0], (macliquid3::m_param).origin[1], (macliquid3::m_param).origin[2]);
		//
		console::dump("Running command: %s\n", render_command.c_str());
		console::system(render_command.c_str());
	}
	//
	global_timer::resume();
}
//
double macflipliquid3::interpolate_fluid( const vec3d &p ) const {
	return array_interpolator3::interpolate(m_fluid,p/m_dx-vec3d(0.5,0.5,0.5));
}
//
double macflipliquid3::interpolate_solid( const vec3d &p ) const {
	return array_interpolator3::interpolate(m_solid,p/m_dx);
}
//
vec3d macflipliquid3::interpolate_velocity( const vec3d &p ) const {
	return macarray_interpolator3::interpolate(m_velocity,vec3d(),m_dx,p);
}
//
void macflipliquid3::draw( graphics_engine &g ) const {
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
	//
	// Draw projection component
	m_macproject->draw(g);
	//
	// Draw FLIP particles
	m_flip->draw(g,m_timestepper->get_current_time());
	//
	// Visualize solid
	shared_array3<float> solid_to_visualize(m_solid.shape());
	if( ! m_gridutility->assign_visualizable_solid(m_dylib,m_dx,solid_to_visualize())) solid_to_visualize->copy(m_solid);
	if( array_utility3::levelset_exist(solid_to_visualize())) m_gridvisualizer->draw_solid(g,solid_to_visualize());
	//
	// Visualize levelset
	m_gridvisualizer->draw_fluid(g,m_solid,m_fluid);
}
//
extern "C" module * create_instance() {
	return new macflipliquid3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//