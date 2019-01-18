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
#include <shiokaze/utility/utility.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/macarray_extrapolator3.h>
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
	m_param.PICFLIP = 0.95;
}
//
void macflipliquid3::configure( configuration &config ) {
	//
	config.get_double("PICFLIP",m_param.PICFLIP,"PICFLIP blending factor");
	assert( m_param.PICFLIP >= 0.0 && m_param.PICFLIP <= 1.0 );
	//
	macliquid3::configure(config);
}
//
void macflipliquid3::post_initialize () {
	//
	macliquid3::post_initialize();
	extend_both();
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( ">>> Started FLIP initialization\n" );
	//
	m_flip->assign_solid(m_solid);
	m_flip->seed(m_fluid,m_velocity);
	//
	console::dump( "<<< Initialization finished. Took %s\n", timer.stock("initialization").c_str());
}
//
void macflipliquid3::idle() {
	//
	scoped_timer timer(this);
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity)/m_dx);
	double CFL = m_timestepper->get_current_CFL();
	unsigned step = m_timestepper->get_step_count();
	timer.tick(); console::dump( ">>> %s step started (dt=%.2e,CFL=%.2f)...\n", dt, CFL, console::nth(step).c_str());
	//
	shared_macarray3<double> face_density(m_shape);
	shared_macarray3<double> save_velocity(m_shape);
	shared_macarray3<double> momentum(m_shape);
	shared_macarray3<double> mass(m_shape);
	//
	// Advect FLIP particles and get the levelset after the advection
	m_flip->advect(m_velocity,m_timestepper->get_current_time(),dt);
	m_flip->get_levelset(m_fluid);
	m_macsurfacetracker->assign(m_solid,m_fluid);
	//
	// Grid velocity advection
	m_macadvection->advect_vector(m_velocity,m_velocity,dt,"velocity");
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
	shared_macarray3<double> overwritten_velocity(m_shape);
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
	m_flip->export_mesh_and_ballistic_particles(frame,m_export_path);
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
void macflipliquid3::draw( graphics_engine &g, int width, int height ) const {
	//
	g.color4(1.0,1.0,1.0,0.5);
	graphics_utility::draw_wired_box(g);
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
	shared_array3<double> solid_to_visualize(m_solid.shape());
	if( ! m_gridutility->assign_visualizable_solid(m_dylib,m_dx,solid_to_visualize())) solid_to_visualize->copy(m_solid);
	if( array_utility3::levelset_exist(solid_to_visualize())) m_gridvisualizer->draw_solid(g,solid_to_visualize());
	//
	// Visualize levelset
	m_macsurfacetracker->draw(g);
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