/*
**	macflipsmoke3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Aug 22, 2017.
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
#include "macflipsmoke3.h"
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/macarray_interpolator3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
macflipsmoke3::macflipsmoke3 () {
	//
	m_param.PICFLIP = 0.95;
	m_param.gridmass = 1.0;
}
//
void macflipsmoke3::configure( configuration &config ) {
	//
	config.get_double("GridMass",m_param.gridmass,"Mass of grid cell");
	config.get_double("PICFLIP",m_param.PICFLIP,"PICFLIP blending factor");
	assert( m_param.PICFLIP >= 0.0 && m_param.PICFLIP <= 1.0 );
	//
	macsmoke3::configure(config);
}
//
void macflipsmoke3::post_initialize () {
	//
	macsmoke3::post_initialize();
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( ">>> Started FLIP initialization\n" );
	//
	shared_array3<Real> fluid(m_shape);
	fluid->set_as_levelset(m_dx);
	m_flip->resample(fluid(),[&](const vec3d &p){ return interpolate_solid(p); },m_velocity);
	//
	console::dump( "<<< Initialization finished. Took %s\n", timer.stock("initialization").c_str());
}
//
void macflipsmoke3::idle() {
	//
	scoped_timer timer(this);
	//
	// Add to graph
	add_to_graph();
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity),m_dx);
	double CFL = m_timestepper->get_current_CFL();
	unsigned step = m_timestepper->get_step_count();
	timer.tick(); console::dump( ">>> %s step started (dt=%.2e,CFL=%.2f)...\n", dt, CFL, console::nth(step).c_str());
	//
	// Advect FLIP particles and get the levelset after the advection
	m_flip->advect(
		[&](const vec3d &p){ return interpolate_solid(p); },
		[&](const vec3d &p){ return interpolate_velocity(p); },
		m_timestepper->get_current_time(),dt);
	//
	// Correct positions
	m_flip->correct([&](const vec3d &p){ return -1.0; },m_velocity);
	//
	// Reseed particles
	shared_array3<Real> fluid(m_shape);
	fluid->set_as_levelset(m_dx);
	m_flip->resample(m_fluid,
		[&](const vec3d &p){ return interpolate_solid(p); },
		m_velocity
	);
	//
	// Advection
	if( (macsmoke3::m_param).use_dust ) advect_dust_particles(m_velocity,dt);
	else {
		m_density.dilate(std::ceil(m_timestepper->get_current_CFL()));
		m_macadvection->advect_scalar(m_density,m_velocity,m_fluid,dt,"density");
		double minimal_density = (macsmoke3::m_param).minimal_density;
		m_density.parallel_actives([&](auto &it) {
			if( std::abs(it()) <= minimal_density ) it.set_off();
		});
	}
	//
	// Splat momentum and mass of FLIP particles onto grids
	shared_macarray3<macflip3_interface::mass_momentum3> mass_and_momentum(m_shape);
	m_flip->splat(mass_and_momentum());
	//
	// Overwrite grid velocity
	m_velocity.parallel_actives([&]( int dim, int i, int j, int k, auto &it, int tn) {
		const auto value = mass_and_momentum()[dim](i,j,k);
		if( value.mass ) it.set(value.momentum / value.mass);
	});
	//
	// Save the current velocity
	shared_macarray3<Real> save_velocity(m_velocity);
	//
	// Add external force
	inject_external_force(m_velocity);
	//
	// Add buoyancy force
	add_buoyancy_force (m_velocity,m_density,dt);
	//
	// Add source
	add_source (m_velocity,m_density,m_timestepper->get_current_time(),dt);
	//
	// Project
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
	//
	// Update FLIP momentum
	m_flip->update(save_velocity(),m_velocity,dt,vec3d(),m_param.PICFLIP);
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
	//
	console::dump( "<<< %s step done. Took %s\n", console::nth(step).c_str(), timer.stock("simstep").c_str());
	//
	// Export density
	export_density();
}
//
double macflipsmoke3::interpolate_solid( const vec3d &p ) const {
	return array_interpolator3::interpolate(m_solid,p/m_dx);
}
//
vec3d macflipsmoke3::interpolate_velocity( const vec3d &p ) const {
	return macarray_interpolator3::interpolate(m_velocity,vec3d(),m_dx,p);
}
//
void macflipsmoke3::draw( graphics_engine &g ) const {
	//
	g.color4(1.0,1.0,1.0,0.5);
	graphics_utility::draw_wired_box(g);
	//
	// Draw density
	if( (macsmoke3::m_param).use_dust ) draw_dust_particles(g);
	else m_gridvisualizer->draw_density(g,m_density);
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
	// Draw graph
	m_graphplotter->draw(g);
}
//
extern "C" module * create_instance() {
	return new macflipsmoke3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//