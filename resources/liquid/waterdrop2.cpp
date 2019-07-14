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
static bool noSolid (false);
static double radius (0.5);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Waterdrop Scene 2D","Waterdrop");
	config.get_bool("NoSolid",noSolid,"Should remove solids");
	config.get_double("ContainerRadius",radius,"Solid container radius");
}
//
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	return dictionary;
}
//
extern "C" double fluid( const vec2d &p ) {
	return std::min(p[1]-0.245,(p-vec2d(0.5,0.37)).len()-0.075);
}
//
extern "C" double solid( const vec2d &p ) {
	if( noSolid ) {
		return 1.0;
	} else {
		return radius-(p-vec2d(0.5,0.5)).len();
	}
}
//
extern "C" const char *license() {
	return "MIT";
}