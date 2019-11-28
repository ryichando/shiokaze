/*
**	injection2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 26, 2019.
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
#include <string>
#include <cmath>
#include <random>
//
SHKZ_USING_NAMESPACE
//
static double g_water_radius (0.025);
static double g_water_level (0.1);
static double g_inject_height (0.4);
static double g_inject_speed (1.0);
static bool g_fix_volume (false);
static vec2d g_inject_center (0.1,0.4);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Injection Scene 2D","Injection");
	config.get_double("Radius",g_water_radius,"Radius of water");
	config.get_double("WaterLevel",g_water_level,"Water level");
	config.get_double("InjectHeight",g_inject_height,"Injection height");
	config.get_double("InjectSpeed",g_inject_speed,"Injection speed");
	config.get_bool("FixVolume",g_fix_volume,"Fix total volume");
}
//
extern "C" double fluid( const vec2d &p ) {
	return p[1]-g_water_level;
}
//
extern "C" bool check_inject( double dx, double dt, double time, unsigned step ) {
	return time < 5.0;
}
//
extern "C" bool inject( const vec2d &p, double dx, double dt, double time, unsigned step, double &fluid, vec2d &velocity ) {
	//
	fluid = (p-g_inject_center).len()-g_water_radius;
	velocity = vec2d(g_inject_speed,0.0);
	return true;
}
//
extern "C" void post_inject( double dx, double dt, double time, unsigned step, double &volume_change ) {
	if( g_fix_volume ) volume_change = 0.0;
	else {
		if( time > 0.0 ) {
			const double area = g_water_radius;
			volume_change = dt * g_inject_speed * area;
		} else {
			volume_change = M_PI * (g_water_radius*g_water_radius);
		}
	}
}
//
extern "C" const char *license() {
	return "MIT";
}
//