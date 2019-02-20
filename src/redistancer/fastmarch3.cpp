/*
**	fastmarch3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 3, 2017. 
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
#include <shiokaze/pointgridhash/pointgridhash3_interface.h>
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <shiokaze/redistancer/redistancer3_interface.h>
#include <shiokaze/utility/meshutility3_interface.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/core/console.h>
#include <shiokaze/array/shared_array3.h>
#include <list>
#include "unstructured_fastmarch3.h"
//
SHKZ_USING_NAMESPACE
//
class fastmarch3 : public redistancer3_interface {
private:
	//
	LONG_NAME("FastMarch 3D")
	ARGUMENT_NAME("FastMarch")
	//
	virtual void redistance( array3<double> &phi_array, unsigned width ) override {
		//
		std::vector<vec3d> positions;
		std::vector<std::vector<size_t> > connections;
		std::vector<double> levelset;
		//
		// Generate a mesh
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		m_mesher->generate_mesh(phi_array,vertices,faces);
		//
		// Sort triangles
		std::vector<vec3d> face_centers(faces.size());
		m_parallel.for_each(faces.size(),[&](size_t n) {
			vec3d center = (vertices[faces[n][0]]+vertices[faces[n][1]]+vertices[faces[n][2]]) / 3.0;
			face_centers[n] = center;
		});
		m_hashtable->sort_points(face_centers);
		//
		// Compute the closest distance
		shared_array3<double> fixed_dists(phi_array.shape());
		fixed_dists->activate_as(phi_array);
		fixed_dists->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			double min_d = 1.0;
			double sgn = (phi_array(i,j,k) > 0.0 ? 1.0 : -1.0);
			//
			std::vector<size_t> face_neighbors = m_hashtable->get_cell_neighbors(vec3i(i,j,k),pointgridhash3_interface::USE_NODAL);
			for( unsigned n=0; n<face_neighbors.size(); ++n ) {
				const unsigned &idx = face_neighbors[n];
				vec3d p = m_dx*vec3d(i+0.5,j+0.5,k+0.5);
				vec3d out;
				double d = m_meshutility->point_triangle_distance(p,vertices[faces[idx][0]],vertices[faces[idx][1]],vertices[faces[idx][2]],out);
				if( d < min_d ) {
					min_d = d;
				}
			}
			if( min_d < 1.0 ) it.set(sgn * min_d);
			else it.set_off();
		});
		//
		m_gridutility->trim_narrowband(phi_array);
		phi_array.flood_fill();
		phi_array.dilate(width);
		//
		size_t maximal_index (0);
		shared_array3<size_t> indices(phi_array.shape());
		phi_array.const_serial_actives([&]( int i, int j, int k, const auto &it ) {
			indices().set(i,j,k,maximal_index++);
		});
		//
		positions.resize(maximal_index);
		connections.resize(maximal_index);
		levelset.resize(maximal_index);
		std::vector<char> fixed(maximal_index);
		//
		phi_array.const_parallel_actives([&](int i, int j, int k, const auto &it, int tn) {
			//
			size_t index = indices()(i,j,k);
			for( int ii=-1; ii<=1; ++ii ) for( int jj=-1; jj<=1; ++jj ) for( int kk=-1; kk<=1; ++kk ) {
				vec3i q = vec3i(i+ii,j+jj,k+kk);
				if( ii==0 && jj==0 && kk==0 ) continue;
				if( ! indices().shape().out_of_bounds(q) && indices().active(q)) {
					connections[index].push_back(indices()(q));
				}
				//
				levelset[index] = fixed_dists->active(i,j,k) ? fixed_dists()(i,j,k) : it();
				positions[index] = m_dx * vec3i(i,j,k).cell();
				fixed[index] = fixed_dists->active(i,j,k);
			}
		});
		//
		// Perform fast march
		unstructured_fastmarch3::fastmarch(positions,connections,levelset,fixed,1.0,m_parallel,m_meshutility.get());
		//
		phi_array.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			double value = levelset[indices()(i,j,k)];
			if( std::abs(value) > m_dx*(double)width ) it.set_off();
			else it.set(value);
		});
		//
		phi_array.set_as_levelset(m_dx*(double)width);
		phi_array.flood_fill();
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		m_dx = dx;
	}
	//
	cellmesher3_driver m_mesher{this,"marchingcubes"};
	meshutility3_driver m_meshutility{this,"meshutility3"};
	pointgridhash3_driver m_hashtable{this,"pointgridhash3"};
	gridutility3_driver m_gridutility{this,"gridutility3"};
	parallel_driver m_parallel{this};
	//
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new fastmarch3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//