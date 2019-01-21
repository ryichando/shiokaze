/*
**	dualmc.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 30, 2018. All rights reserved.
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
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include "dualmc/dualmc.h"
//
SHKZ_USING_NAMESPACE
using namespace dualmc;
//
class dualmc_wrapper : public cellmesher3_interface {
private:
	//
	LONG_NAME("Dual Marching Cubes Mesh Generator 3D")
	AUTHOR_NAME("Dominik Wodniok")
	//
	virtual void generate_mesh( const array3<double> &levelset, std::vector<vec3d> &vertices, std::vector<std::vector<size_t> > &faces ) const override {
		//
		assert(m_dx);
		vec3d global_origin = levelset.shape()==m_shape.nodal() ? vec3d() : m_dx * vec3d(0.5,0.5,0.5);
		//
		// Convert to linear voxel data
		shape3 s = levelset.shape();
		std::vector<double> data = levelset.linearize();
		//
		DualMC<double> dmc;
		std::vector<Vertex> _vertices;
		std::vector<Quad> _quads;
		//
		dmc.build(data.data(),s.w,s.h,s.d,0.0,true,false,_vertices,_quads);
		vertices.resize(_vertices.size());
		for( size_t n=0; n<vertices.size(); ++n ) {
			vertices[n] = global_origin + m_dx * vec3d(_vertices[n].x,_vertices[n].y,_vertices[n].z);
		}
		faces.resize(_quads.size());
		for( size_t n=0; n<faces.size(); ++n ) {
			faces[n].resize(4);
			faces[n][0] = _quads[n].i0;
			faces[n][1] = _quads[n].i1;
			faces[n][2] = _quads[n].i2;
			faces[n][3] = _quads[n].i3;
		}
	}
	//
	virtual void initialize ( const shape3 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	shape3 m_shape;
	double m_dx {0.0};
	//
};
//
//
extern "C" module * create_instance() {
	return new dualmc_wrapper;
}
//
extern "C" const char *license() {
	return "BSD-3-Clause";
}
//