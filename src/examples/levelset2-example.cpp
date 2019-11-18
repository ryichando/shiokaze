/*
**	levelset2-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 9, 2018.
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
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
class levelset2 : public drawable {
private:
	//
	LONG_NAME("Levelset 2D")
	ARGUMENT_NAME("LevelsetExample")
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
	void fill( double time ) {
		//
		m_array.parallel_all([&](int i, int j, auto &it) {
			double r (0.225);
			double w (0.25);
			vec2d center0(0.5+w*cos(m_time),0.5+0.75*r);
			vec2d center1(0.5-w*cos(m_time),0.5-0.75*r);
			vec2d p = m_dx*vec2i(i,j).cell();
			double d (1.0);
			d = std::min(d,(p-center0).len()-r);
			d = std::min(d,(p-center1).len()-r);
			if( std::abs(d) < 2.0*m_dx) it.set(d);
			else it.set_off();
		});
		//
		m_array.flood_fill();
	}
	//
	virtual void post_initialize() override {
		//
		m_array.initialize(m_shape);
		m_array.set_as_levelset(2.0*m_dx);
		m_time = 0.0;
		fill(m_time);
		m_camera->set_bounding_box(vec2d().v,m_shape.box(m_dx).v);
	}
	//
	virtual void idle() override {
		//
		m_time += 0.01;
		fill(m_time);
	}
	//
	virtual bool keyboard( int key, int action, int mods ) override {
		//
		if( action == UI_interface::PRESS && key == KEY_M ) {
			m_mode = ! m_mode;
			return true;
		}
		return drawable::keyboard(key,action,mods);
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		m_gridvisualizer->draw_grid(g);
		g.color4(0.5,0.6,1.0,0.5);
		m_gridvisualizer->draw_levelset(g,m_array);
		if( m_mode ) m_gridvisualizer->draw_active(g,m_array);
		else m_gridvisualizer->draw_inside(g,m_array);
		//
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec2d(0.01,0.01).v, "Press \"M\" to toggle mode");
	}
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		double ratio = m_shape[1] / (double) m_shape[0];
		height = ratio * width;
	}
	//
	bool m_mode {false};
	array2<Real> m_array {this};
	shape2 m_shape {64,64};
	double m_dx, m_time {0.0};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
};
//
extern "C" module * create_instance() {
	return new levelset2;
}
//
extern "C" const char *license() {
	return "MIT";
}