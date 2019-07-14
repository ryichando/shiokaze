/*
**	dilate3.h
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
#ifndef SHKZ_DILATE3_H
#define SHKZ_DILATE3_H
//
#include <shiokaze/core/common.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/array/array_core3.h>
#include <vector>
#include <functional>
#include <set>
#include <cstring>
//
SHKZ_BEGIN_NAMESPACE
//
class dilate3 {
public:
	//
	struct active_state3 {
		vec3i pi;
		std::vector<unsigned char> buffer;
	};
	//
	template <class N> static void dilate( array_core3 *core, std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index)> func, const parallel_driver &parallel ) {
		//
		shape3 shape;
		unsigned element_bytes;
		core->get(shape[0],shape[1],shape[2],element_bytes);
		//
		std::vector<std::vector<vec3i> > dilate_coords(parallel.get_thread_num());
		core->const_parallel_actives([&]( int i, int j, int k, const void *value_ptr, const bool &filled, int thread_index ) {
			bool qi_filled;
			for( int dim : DIMS3 ) for( int dir=-1; dir<=1; dir+=2 ) {
				vec3i qi = vec3i(i,j,k)+dir*vec3i(dim==0,dim==1,dim==2);
				if( ! shape.out_of_bounds(qi) && ! (*core)(qi[0],qi[1],qi[2],qi_filled)) {
					dilate_coords[thread_index].push_back(qi);
				}
			}
		},parallel);
		//
		auto compare = [shape]( const vec3i &pi, const vec3i &pj ) {
			N w (shape.w);
			N h (shape.h);
			return (w*h)*N(pi[2])+w*N(pi[1])+N(pi[0]) < (w*h)*N(pj[2])+w*N(pj[1])+N(pj[0]);
		};
		//
		std::set<vec3i,std::function<bool(const vec3i &pi,const vec3i &pj)> > assembled(compare);
		for( const auto &e : dilate_coords ) {
			for( const auto &it : e ) { assembled.insert(it); }
		}
		//
		std::vector<vec3i> result;
		result.assign(assembled.begin(),assembled.end());
		//
		std::vector<active_state3> active_states[parallel.get_thread_num()];
		parallel.for_each(result.size(),[&]( size_t q, int thread_index ) {
			vec3i pi = result[q];
			bool active (false);
			active_state3 state;
			state.pi = pi;
			state.buffer.resize(element_bytes);
			bool filled;
			(*core)(pi[0],pi[1],pi[2],filled);
			func(pi[0],pi[1],pi[2],element_bytes ? state.buffer.data() : nullptr,active,filled,thread_index);
			if( active ) {
				active_states[thread_index].push_back(state);
			}
		});
		//
		for( const auto &e : active_states ) {
			for( const auto &state : e ) {
				core->set(state.pi[0],state.pi[1],state.pi[2],[&](void *value_ptr, bool &active) {
					active = true;
					if( element_bytes ) std::memcpy(value_ptr,state.buffer.data(),element_bytes);
				});
			}
		}
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//