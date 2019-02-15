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
		m_gridutility->trim_narrowband(phi_array,width);
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
				levelset[index] = it();
				positions[index] = m_dx * vec3i(i,j,k).cell();
			}
		});
		//
		// Perform fast march
		unstructured_fastmarch3::fastmarch(positions,connections,levelset,1.0,m_parallel,m_meshutility.get());
		//
		phi_array.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			double value = levelset[indices()(i,j,k)];
			if( std::abs(value) > (double)width*m_dx ) it.set_off();
			else it.set(value);
		});
		//
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