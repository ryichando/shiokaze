/*
**	cylinders3.cpp
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
#include <map>
#include <string>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
static double width (0.202), height (0.302), level (0.095);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Cylinder Scene 3D","Cylinders");
	config.get_double("Width",width,"Width of the dam");
	config.get_double("Height",height,"Height of the dam");
	config.get_double("Level",level,"Height of the pool");
}
//
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	dictionary["ResolutionX"] = "128";
	dictionary["ResolutionY"] = "64";
	dictionary["ResolutionZ"] = "32";
	dictionary["TargetPos"] = "0.5,0.2,0.25";
	dictionary["OriginPos"] = "0.5,1.0,2.2";
	return dictionary;
}
//
extern "C" double fluid( const vec3d &p ) {
	double value = -1e9;
	value = std::max(value,p[0]-width);
	value = std::max(value,p[1]-height);
	value = std::min(value,p[1]-level);
	return value;
}
//
extern "C" double solid( const vec3d &p ) {
	auto cylinder = [&]( const vec3d &p, vec3d center, double height, double r ) {
		return std::max(std::hypot(center[0]-p[0],center[2]-p[2])-r,p[1]-height);
	};
	double radius = 0.01;
	double value = 1.0;
	double height = 0.3;
	double shift = 0.1;
	double shift_d = -0.025;
	value = std::min(value,cylinder(p,vec3d(0.3+shift,-1.0,0.2+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.3+shift,-1.0,0.1+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.4+shift,-1.0,0.15+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.5+shift,-1.0,0.2+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.6+shift,-1.0,0.15+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.5+shift,-1.0,0.1+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.7+shift,-1.0,0.2+shift_d),height,radius));
	value = std::min(value,cylinder(p,vec3d(0.7+shift,-1.0,0.1+shift_d),height,radius));
	return value;
}
//
extern "C" double solid_visualize( const vec3d &p ) {
	return solid(p);
}
//
extern "C" const char *license() {
	return "MIT";
}