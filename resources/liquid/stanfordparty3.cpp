/*
**	stanfordparty3.cpp
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
#include <shiokaze/polygon/polygon3_interface.h>
#include <shiokaze/polygon/polygon3_utility.h>
#include <shiokaze/meshlevelset/meshlevelset_interface.h>
#include <shiokaze/core/filesystem.h>
#include <map>
#include <string>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
static double level (0.1);
static double stride (0.042);
static double shift_z (0.2);
static std::string name ("Stanford Party Scene 3D");
static std::string argname ("StanfordParty");
//
#define MAX_SLOT	7
#define LIQUID_NUM	4
//
polygon3_ptr polygon;
std::vector<meshlevelset_ptr> levelsets(MAX_SLOT);
//
static double dx (0.0);
//
extern "C" void load( configuration &config ) {
	//
	configuration::auto_group group(config,name,argname);
	polygon = polygon3_interface::quick_load_module(config,"polygon3");
	for( unsigned n=0; n<MAX_SLOT; ++n ) levelsets[n] = meshlevelset_interface::quick_load_module(config,"SDFGen");
}
//
extern "C" void unload () {
	polygon.reset();
	for( unsigned n=0; n<MAX_SLOT; ++n ) levelsets[n].reset();
}
//
extern "C" void configure( configuration &config ) {
	//
	for( unsigned n=0; n<MAX_SLOT; ++n ) {
		levelsets[n]->recursive_configure(config);
	}
	configuration::auto_group group(config,name,argname);
	config.get_double("Level",level,"Height of the pool");
}
//
extern "C" void initialize( const shape3 &shape, double dx ) {
	//
	::dx = dx;
	int idx (0);
	//
	std::vector<vec3d> vertices;
	std::vector<std::vector<size_t> > faces;
	//
	for( unsigned n=0; n<MAX_SLOT; ++n ) {
		levelsets[n]->recursive_initialize({{"dx",&dx}});
	}
	//
	polygon->load_mesh(filesystem::find_resource_path("objects","bunny_watertight_low.ply"));
	polygon->get_mesh(vertices,faces);
	polygon3_utility::transform(vertices,vec3d(0.2,level+idx*stride,0.125+shift_z),0.2,1,180.0);
	levelsets[0]->set_mesh(vertices,faces);
	idx ++;
	//
	polygon->load_mesh(filesystem::find_resource_path("objects","armadillo.ply"));
	polygon->get_mesh(vertices,faces);
	polygon3_utility::transform(vertices,vec3d(0.39,level+idx*stride,0.14+shift_z),0.2,1,180.0);
	levelsets[1]->set_mesh(vertices,faces);
	idx ++;
	//
	polygon->load_mesh(filesystem::find_resource_path("objects","dragon_s.ply"));
	polygon->get_mesh(vertices,faces);
	polygon3_utility::transform(vertices,vec3d(0.57,level+idx*stride,0.175+shift_z),0.22,1,0.0);
	levelsets[2]->set_mesh(vertices,faces);
	idx ++;
	//
	polygon->load_mesh(filesystem::find_resource_path("objects","dragon.ply"));
	polygon->get_mesh(vertices,faces);
	polygon3_utility::transform(vertices,vec3d(0.78,level+idx*stride,0.175+shift_z),0.3,1,180.0);
	levelsets[3]->set_mesh(vertices,faces);
	idx ++;
	//
	std::vector<vec3d> lucy_vertices;
	//
	polygon->load_mesh(filesystem::find_resource_path("objects","lucy.ply"));
	polygon->get_mesh(lucy_vertices,faces);
	//
	vertices = lucy_vertices;
	polygon3_utility::transform(vertices,vec3d(0.25,-0.01,0.18),0.25,1,180.0);
	idx ++;
	levelsets[4]->set_mesh(vertices,faces);
	//
	vertices = lucy_vertices;
	polygon3_utility::transform(vertices,vec3d(0.5,-0.01,0.18),0.25,1,180.0);
	idx ++;
	levelsets[5]->set_mesh(vertices,faces);
	//
	vertices = lucy_vertices;
	polygon3_utility::transform(vertices,vec3d(0.75,-0.01,0.18),0.25,1,180.0);
	idx ++;
	levelsets[6]->set_mesh(vertices,faces);
	//
	for( int n=0; n<MAX_SLOT; ++n ) {
		levelsets[n]->generate_levelset();
	}
}
//
extern "C" std::map<std::string,std::string> get_default_parameters() {
	std::map<std::string,std::string> dictionary;
	dictionary["ResolutionX"] = "128";
	dictionary["ResolutionY"] = "64";
	dictionary["ResolutionZ"] = "64";
	dictionary["TargetPos"] = "0.5,0.25,0.25";
	dictionary["OriginPos"] = "0.5,0.7,2.5";
	dictionary["FPS"] = "150";
	return dictionary;
}
//
extern "C" double fluid( const vec3d &p ) {
	double value = p[1]-level;
	for( int n=0; n<LIQUID_NUM; ++n ) {
		value = std::min(value,levelsets[n]->get_levelset(p));
	}
	return value;
}
//
extern "C" double solid( const vec3d &p ) {
	double value = 1.0;
	for( int n=LIQUID_NUM; n<MAX_SLOT; ++n ) {
		value = std::min(value,levelsets[n]->get_levelset(p));
	}
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