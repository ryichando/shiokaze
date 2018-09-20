/*
**	macflipsmoke2.cpp
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
#include "macflipsmoke2.h"
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
macflipsmoke2::macflipsmoke2 () {
	//
	m_param.PICFLIP = 0.95;
	m_param.gridmass = 1.0;
}
//
void macflipsmoke2::configure( configuration &config ) {
	//
	config.get_double("GridMass",m_param.gridmass,"Mass of grid cell");
	config.set_bool("LooseInterior",false);
	config.get_double("PICFLIP",m_param.PICFLIP,"PICFLIP blending factor");
	assert( m_param.PICFLIP >= 0.0 && m_param.PICFLIP <= 1.0 );
	//
	macsmoke2::configure(config);
}
//
void macflipsmoke2::post_initialize () {
	//
	macsmoke2::post_initialize();
	//
	m_flip->assign_solid(m_solid);
	m_flip->seed(m_fluid,m_velocity);
}
//
void macflipsmoke2::idle() {
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity)/m_dx);
	//
	// Advect FLIP particles and get the levelset after the advection
	m_flip->advect(m_velocity,m_timestepper->get_current_time(),dt);
	//
	// Advect density and velocity
	if( (macsmoke2::m_param).use_dust ) advect_dust_particles(m_velocity,dt);
	else {
		m_density.dilate(std::ceil(m_timestepper->get_current_CFL()));
		m_macadvection->advect_scalar(m_density,m_velocity,dt);
		double minimal_density = (macsmoke2::m_param).minimal_density;
		m_density.parallel_actives([&](auto &it) {
			if( std::abs(it()) <= minimal_density ) it.set_off();
		});
	}
	//
	// Splat momentum and mass of FLIP particles onto grids
	shared_macarray2<double> mass(m_shape);
	shared_macarray2<double> momentum(m_shape);
	m_flip->splat(momentum(),mass());
	//
	// Overwrite grid velocity
	auto mass_accessors = mass->get_const_accessors();
	auto momentum_accessors = momentum->get_const_accessors();
	//
	m_velocity.parallel_actives([&]( int dim, int i, int j, auto &it, int tn) {
		double m = mass_accessors[tn](dim,i,j);
		if( m ) it.set(momentum_accessors[tn](dim,i,j) / m);
	});
	//
	// Save the current velocity
	shared_macarray2<double> save_velocity(m_velocity);
	//
	// Add external force
	inject_external_force(m_velocity);
	//
	// Add buoyancy force
	add_buoyancy_force(m_velocity,m_density,dt);
	//
	// Add source
	add_source(m_velocity,m_density,m_timestepper->get_current_time(),dt);
	//
	// Project
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
	//
	// Update FLIP momentum
	m_flip->update(save_velocity(),m_velocity,dt,vec2d(),m_param.PICFLIP);
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void macflipsmoke2::draw( const graphics_engine &g, int width, int height ) const {
	//
	// Draw grid lines
	m_gridvisualizer->draw_grid(g);
	//
	// Draw FLIP
	m_flip->draw(g,m_timestepper->get_current_time());
	//
	// Draw density
	if( (macsmoke2::m_param).use_dust ) draw_dust_particles(g);
	else m_gridvisualizer->draw_density(g,m_density);
	//
	// Draw projection component
	m_macproject->draw(g);
	//
	// Draw m_solid levelset
	m_gridvisualizer->draw_solid(g,m_solid);
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
	//
	// Draw energy
	graphics_utility::draw_number(g,"Energy",m_macutility->get_kinetic_energy(m_solid,m_fluid,m_velocity),width,height);
}
//
extern "C" module * create_instance() {
	return new macflipsmoke2;
}
//
extern "C" const char *license() {
	return "MIT";
}
//