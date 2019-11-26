/*
**	maze2.cpp
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
#include <shiokaze/math/shape.h>
#include <shiokaze/utility/utility.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
static double dx, w, r, s;
static unsigned default_gn (128);
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
extern "C" void initialize( const shape2 &shape, double dx ) {
	::dx = dx;
	w = round(1.0 / 6.0 / dx) * dx;
	r = w / 5.0;
	s = 30.0;
}
//
extern "C" double solid( const vec2d &p ) {
	//
	double value (1.0);
	double dx = ::dx * 1.45;
	// 1
	value = std::min(value,utility::box(p,vec2d(w-dx,-1.0),vec2d(w+dx,1.0-w+dx)));
	// 2
	value = std::min(value,utility::box(p,vec2d(w-dx,1.0-w-dx),vec2d(1.0-w+dx,1.0-w+dx)));
	// 3
	value = std::min(value,utility::box(p,vec2d(1.0-w-dx,w-dx),vec2d(1.0-w+dx,1.0-w+dx)));
	// 4
	value = std::min(value,utility::box(p,vec2d(2.0*w-dx,w-dx),vec2d(1.0-w+dx,w+dx)));
	// 5
	value = std::min(value,utility::box(p,vec2d(2.0*w-dx,w-dx),vec2d(2.0*w+dx,1.0-2.0*w+dx)));
	// 6
	value = std::min(value,utility::box(p,vec2d(2.0*w-dx,1.0-2.0*w-dx),vec2d(1.0-2.0*w+dx,1.0-2.0*w+dx)));
	// 7
	value = std::min(value,utility::box(p,vec2d(1.0-2.0*w-dx,2.0*w-dx),vec2d(1.0-2.0*w+dx,1.0-2.0*w+dx)));
	// 8
	value = std::min(value,utility::box(p,vec2d(3.0*w-dx,2.0*w-dx),vec2d(1.0-2.0*w+dx,2.0*w+dx)));
	// 9
	value = std::min(value,utility::box(p,vec2d(3.0*w-dx,2.0*w-dx),vec2d(3.0*w+dx,1.0-3.0*w+dx)));
	//
	return value;
}
//
//
extern "C" vec2d velocity( const vec2d &p ) {
	if( (vec2d(0.5*w,0.5-0.5*w)-p).len() < r ) {
		return vec2d(-s,0.0);
	} else {
		return vec2d();
	}
}
//