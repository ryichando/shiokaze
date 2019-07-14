/*
**	macvisualizer3.cpp
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
#include <shiokaze/visualizer/macvisualizer3_interface.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
//
class macvisualizer3 : public macvisualizer3_interface {
private:
	//
	MODULE_NAME("macvisualizer3")
	//
	virtual void draw_velocity( graphics_engine &g, const macarray3<float> &velocity ) const override {
		if( m_param.draw_velocity ) {
			for( int dim : DIMS3 ) {
				if( dim == 0 ) g.color4(0.5,0.5,1.0,0.75);
				else if( dim == 1 ) g.color4(1.0,0.5,0.5,0.75);
				else g.color4(0.5,1.0,0.5,0.75);
				g.begin(graphics_engine::MODE::LINES);
				velocity.const_serial_actives([&]( int dim, int i, int j, int k, const auto &it ) {
					double u = it();
					vec3d p0 = m_dx*vec3d(i+0.5*(dim!=0),j+0.5*(dim!=1),k+0.5*(dim!=2));
					vec3d p1 = p0+m_dx*u*vec3d(dim==0,dim==1,dim==2);
					g.vertex3v(p0.v);
					g.vertex3v(p1.v);
				});
				g.end();
			}
		}
	}
	//
	virtual void visualize_scalar( graphics_engine &g, const macarray3<float> &array ) const override {
		// TO BE IMPLEMENTED
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		m_dx = dx;
	}
	virtual void configure( configuration &config ) override {
		config.get_bool("drawVelocity",m_param.draw_velocity,"Should draw velocity");
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
	return new macvisualizer3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
