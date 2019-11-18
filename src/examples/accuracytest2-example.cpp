/*
**	accuracytest2-example.cpp
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
#include <shiokaze/array/array2.h>
#include <shiokaze/array/macarray2.h>
#include <shiokaze/array/shared_bitarray2.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/projection/macproject2_interface.h>
#include <shiokaze/visualizer/macvisualizer2_interface.h>
#include <cmath>
#define _USE_MATH_DEFINES
//
SHKZ_USING_NAMESPACE
//
class accuracytest2 : public drawable {
private:
	//
	LONG_NAME("Accuracy Test 2D")
	ARGUMENT_NAME("AccuracyExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
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
	static Real green_function( const vec2d &p, Real r ) {
		//
		double d = (p-vec2d(0.5,0.5)).len();
		return -1.0/(2.0*M_PI) * (log(d)-log(r));
	}
	//
	static vec2r derivative_green_function( const vec2d &p ) {
		//
		vec2r r = vec2d(0.5,0.5)-p;
		return 1.0/(2.0*M_PI) * r / r.norm2();
	}
	//
	virtual void post_initialize() override {
		//
		m_fluid.initialize(m_shape);
		m_fluid.set_as_levelset(2.0*m_dx);
		m_solid.initialize(m_shape,1.0);
		m_velocity.initialize(m_shape);
		//
		m_velocity.parallel_all([&]( int dim, int i, int j, auto &it ) {
			vec2d p = m_dx*vec2i(i,j).face(dim);
			it.set(derivative_green_function(p)[dim]);
		});
		//
		m_fluid.parallel_all([&](int i, int j, auto &it) {
			vec2d center(0.5,0.5);
			vec2d p = m_dx*vec2i(i,j).cell();
			double d = (p-center).len()-this->m_r;
			if( std::abs(d) < 2.0*m_dx) it.set(d);
			else it.set_off();
		});
		m_fluid.flood_fill();
		//
		m_macproject->project(1.0,m_velocity,m_solid,m_fluid);
		const auto &pressure = *m_macproject->get_pressure();
		shared_bitarray2 surface_flag(m_shape);
		m_velocity.const_serial_actives([&](int dim, int i, int j, const auto &it) {
			Real levelset[2] = { m_fluid(m_shape.clamp(i,j)), m_fluid(m_shape.clamp(i-(dim==0),j-(dim==1))) };
			if( levelset[0] * levelset[1] < 0.0 ) {
				if( levelset[0] < 0.0 ) surface_flag().set(i,j);
				if( levelset[1] < 0.0 ) surface_flag().set(i-(dim==0),j-(dim==1));
			}
		});
		//
		Real inf_norm (0.0);
		int sum (0);
		surface_flag->const_serial_actives([&]( int i, int j ) {
			inf_norm += std::abs(green_function(m_dx*vec2d(i,j).cell(),m_r)-pressure(i,j));
			sum ++;
		});
		inf_norm /= sum;
		//
		if( m_prev_norm ) {
			printf( "inf_norm = %.2e (factor=%.2e)\n", inf_norm, m_prev_norm/inf_norm );
		} else {
			printf( "inf_norm = %.2e\n", inf_norm );
		}
		m_prev_norm = inf_norm;
		//
		m_camera->set_bounding_box(vec2d().v,m_shape.box(m_dx).v);
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
		g.draw_string(vec2d(0.01,0.01).v, "Press \"C\" to double resolutions");
	}
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		double ratio = m_shape[1] / (double) m_shape[0];
		height = ratio * width;
	}
	//
	array2<Real> m_fluid{this};
	array2<Real> m_solid{this};
	macarray2<Real> m_velocity{this};
	//
	shape2 m_shape {8,8};
	double m_dx;
	double m_r {0.25+0.125};
	double m_prev_norm {0.0};
	//
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	macproject2_driver m_macproject{this,"macpressuresolver2"};
	macvisualizer2_driver m_macvisualizer{this,"macvisualizer2"};
};
//
extern "C" module * create_instance() {
	return new accuracytest2;
}
//
extern "C" const char *license() {
	return "MIT";
}