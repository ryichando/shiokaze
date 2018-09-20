/*
**	macbackwardflipsmoke3.cpp
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
#include "macbackwardflipsmoke3.h"
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
//
SHKZ_USING_NAMESPACE
//
macbackwardflipsmoke3::macbackwardflipsmoke3 () {
	m_use_regular_velocity_advection = false;
}
//
void macbackwardflipsmoke3::configure( configuration &config ) {
	//
	config.get_bool("UseRegularVelocityAdvection",m_use_regular_velocity_advection);
	macsmoke3::configure(config);
}
//
void macbackwardflipsmoke3::idle() {
	//
	// Compute the timestep size
	scoped_timer timer{this};
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity)/m_dx);
	double CFL = m_timestepper->get_current_CFL();
	unsigned step = m_timestepper->get_step_count();
	timer.tick(); console::dump( ">>> %s step (dt=%.2e,CFL=%.2f) started...\n", dt, CFL, console::nth(step).c_str());
	//
	// Set of variables
	shared_macarray3<double> velocity_reconstructed(m_shape);
	shared_macarray3<double> u_reconstructed(m_shape);
	//
	// Save the current density and velocity
	shared_array3<double> density0 (m_density);
	shared_macarray3<double> velocity0(m_velocity);
	//
	// Backtrace the velocity back in time
	m_backwardflip->backtrace(m_solid,m_fluid);
	//
	// Fetch the new reconstructed velocity
	if(m_backwardflip->fetch(velocity_reconstructed())) {
		//
		// Extrapolate and constrain the reconstructed velocity
		int extrapolated_width (3);
		m_macutility->extrapolate_and_constrain_velocity(m_solid,velocity_reconstructed(),extrapolated_width);
		//
		// Compute the dirty velocity
		shared_macarray3<double> u(velocity_reconstructed());
		m_macadvection->advect_vector(u(),m_velocity,dt);
		m_velocity.copy(u());
		//
	} else {
		velocity_reconstructed->copy(velocity0());
		m_macadvection->advect_vector(m_velocity,m_velocity,dt);
	}
	//
	if( m_param.use_dust ) {
		advect_dust_particles(velocity0(),dt);
	} else {
		if( ! m_backwardflip->fetch(m_density)) {
			density0->copy(m_density);
			m_macadvection->advect_scalar(m_density,velocity0(),dt);
		}
	}
	//
	// Save the velocity before projection
	shared_macarray3<double> velocity_b4_proj(m_velocity);
	//
	// Add external force
	inject_external_force (m_velocity);
	//
	// Add buoyancy force
	add_buoyancy_force (m_velocity,m_density,dt);
	//
	// Add source
	shared_array3<double> density_added(m_shape.cell());
	add_source(m_velocity,density_added(),m_timestepper->get_current_time(),dt);
	//
	// Project
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
	//
	if( m_use_regular_velocity_advection ) {
		m_backwardflip->registerBuffer(m_velocity,velocity0(),nullptr,nullptr,&m_density,&density0(),&density_added(),dt);
	} else {
		//
		// Put buffer with one layer
		shared_macarray3<double> g(m_velocity);
		g() -= velocity_b4_proj();
		m_backwardflip->registerBuffer(m_velocity,velocity0(),&velocity_reconstructed(),&g(),&m_density,&density0(),&density_added(),dt);
	}
	//
	// Write down kinetic energy
	double kinetic_energy = m_macutility->get_kinetic_energy(m_solid,m_fluid,m_velocity);
	console::write("kinetic_energy",kinetic_energy);
	//
	console::dump( "<<< %s step done. Took %s\n", console::nth(step).c_str(), timer.stock("macbackwardflipsmoke3_simstep").c_str());
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
	//
	// Export density
	export_density();
}
//
void macbackwardflipsmoke3::draw( const graphics_engine &g, int width, int height ) const {
	//
	macsmoke3::draw(g,width,height);
	m_backwardflip->draw(g);
	//
	if( m_param.use_dust ) draw_dust_particles(g);
	else m_gridvisualizer->draw_density(g,m_density);
	//
	// Report kinetic energy
	double kinetic_energy = m_macutility->get_kinetic_energy(m_solid,m_fluid,m_velocity);
	g.color4(1.0,1.0,1.0,1.0);
	g.push_screen_coord(width,height);
	g.draw_string(vec2d(10,15).v, console::format_str("Energy = %.3e",kinetic_energy).c_str());
	g.pop_screen_coord();
}
//
extern "C" module * create_instance() {
	return new macbackwardflipsmoke3;
}
//