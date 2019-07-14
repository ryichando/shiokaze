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
	MODULE_NAME("fastmarch3")
	ARGUMENT_NAME("FastMarch")
	//
	virtual void redistance( array3<float> &phi_array, unsigned width ) override {
		//
		std::vector<vec3f> positions;
		std::vector<std::vector<size_t> > connections;
		std::vector<float> levelset;
		//
		// Generate a mesh
		shared_array3<std::vector<std::array<vec3d,3> > > triangles(phi_array.shape()-shape3(1,1,1));
		triangles->activate_as(phi_array);
		triangles->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			bool skip (false);
			double v[2][2][2];
			for( int ni=0; ni<2; ni++ ) for( int nj=0; nj<2; nj++ ) for( int nk=0; nk<2; nk++ ) {
				if( skip ) break;
				if( phi_array.active(i+ni,j+nj,k+nk)) {
					v[ni][nj][nk] = phi_array(i+ni,j+nj,k+nk);
				} else {
					skip = true;
					break;
				}
			}
			if( ! skip ) {
				auto patches = m_meshutility->polygonise_levelset(v);
				for( unsigned n=0; n<patches.size(); ++n ) {
					auto ptr = it.ptr();
					if( ptr ) ptr->push_back(patches[n]);
					else it.set({patches[n]});
				}
			}
		});
		//
		// Compute the closest distance
		shared_array3<float> fixed_dists(phi_array.shape());
		fixed_dists->activate_as(phi_array);
		fixed_dists->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			double min_d = 1.0;
			vec3d origin = m_dx*vec3d(i,j,k);
			double sgn = std::copysign(1.0,phi_array(i,j,k));
			//
			int w (1);
			for( int ni=i-w; ni<=i+w-1; ++ni ) for( int nj=j-w; nj<=j+w-1; ++nj ) for( int nk=k-w; nk<=k+w-1; ++nk ) {
				if( ! triangles->shape().out_of_bounds(ni,nj,nk)) {
					if( triangles->active(ni,nj,nk) ) {
						const std::vector<std::array<vec3d,3> > &patches = triangles()(ni,nj,nk);
						for( int m=0; m<patches.size(); ++m ) {
							const std::array<vec3d,3> &patch = patches[m];
							vec3d o = m_dx*vec3d(ni,nj,nk);
							vec3d out;
							vec3d p0 = m_dx*patch[0]+o;
							vec3d p1 = m_dx*patch[1]+o;
							vec3d p2 = m_dx*patch[2]+o;
							double d = m_meshutility->point_triangle_distance(origin,p0,p1,p2,out);
							if( d < min_d ) {
								min_d = d;
							}
						}
					}
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