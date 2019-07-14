/*
**	treearray3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 29, 2019.
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
#ifndef SHKZ_TREEARRAY3_H
#define SHKZ_TREEARRAY3_H
//
#include <shiokaze/array/array_core3.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/array/shape.h>
#include <vector>
#include <cmath>
#include <functional>
#include <limits>
#include <thread>
#include <cstring>
#include "bitcount/bitcount.h"
#include "dilate3.h"
//
SHKZ_BEGIN_NAMESPACE
//
struct host3;
struct leaf3;
struct leaf_cache3 {
	const host3 *host;
	leaf3 *ptr {nullptr};
};
//
struct host3 {
	//
	struct Parameters {
		unsigned tile_size {32};
		unsigned max_depth {1028};
		unsigned max_buffer {65536};
		bool support_cache {true};
		bool debug {false};
	};
	//
	Parameters param;
	//
	leaf_cache3* generate_cache() const {
		//
		if( param.support_cache ) {
			leaf_cache3 *result = new leaf_cache3;
			result->host = this;
			return result;
		} else {
			return nullptr;
		}
	}
	//
	void destroy_cache( leaf_cache3 *cache ) const {
		if( cache ) delete cache;
	}
	//
	std::vector<unsigned char> log2_global_size_per_depth;
	unsigned char element_bytes {0};
	unsigned char total_depth {0};
	shape3 shape;
};
//
struct leaf3 {
	//
	leaf3(	const host3 &host,
			leaf3 *parent,
			const shape3 &shape,
			const vec3i &origin ) : m_host(host), m_parent(parent), m_shape(shape), m_origin(origin) {
	}
	//
	leaf3( leaf3 *parent, const leaf3 *leaf ) : m_host(leaf->m_host) {
		//
		m_parent = parent;
		m_shape = leaf->m_shape;
		m_fill_mask = leaf->m_fill_mask;
		m_origin = leaf->m_origin;
	}
	virtual ~leaf3() = default;
	//
	virtual bool value_exist( const vec3i &local_pi ) const = 0;
	//
	leaf3 * find_root( int i, int j, int k, int &attempts ) {
		vec3i local_pi = convert_to_local(vec3i(i,j,k));
		if( m_shape.out_of_bounds(local_pi)) {
			++ attempts;
			return m_parent ? m_parent->find_root(i,j,k,attempts) : nullptr;
		} else {
			return this;
		}
	}
	//
	const leaf3 * find_root( int i, int j, int k, int &attempts ) const {
		return const_cast<leaf3 *>(this)->find_root(i,j,k,attempts);
	}
	//
	void set_cache( leaf3 *leaf, leaf_cache3 *cache ) const {
		if( cache ) cache->ptr = leaf;
	}
	//
	void alloc_fill_mask() {
		if( m_fill_mask.empty()) m_fill_mask.resize(ceil_div_8(m_shape.count()));
	}
	//
	virtual void fill_all() {
		alloc_fill_mask();
		size_t size = m_shape.count();
		size_t size0 = size >> 3;
		for( size_t n=0; n<size0; ++n ) m_fill_mask[n] = 0xFF;
		for( size_t n=size0; n<size; ++n ) set_filled(n);
	}
	//
	unsigned count_filled() const {
		return bitcount::count(m_fill_mask.data(),m_fill_mask.size(),nullptr);;
	}
	//
	void set_filled( const size_t &n ) {
		//
		alloc_fill_mask();
		unsigned char &mask = *(m_fill_mask.data()+(n>>3));
		mask |= 1UL << (n&7);
	}
	void set_filled( const vec3i &local_pi ) {
		set_filled(m_shape.encode(local_pi));
	}
	//
	void unset_filled( const size_t &n ) {
		//
		alloc_fill_mask();
		unsigned char &mask = *(m_fill_mask.data()+(n>>3));
		mask &= ~(1UL << (n&7));
	}
	//
	void unset_filled( const vec3i &local_pi ) {
		unset_filled(m_shape.encode(local_pi));
	}
	//
	void clear_filled() {
		if( ! m_fill_mask.empty()) {
			for( auto &e : m_fill_mask ) e = 0x00;
		}
	}
	//
	bool filled( const size_t &n ) const {
		//
		if( m_fill_mask.empty()) return false;
		const unsigned char &mask = *(m_fill_mask.data()+(n>>3));
		return (mask >> (n&7)) & 1U;
	}
	//
	bool filled( const vec3i &local_pi ) const {
		return filled(m_shape.encode(local_pi));
	}
	//
	virtual size_t count() const = 0;
	virtual bool set( const vec3i &global_pi, std::function<void(void *value_ptr, bool &active)> func, leaf_cache3 *cache ) = 0;
	virtual const void * operator()( const vec3i &global_pi, bool &filled, leaf_cache3 *cache ) const = 0;
	virtual bool flood_fill( std::function<bool(void *value_ptr)> inside_func ) = 0;
	virtual bool deletable() const = 0;
	virtual void prune( leaf_cache3 *cache ) = 0;
	//
	vec3i convert_to_local( const vec3i &global_pi ) const {
		//
		vec3i result = global_pi-m_origin;
		result[0] = (size_t)result[0] >> m_log2_global_tile_size;
		result[1] = (size_t)result[1] >> m_log2_global_tile_size;
		result[2] = (size_t)result[2] >> m_log2_global_tile_size;
		return result;
	}
	//
	vec3i convert_to_global( const vec3i &local_pi ) const {
		//
		vec3i result = local_pi;
		result[0] <<= m_log2_global_tile_size;
		result[1] <<= m_log2_global_tile_size;
		result[2] <<= m_log2_global_tile_size;
		return result+m_origin;
	}
	//
	unsigned ceil_div_8( unsigned n ) const {
		unsigned result = n >> 3;
		if( (result << 3) != n ) ++result;
		return result;
	}
	//
	void const_loop_all(
		std::function<bool(unsigned n)> func,
		std::function<bool(unsigned skip_byte_num)> skip_func,
		int thread_index=0, int total_threads=1 ) const {
		//
		unsigned size = m_shape.count();
		unsigned size0 = ceil_div_8(size);
		for( unsigned n0=0; n0<size0; ++n0 ) {
			if( n0 % total_threads == thread_index ) {
				if( skip_func(n0)) continue;
				for( unsigned char n1=0; n1<8; ++n1 ) {
					unsigned n = (n0 << 3)+n1;
					if( n < size ) {
						if(func(n)) break;
					}
				}
			}
		}
	}
	//
	void loop_all(
		std::function<bool(unsigned n)> func,
		std::function<bool(unsigned skip_byte_num)> skip_func,
		int thread_index=0, int total_threads=1 ) {
		//
		unsigned size = m_shape.count();
		unsigned size0 = ceil_div_8(size);
		for( unsigned n0=0; n0<size0; ++n0 ) {
			if( n0 % total_threads == thread_index ) {
				if( skip_func(n0)) continue;
				for( unsigned char n1=0; n1<8; ++n1 ) {
					unsigned n = (n0 << 3)+n1;
					if( n < size ) {
						if(func(n)) break;
					}
				}
			}
		}
	}
	//
	void const_loop_inside(
		std::function<bool(unsigned n)> func,
		std::function<bool(unsigned skip_byte_num)> skip_func,
		int thread_index=0, int total_threads=1 ) const {
		//
		unsigned size = m_shape.count();
		unsigned size0 = ceil_div_8(size);
		for( unsigned n0=0; n0<size0; ++n0 ) {
			if( n0 % total_threads == thread_index ) {
				if( skip_func(n0)) continue;
				for( unsigned char n1=0; n1<8; ++n1 ) {
					unsigned n = (n0 << 3)+n1;
					if( n < size ) {
						if( filled(n)) {
							if(func(n)) break;
						}
					}
				}
			}
		}
	}
	//
	virtual void parallel_actives ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) = 0;
	virtual void const_parallel_actives ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) const = 0;
	virtual void serial_actives ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) = 0;
	virtual void const_serial_actives ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &filled )> func ) const = 0;
	virtual void serial_all ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) = 0;
	virtual void const_parallel_all ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) const = 0;
	virtual void const_serial_all ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled )> func ) const = 0;
	virtual void const_parallel_inside ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, int thread_index )> func, int thread_index, int total_threads ) const = 0;
	virtual void const_serial_inside ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active )> func ) const = 0;
	//
	const host3 &m_host;
	leaf3 *m_parent {nullptr};
	shape3 m_shape;
	std::vector<unsigned char> m_fill_mask;
	vec3i m_origin;
	unsigned char m_log2_global_tile_size {0};
};
//
struct terminal_leaf3 : public leaf3 {
	//
	terminal_leaf3( const host3 &host,
					leaf3 *parent,
					const shape3 &shape,
					const vec3i &origin ) : leaf3(host,parent,shape,origin) {
		//
		m_data.resize(m_host.element_bytes*shape.count());
		m_mask.resize(ceil_div_8(shape.count()));
	}
	//
	terminal_leaf3( leaf3 *parent, const terminal_leaf3 *leaf ) : leaf3(parent,leaf) {
		//
		m_data = leaf->m_data;
		m_mask = leaf->m_mask;
	}
	//
	virtual bool value_exist( const vec3i &local_pi ) const override {
		return active(local_pi);
	}
	//
	virtual size_t count() const override {
		return bitcount::count(m_mask.data(),m_mask.size(),nullptr);
	}
	//
	virtual bool deletable() const override {
		return count() == 0;
	}
	//
	virtual void prune( leaf_cache3 *cache ) override {}
	//
	virtual bool set( const vec3i &global_pi, std::function<void(void *value_ptr, bool &active)> func, leaf_cache3 *cache ) override {
		//
		vec3i local_pi = convert_to_local(global_pi);
		size_t n = m_shape.encode(local_pi);
		bool active_flag = active(n);
		set_cache(this,cache);
		//
		if( func ) {
			func( m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr,active_flag);
			if( active_flag ) set_mask(n);
			else unset_mask(n);
		} else {
			set_filled(n);
		}
		return active_flag;
	}
	//
	virtual const void * operator()( const vec3i &global_pi, bool &filled, leaf_cache3 *cache ) const override {
		//
		vec3i local_pi = convert_to_local(global_pi);
		size_t n = m_shape.encode(local_pi);
		filled = this->filled(n);
		set_cache((leaf3 *)this,cache);
		//
		thread_local char tmp;
		if( active(n)) return m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : (void *)&tmp;
		else return nullptr;
	}
	//
	void set_mask( const size_t &n ) {
		//
		unsigned char &mask = *(m_mask.data()+(n>>3));
		mask |= 1UL << (n&7);
	}
	//
	void set_mask( const vec3i &local_pi ) {
		set_mask(m_shape.encode(local_pi));
	}
	//
	void unset_mask( const size_t &n ) {
		//
		unsigned char &mask = *(m_mask.data()+(n>>3));
		mask &= ~(1UL << (n&7));
	}
	//
	void unset_mask( const vec3i &local_pi ) {
		unset_mask(m_shape.encode(local_pi));
	}
	//
	bool active( const size_t &n ) const {
		//
		const unsigned char &mask = *(m_mask.data()+(n>>3));
		return (mask >> (n&7)) & 1U;
	}
	//
	bool active( const vec3i &local_pi ) const {
		return active(m_shape.encode(local_pi));
	}
	//
	bool flood_fill_local( std::function<bool(const vec3i &local_pi)> inside_func ) {
		//
		alloc_fill_mask();
		clear_filled();
		//
		std::stack<vec3i> queue;
		auto markable = [&]( vec3i local_pi, bool default_result ) {
			if( ! m_shape.out_of_bounds(local_pi)) {
				if( ! filled(local_pi)) {
					if( value_exist(local_pi) ) {
						return inside_func(local_pi);
					} else {
						return default_result;
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
		};
		//
		size_t count = m_shape.count();
		m_shape.for_each([&]( int local_i, int local_j, int local_k ) {
			vec3i local_pi (local_i,local_j,local_k);
			if( markable(local_pi,false)) {
				if( value_exist(local_pi) ) {
					queue.push(local_pi);
					while(! queue.empty()) {
						vec3i qi = queue.top();
						set_filled(qi);
						queue.pop();
						for( int dim : DIMS3 ) for( int dir=-1; dir<=1; dir+=2 ) {
							vec3i ni = qi+dir*vec3i(dim==0,dim==1,dim==2);
							if( markable(ni,true)) queue.push(ni);
						}
					}
				}
			}
		});
		//
		return count_filled() == m_shape.count();
	}
	//
	virtual bool flood_fill( std::function<bool(void *value_ptr)> inside_func ) override {
		//
		return flood_fill_local([&]( const vec3i &local_pi ) {
			size_t n = m_shape.encode(local_pi);
			return inside_func(m_data.data()+n*m_host.element_bytes);
		});
	}
	//
	virtual void parallel_actives ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) override {
		//
		const vec3i &o = m_origin;
		loop_all([&]( unsigned n ) {
			bool active_flag = active(n);
			if( active_flag ) {
				bool fill_flag = filled(n);
				void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
				vec3i local_pi = m_shape.decode(n);
				func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,active_flag,fill_flag,thread_index);
				if( ! active_flag ) {
					unset_mask(n);
				}
			}
			return false;
		},[&]( unsigned num_byte ) { return ! *(m_mask.data()+num_byte); },thread_index,total_threads);
	}
	//
	virtual void const_parallel_actives ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) const override {
		//
		const vec3i &o = m_origin;
		const_loop_all([&]( unsigned n ) {
			if( active(n)) {
				bool fill_flag = filled(n);
				const void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
				vec3i local_pi = m_shape.decode(n);
				func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,fill_flag,thread_index);
			}
			return false;
		},[&]( unsigned num_byte ) { return ! *(m_mask.data()+num_byte); },thread_index,total_threads);
	}
	//
	virtual void serial_actives ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) override {
		//
		const vec3i &o = m_origin;
		//
		loop_all([&]( unsigned n ) {
			bool active_flag = active(n);
			bool result (false);
			if( active_flag ) {
				bool fill_flag = filled(n);
				void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
				vec3i local_pi = m_shape.decode(n);
				result = func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,active_flag,fill_flag);
				if( ! active_flag ) {
					unset_mask(n);
				}
			}
			return result;
		},[&]( unsigned num_byte ) { return ! *(m_mask.data()+num_byte); });
	}
	//
	virtual void const_serial_actives ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &filled )> func ) const override {
		//
		const vec3i &o = m_origin;
		const_loop_all([&]( unsigned n ) {
			bool result (false);
			if( active(n)) {
				bool fill_flag = filled(n);
				const void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
				vec3i local_pi = m_shape.decode(n);
				result = func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,fill_flag);
			}
			return result;
		},[&]( unsigned num_byte ) { return ! *(m_mask.data()+num_byte); });
	}
	//
	virtual void serial_all ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) override {
		//
		const vec3i &o = m_origin;
		loop_all([&]( unsigned n ) {
			void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
			vec3i local_pi = m_shape.decode(n);
			bool active_flag = active(n);
			bool fill_flag = filled(n);
			bool result = func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,active_flag,fill_flag);
			if( active_flag ) {
				set_mask(n);
			} else {
				unset_mask(n);
			}
			return result;
		},[&]( unsigned num_byte ) { return false; });
	}
	//
	virtual void const_parallel_all ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) const override {
		//
		const vec3i &o = m_origin;
		const_loop_all([&]( unsigned n ) {
			const void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
			vec3i local_pi = m_shape.decode(n);
			bool active_flag = active(n);
			bool fill_flag = filled(n);
			func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,active_flag,fill_flag,thread_index);
			return false;
		},[&]( unsigned num_byte ) { return false; },thread_index,total_threads);
	}
	//
	virtual void const_serial_all ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled )> func ) const override {
		//
		const vec3i &o = m_origin;
		const_loop_all([&]( unsigned n ) {
			const void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
			vec3i local_pi = m_shape.decode(n);
			bool active_flag = active(n);
			bool fill_flag = filled(n);
			return func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],ptr,active_flag,fill_flag);
		},[&]( unsigned num_byte ) { return false; });
	}
	//
	virtual void const_parallel_inside ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, int thread_index )> func, int thread_index, int total_threads ) const override {
		//
		const vec3i &o = m_origin;
		const_loop_inside([&]( unsigned n ) {
			bool active_flag = active(n);
			const void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
			vec3i local_pi = m_shape.decode(n);
			func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],active_flag ? ptr : nullptr,active_flag,thread_index);
			return false;
		},[&]( unsigned num_byte ) { return *(m_fill_mask.data()+num_byte) == 0; },thread_index,total_threads);
	}
	//
	virtual void const_serial_inside ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active )> func ) const override {
		//
		const vec3i &o = m_origin;
		const_loop_inside([&]( unsigned n ) {
			bool active_flag = active(n);
			vec3i local_pi = m_shape.decode(n);
			const void *ptr = m_host.element_bytes ? m_data.data()+n*m_host.element_bytes : nullptr;
			return func(o[0]+local_pi[0],o[1]+local_pi[1],o[2]+local_pi[2],active_flag ? ptr : nullptr,active_flag);
		},[&]( unsigned num_byte ) { return *(m_fill_mask.data()+num_byte) == 0; });
	}
	//
	std::vector<unsigned char> m_mask;
	std::vector<unsigned char> m_data;
};
//
struct intermediate_leaf3 : public leaf3 {
	//
	intermediate_leaf3( const host3 &host,
						intermediate_leaf3 *parent,
						const shape3 &shape, 
						const vec3i &origin,
						unsigned char depth ) : leaf3(host,parent,shape,origin), m_depth(depth) {
		//
		m_children.resize(shape.count(),nullptr);
		m_log2_global_tile_size = host.log2_global_size_per_depth[depth];
	}
	//
	intermediate_leaf3( intermediate_leaf3 *parent, const intermediate_leaf3 *leaf ) : leaf3(parent,leaf) {
		//
		m_children.resize(leaf->m_children.size(),nullptr);
		m_depth = leaf->m_depth;
		m_num_children = leaf->m_num_children;
		m_log2_global_tile_size = leaf->m_log2_global_tile_size;
		//
		for( size_t n=0; n<leaf->m_children.size(); ++n ) {
			const leaf3 *child = leaf->m_children[n];
			const auto &intermediate_leaf = dynamic_cast<const intermediate_leaf3 *>(child);
			if( intermediate_leaf ) {
				m_children[n] = new intermediate_leaf3(this,intermediate_leaf);
			} else {
				const auto &terminal_leaf = dynamic_cast<const terminal_leaf3 *>(child);
				if( terminal_leaf ) m_children[n] = new terminal_leaf3(this,terminal_leaf);
			}
		}
	}
	//
	virtual ~intermediate_leaf3() {
		//
		if( m_num_children ) {
			for( auto &child : m_children ) {
				if( child ) {
					delete child;
					child = nullptr;
				}
			}
			m_num_children = 0;
		}
	}
	//
	virtual size_t count() const override {
		//
		size_t sum (0);
		if( m_num_children ) {
			for( const auto &child : m_children ) if( child ) sum += child->count();
		}
		return sum;
	}
	//
	virtual bool deletable() const override {
		return m_num_children == 0;
	}
	//
	virtual void prune( leaf_cache3 *cache ) override {
		//
		if( m_num_children ) {
			for( size_t n=0; n<m_children.size(); ++n ) {
				if( m_children[n] ) m_children[n]->prune(cache);
			}
			//
			for( size_t n=0; n<m_children.size(); ++n ) {
				auto &child = m_children[n];
				if( child && child->deletable()) {
					if( cache->ptr == child ) {
						set_cache(nullptr,cache);
					}
					delete child;
					child = nullptr;
					assert( m_num_children );
					-- m_num_children;
				}
			}
		}
	}
	//
	virtual void fill_all() override {
		leaf3::fill_all();
		if( m_num_children ) {
			for( const auto &child : m_children ) if( child ) child->fill_all();
		}
	}
	//
	virtual bool value_exist( const vec3i &local_pi ) const override {
		if( ! m_num_children ) return false;
		return m_children[m_shape.encode(local_pi)] != nullptr;
	}
	//
	virtual bool set( const vec3i &global_pi, std::function<void(void *value_ptr, bool &active)> func, leaf_cache3 *cache ) override {
		//
		vec3i local_pi = convert_to_local(global_pi);
		vec3i o = convert_to_global(local_pi);
		//
		size_t n = m_shape.encode(local_pi);
		assert( n < m_children.size());
		//
		bool active_flag (false);
		if( ! m_children[n] ) {
			//
			thread_local std::vector<unsigned char> data;
			//
			if( func ) {
				if( m_host.element_bytes ) {
					data.resize(m_host.element_bytes);
					func(data.data(),active_flag);
				} else {
					func(nullptr,active_flag);
				}
			}
			//
			if( func == nullptr || active_flag ) {
				//
				if( m_host.total_depth == m_depth+1 ) {
					shape3 terminal_shape(m_host.param.tile_size,m_host.param.tile_size,m_host.param.tile_size);
					for( int dim : DIMS3 ) {
						terminal_shape[dim] += std::min(0,(int)m_host.shape[dim]-(int)o[dim]-(int)terminal_shape[dim]);
					}
					m_children[n] = new terminal_leaf3(m_host,this,terminal_shape,o);
					++ m_num_children;
				} else {
					shape3 child_shape;
					const unsigned log2_next_global_tile_size = m_host.log2_global_size_per_depth[m_depth+1];
					for( int dim : DIMS3 ) {
						unsigned w = m_host.shape[dim]-o[dim];
						unsigned k = w >> log2_next_global_tile_size;
						unsigned char odd = w - (k << log2_next_global_tile_size) ? 1 : 0;
						child_shape[dim] = std::min(m_host.param.tile_size,k+odd);
					}
					m_children[n] = new intermediate_leaf3(m_host,this,child_shape,o,m_depth+1);
					++ m_num_children;
				}
				if( filled(n) ) m_children[n]->fill_all();
				if( func ) {
					m_children[n]->set(global_pi,[&](void *value_ptr, bool &active) {
						if( m_host.element_bytes ) std::memcpy(value_ptr,data.data(),m_host.element_bytes);
						active = true;
					},cache);
				} else {
					m_children[n]->set(global_pi,nullptr,cache);
				}
			}
		} else {
			if( func ) {
				//
				active_flag = m_children[n]->set(global_pi,func,cache);
				//
				if( ! active_flag && m_children[n]->deletable()) {
					delete m_children[n];
					m_children[n] = nullptr;
					set_cache(nullptr,cache);
					assert( m_num_children );
					-- m_num_children;
				}
			} else {
				m_children[n]->set(global_pi,nullptr,cache);
			}
		}
		//
		return active_flag;
	}
	//
	virtual const void * operator()( const vec3i &global_pi, bool &filled, leaf_cache3 *cache ) const override {
		//
		size_t n = m_shape.encode(convert_to_local(global_pi));
		const auto child = m_children[n];
		if( child ) {
			return child->operator()(global_pi,filled,cache);
		} else {
			set_cache((leaf3 *)this,cache);
			filled = this->filled(n);
		}
		return nullptr;
	}
	//
	virtual bool flood_fill( std::function<bool(void *value_ptr)> inside_func ) override {
		//
		clear_filled();
		for( size_t n=0; n<m_children.size(); ++n ) {
			if( m_children[n] ) {
				if( m_children[n]->flood_fill(inside_func) ) {
					set_filled(n);
				}
			}
		}
		//
		std::vector<bool> flags (m_shape.count(),false);
		std::stack<size_t> start_queue;
		const size_t global_tile_size = 1UL << m_host.log2_global_size_per_depth[m_depth];
		//
		leaf_cache3 *cache = m_host.generate_cache();
		m_shape.for_each([&]( int local_i, int local_j, int local_k ) {
			//
			size_t n = m_shape.encode(local_i,local_j,local_k);
			bool adjacent_filled (false);
			//
			if( ! m_children[n] ) {
				for( int dim : DIMS3 ) {
					for( int dir=-1; dir<=1; dir+=2 ) {
						vec3i qi = vec3i(local_i,local_j,local_k)+dir*vec3i(dim==0,dim==1,dim==2);
						if( ! m_shape.out_of_bounds(qi)) {
							size_t m = m_shape.encode(qi);
							if( m_children[m] ) {
								vec3i query_pi;
								if( dir == 1 ) {
									query_pi = m_children[m]->m_origin;
								} else {
									query_pi = m_children[m]->m_origin + vec3i(dim==0,dim==1,dim==2) * (global_tile_size - 1);
								}
								if(filled(m)) {
									adjacent_filled = true;
									flags[n] = true;
									break;
								} else {
									(*m_children[m])(query_pi,adjacent_filled,cache);
									adjacent_filled = adjacent_filled;
									if( adjacent_filled ) {
										flags[n] = true;
										break;
									}
								}
							}
						}
					}
					if( adjacent_filled ) break;
				}
			}
		});
		m_host.destroy_cache(cache);
		//
		m_shape.for_each([&]( int local_i, int local_j, int local_k ) {
			size_t n = m_shape.encode(local_i,local_j,local_k);
			if( flags[n]) {
				start_queue.push(n);
			}
		});
		//
		auto markable = [&]( vec3i ni ) {
			if( ! m_shape.out_of_bounds(ni)) {
				size_t n = m_shape.encode(ni);
				return this->filled(n) == false && ( ! m_children[n] || m_children[n]->count_filled() == 0 );
			} else {
				return false;
			}
		};
		//
		while( ! start_queue.empty()) {
			//
			size_t n = start_queue.top();
			start_queue.pop();
			vec3i pi = m_shape.decode(n);
			//
			std::stack<vec3i> queue;
			queue.push(pi);
			//
			while( ! queue.empty()) {
				//
				vec3i qi = queue.top();
				size_t m = m_shape.encode(qi);
				set_filled(m);
				if( m_children[m] && ! m_children[m]->count_filled()) {
					m_children[m]->fill_all();
				}
				queue.pop();
				for( int dim : DIMS3 ) for( int dir=-1; dir<=1; dir+=2 ) {
					vec3i ni = qi+dir*vec3i(dim==0,dim==1,dim==2);
					if( markable(ni)) queue.push(ni);
				}
			}
		}
		//
		return count_filled() == m_shape.count();
	}
	//
	virtual void parallel_actives ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) override {
		//
		if( m_num_children ) {
			for( size_t n=0; n<m_children.size(); ++n ) {
				auto &child = m_children[n];
				if( child ) {
					child->parallel_actives(func,thread_index,total_threads);
				}
			}
		}
	}
	//
	virtual void const_parallel_actives ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) const override {
		//
		if( m_num_children ) {
			for( const auto &child : m_children ) if( child ) child->const_parallel_actives(func,thread_index,total_threads);
		}
	}
	//
	virtual void serial_actives ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) override {
		//
		if( m_num_children ) {
			for( size_t n=0; n<m_children.size(); ++n ) {
				auto &child = m_children[n];
				if( child ) {
					child->serial_actives(func);
				}
			}
		}
	}
	//
	virtual void const_serial_actives ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &filled )> func ) const override {
		//
		if( m_num_children ) {
			for( const auto &child : m_children ) if( child ) child->const_serial_actives(func);
		}
	}
	//
	virtual void serial_all ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) override {
		//
		thread_local std::vector<unsigned char> buffer;
		buffer.resize(m_host.element_bytes);
		//
		auto cache = m_host.generate_cache();
		for( size_t n=0; n<m_children.size(); ++n ) {
			auto &child = m_children[n];
			if( child ) {
				child->serial_all(func);
			} else {
				//
				const size_t tile_size = 1UL << m_host.log2_global_size_per_depth[m_depth];
				vec3i local_pi = m_shape.decode(n);
				bool fill_flag = filled(n);
				//
				shape3(tile_size,tile_size,tile_size).for_each([&]( int i, int j, int k ) {
					vec3i global_pi = convert_to_global(local_pi) + vec3i(i,j,k);
					if( ! m_host.shape.out_of_bounds(global_pi)) {
						bool active_flag (false);
						func(global_pi[0],global_pi[1],global_pi[2],buffer.data(),active_flag,fill_flag);
						if( active_flag ) {
							set(global_pi,[&](void *value_ptr, bool &active) {
								if( m_host.element_bytes ) std::memcpy(value_ptr,buffer.data(),m_host.element_bytes);
								active = true;
							},cache);
						}
					}
				});
			}
		}
		m_host.destroy_cache(cache);
	}
	//
	virtual void const_parallel_all ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_index )> func, int thread_index, int total_threads ) const override {
		//
		for( size_t n=0; n<m_children.size(); ++n ) {
			auto &child = m_children[n];
			if( child ) {
				child->const_parallel_all(func,thread_index,total_threads);
			} else {
				//
				if( n % total_threads == thread_index ) {
				//
					const size_t tile_size = 1UL << m_host.log2_global_size_per_depth[m_depth];
					vec3i local_pi = m_shape.decode(n);
					//
					bool active_flag (false);
					bool fill_flag = filled(n);
					//
					shape3(tile_size,tile_size,tile_size).for_each([&]( int i, int j, int k ) {
						vec3i global_pi = convert_to_global(local_pi) + vec3i(i,j,k);
						if( ! m_host.shape.out_of_bounds(global_pi)) {
							func(global_pi[0],global_pi[1],global_pi[2],nullptr,active_flag,fill_flag,thread_index);
						}
					});
				}
			}
		}
	}
	//
	virtual void const_serial_all ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled )> func ) const override {
		//
		for( size_t n=0; n<m_children.size(); ++n ) {
			auto &child = m_children[n];
			if( child ) {
				child->const_serial_all(func);
			} else {
				//
				const size_t tile_size = 1UL << m_host.log2_global_size_per_depth[m_depth];
				vec3i local_pi = m_shape.decode(n);
				vec3i local_origin = convert_to_global(local_pi);
				//
				bool active_flag (false);
				bool fill_flag = filled(n);
				//
				shape3(tile_size,tile_size,tile_size).for_each([&]( int i, int j, int k ) {
					vec3i global_pi = local_origin + vec3i(i,j,k);
					if( ! m_host.shape.out_of_bounds(global_pi)) {
						func(global_pi[0],global_pi[1],global_pi[2],nullptr,active_flag,fill_flag);
					}
				});
			}
		}
	}
	//
	virtual void const_parallel_inside ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, int thread_index )> func, int thread_index, int total_threads ) const override {
		//
		for( size_t n=0; n<m_children.size(); ++n ) {
			auto &child = m_children[n];
			if( child ) {
				child->const_parallel_inside(func,thread_index,total_threads);
			} else {
				//
				if( n % total_threads == thread_index ) {
				//
					const size_t tile_size = 1UL << m_host.log2_global_size_per_depth[m_depth];
					vec3i local_pi = m_shape.decode(n);
					vec3i local_origin = convert_to_global(local_pi);
					//
					bool active_flag (false);
					bool fill_flag = filled(n);
					//
					if( fill_flag ) {
						shape3(tile_size,tile_size,tile_size).for_each([&]( int i, int j, int k ) {
							vec3i global_pi = local_origin + vec3i(i,j,k);
							if( ! m_host.shape.out_of_bounds(global_pi)) {
								func(global_pi[0],global_pi[1],global_pi[2],nullptr,active_flag,thread_index);
							}
						});
					}
				}
			}
		}
	}
	//
	virtual void const_serial_inside ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active )> func ) const override {
		//
		for( size_t n=0; n<m_children.size(); ++n ) {
			auto &child = m_children[n];
			if( child ) {
				child->const_serial_inside(func);
			} else {
				//
				const size_t tile_size = 1UL << m_host.log2_global_size_per_depth[m_depth];
				vec3i local_pi = m_shape.decode(n);
				vec3i local_origin = convert_to_global(local_pi);
				//
				bool active_flag (false);
				bool fill_flag = filled(n);
				//
				if( fill_flag ) {
					shape3(tile_size,tile_size,tile_size).for_each([&]( int i, int j, int k ) {
						vec3i global_pi = local_origin + vec3i(i,j,k);
						if( ! m_host.shape.out_of_bounds(global_pi)) {
							func(global_pi[0],global_pi[1],global_pi[2],nullptr,active_flag);
						}
					});
				}
			}
		}
	}
	//
	std::vector<leaf3 *> m_children;
	unsigned char m_depth {0};
	unsigned m_num_children {0};
};
//
struct cache_struct {
	cache_struct ( const host3 &host ) : host(host) {
		ptr = host.generate_cache();
	}
	~cache_struct() {
		host.destroy_cache(ptr);
	}
	leaf_cache3 *ptr;
	const host3 &host;
};
//
class treearray3 : public array_core3 {
public:
	//
	LONG_NAME("Tree Array 3D")
	ARGUMENT_NAME("TreeArray")
	MODULE_NAME("treearray3")
	//
	virtual ~treearray3() {
		dealloc();
	}
	//
	void dealloc () {
		if( m_root ) {
			delete m_root;
			m_root = nullptr;
		}
	}
	//
	leaf_cache3 * get_cache() const {
		//
		if( ! m_host.param.support_cache ) return nullptr;
		//
		thread_local std::thread::id thread_id = std::this_thread::get_id();
		if( thread_id == m_main_thread_id ) return m_main_cache;
		//
		thread_local std::vector<std::pair<void *,std::shared_ptr<cache_struct> > > cache_list;
		for( const auto &c : cache_list ) {
			if( c.first == (void *)this ) return c.second->ptr;
		}
		cache_list.push_back({(void *)this,std::make_shared<cache_struct>(m_host)});
		return cache_list.back().second->ptr;
	};
	//
	virtual void configure( configuration &config ) override {
		config.get_unsigned("TileSize",m_host.param.tile_size,"Tile size per dimension");
		config.get_unsigned("MaxDepth",m_host.param.max_depth,"Maximal depth allowed");
		config.get_unsigned("MaxBuffer",m_host.param.max_buffer,"Maximal buffer size");
		config.get_bool("EnableCache",m_host.param.support_cache,"Enable cache");
		//
		assert( utility::is_power_of_two(m_host.param.tile_size) );
		assert( m_host.param.tile_size * m_host.param.tile_size * m_host.param.tile_size <= std::numeric_limits<unsigned>::max());
	}
	//
	virtual void initialize( unsigned nx, unsigned ny, unsigned nz, unsigned element_bytes ) override {
		//
		assert( element_bytes <= std::numeric_limits<unsigned char>::max() );
		assert( nx <= std::numeric_limits<int>::max() );
		assert( ny <= std::numeric_limits<int>::max() );
		assert( nz <= std::numeric_limits<int>::max() );
		dealloc();
		//
		m_host.total_depth = std::min(m_host.param.max_depth,(unsigned)std::ceil(log(std::max(nx,std::max(ny,nz)))/log(m_host.param.tile_size)));
		assert( m_host.total_depth >= 1 );
		if( m_host.param.debug ) printf( "treearray3: total depth = %u\n", m_host.total_depth );
		//
		unsigned log2_tile_size = utility::log2(m_host.param.tile_size);
		m_host.log2_global_size_per_depth.resize(m_host.total_depth);
		for( int depth=0; depth < m_host.total_depth; ++ depth ) {
			m_host.log2_global_size_per_depth[depth] = log2_tile_size + (log2_tile_size * (m_host.total_depth-1-depth));
		}
		//
		m_host.shape = shape3(nx,ny,nz);
		m_host.element_bytes = element_bytes;
		//
		shape3 child_shape;
		const size_t &next_global_tile_size = 1UL << m_host.log2_global_size_per_depth[0];
		for( int dim : DIMS3 ) {
			child_shape[dim] = std::ceil(m_host.shape[dim]/(double)next_global_tile_size);
		}
		//
		m_root = new intermediate_leaf3(m_host,nullptr,child_shape,vec3i(),0);
		if( m_host.param.support_cache ) {
			m_main_cache = m_host.generate_cache();
			m_main_thread_id = std::this_thread::get_id();
		}
	}
	//
	virtual void send_message( unsigned message, void *ptr ) override {
		//
		if( message == 'DEBG') {
			m_host.param.debug = ptr == (void *)1;
		}
	}
	//
	virtual void get( unsigned &nx, unsigned &ny, unsigned &nz, unsigned &element_bytes ) const override {
		//
		nx = m_host.shape[0];
		ny = m_host.shape[1];
		nz = m_host.shape[2];
		element_bytes = m_host.element_bytes;
	}
	//
	virtual size_t count( const parallel_driver &parallel ) const override {
		return m_root ? m_root->count() : 0;
	}
	//
	virtual void copy( const array_core3 &array, std::function<void(void *target, const void *src)> copy_func, const parallel_driver &parallel ) override {
		//
		dealloc();
		//
		auto mate_array = dynamic_cast<const treearray3 *>(&array);
		if( mate_array ) {
			//
			m_host = mate_array->m_host;
			//
			auto intermediate_leaf = dynamic_cast<intermediate_leaf3 *>(mate_array->m_root);
			assert(intermediate_leaf);
			m_root = new intermediate_leaf3(nullptr,intermediate_leaf);
			//
		} else {
			//
			unsigned nx, ny, nz, element_bytes;
			array.get(nx,ny,nz,element_bytes);
			initialize(nx,ny,nz,element_bytes);
			//
			array.const_serial_actives([&](int i, int j, int k, const void *src_ptr, const bool &filled) {
				set(i,j,k,[&](void *dst_ptr, bool &active) {
					copy_func(dst_ptr,src_ptr);
					active = true;
				});
				return false;
			});
			//
			array.const_serial_inside([&](int i, int j, int k, const void *src_ptr, const bool &active) {
				set(i,j,k,nullptr);
				return false;
			});
		}
		//
		if( m_main_cache ) {
			m_host.destroy_cache(m_main_cache);
			m_main_cache = m_host.generate_cache();
			m_main_thread_id = std::this_thread::get_id();
		}
	}
	//
	bool check_bound( int i, int j, int k ) const {
		if( i >= 0 && j >= 0 && k >=0 && i < m_host.shape[0] && j < m_host.shape[1] && k < m_host.shape[2] ) {
			return true;
		} else {
			printf( "Out of bounds (i=%d,j=%d,k=%d), (w=%d,h=%d,d=%d)\n", i, j, k, m_host.shape[0], m_host.shape[1], m_host.shape[2] );
			return false;
		}
	}
	//
	virtual void set( int i, int j, int k, std::function<void(void *value_ptr, bool &active)> func ) override {
		//
		assert(check_bound(i,j,k));
		assert(m_root);
		int attempts (0);
		auto cache = get_cache();
		find_root(i,j,k,cache,attempts)->set(vec3i(i,j,k),func,cache);
	}
	//
	virtual const void * operator()( int i, int j, int k, bool &filled ) const override {
		//
		assert(check_bound(i,j,k));
		int attempts (0);
		auto cache = get_cache();
		const auto leaf = find_root(i,j,k,cache,attempts);
		return leaf->operator()(vec3i(i,j,k),filled,cache);
	}
	//
	virtual void parallel_actives ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) override {
		//
		if( m_root ) {
			unsigned total_threads = parallel.get_thread_num();
			parallel.for_each(total_threads,[&]( size_t thread_index ) {
				m_root->parallel_actives(func,thread_index,total_threads);
			});
			m_root->prune(m_main_cache);
		}
	}
	//
	virtual void serial_actives ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) override {
		//
		if( m_root ) {
			m_root->serial_actives(func);
			m_root->prune(m_main_cache);
		}
	}
	//
	virtual void const_parallel_actives ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) const override {
		//
		if( m_root ) {
			unsigned total_threads = parallel.get_thread_num();
			parallel.for_each(total_threads,[&]( size_t thread_index ) {
				m_root->const_parallel_actives(func,thread_index,total_threads);
			});
		}
	}
	//
	virtual void const_serial_actives ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &filled )> func ) const override {
		if( m_root ) m_root->const_serial_actives(func);
	}
	//
	virtual void parallel_all ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) override {
		//
		size_t buffer_size = std::min((size_t)m_host.param.max_buffer,m_host.shape.count());
		std::vector<unsigned char> buffer(buffer_size * m_host.element_bytes);
		std::vector<unsigned char> flags(buffer_size);
		//
		size_t advanced (0);
		while( true ) {
			//
			size_t advance_size = std::min(buffer_size,(size_t)(m_host.shape.count()-advanced));
			parallel.for_each(advance_size,[&]( size_t n, int thread_index ) {
				//
				bool prev_active_flag, active_flag, filled_flag;
				vec3i coord = m_host.shape.decode(n+advanced);
				void *dst_ptr = m_host.element_bytes ? buffer.data()+n*m_host.element_bytes : nullptr;
				const void *src_ptr = operator()(coord[0],coord[1],coord[2],filled_flag);
				if( src_ptr ) {
					std::memcpy(dst_ptr,src_ptr,m_host.element_bytes);
					prev_active_flag = true;
				} else {
					prev_active_flag = false;
				}
				active_flag = prev_active_flag;
				func(coord[0],coord[1],coord[2],dst_ptr,active_flag,filled_flag,thread_index);
				unsigned char flag (0);
				if( active_flag ) flag |= 1UL;
				if( prev_active_flag != active_flag ) flag |= 2UL;
				flags[n] = flag;
				//
			});
			//
			for( size_t n=0; n<advance_size; ++n ) {
				vec3i pi = m_host.shape.decode(n+advanced);
				if( flags[n] ) {
					set(pi[0],pi[1],pi[2],[&](void *value_ptr, bool &active) {
						active = flags[n] & 1UL;
						if( active && m_host.element_bytes ) {
							std::memcpy(value_ptr,buffer.data()+n*m_host.element_bytes,m_host.element_bytes);
						}
					});
				}
			}
			//
			advanced += advance_size;
			if( advanced == m_host.shape.count()) break;
		}
	}
	//
	virtual void serial_all ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) override {
		//
		if( m_root ) {
			m_root->serial_all(func);
			m_root->prune(m_main_cache);
		}
	}
	//
	virtual void const_parallel_all ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) const override {
		//
		if( m_root ) {
			unsigned total_threads = parallel.get_thread_num();
			parallel.for_each(total_threads,[&]( size_t thread_index ) {
				m_root->const_parallel_all(func,thread_index,total_threads);
			});
		}
	}
	//
	virtual void const_serial_all ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled )> func ) const override {
		if( m_root ) m_root->const_serial_all(func);
	}
	//
	virtual void dilate( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index)> func, const parallel_driver &parallel ) override {
		dilate3::dilate<__uint128_t>(this,func,parallel);
	}
	//
	virtual void flood_fill( std::function<bool(void *value_ptr)> inside_func, const parallel_driver &parallel ) override {
		if( m_root ) m_root->flood_fill(inside_func);
	}
	//
	virtual void const_parallel_inside ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, int thread_index )> func, const parallel_driver &parallel ) const override {
		if( m_root ) {
			unsigned total_threads = parallel.get_thread_num();
			parallel.for_each(total_threads,[&]( size_t thread_index ) {
				m_root->const_parallel_inside(func,thread_index,total_threads);
			});
		}
	}
	//
	virtual void const_serial_inside ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active )> func ) const override {
		if( m_root ) m_root->const_serial_inside(func);
	}
	//
	leaf3 * find_root( int i, int j, int k, leaf_cache3 *cache, int &attempts ) {
		if( cache && m_host.param.support_cache ) {
			assert( cache->host == &m_host );
			if( cache->ptr ) {
				leaf3 *result = cache->ptr->find_root(i,j,k,attempts);
				if( result ) return result;
			}
		}
		return m_root;
	}
	//
	const leaf3 * find_root( int i, int j, int k, leaf_cache3 *cache, int &attempts ) const {
		return const_cast<treearray3 *>(this)->find_root(i,j,k,cache,attempts);
	}
	//
	host3 m_host;
	intermediate_leaf3 *m_root {nullptr};
	leaf_cache3 *m_main_cache {nullptr};
	std::thread::id m_main_thread_id;
};
//
extern "C" module * create_instance() {
	return new treearray3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
SHKZ_END_NAMESPACE
//
#endif
//