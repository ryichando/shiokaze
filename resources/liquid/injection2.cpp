/*
**	injection2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 21, 2018.
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
static unsigned scene (0);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Injection 2D","Injection");
	config.get_unsigned("Scene",scene,"Scene number");
}
//
extern "C" double fluid( const vec2d &p ) {
	return p[1]-0.12;
}
//
extern "C" void inject( const vec2d &p, double time, double &fluid, vec2d &velocity ) {
	if( time < 1.0 ) {
		if( scene == 0 ) {
			fluid = (p-vec2d(0.5,0.37)).len()-0.05;
			velocity = vec2d(0.0,-0.75);
		} else {
			fluid = (p-vec2d(0.1,0.37)).len()-0.05;
			velocity = vec2d(1.0,0.0);
		}
	}
}
//
extern "C" double solid( const vec2d &p ) {
	if( scene == 0 ) {
		return 0.5-(p-vec2d(0.5,0.5)).len();
	} else {
		return 1.0;
	}
}
//
extern "C" const char *license() {
	return "MIT";
}