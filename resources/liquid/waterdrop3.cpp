/*
**	waterdrop3.cpp
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
static double radius (0.5);
static double thickness (0.03);
static double height (0.3);
static bool container (false);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Waterdrop Scene 3D","Waterdrop");
	config.get_bool("Container",container,"Whether to place container");
	if( container ) {
		config.get_double("ContainerRadius",radius,"Radius of the solid hemisphere container");
		config.get_double("ContainerThickness",thickness,"Thickness of the solid hemisphere container");
		config.get_double("ContainerHeight",height,"Height of the solid hemisphere container");
	}
}
//
extern "C" double fluid( const vec3d &p ) {
	return std::min(p[1]-0.245,(p-vec3d(0.5,0.37,0.5)).len()-0.075);
}
//
extern "C" double solid( const vec3d &p ) {
	if( container ) return radius-thickness-(p-vec3d(0.5,0.5,0.5)).len();
	else return 1.0;
}
//
extern "C" double solid_visualize( const vec3d &p ) {
	if( container ) return std::max(p[1]-height,std::max(solid(p),-solid(p)-thickness));
	else return 1.0;
}
//
extern "C" const char *license() {
	return "MIT";
}