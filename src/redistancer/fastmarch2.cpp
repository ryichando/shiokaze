/*
**	fastmarch2.cpp
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
#include <shiokaze/redistancer/redistancer2_interface.h>
#include <shiokaze/utility/meshutility2_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/utility/gridutility2_interface.h>
#include <list>
#include "unstructured_fastmarch2.h"
//
SHKZ_USING_NAMESPACE
//
class fastmarch2 : public redistancer2_interface {
protected:
	//
	LONG_NAME("FastMarch 2D")
	MODULE_NAME("fastmarch2")
	ARGUMENT_NAME("FastMarch")
	//
	virtual void redistance( array2<float> &phi_array, unsigned width ) override {
		//
		std::vector<float> levelset;
		//
		// Generate contours
		shared_array2<std::vector<vec2d> > contours(phi_array.shape()-shape2(1,1));
		contours->activate_as(phi_array);
		contours->parallel_actives([&](int i, int j, auto &it, int tn) {
			vec2d lines[8];	double v[2][2]; int pnum; vec2d vertices[2][2];
			bool skip (false);
			for( int ni=0; ni<2; ni++ ) for( int nj=0; nj<2; nj++ ) {
				if( skip ) break;
				if( phi_array.active(i+ni,j+nj)) {
					v[ni][nj] = phi_array(i+ni,j+nj);
					vertices[ni][nj] = m_dx*vec2d(i+ni,j+nj);
				} else {
					skip = true;
					break;
				}
			}
			if( ! skip ) {
				m_meshutility->march_points(v,vertices,lines,pnum,false);
				if( pnum ) {
					for( unsigned n=0; n<pnum; ++n ) {
						auto ptr = it.ptr();
						if( ptr ) ptr->push_back(lines[n]);
						else it.set({lines[n]});
					}
				}
			}
		});
		//
		// Compute the closest distance
		shared_array2<float> fixed_dists(phi_array.shape());
		fixed_dists->activate_as(phi_array);
		fixed_dists->parallel_actives([&](int i, int j, auto &it, int tn) {
			//
			double min_d = 1.0;
			vec2d origin = m_dx*vec2d(i,j);
			double sgn = (phi_array(i,j)>0.0 ? 1.0 : -1.0);
			//
			int w (1);
			for( int ni=i-w; ni<=i+w-1; ++ni ) for( int nj=j-w; nj<=j+w-1; ++nj ) {
				if( ! contours->shape().out_of_bounds(ni,nj)) {
					if( contours->active(ni,nj) ) {
						const std::vector<vec2d> &lines = contours()(ni,nj);
						for( int m=0; m<lines.size(); m+=2 ) {
							vec2d out = origin;
							m_meshutility->distance(lines[m],lines[m+1],out);
							double d = (out-origin).len();
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
		shared_array2<size_t> indices(phi_array.shape());
		phi_array.const_serial_actives([&]( int i, int j, const auto &it ) {
			indices().set(i,j,maximal_index++);
		});
		//
		levelset.resize(maximal_index);
		std::vector<char> fixed(maximal_index);
		std::vector<vec2i> index_vector(maximal_index);
		//
		phi_array.const_parallel_actives([&](int i, int j, const auto &it, int tn) {
			//
			size_t index = indices()(i,j);
			index_vector[index] = vec2i(i,j);
			//
			levelset[index] = fixed_dists->active(i,j) ? fixed_dists()(i,j) : it();
			fixed[index] = fixed_dists->active(i,j);
		});
		//
		// Perform fast march
		unstructured_fastmarch2::fastmarch(
			[&]( size_t index ) {
				return m_dx * index_vector[index].cell();
			},[&]( size_t index, std::function<void( size_t j )> func ) {
			vec2i pi(index_vector[index]);
			for( int ii=-1; ii<=1; ++ii ) for( int jj=-1; jj<=1; ++jj ) {
				vec2i q = pi+vec2i(ii,jj);
				if( ! indices().shape().out_of_bounds(q) && indices().active(q)) {
					func(indices()(q));
				}
			}
		},levelset,fixed,1.0,m_parallel,m_meshutility.get());
		//
		phi_array.parallel_actives([&](int i, int j, auto &it, int tn) {
			double value = levelset[indices()(i,j)];
			if( std::abs(value) > (double)width*m_dx ) it.set_off();
			else it.set(value);
		});
		//
		phi_array.set_as_levelset(m_dx*(double)width);
		phi_array.flood_fill();
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		m_dx = dx;
	}
	//
	meshutility2_driver m_meshutility{this,"meshutility2"};
	gridutility2_driver m_gridutility{this,"gridutility2"};
	parallel_driver m_parallel{this};
	//
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new fastmarch2;
}
//
extern "C" const char *license() {
	return "MIT";
}
//