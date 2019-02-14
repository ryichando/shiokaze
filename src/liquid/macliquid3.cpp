/*
**	macliquid3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 18, 2017.
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
#include "macliquid3.h"
#include <shiokaze/core/filesystem.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_upsampler3.h>
#include <shiokaze/array/macarray_extrapolator3.h>
#include <shiokaze/array/array_gradient3.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/core/filesystem.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
macliquid3::macliquid3() {
	//
	m_shape = shape3(64,32,64);
	m_dx = m_shape.dx();
}
//
void macliquid3::load( configuration &config ) {
	//
	std::string name("waterdrop3"); config.get_string("Name",name,"Scene file name");
	m_dylib.open_library(filesystem::resolve_libname(name));
	m_dylib.load(config);
	m_dylib.overwrite(config);
}
//
void macliquid3::configure( configuration &config ) {
	//
	if( console::get_root_path().size()) {
		m_export_path = console::get_root_path() + "/mesh";
		if( ! filesystem::is_exist(m_export_path)) filesystem::create_directory(m_export_path);
	}
	//
	m_dylib.configure(config);
	//
	m_param.render_mesh = console::system("mitsuba > /dev/null 2>&1") == 0;
	//
	config.get_vec3d("Gravity",m_param.gravity.v,"Gravity vector");
	config.get_bool("VolumeCorrection",m_param.volume_correction,"Should perform volume correction");
	config.get_double("VolumeChangeTolRatio",m_param.volume_change_tol_ratio,"Volume change tolerance ratio");
	config.get_string("MeshPath",m_export_path, "Path to the directory to export meshes");
	config.get_bool("RenderMesh",m_param.render_mesh,"Whether to render mesh files");
	config.get_bool("RenderTransparent",m_param.render_transparent,"Whether to render transparent view");
	config.get_unsigned("RenderSampleCount",m_param.render_sample_count,"Sample count for rendering");
	config.get_unsigned("RenderTransparentSampleCount",m_param.render_transparent_sample_count,"Sample count for transparent rendering");
	config.get_vec3d("TargetPos",m_param.target.v,"Camera target position");
	config.get_vec3d("OriginPos",m_param.origin.v,"Camera origin position");
	config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
	config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
	config.get_unsigned("ResolutionZ",m_shape[2],"Resolution towards Z axis");
	double scale (1.0);
	config.get_double("ResolutionScale",scale,"Resolution doubling scale");
	//
	m_shape *= scale;
	m_dx = m_shape.dx();
	//
	m_doubled_shape = 2 * m_shape;
	m_half_dx = 0.5 * m_dx;
	//
	m_highres_mesher->set_environment("shape",&m_doubled_shape);
	m_highres_mesher->set_environment("dx",&m_half_dx);
}
//
void macliquid3::post_initialize() {
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( ">>> Started initialization (%dx%dx%d)\n", m_shape[0], m_shape[1], m_shape[2] );
	//
	auto initialize_func = reinterpret_cast<void(*)(const shape3 &m_shape, double m_dx)>(m_dylib.load_symbol("initialize"));
	if( initialize_func ) {
		initialize_func(m_shape,m_dx);
	}
	//
	// Initialize arrays
	m_force_exist = false;
	m_velocity.initialize(m_shape);
	m_external_force.initialize(m_shape);
	m_solid.initialize(m_shape.nodal());
	m_fluid.initialize(m_shape.cell());
	//
	// Assign initial variables from script
	m_macutility->assign_initial_variables(m_dylib,m_velocity,&m_solid,&m_fluid);
	m_velocity.set_touch_only_actives(true);
	//
	// Assign to surface tracker
	m_macsurfacetracker->assign(m_solid,m_fluid);
	timer.tick(); console::dump( "Computing the initial volume..." );
	m_initial_volume = m_gridutility->get_volume(m_solid,m_fluid);
	//
	shared_macarray3<double> velocity_actives(m_velocity.type());
	for( int dim : DIMS3 ) {
		velocity_actives()[dim].activate_inside_as(m_fluid);
		velocity_actives()[dim].activate_inside_as(m_fluid,vec3i(dim==0,dim==1,dim==2));
	}
	m_velocity.copy_active_as(velocity_actives());
	//
	console::dump( "Done. Volume = %.3f. Took %s.\n", m_initial_volume, timer.stock("initialize_compute_volume").c_str());
	//
	double max_u = m_macutility->compute_max_u(m_velocity);
	if( max_u ) {
		//
		// Projection
		double CFL = m_timestepper->get_target_CFL();
		m_macproject->project(CFL*m_dx/max_u,m_velocity,m_solid,m_fluid);
	}
	//
	console::dump( "<<< Initialization finished. Took %s\n", timer.stock("initialization").c_str());
}
//
void macliquid3::setup_window( std::string &name, int &width, int &height ) const {
	height = width;
}
//
void macliquid3::inject_external_force( macarray3<double> &velocity, double dt ) {
	//
	if( m_force_exist ) {
		velocity += m_external_force;
		m_external_force.clear();
		m_force_exist = false;
	}
	// Add gravity force
	m_velocity += dt*m_param.gravity;
}
//
void macliquid3::set_volume_correction( macproject3_interface *m_macproject ) {
	//
	// Compute volume
	scoped_timer timer(this);
	timer.tick(); console::dump( "Computing volume..." );
	double volume = m_gridutility->get_volume(m_solid,m_fluid);
	console::dump( "Done. Volume = %.3f (Volume change: %.2f%%). Took %s\n", volume, 100.0*volume/m_initial_volume, timer.stock("compute_volume").c_str());
	console::write("volume", volume);
	console::write("volume_change", volume/m_initial_volume);
	if( volume/m_initial_volume < 0.01 ) {
		console::dump( "Volume is nearly zero. Quitting...\n");
		exit(0);
	}
	//
	// Set volume correction if requested
	if( m_param.volume_correction ) {
		double change_ratio = std::abs(1.0-volume/m_initial_volume);
		if( change_ratio > m_param.volume_change_tol_ratio ) {
			double target_volume;
			if( volume > m_initial_volume ) {
				target_volume = (1.0+m_param.volume_change_tol_ratio) * m_initial_volume;
			} else {
				target_volume = (1.0-m_param.volume_change_tol_ratio) * m_initial_volume;
			}
			console::dump( "Report: volume correction is turned on. (target=%.3f, original=%.3f)\n", target_volume, m_initial_volume );
			m_macproject->set_target_volume(volume,target_volume);
		} else {
			console::dump( "Report: volume correction is not turned on (change ratio does not exceed %g but is only %.4f).\n", m_param.volume_change_tol_ratio, change_ratio);
		}
	}
}
//
void macliquid3::extend_both() {
	//
	scoped_timer timer(this);
	//
	timer.tick(); console::dump( "Extending velocity field...");
	unsigned width = m_fluid.get_levelset_halfwidth()+m_timestepper->get_current_CFL();
	macarray_extrapolator3::extrapolate<double>(m_velocity,width);
	m_macutility->constrain_velocity(m_solid,m_velocity);
	m_fluid.dilate(width);
	console::dump( "Done. Count=%d. Took %s\n", width, timer.stock("extend_velocity").c_str());
}
//
void macliquid3::idle() {
	//
	scoped_timer timer(this);
	//
	unsigned step = m_timestepper->get_step_count()+1;
	timer.tick(); console::dump( ">>> %s step started...\n", console::nth(step).c_str());
	//
	// Compute the timestep size
	timer.tick(); console::dump( "Computing time step...");
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity)/m_dx);
	double CFL = m_timestepper->get_current_CFL();
	console::dump( "Done. dt=%.2e,CFL=%.2f. Took %s\n", dt, CFL, timer.stock("compute_timestep").c_str());
	//
	// Extend both the velocity field and the level set
	extend_both();
	//
	// Advect surface
	m_macsurfacetracker->assign(m_solid,m_fluid);
	m_macsurfacetracker->advect(m_velocity,dt);
	m_macsurfacetracker->get(m_fluid);
	//
	// Velocity advection
	shared_macarray3<double> velocity_save(m_velocity);
	m_macadvection->advect_vector(m_velocity,velocity_save(),m_fluid,dt,"velocity");
	//
	// Add external force
	inject_external_force(m_velocity,dt);
	//
	// Set volume correction
	set_volume_correction(m_macproject.get());
	//
	// Projection
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
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
void macliquid3::export_mesh() const {
	//
	scoped_timer timer(this);
	if( m_export_path.size()) {
		int frame = m_timestepper->should_export_frame();
		if( frame ) {
			timer.tick(); console::dump( ">>> Exporting %s mesh (time=%g secs)\n", console::nth(frame).c_str(),m_timestepper->get_current_time());
			do_export_mesh(frame);
			console::dump( "<<< Done. Took %s\n", timer.stock("export_mesh").c_str());
			if( m_param.render_mesh ) {
				render_mesh(frame);
			}
		}
	}
}
//
void macliquid3::do_export_mesh( unsigned frame ) const {
	//
	scoped_timer timer(this);
	//
	assert(m_export_path.size());
	//
	auto vertex_color_func = [&](const vec3d &p) { return p; };
	auto uv_coordinate_func = [&](const vec3d &p) { return vec2d(p[0],0.0); };
	//
	timer.tick(); console::dump( "Generating mesh..." );
	m_macsurfacetracker->export_fluid_mesh(m_export_path,frame,vertex_color_func,uv_coordinate_func);
	console::dump( "Done. Took %s\n", timer.stock("generate_mesh").c_str());
	do_export_solid_mesh();
}
//
void macliquid3::render_mesh( unsigned frame ) const {
	//
	scoped_timer timer(this);
	global_timer::pause();
	//
	assert(console::get_root_path().size());
	//
	std::string mitsuba_path = console::get_root_path() + "/liquid_mitsuba";
	std::string copy_from_path = filesystem::find_resource_path("liquid","mitsuba");
	if( ! filesystem::is_exist(mitsuba_path)) {
		if( filesystem::is_exist(copy_from_path)) {
			console::run( "cp -r %s %s", copy_from_path.c_str(), mitsuba_path.c_str());
		} else {
			console::dump( "Could not lcoate mitsuba files (%s).\n", copy_from_path.c_str());
			exit(0);
		}
	}
	//
	std::string render_command = console::format_str("cd %s; python render.py %d %d %g %g %g %g %g %g %s",
				mitsuba_path.c_str(),
				frame,
				m_param.render_sample_count,
				m_param.target[0], m_param.target[1], m_param.target[2],
				m_param.origin[0], m_param.origin[1], m_param.origin[2],"mesh");
	//
	console::dump("Running command: %s\n", render_command.c_str());
	console::system(render_command.c_str());
	//
	if( m_param.render_transparent ) {
		//
		std::string render_command = console::format_str("cd %s; python render.py %d %d %g %g %g %g %g %g %s",
				mitsuba_path.c_str(),
				frame,
				m_param.render_transparent_sample_count,
				m_param.target[0], m_param.target[1], m_param.target[2],
				m_param.origin[0], m_param.origin[1], m_param.origin[2],"transparent");
		//
		console::dump("Running command: %s\n", render_command.c_str());
		console::system(render_command.c_str());
	}
	//
	global_timer::resume();
}
//
void macliquid3::do_export_solid_mesh() const {
	//
	scoped_timer timer(this);
	auto uv_coordinate_func = [&](const vec3d &p) {
		return vec2d(p[0],p[2]);
	};
	//
	std::string static_solids_directory_path = console::format_str("%s/static_solids",m_export_path.c_str());
	std::string path_wo_suffix = console::format_str("%s/levelset_solid",static_solids_directory_path.c_str());
	//
	if( ! filesystem::is_exist(static_solids_directory_path)) {
		//
		filesystem::create_directory(static_solids_directory_path);
		meshexporter3_driver &m_mesh_exporter = const_cast<meshexporter3_driver &>(this->m_mesh_exporter);
		//
		if( array_utility3::levelset_exist(m_solid)) {
			//
			timer.tick(); console::dump( "Generating solid mesh..." );
			shared_array3<double> solid_to_visualize(m_doubled_shape.nodal());
			if( ! m_gridutility->assign_visualizable_solid(m_dylib,m_half_dx,solid_to_visualize())) {
				array_upsampler3::upsample_to_double_nodal<double>(m_solid,m_dx,solid_to_visualize());
			}
			//
			std::vector<vec3d> vertices;
			std::vector<std::vector<size_t> > faces;
			m_highres_mesher->generate_mesh(solid_to_visualize(),vertices,faces);
			//
			m_mesh_exporter->set_mesh(vertices,faces);
			std::vector<vec2d> uv_coordinates (vertices.size());
			for( unsigned n=0; n<vertices.size(); ++n ) {
				uv_coordinates[n] = uv_coordinate_func(vertices[n]);
			}
			//
			m_mesh_exporter->set_texture_coordinates(uv_coordinates);
			m_mesh_exporter->export_ply(console::format_str("%s.ply",path_wo_suffix.c_str()));
			//
			console::dump("Done. Took %s.\n", timer.stock("export_solid_mesh").c_str());
			//
		} else {
			//
			// Export dummy solid file
			std::vector<vec3d> vertices;
			std::vector<std::vector<size_t> > faces(1);
			vertices.push_back(vec3d(1e3,1e3,1e3));
			vertices.push_back(vec3d(1e3+1.0,1e3,1e3));
			vertices.push_back(vec3d(1e3,1e3,1e3+1.0));
			faces[0].push_back(0);
			faces[0].push_back(1);
			faces[0].push_back(2);
			m_mesh_exporter->set_mesh(vertices,faces);
			m_mesh_exporter->export_ply(console::format_str("%s.ply",path_wo_suffix.c_str()));
		}
	}
}
//
void macliquid3::draw( graphics_engine &g, int width, int height ) const {
	//
	g.color4(1.0,1.0,1.0,0.5);
	graphics_utility::draw_wired_box(g);
	//
	// Draw projection component
	m_macproject->draw(g);
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
	//
	// Visualize solid
	shared_array3<double> solid_to_visualize(m_solid.shape());
	if( ! m_gridutility->assign_visualizable_solid(m_dylib,m_dx,solid_to_visualize())) solid_to_visualize->copy(m_solid);
	if( array_utility3::levelset_exist(solid_to_visualize())) m_gridvisualizer->draw_solid(g,solid_to_visualize());
	//
	// Visualize levelset
	m_macsurfacetracker->draw(g);
}
//
extern "C" module * create_instance() {
	return new macliquid3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//