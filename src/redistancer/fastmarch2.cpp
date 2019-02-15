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
private:
	//
	LONG_NAME("FastMarch 2D")
	ARGUMENT_NAME("FastMarch")
	//
	virtual void redistance( array2<double> &phi_array, unsigned width, double dx ) override {
		//
		std::vector<vec2d> positions;
		std::vector<std::vector<size_t> > connections;
		std::vector<double> levelset;
		//
		m_gridutility->trim_narrowband(phi_array,width);
		//
		size_t maximal_index (0);
		shared_array2<size_t> indices(phi_array.shape());
		phi_array.const_serial_actives([&]( int i, int j, const auto &it ) {
			indices().set(i,j,maximal_index++);
		});
		//
		positions.resize(maximal_index);
		connections.resize(maximal_index);
		levelset.resize(maximal_index);
		//
		phi_array.const_parallel_actives([&](int i, int j, const auto &it, int tn) {
			//
			size_t index = indices()(i,j);
			for( int ii=-1; ii<=1; ++ii ) for( int jj=-1; jj<=1; ++jj ) {
				vec2i q = vec2i(i+ii,j+jj);
				if( ii==0 && jj==0 ) continue;
				if( ! indices().shape().out_of_bounds(q) && indices().active(q)) {
					connections[index].push_back(indices()(q));
				}
				//
				levelset[index] = it();
				positions[index] = dx * vec2i(i,j).cell();
			}
		});
		//
		// Perform fast march
		unstructured_fastmarch2::fastmarch(positions,connections,levelset,1.0,m_parallel,m_meshutility.get());
		//
		phi_array.parallel_actives([&](int i, int j, auto &it, int tn) {
			double value = levelset[indices()(i,j)];
			if( std::abs(value) > (double)width*dx ) it.set_off();
			else it.set(value);
		});
		//
		phi_array.flood_fill();
	}
	//
	meshutility2_driver m_meshutility{this,"meshutility2"};
	gridutility2_driver m_gridutility{this,"gridutility2"};
	parallel_driver m_parallel{this};
	//
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