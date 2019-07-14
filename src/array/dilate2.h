/*
**	dilate2.h
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
#ifndef SHKZ_DILATE2_H
#define SHKZ_DILATE2_H
//
#include <shiokaze/array/shape.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/array/array_core2.h>
#include <vector>
#include <functional>
#include <set>
#include <cstring>
//
SHKZ_BEGIN_NAMESPACE
//
class dilate2 {
public:
	//
	struct active_state2 {
		vec2i pi;
		std::vector<unsigned char> buffer;
	};
	//
	template <class N> static void dilate( array_core2 *core, std::function<void(int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_index)> func, const parallel_driver &parallel ) {
		//
		shape2 shape;
		unsigned element_bytes;
		core->get(shape[0],shape[1],element_bytes);
		//
		std::vector<std::vector<vec2i> > dilate_coords(parallel.get_thread_num());
		std::vector<void *> caches(parallel.get_thread_num());
		//
		for( auto &cache : caches ) cache = core->generate_cache();
		core->const_parallel_actives([&]( int i, int j, const void *value_ptr, const bool &filled, int thread_index ) {
			bool qi_filled;
			for( int dim : DIMS2 ) for( int dir=-1; dir<=1; dir+=2 ) {
				vec2i qi = vec2i(i,j)+dir*vec2i(dim==0,dim==1);
				if( ! shape.out_of_bounds(qi) && ! (*core)(qi[0],qi[1],qi_filled,caches[thread_index])) {
					dilate_coords[thread_index].push_back(qi);
				}
			}
		},parallel);
		//
		auto compare = [shape]( const vec2i &pi, const vec2i &pj ) {
			N w (shape.w);
			return w*N(pi[1])+N(pi[0]) < w*N(pj[1])+N(pj[0]);
		};
		//
		std::set<vec2i,std::function<bool(const vec2i &pi,const vec2i &pj)> > assembled(compare);
		for( const auto &e : dilate_coords ) {
			for( const auto &it : e ) { assembled.insert(it); }
		}
		//
		std::vector<vec2i> result;
		result.assign(assembled.begin(),assembled.end());
		//
		std::vector<active_state2> active_states[parallel.get_thread_num()];
		parallel.for_each(result.size(),[&]( size_t q, int thread_index ) {
			vec2i pi = result[q];
			bool active (false);
			active_state2 state;
			state.pi = pi;
			state.buffer.resize(element_bytes);
			bool filled;
			(*core)(pi[0],pi[1],filled,caches[thread_index]);
			func(pi[0],pi[1],element_bytes ? state.buffer.data() : nullptr,active,filled,thread_index);
			if( active ) {
				active_states[thread_index].push_back(state);
			}
		});
		//
		for( const auto &e : active_states ) {
			for( const auto &state : e ) {
				core->set(state.pi[0],state.pi[1],[&](void *value_ptr, bool &active) {
					active = true;
					if( element_bytes ) std::memcpy(value_ptr,state.buffer.data(),element_bytes);
				},caches[0]);
			}
		}
		//
		for( auto &cache : caches ) core->destroy_cache(cache);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//