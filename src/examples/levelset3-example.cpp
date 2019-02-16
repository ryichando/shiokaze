/*
**	levelset3-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2018.
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
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/visualizer/gridvisualizer3_interface.h>
//
SHKZ_USING_NAMESPACE
//
class levelset3 : public drawable {
private:
	//
	LONG_NAME("Levelset 3D")
	ARGUMENT_NAME("LevelsetExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("ResolutionX",shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",shape[1],"Resolution towards Y axis");
		config.get_unsigned("ResolutionZ",shape[2],"Resolution towards Z axis");
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
		array.parallel_all([&](int i, int j, int k, auto &it) {
			double r (0.225);
			double w (0.25);
			vec3d center0(0.5+w*cos(time),0.5+0.75*r,0.5);
			vec3d center1(0.5-w*cos(time),0.5-0.75*r,0.5);
			vec3d p = dx*vec3i(i,j,k).cell();
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
		array.set_as_levelset(2.0*dx);
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
		g.color4(1.0,1.0,1.0,0.5);
		graphics_utility::draw_wired_box(g);
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
	bool mode {false};
	array3<double> array {this};
	shape3 shape {32,32,32};
	double dx, time {0.0};
	gridvisualizer3_driver gridvisualizer{this,"gridvisualizer3"};
};
//
extern "C" module * create_instance() {
	return new levelset3;
}
//
extern "C" const char *license() {
	return "MIT";
}