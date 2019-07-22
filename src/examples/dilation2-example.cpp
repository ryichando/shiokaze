/*
**	dilation2-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 2, 2018.
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
#include <shiokaze/core/console.h>
#include <shiokaze/array/array2.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
//
SHKZ_USING_NAMESPACE
//
class dilation2 : public drawable {
private:
	//
	LONG_NAME("Dilation 2D")
	ARGUMENT_NAME("DilationExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
		//
		double view_scale (1.0);
		config.get_double("ViewScale",view_scale,"View scale");
		//
		double resolution_scale (1.0);
		config.get_double("ResolutionScale",resolution_scale,"Resolution doubling scale");
		//
		m_shape *= resolution_scale;
		m_dx = view_scale * m_shape.dx();
		//
		set_environment("shape",&m_shape);
		set_environment("dx",&m_dx);
	}
	//
	virtual void post_initialize() override {
		m_array.initialize(m_shape);
		m_camera->set_bounding_box(vec2d().v,m_shape.box(m_dx).v,true);
	}
	//
	virtual bool keyboard( int key, int action, int mods ) override {
		// Dump keyboard event
		if( action == UI_interface::PRESS ) {
			console::dump( "Keyboard %c\n", key );
			if ( key == UI_interface::KEY_R ) {
				reinitialize();
			} else if( key == UI_interface::KEY_C ) {
				console::dump( "Count = %d\n", m_array.count());
			} else if( key == UI_interface::KEY_P ) {
				auto &config = configurable::get_global_configuration();
				config.print_variables();
			}
		}
		return true;
	}
	void fill( double x, double y ) {
		int i = m_shape[0] * x;
		int j = m_shape[1] * y;
		m_array.set(m_shape.clamp(i,j),1.0);
	}
	virtual void drag( double x, double y, double z, double u, double v, double w ) override {
		m_array.dilate( [&](int i, int j, auto& it) {
			it.set(1.0);
		});
	}
	virtual void mouse( double x, double y, double z, int button, int action, int mods ) override {
		if( action == UI_interface::PRESS ) {
			fill(x,y);
		}
	}
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		double ratio = m_shape[1] / (double) m_shape[0];
		height = ratio * width;
	}
	virtual void draw( graphics_engine &g ) const override {
		//
		// Draw things
		m_gridvisualizer->draw_grid(g);
		m_gridvisualizer->draw_density(g,m_array);
		//
		// Draw a message
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec2d(0.025,0.025).v, "Press \"R\" to reset");
	}
	//
	array2<float> m_array {this};
	shape2 m_shape {42,32};
	double m_dx;
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
};
//
extern "C" module * create_instance() {
	return new dilation2;
}
//
extern "C" const char *license() {
	return "MIT";
}