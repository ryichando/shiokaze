/*
**	macvisualizer2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 21, 2017. 
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
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/visualizer/macvisualizer2_interface.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <algorithm>
#include <limits>
//
SHKZ_USING_NAMESPACE
//
class macvisualizer2 : public macvisualizer2_interface {
private:
	//
	virtual void draw_velocity( const graphics_engine &g, const macarray2<double> &velocity ) const override {
		if( m_param.draw_velocity ) {
			//
			shared_array2<vec2d> cell_velocity(velocity.shape());
			velocity.convert_to_full(*cell_velocity.get());
			//
			g.color4(1.0,1.0,1.0,0.5);
			auto velocity_acessor = cell_velocity->get_const_accessor();
			serial::for_each2(velocity.shape(),[&](int i, int j) {
				vec2d p0 = m_dx*vec2i(i,j).cell();
				vec2d p1 = p0+m_dx*velocity_acessor(i,j);
				graphics_utility::draw_arrow(g,p0.v,p1.v);
			});
		}
	}
	//
	virtual void visualize_scalar( const graphics_engine &g, const macarray2<double> &array ) const override {
		//
		double maxv = std::numeric_limits<double>::min();
		double minv = std::numeric_limits<double>::max();
		array.const_serial_actives([&](int dim, int i, int j, const auto &it) {
			double value = it();
			maxv = std::max(maxv,value);
			minv = std::min(minv,value);
		});
		double det = maxv-minv;
		if( std::abs(det) > 1e-2 ) {
			//
			auto accessor = array.get_const_accessor();
			g.line_width(2.0);
			g.begin(graphics_engine::MODE::LINES);
			array.const_serial_all([&]( int dim, int i, int j, const auto &it ) {
				//
				auto set_color = [&](unsigned i, unsigned j) {
					double v = accessor(dim,i,j);
					double normp = v ? 2.0*(v-minv)/det-1.0 : 0.0;
					g.color4(normp>0,0.3,normp<=0,std::abs(normp));
				};
				//
				set_color(i,j);
				g.vertex2v((m_dx*vec2d(i,j)).v);
				set_color(i+(dim!=0),j+(dim!=1));
				g.vertex2v((m_dx*vec2d(i+(dim!=0),j+(dim!=1))).v);
			});
			g.end();
			g.line_width(1.0);
		}
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		m_dx = dx;
	}
	virtual void configure( configuration &config ) override {
		config.get_bool("DrawVelocity",m_param.draw_velocity,"Should draw velocity");
	}
	//
	struct Parameters {
		bool draw_velocity {true};
	};
	//
	Parameters m_param;
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new macvisualizer2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//