/*
**	polygon3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 1, 2017.
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
#include <shiokaze/polygon/polygon3_interface.h>
#include <shiokaze/core/console.h>
#include "rply/rply.h"
//
SHKZ_USING_NAMESPACE
//
typedef struct _mesh3 {
	std::vector<double> vertices_x;
	std::vector<double> vertices_y;
	std::vector<double> vertices_z;
	std::vector<size_t> faces_0;
	std::vector<size_t> faces_1;
	std::vector<size_t> faces_2;
	unsigned num_vert;
	unsigned num_faces;
} mesh3;
//
static int vertex_cb(p_ply_argument argument) {
	long value_index;
	mesh3 *mesh;
	ply_get_argument_user_data(argument, (void **)&mesh, &value_index);
	double value = ply_get_argument_value(argument);
	switch (value_index) {
		case 0:
			mesh->vertices_x.push_back(value);
			break;
		case 1:
			mesh->vertices_y.push_back(value);
			break;
		case 2:
			mesh->vertices_z.push_back(value);
			break;
	}
	return 1;
}
//
static int face_cb(p_ply_argument argument) {
	long length, value_index;
	long idata;
	mesh3 *mesh;
	ply_get_argument_user_data(argument, (void **)&mesh, &idata);
	ply_get_argument_property(argument, nullptr, &length, &value_index);
	double index = ply_get_argument_value(argument);
	switch (value_index) {
		case 0:
			mesh->faces_0.push_back(index);
			break;
		case 1:
			mesh->faces_1.push_back(index);
			break;
		case 2:
			mesh->faces_2.push_back(index);
			break;
	}
	return 1;
}
//
class polygon3 : public polygon3_interface {
protected:
	//
	virtual bool load_mesh( std::string path ) override {
		//
		mesh3 mesh;
		size_t nvertices, ntriangles;
		p_ply ply = ply_open(path.c_str(),nullptr,0,&mesh);
		if (!ply) {
			console::dump( "Could not open PLY file: %s\n", path.c_str() );
			exit(0);
			return false;
		}
		if (!ply_read_header(ply)) {
			console::dump( "Could not read header file: %s\n", path.c_str() );
			exit(0);
			return false;
		}
		nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, &mesh, 0);
		ply_set_read_cb(ply, "vertex", "y", vertex_cb, &mesh, 1);
		ply_set_read_cb(ply, "vertex", "z", vertex_cb, &mesh, 2);
		ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, &mesh, 0);
		mesh.num_vert = nvertices;
		mesh.num_faces = ntriangles;
		mesh.vertices_x.reserve(nvertices);
		mesh.vertices_y.reserve(nvertices);
		mesh.vertices_z.reserve(nvertices);
		mesh.faces_0.reserve(ntriangles);
		mesh.faces_1.reserve(ntriangles);
		mesh.faces_2.reserve(ntriangles);
		if (!ply_read(ply)) {
			console::dump( "Error in reading PLY file: %s\n", path.c_str() );
			exit(0);
			return false;
		}
		ply_close(ply);
		m_vertices.resize(nvertices);
		m_faces.resize(ntriangles);

		for( size_t n=0; n<nvertices; n++ ) {
			m_vertices[n][0] = -mesh.vertices_x[n];
			m_vertices[n][1] = mesh.vertices_y[n];
			m_vertices[n][2] = mesh.vertices_z[n];
		}
		for( size_t n=0; n<ntriangles; n++ ) {
			m_faces[n].resize(3);
			m_faces[n][0] = mesh.faces_0[n];
			m_faces[n][1] = mesh.faces_1[n];
			m_faces[n][2] = mesh.faces_2[n];
		}
		//
		return true;
	}
	virtual void get_mesh( std::vector<vec3d> &vertices, std::vector<std::vector<size_t> > &faces ) override {
		vertices = m_vertices;
		faces = m_faces;
	}
	//
	std::vector<vec3d> m_vertices;
	std::vector<std::vector<size_t> > m_faces;
};
//
extern "C" module * create_instance() {
	return new polygon3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//