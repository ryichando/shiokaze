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
#include <shiokaze/math/shape.h>
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
static double g_inject_time (5.0);
static bool g_fix_volume (false);
static vec2d g_inject_center (0.1,0.4);
static int g_version (0);
static unsigned g_counter (0);
static std::mt19937 g_rand_src;
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Injection Scene 2D","Injection");
	config.get_double("Radius",g_water_radius,"Radius of water");
	config.get_double("WaterLevel",g_water_level,"Water level");
	config.get_double("InjectHeight",g_inject_height,"Injection height");
	config.get_double("InjectSpeed",g_inject_speed,"Injection speed");
	config.get_double("InjectTime",g_inject_time,"Injection time");
	config.get_bool("FixVolume",g_fix_volume,"Fix total volume");
	config.get_integer("Version",g_version,"Scene version");
}
//
extern "C" void initialize( const shape2 &shape, double dx ) {
	g_counter = 0;
	g_rand_src = std::mt19937(3);
}
//
extern "C" double fluid( const vec2d &p ) {
	return p[1]-g_water_level;
}
//
extern "C" double solid( const vec2d &p ) {
	if( g_version == 0 ) {
		return 1.0;
	} else if( g_version == 1 ) {
		double value (1.0);
		const double height (0.22);
		const double radius (0.09);
		value = std::min(value,(p-vec2d(0.25,height)).len()-radius);
		value = std::min(value,(p-vec2d(0.5,height)).len()-radius);
		value = std::min(value,(p-vec2d(0.75,height)).len()-radius);
		return value;
	}
	return 1.0;
}
//
extern "C" bool check_inject( double dx, double dt, double time, unsigned step ) {
	g_counter ++;
	if( g_version == 2 ) {
		std::uniform_real_distribution<double> rand_dist_x(0.1,0.2);
		std::uniform_real_distribution<double> rand_dist_y(0.2,0.45);
		g_inject_center = vec2d(rand_dist_x(g_rand_src),rand_dist_y(g_rand_src));
	}
	return time < g_inject_time;
}
//
extern "C" bool inject( const vec2d &p, double dx, double dt, double time, unsigned step, double &fluid, vec2d &velocity ) {
	//
	bool result (false);
	if( g_version < 2 ) {
		fluid = (p-g_inject_center).len()-g_water_radius;
		result = true;
	} else {
		if( g_counter % 20 == 1 ) {
			fluid = (p-g_inject_center).len()-g_water_radius;
			result = true;
		}
	}
	//
	if( g_version == 0 || g_version == 2 ) {
		velocity = vec2d(g_inject_speed,0.0);
	} else if( g_version == 1 ) {
		velocity = vec2d(0.0,-g_inject_speed);
	}
	return result;
}
//
extern "C" void post_inject( double dx, double dt, double time, unsigned step, double &volume_change ) {
	if( g_fix_volume ) volume_change = 0.0;
	else {
		if( time > 0.0 && g_version < 2) {
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