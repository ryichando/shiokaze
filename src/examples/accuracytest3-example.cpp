/*
**	accuracytest3-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 14, 2019.
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
#include <shiokaze/ui/drawable.h>
#include <shiokaze/array/array3.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/array/shared_bitarray3.h>
#include <shiokaze/visualizer/gridvisualizer3_interface.h>
#include <shiokaze/projection/macproject3_interface.h>
#include <shiokaze/visualizer/macvisualizer3_interface.h>
#include <cmath>
#define _USE_MATH_DEFINES
//
SHKZ_USING_NAMESPACE
//
class accuracytest3 : public drawable {
private:
	//
	LONG_NAME("Accuracy Test 3D")
	ARGUMENT_NAME("AccuracyExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
		config.get_unsigned("ResolutionZ",m_shape[2],"Resolution towards Y axis");
		//
		double resolution_scale (1.0);
		config.get_double("ResolutionScale",resolution_scale,"Resolution doubling scale");
		//
		m_shape *= resolution_scale;
		m_dx = m_shape.dx();
		//
		set_environment("shape",&m_shape);
		set_environment("dx",&m_dx);
		//
		config.set_default_double("Residual",1e-18);
		config.set_default_double("EpsFluid",1e-18);
		config.set_default_bool("VolumeCorrection",false);
	}
	//
	static double green_function( const vec3d &p, double r ) {
		//
		double d = (p-vec3d(0.5,0.5,0.5)).len();
		return 1.0/(4.0*M_PI*d)-1.0/(4.0*M_PI*r);
	}
	//
	static vec3d derivative_green_function( const vec3d &p ) {
		//
		vec3d r = vec3d(0.5,0.5,0.5)-p;
		return 1.0/(4.0*M_PI) * r.normal() / r.norm2();
	}
	//
	virtual void post_initialize() override {
		//
		m_fluid.initialize(m_shape);
		m_fluid.set_as_levelset(2.0*m_dx);
		m_solid.initialize(m_shape,1.0);
		m_velocity.initialize(m_shape);
		//
		m_velocity.parallel_all([&]( int dim, int i, int j, int k, auto &it ) {
			vec3d p = m_dx*vec3i(i,j,k).face(dim);
			it.set(derivative_green_function(p)[dim]);
		});
		//
		m_fluid.parallel_all([&](int i, int j, int k, auto &it) {
			vec3d center(0.5,0.5,0.5);
			vec3d p = m_dx*vec3i(i,j,k).cell();
			double d = (p-center).len()-this->m_r;
			if( std::abs(d) < 2.0*m_dx) it.set(d);
			else it.set_off();
		});
		m_fluid.flood_fill();
		//
		m_macproject->project(1.0,m_velocity,m_solid,m_fluid);
		const auto &pressure = *m_macproject->get_pressure();
		shared_bitarray3 surface_flag(m_shape);
		m_velocity.const_serial_actives([&](int dim, int i, int j, int k, const auto &it) {
			Real levelset[2] = { m_fluid(m_shape.clamp(i,j,k)), m_fluid(m_shape.clamp(i-(dim==0),j-(dim==1),k-(dim==2))) };
			if( levelset[0] * levelset[1] < 0.0 ) {
				if( levelset[0] < 0.0 ) surface_flag().set(i,j,k);
				if( levelset[1] < 0.0 ) surface_flag().set(i-(dim==0),j-(dim==1),k-(dim==2));
			}
		});
		//
		Real inf_norm (0.0);
		int sum (0);
		surface_flag->const_serial_actives([&]( int i, int j, int k ) {
			inf_norm += std::abs(green_function(m_dx*vec3d(i,j,k).cell(),m_r)-pressure(i,j,k));
			sum ++;
		});
		inf_norm /= sum;
		//
		if( m_prev_norm ) {
			printf( "inf_norm = %.2e (factor=%.2e)\n", inf_norm, log(m_prev_norm/inf_norm)/log(2) );
		} else {
			printf( "inf_norm = %.2e\n", inf_norm );
		}
		m_prev_norm = inf_norm;
		//
		m_camera->set_bounding_box(vec3d().v,m_shape.box(m_dx).v);
	}
	//
	virtual bool keyboard( int key, int action, int mods ) override {
		//
		if( action == UI_interface::PRESS && key == KEY_C ) {
			//
			m_shape *= 2.0;
			m_dx = m_shape.dx();
			reinitialize();
			//
			return true;
		}
		return drawable::keyboard(key,action,mods);
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		m_gridvisualizer->draw_grid(g);
		g.color4(0.5,0.6,1.0,0.5);
		//
		m_gridvisualizer->draw_levelset(g,m_fluid);
		m_gridvisualizer->visualize_cell_scalar(g,*m_macproject->get_pressure());
		m_macvisualizer->draw_velocity(g,m_velocity);
		//
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec3d().v, "Press \"C\" to double resolutions");
	}
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		double ratio = m_shape[1] / (double) m_shape[0];
		height = ratio * width;
	}
	//
	array3<Real> m_fluid{this};
	array3<Real> m_solid{this};
	macarray3<Real> m_velocity{this};
	//
	shape3 m_shape {8,8,8};
	double m_dx;
	double m_r {0.35};
	double m_prev_norm {0.0};
	//
	gridvisualizer3_driver m_gridvisualizer{this,"gridvisualizer3"};
	macproject3_driver m_macproject{this,"macpressuresolver3"};
	macvisualizer3_driver m_macvisualizer{this,"macvisualizer3"};
};
//
extern "C" module * create_instance() {
	return new accuracytest3;
}
//
extern "C" const char *license() {
	return "MIT";
}