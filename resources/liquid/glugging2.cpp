/*
**	glugging2.cpp
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
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	//
	dictionary["ResolutionX"] = std::to_string(default_gn);
	dictionary["ResolutionY"] = std::to_string(default_gn);
	dictionary["Projection"] = "macstreamfuncsolver2";
	return dictionary;
}
//
extern "C" double fluid( const vec2d &p ) {
	double value = 1e9;
	double r = 0.12;
	value = std::min(value,std::abs(p[1]-0.5-r)-r);
	return value;
}
//
extern "C" double solid( const vec2d &p ) {
	double value = 1e9;
	double r = 0.23;
	value = std::min(value,(p-vec2d(0.5,0.75)).len()-r);
	value = std::min(value,(p-vec2d(0.5,0.25)).len()-r);
	value = std::min(value,utility::box(p,vec2d(0.45,0.45),vec2d(0.55,0.55)));
	return -value;
}
//
extern "C" const char *license() {
	return "MIT";
}