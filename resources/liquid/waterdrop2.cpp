/*
**	waterdrop2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 20, 2017.
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
//
SHKZ_USING_NAMESPACE
//
static bool g_no_solid (false);
static double g_radius (0.5);
static double g_water_height (0.37);
static double g_water_radius (0.075);
static double g_water_level (0.245);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Waterdrop Scene 2D","Waterdrop");
	config.get_bool("NoSolid",g_no_solid,"Should remove solids");
	config.get_double("ContainerRadius",g_radius,"Solid container radius");
	config.get_double("Radius",g_water_radius,"Radius of water");
	config.get_double("WaterLevel",g_water_level,"Water level");
	config.get_double("WaterHeight",g_water_height,"Water height");
}
//
extern "C" double fluid( const vec2d &p ) {
	return std::min(p[1]-g_water_level,(p-vec2d(0.5,g_water_height)).len()-g_water_radius);
}
//
extern "C" double solid( const vec2d &p ) {
	if( g_no_solid ) {
		return 1.0;
	} else {
		return g_radius-(p-vec2d(0.5,0.5)).len();
	}
}
//
extern "C" const char *license() {
	return "MIT";
}