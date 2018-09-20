/*
**	glugging3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 6, 2018.
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
#include <shiokaze/utility/utility.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/core/configuration.h>
#include <string>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
static unsigned default_gn (64);
//
static double cylinder ( const vec3d &p, vec3d center, double height, double r ) {
	return std::max(std::hypot(center[0]-p[0],center[2]-p[2])-r,(p-center).len()-height);
};
//
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	//
	dictionary["ResolutionX"] = std::to_string(default_gn);
	dictionary["ResolutionY"] = std::to_string(default_gn);
	dictionary["TargetPos"] = "0.25,0.5,0.25";
	dictionary["OriginPos"] = "0.25,1.3,3.5";
	dictionary["Projection"] = "macstreamfuncsolver3";
	dictionary["VolumeCorrection"] = "No";
	dictionary["MaxFrame"] = "900";
	return dictionary;
}
//
extern "C" double fluid( const vec3d &p ) {
	return std::max(utility::box(p,vec3d(-1.0,0.5,-1.0),vec3d(2.0,0.75,2.0)),0.27-(p-vec3d(0.27,0.25,0.25)).len());
}
//
extern "C" double solid( const vec3d &p ) {
	double value (-1.0);
	double r = 0.21;
	value = std::max(value,r-(vec3d(0.25,0.75,0.25)-p).len());
	value = std::max(value,r-(vec3d(0.25,0.25,0.25)-p).len());
	value = std::max(value,-cylinder(p,vec3d(0.25,0.5,0.25),0.2,0.05));
	return value;
}
//
extern "C" double solid_visualize( const vec3d &p ) {
	return std::max(std::max(solid(p),-solid(p)-0.02),p[2]-0.25);
}
//
extern "C" const char *license() {
	return "MIT";
}