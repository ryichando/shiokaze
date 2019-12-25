/*
**	movingsolid2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on December 12, 2019.
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
#include <shiokaze/math/vec.h>
#include <shiokaze/core/configuration.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <string>
#include <cmath>
#include <tuple>
//
SHKZ_USING_NAMESPACE
//
static double g_water_level (0.245);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Moving Solid Scene 2D","MovingSolid");
	config.get_double("WaterLevel",g_water_level,"Water level");
}
//
extern "C" std::tuple<double,vec2d> moving_solid( double time, const vec2d &p ) {
	return std::make_tuple(0.0,vec2d());
}
//
extern "C" double fluid( const vec2d &p ) {
	return p[1]-g_water_level;
}
//
extern "C" void draw( graphics_engine &g, double time ) {
	//
	const vec2d center(0.5+0.25*sin(2.5*time),0.25);
	const double r (0.1);
	//
	g.color4(0.5,0.5,0.4,1.0);
	graphics_utility::draw_circle(g,center.v,r,graphics_engine::MODE::TRIANGLE_FAN);
	//
	g.color4(1.0,1.0,1.0,1.0);
	graphics_utility::draw_circle(g,center.v,r,graphics_engine::MODE::LINE_LOOP);
}
//
extern "C" const char *license() {
	return "MIT";
}
//