/*
**	macflipliquid2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 17, 2017. 
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
#include "macflipliquid2.h"
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/macarray_extrapolator2.h>
#include <shiokaze/array/macarray_interpolator2.h>
#include <shiokaze/array/array_interpolator2.h>
#include <shiokaze/utility/utility.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
macflipliquid2::macflipliquid2 () {
	m_param.PICFLIP = 0.95;
}
//
void macflipliquid2::configure( configuration &config ) {
	//
	config.get_double("PICFLIP",m_param.PICFLIP,"PICFLIP blending factor");
	assert( m_param.PICFLIP >= 0.0 && m_param.PICFLIP <= 1.0 );
	//
	macliquid2::configure(config);
}
//
void macflipliquid2::post_initialize () {
	//
	macliquid2::post_initialize();
	extend_both();
	//
	m_flip->assign_solid(m_solid);
	m_flip->seed(m_fluid,m_velocity);
}
//
void macflipliquid2::idle() {
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity)/m_dx);
	unsigned step = m_timestepper->get_step_count();
	//
	shared_macarray2<double> face_density(m_shape);
	shared_macarray2<double> save_velocity(m_shape);
	shared_macarray2<double> momentum(m_shape);
	shared_macarray2<double> mass(m_shape);
	//
	// Advect FLIP particles and get the levelset after the advection
	m_flip->advect(m_velocity,m_timestepper->get_current_time(),dt);
	m_flip->get_levelset(m_fluid);
	m_macsurfacetracker->assign(m_solid,m_fluid);
	//
	// Grid velocity advection
	m_macadvection->advect_vector(m_velocity,m_velocity,dt);
	//
	// Splat momentum and mass of FLIP particles onto grids
	m_flip->splat(momentum(),mass());
	//
	// Compute face mass
	m_macutility->compute_face_density(m_solid,m_fluid,face_density());
	//
	// Compute the combined grid velocity
	auto mass_accessors = mass->get_const_accessors();
	auto face_density_accessors = face_density->get_const_accessors();
	auto velocity_accessors = m_velocity.get_const_accessors();
	auto momentum_accessors = momentum->get_const_accessors();
	//
	shared_macarray2<double> overwritten_velocity(m_shape);
	overwritten_velocity->activate_as(mass());
	overwritten_velocity->parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
		double m = mass_accessors[tn](dim,i,j);
		double grid_mass = std::max(0.0,face_density_accessors[tn](dim,i,j)-m);
		it.set((grid_mass*velocity_accessors[tn](dim,i,j)+momentum_accessors[tn](dim,i,j)) / (grid_mass+m));
	});
	//
	// Velocity overwrite
	auto velocity_accessor = m_velocity.get_serial_accessor();
	overwritten_velocity->const_serial_actives([&](int dim, int i, int j, auto &it) {
		velocity_accessor.set(dim,i,j,it());
	});
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
	// Update FLIP velocity
	m_flip->update(save_velocity(),m_velocity,dt,(macliquid2::m_param).gravity,m_param.PICFLIP);
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void macflipliquid2::draw( const graphics_engine &g, int width, int height ) const {
	//
	// Draw grid lines
	m_gridvisualizer->draw_grid(g);
	//
	// Draw FLIP
	m_flip->draw(g,m_timestepper->get_current_time());
	//
	// Draw projection component
	m_macproject->draw(g);
	//
	// Draw solid levelset
	m_gridvisualizer->draw_solid(g,m_solid);
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
}
//
extern "C" module * create_instance() {
	return new macflipliquid2;
}
//
extern "C" const char *license() {
	return "MIT";
}
//