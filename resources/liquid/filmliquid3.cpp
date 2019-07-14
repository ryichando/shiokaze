/*
**	filmliquid3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Februrary 3, 2018.
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
//
static double g_tilt (0.1);
static double g_height (0.3);
//
SHKZ_USING_NAMESPACE
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Film Water 3D","FilmWater");
	config.get_double("Tilt",g_tilt,"Tilt degree");
	config.get_double("Height",g_height,"Height");
}
//
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	dictionary["Gravity"] = "0.0,0.0";
	dictionary["SurfaceTension"] = "0.01";
	return dictionary;
}
//
extern "C" double fluid( const vec3d &p ) {
	return std::max(p[1]-g_height+g_tilt*p[0],g_height-p[1]+g_tilt*p[0]-2.5*g_tilt);
}
//