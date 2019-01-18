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
		config.get_unsigned("ResolutionX",shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",shape[1],"Resolution towards Y axis");
		//
		double scale (1.0);
		config.get_double("ResolutionScale",scale,"Resolution doubling scale");
		shape *= scale;
		dx = shape.dx();
		//
		set_environment("shape",&shape);
		set_environment("dx",&dx);
	}
	//
	void fill( double time ) {
		//
		array.parallel_all([&](int i, int j, auto &it) {
			double r (0.225);
			double w (0.25);
			vec2d center0(0.5+w*cos(time),0.5+0.75*r);
			vec2d center1(0.5-w*cos(time),0.5-0.75*r);
			vec2d p = dx*vec2i(i,j).cell();
			double d (1.0);
			d = std::min(d,(p-center0).len()-r);
			d = std::min(d,(p-center1).len()-r);
			if( std::abs(d) < 2.0*dx) it.set(d);
			else it.set_off();
		});
		//
		array.flood_fill();
	}
	//
	virtual void post_initialize() override {
		//
		array.initialize(shape);
		array.set_as_levelset(dx);
		time = 0.0;
		fill(time);
	}
	//
	virtual void idle() override {
		//
		time += 0.01;
		fill(time);
	}
	//
	virtual bool keyboard ( char key ) override {
		//
		if( key == 'M' ) {
			mode = ! mode;
			return true;
		}
		return drawable::keyboard(key);
	}
	//
	virtual void draw( graphics_engine &g, int width, int height ) const override {
		//
		gridvisualizer->draw_grid(g);
		g.color4(0.5,0.6,1.0,0.5);
		gridvisualizer->draw_levelset(g,array);
		if( mode ) gridvisualizer->draw_active(g,array);
		else gridvisualizer->draw_inside(g,array);
		//
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec2d(0.01,0.01).v, "Press \"M\" to toggle mode");
	}
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		double ratio = shape[1] / (double) shape[0];
		height = ratio * width;
	}
	//
	bool mode {false};
	array2<double> array {this};
	shape2 shape {64,64};
	double dx, time {0.0};
	gridvisualizer2_driver gridvisualizer{this,"gridvisualizer2"};
};
//
extern "C" module * create_instance() {
	return new levelset2;
}
//
extern "C" const char *license() {
	return "MIT";
}