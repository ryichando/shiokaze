/*
**	taylor_green_vortex2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on January 25, 2018.
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
#include <map>
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
	dictionary["FPS"] = "300";
	dictionary["ResolutionX"] = std::to_string(default_gn);
	dictionary["ResolutionY"] = std::to_string(default_gn);
	//
	return dictionary;
}
//
extern "C" vec2d velocity( const vec2d &p ) {
	const double x = 2.0*M_PI*(p[0]-0.25);
	const double y = 2.0*M_PI*(p[1]-0.25);
	//
	double u = cos(x)*sin(y);
	double v = -sin(x)*cos(y);
	double s = 3.0;
	//
	return -s*vec2d(u,v);
}
//