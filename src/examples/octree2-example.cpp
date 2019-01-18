/*
**	octree2-example.cpp
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
#include <shiokaze/octree/octree2.h>
//
SHKZ_USING_NAMESPACE
//
class octree2_example : public drawable {
private:
	//
	LONG_NAME("Octree 2D")
	ARGUMENT_NAME("OctreeExample")
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
	virtual void cursor( int width, int height, double x, double y ) override {
		octree.build_octree( [&](const vec2d &p){ 
			return std::max(dx,(p-vec2d(x,y)).len());
		},8);
	}
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		double ratio = shape[1] / (double) shape[0];
		height = ratio * width;
	}
	virtual void draw( graphics_engine &g, int width, int height ) const override {
		octree.draw_octree(g);
	}
	//
	shape2 shape {64,64};
	double dx;
	octree2 octree;
};
//
extern "C" module * create_instance() {
	return new octree2_example;
}
//
extern "C" const char *license() {
	return "MIT";
}