/*
**	timestepper.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 11, 2017. 
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
#include <shiokaze/timestepper/timestepper_interface.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/global_timer.h>
#include <algorithm>
#include <cassert>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
class timestepper : public timestepper_interface {
public:
	//
	timestepper () {
		//
	#ifdef USE_OPENGL
		m_maximal_frame = 0;
	#else
		m_maximal_frame = 600;
	#endif
	}
protected:
	//
	// Advance time by the maximal velocity. Returns delta t (time step size)
	virtual double advance( double max_velocity, double dx ) override {
		//
		double max_unit_u = max_velocity / dx;
		assert( m_FPS && m_CFL );
		double max_dt = std::max( m_min_dt, 1.0 / m_FPS );
		//
		double dt;
		if( m_fixed_timestep ) {
			//
			dt = m_fixed_timestep;
			m_accumulated_time += dt;
			//
			while( m_accumulated_time >= 1.0 / m_FPS ) {
				//
				m_accumulated_time = std::max(0.0, m_accumulated_time - 1.0 / m_FPS );
				m_should_export_video = true;
				++ m_frame;
				m_simulation_time_one_video_frame_prev = m_simulation_time_one_video_frame;
				m_simulation_time_one_video_frame = global_timer::get_milliseconds();
				//
				console::write("timestepper_time_per_video_frame",get_simulation_time_per_video_frame());
				console::write("timestepper_frame_step",m_step+1);
			}
			//
		} else {
			if( max_unit_u ) {
				dt = std::max( m_min_dt, std::min( max_dt, m_CFL / max_unit_u ));
			} else {
				dt = m_min_dt;
			}
			//
			assert( m_accumulated_time < 1.0 / m_FPS );	
			if( m_accumulated_time+dt >= 1.0 / m_FPS ) {
				dt = 1.0 / m_FPS - m_accumulated_time;
				if( dt < m_min_dt ) {
					double odd = m_min_dt - dt;
					dt = m_min_dt;
					m_accumulated_time = odd;
				} else {
					m_accumulated_time = 0.0;
				}
				m_should_export_video = true;
				++ m_frame;
				m_simulation_time_one_video_frame_prev = m_simulation_time_one_video_frame;
				m_simulation_time_one_video_frame = global_timer::get_milliseconds();
				//
				console::write("timestepper_time_per_video_frame",get_simulation_time_per_video_frame());
				console::write("timestepper_frame_step",m_step+1);
				//
			} else {
				m_accumulated_time += dt;
				m_should_export_video = false;
			}
		}
		//
		m_simulation_time_per_step_prev = m_simulation_time_per_step;
		m_simulation_time_per_step = global_timer::get_milliseconds();
		//
		console::write("timestepper_time_per_step",get_simulation_time_per_step());
		//
		m_time += dt;
		m_current_CFL = std::ceil(dt * max_unit_u);
		++ m_step;
		//
		console::write("timestepper_dt",dt);
		console::write("timestepper_CFL",m_current_CFL);
		//
		console::set_time(m_time);
		//
		return dt;
	}
	//
	// Export a video frame if returned non-zero integer (returns frame number)
	virtual int should_export_frame() const override {
		if ( m_should_export_video ) {
			return m_frame;
		} else {
			return 0;
		}
	}
	//
	// Get simulation time spent for computing one video frame
	virtual double get_simulation_time_per_video_frame() const override {
		return m_simulation_time_one_video_frame - m_simulation_time_one_video_frame_prev;
	}
	//
	// Get simulation time spent for computing one time step
	virtual double get_simulation_time_per_step() const override {
		return m_simulation_time_per_step - m_simulation_time_per_step_prev;
	}
	//
	// Get current time
	virtual double get_current_time() const override { return m_time; }
	//
	// Get simulation time (time spent for calculation)
	virtual double get_total_calculation_time() const override { return global_timer::get_milliseconds() - m_simulation_time0; }
	//
	// Get current CFL
	virtual double get_current_CFL() const override { return m_current_CFL; }
	//
	// Get the target CFL
	virtual double get_target_CFL() const override { return m_CFL; }
	//
	// Get time step counter
	virtual unsigned get_step_count() const override { return m_step; }
	//
	// Get if we should terminate the simulation
	virtual bool should_quit() const override { return m_maximal_frame ? m_frame >= m_maximal_frame : false; }
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_double("TimeStep",m_fixed_timestep,"Target time step");
		config.get_double("FPS",m_FPS,"Frame per second");
		config.get_double("CFL",m_CFL,"Target CFL number");
		config.get_unsigned("MaxSubsteps",m_maximal_substeps,"Maximal substeps");
		config.get_unsigned("MaxFrame",m_maximal_frame,"Maximal video frame count");
		m_min_dt = 1.0 / ((double)m_maximal_substeps * m_FPS);
	}
	//
	virtual void post_initialize() override {
		//
		m_time = 0.0;
		m_frame = 0;
		m_step = 0;
		m_accumulated_time = 0.0;
		m_simulation_time0 = global_timer::get_milliseconds();
		m_simulation_time_one_video_frame = m_simulation_time_one_video_frame_prev = m_simulation_time0;
		m_simulation_time_per_step_prev = m_simulation_time_per_step = m_simulation_time0;
		//
		m_current_CFL = 0.0;
		console::set_time(0.0);
	}
	//
	double m_time, m_FPS {60.0}, m_CFL {3.0}, m_min_dt {0.1/60.0};
	double m_accumulated_time;
	double m_simulation_time0;
	double m_simulation_time_one_video_frame_prev;
	double m_simulation_time_one_video_frame;
	double m_simulation_time_per_step_prev;
	double m_simulation_time_per_step;
	double m_fixed_timestep {0.0};
	double m_current_CFL;
	bool m_should_export_video {false};
	int m_frame;
	unsigned m_maximal_frame;
	unsigned m_maximal_substeps {10};
	unsigned m_step;
	//
};
//
extern "C" module * create_instance() {
	return new timestepper();
}
//
extern "C" const char *license() {
	return "MIT";
}
//