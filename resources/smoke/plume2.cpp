/*
**	plume2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on August 15, 2017.
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
extern "C" std::map<std::string,std::string> get_default_parameters () {
	std::map<std::string,std::string> dictionary;
	return dictionary;
}
//
extern "C" void add ( const vec2d &p, vec2d &u, double &d, double time, double dt ) {
	if( time < 1.0 ) {
		vec2d center (0.15,0.15);
		if( (p-center).len() < 0.1 ) {
			double dist = (p-center).len();
			double r (0.075);
			double s (10.0);
			double v = std::min(10.0,std::max(0.0,s*(r-dist)/r));
			d = 4.0 * dt;
			u = vec2d( 3.0 * dt *v,0.0);
		}
	}
}
//
extern "C" const char *license() {
	return "MIT";
}
//