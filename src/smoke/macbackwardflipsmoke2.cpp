/*
**	macbackwardflipsmoke2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 8, 2017.
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
#include "macbackwardflipsmoke2.h"
#include <shiokaze/array/shared_array2.h>
//
SHKZ_USING_NAMESPACE
//
macbackwardflipsmoke2::macbackwardflipsmoke2 () {
	m_use_regular_velocity_advection = false;
}
//
void macbackwardflipsmoke2::configure( configuration &config ) {
	config.get_bool("UseRegularVelocityAdvection",m_use_regular_velocity_advection);
	macsmoke2::configure(config);
}
//
void macbackwardflipsmoke2::idle() {
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity),m_dx);
	//
	// Set of variables
	shared_macarray2<Real> velocity_reconstructed(m_shape);
	//
	// Save the current density and velocity
	shared_array2<Real> density0 (m_density);
	shared_macarray2<Real> velocity0(m_velocity);
	//
	// Backtrace the velocity back in time
	m_backwardflip->backtrace(m_solid,m_fluid);
	//
	// Fetch the new reconstructed velocity
	if( m_backwardflip->fetch(velocity_reconstructed())) {
		//
		// Extrapolate and constrain the reconstructed velocity
		m_macutility->extrapolate_and_constrain_velocity(m_solid,velocity_reconstructed(),(macsmoke2::m_param).extrapolated_width);
		//
		// Compute the dirty velocity
		shared_macarray2<Real> u(velocity_reconstructed());
		m_macadvection->advect_vector(u(),m_velocity,m_fluid,dt);
		m_velocity.copy(u());
		//
	} else {
		//
		velocity_reconstructed->copy(velocity0());
		m_macadvection->advect_vector(m_velocity,m_velocity,m_fluid,dt);
	}
	//
	if( (macsmoke2::m_param).use_dust ) {
		advect_dust_particles(velocity0(),dt);
	} else {
		if( ! m_backwardflip->fetch(m_density)) {
			density0->copy(m_density);
			m_macadvection->advect_scalar(m_density,velocity0(),m_fluid,dt);
		}
	}
	//
	// Save the velocity before projection
	shared_macarray2<Real> velocity_b4_proj(m_velocity);
	//
	// Add external force
	inject_external_force(m_velocity);
	//
	// Add buoyancy force
	add_buoyancy_force (m_velocity,m_density,dt);
	//
	// Add source
	shared_array2<Real> density_added(m_shape);
	add_source(m_velocity,density_added(),m_timestepper->get_current_time(),dt);
	//
	// Project
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
	m_macutility->extrapolate_and_constrain_velocity(m_solid,m_velocity,(macsmoke2::m_param).extrapolated_width);
	//
	if( m_use_regular_velocity_advection ) {
		m_backwardflip->register_buffer(m_velocity,velocity0(),nullptr,nullptr,
			(macsmoke2::m_param).use_dust ? nullptr : &m_density, (macsmoke2::m_param).use_dust ? nullptr : &density0(),
			(macsmoke2::m_param).use_dust ? nullptr : &density_added(),dt);
	} else {
		//
		// Put buffer with one layer
		shared_macarray2<Real> g(m_velocity);
		g() -= velocity_b4_proj();
		//
		m_backwardflip->register_buffer(m_velocity,velocity0(),&velocity_reconstructed(),&g(),
			(macsmoke2::m_param).use_dust ? nullptr : &m_density, (macsmoke2::m_param).use_dust ? nullptr : &density0(),
			&density_added(),dt);
	}
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void macbackwardflipsmoke2::draw( graphics_engine &g ) const {
	//
	macsmoke2::draw(g);
	m_backwardflip->draw(g);
}
//
extern "C" module * create_instance() {
	return new macbackwardflipsmoke2;
}
//
