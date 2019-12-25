/*
**	cube3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on October 29, 2019.
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
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/configuration.h>
#include <shiokaze/core/filesystem.h>
#include <shiokaze/polygon/polygon3_interface.h>
#include <shiokaze/polygon/polygon3_utility.h>
#include <shiokaze/meshlevelset/meshlevelset_interface.h>
#include <string>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
static bool g_use_mesh {false};
static double g_width (0.2);
static unsigned g_default_gn (64);
static vec3d g_center(0.5,0.5,0.5);
static int g_version (0);
static double dx;
static polygon3_ptr polygon;
static meshlevelset_ptr levelset;
static std::string name ("Cube Scene 3D","Cube"), argname ("Cube3");
//
extern "C" void load( configuration &config ) {
	//
	configuration::auto_group group(config,name,argname);
	config.get_bool("UseMesh",g_use_mesh,"Use a mesh file");
	if( g_use_mesh ) {
		polygon = polygon3_interface::quick_load_module(config,"polygon3");
		levelset = meshlevelset_interface::quick_load_module(config,"SDFGen");
	}
}
//
extern "C" void unload () {
	polygon.reset();
	levelset.reset();
}
//
extern "C" void configure( configuration &config ) {
	//
	if( g_use_mesh ) {
		polygon->recursive_configure(config);
		levelset->recursive_configure(config);
	}
	configuration::auto_group group(config,name,argname);
	config.get_double("Width",g_width,"Width of cube");
	config.get_vec3d("Center",g_center.v,"Center of cube");
	config.get_integer("Version",g_version,"Version");
}
//
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	dictionary["ResolutionX"] = std::to_string(g_default_gn);
	dictionary["ResolutionY"] = std::to_string(g_default_gn);
	dictionary["ResolutionZ"] = std::to_string(g_default_gn);
	dictionary["Gravity"] = "0.0,0.0";
	dictionary["SurfaceTension"] = "5e-3";
	dictionary["RegionalVolumeCorrection"] = "Yes";
	dictionary["TimeStep"] = "1e-2";
	dictionary["OriginPos"] = "0.5,1.0,3.5";
	dictionary["TargetPos"] = "0.5,0.45,0.5";
	return dictionary;
}
//
extern "C" void initialize( const shape3 &shape, double dx ) {
	//
	::dx = dx;
	if( g_use_mesh ) {
		levelset->recursive_initialize({{"dx",&dx}});
		//
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		//
		polygon->load_mesh(filesystem::find_resource_path("objects","bunny_watertight_low.ply"));
		polygon->get_mesh(vertices,faces);
		for( auto &v : vertices ) v[0] = -v[0];
		polygon3_utility::transform(vertices,vec3d(0.53,0.26,0.6),0.58,1,180.0);
		levelset->set_mesh(vertices,faces);
		levelset->generate_levelset();
	}
}
//
extern "C" double fluid( const vec3d &p ) {
	if( g_version == 0 ) {
		if( g_use_mesh ) {
			return levelset->get_levelset(p);
		} else {
			return utility::box(p,g_center-vec3d(g_width,g_width,g_width),g_center+vec3d(g_width,g_width,g_width));
		}
	} else if( g_version == 1 ) {
		vec3d center0 = g_center-vec3d(g_width,0.0,0.0);
		vec3d center1 = g_center+vec3d(g_width,0.0,0.0);
		double w = 0.5 * g_width;
		double value (1.0);
		value = std::min(value,utility::box(p,center0-vec3d(w,w,w),center0+vec3d(w,w,w)));
		value = std::min(value,utility::box(p,center1-vec3d(w,w,w),center1+vec3d(w,w,w)));
		return value;
	} else {
		return 1.0;
	}
}
//
extern "C" vec3d velocity( const vec3d &p ) {
	if( ! g_use_mesh ) {
		if( g_version == 0 ) {
			return vec3d();
		} else {
			if( p[0] < 0.5 ) return vec3d(0.7,0.08,0.0);
			else return vec3d(-0.7,-0.08,0.0);
		}
	}
	return vec3d();
}
//
extern "C" const char *license() {
	return "MIT";
}