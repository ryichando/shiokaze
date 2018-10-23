/*
**	lineararray2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 7, 2018.
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
#ifndef SHKZ_LINEARARRAY2_H
#define SHKZ_LINEARARRAY2_H
//
#include <vector>
#include <cmath>
#include <stack>
#include <limits>
#include <cstring>
#include <cassert>
#include <shiokaze/array/array_core2.h>
#include "bitcount/bitcount.h"
#include "dilate2.h"
//
SHKZ_BEGIN_NAMESPACE
//
class lineararray2 : public array_core2 {
public:
	lineararray2 () = default;
private:
	//
	LONG_NAME("Linear Array 2D")
	ARGUMENT_NAME("LinArray")
	//
	virtual void initialize( unsigned nx, unsigned ny, unsigned element_size ) override {
		//
		dealloc();
		//
		m_nx = nx;
		m_ny = ny;
		//
		m_element_size = element_size;
		if( m_element_size ) {
			m_buffer = new unsigned char [m_nx*m_ny*m_element_size];
		}
		//
		m_bit_mask_size = std::ceil(m_nx*m_ny/8.0);
		m_bit_mask = new unsigned char [m_bit_mask_size];
		std::memset(m_bit_mask,0,m_bit_mask_size);
	}
	//
	virtual void get( unsigned &nx, unsigned &ny, unsigned &element_size ) const override {
		nx = m_nx;
		ny = m_ny;
		element_size = m_element_size;
	}
	//
	virtual size_t count( const parallel_driver &parallel ) const override {
		return bitcount::count(m_bit_mask,m_bit_mask_size,&parallel);
	}
	//
	virtual ~lineararray2() {
		dealloc();
	}
	//
	void dealloc () {
		if( m_buffer ) {
			delete [] m_buffer;
			m_buffer = nullptr;
		}
		if( m_bit_mask ) {
			delete [] m_bit_mask;
			m_bit_mask = nullptr;
		}
		if( m_fill_mask ) {
			delete [] m_fill_mask;
			m_fill_mask = nullptr;
		}
		m_buffer = m_bit_mask = nullptr;
	}
	//
	virtual void copy( const array_core2 &array, std::function<void(void *target, const void *src)> copy_func, const parallel_driver *parallel ) override {
		//
		auto mate_array = dynamic_cast<const lineararray2 *>(&array);
		dealloc();
		//
		if( mate_array ) {
			//
			m_nx = mate_array->m_nx;
			m_ny = mate_array->m_ny;
			m_element_size = mate_array->m_element_size;
			m_bit_mask_size = mate_array->m_bit_mask_size;
			//
			if( m_bit_mask_size ) {
				m_bit_mask = new unsigned char [m_bit_mask_size];
				std::memcpy(m_bit_mask,mate_array->m_bit_mask,m_bit_mask_size);
				if( mate_array->m_fill_mask ) {
					m_fill_mask = new unsigned char [m_bit_mask_size];
					std::memcpy(m_fill_mask,mate_array->m_fill_mask,m_bit_mask_size);
				}
			}
			if( mate_array->m_buffer && m_element_size ) {
				size_t size = m_nx*m_ny*m_element_size;
				m_buffer = new unsigned char [size];
				auto copy_body = [&]( size_t n ) {
					const unsigned char &mask = *(m_bit_mask+n/8);
					if((mask >> n%8) & 1U) {
						size_t offset = n*m_element_size;
						copy_func(m_buffer+offset,mate_array->m_buffer+offset);
					}
				};
				if( parallel ) {
					parallel->for_each(m_nx*m_ny,[&](size_t n) { copy_body(n); });
				} else {
					for( size_t n=0; n<m_nx*m_ny; ++n ) copy_body(n);
				}
			}
		} else {
			//
			unsigned m_nx, m_ny, m_element_size;
			array.get(m_nx,m_ny,m_element_size);
			initialize(m_nx,m_ny,m_element_size);
			//
			array.const_serial_actives([&](int i, int j, const void *value_ptr, const bool &filled ) {
				const size_t n = encode(i,j);
				unsigned char &mask = *(m_bit_mask+n/8);
				mask |= 1UL << n%8;
				if( filled ) {
					if( ! m_fill_mask ) {
						m_fill_mask = new unsigned char [m_bit_mask_size]; 
						std::memset(m_fill_mask,0,m_bit_mask_size);
					}
					*(m_fill_mask+n/8) |= 1UL << n%8;
				}
				if( m_buffer ) {
					copy_func(m_buffer+n*m_element_size,value_ptr);
				}
				return false;
			});
			array.const_serial_inside([&](int i, int j, const void *value_ptr, const bool &active ) {
				if( ! active ) {
					const size_t n = encode(i,j);
					if( ! m_fill_mask ) {
						m_fill_mask = new unsigned char [m_bit_mask_size]; 
						std::memset(m_fill_mask,0,m_bit_mask_size);
					}
					*(m_fill_mask+n/8) |= 1UL << n%8;
				}
				return false;
			});
		}
	}
	//
	virtual void* generate_cache() const override {
		return nullptr;
	}
	virtual void destroy_cache( void *cache ) const override {
	}
	//
	bool check_bound( int i, int j ) const {
		if( i >= 0 && j >= 0 && i < m_nx && j < m_ny ) {
			return true;
		} else {
			printf( "Out of bounds (i=%d,j=%d), (w=%d,h=%d)\n", i, j, m_nx, m_ny );
			return false;
		}
	}
	//
	virtual void set( int i, int j, std::function<void(void *value_ptr, bool &active)> func, void *cache=nullptr ) override {
		//
		assert(check_bound(i,j));
		const size_t n = encode(i,j);
		unsigned char &mask = *(m_bit_mask+n/8);
		bool active = (mask >> n%8) & 1U;
		unsigned char *ptr = m_buffer ? m_buffer+n*m_element_size : nullptr;
		//
		func(ptr,active);
		//
		mask |= 1UL << n%8;
		if( active ) mask |= 1UL << n%8;
		else mask &= ~(1UL << n%8);
	}
	//
	virtual const void * operator()( int i, int j, bool &filled, void *cache=nullptr ) const override {
		//
		assert(check_bound(i,j));
		const size_t n = encode(i,j);
		unsigned char &mask = *(m_bit_mask+n/8);
		filled = m_fill_mask ? (*(m_fill_mask+n/8) >> n%8) & 1U : false;
		static char tmp_ptr;
		if( (mask >> n%8) & 1U ) return m_buffer ? m_buffer + n*m_element_size : (void *)&tmp_ptr;
		return nullptr;
	}
	//
	virtual void dilate( std::function<void(int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_index)> func, const parallel_driver &parallel ) override {
		//
		std::vector<size_t> dilate_coords;
		dilate_coords = dilate2::dilate(shape2(m_nx,m_ny),m_bit_mask,m_bit_mask_size,&parallel);
		//
		std::vector<size_t> active_states[parallel.get_maximal_threads()];
		parallel.for_each(dilate_coords.size(),[&]( size_t q, int thread_index ) {
			size_t n = dilate_coords[q];
			int i, j;
			decode(n,i,j);
			bool active (false);
			bool filled = m_fill_mask ? (*(m_fill_mask+n/8) >> n%8) & 1U : false;
			func(i,j, m_buffer ? m_buffer+n*m_element_size : nullptr ,active,filled,thread_index);
			if( active ) {
				active_states[thread_index].push_back(n);
			}
		});
		//
		for( const auto &e : active_states ) {
			for( const auto &n : e ) {
				unsigned char &mask = *(m_bit_mask+n/8);
				mask |= 1UL << n%8;
			}
		}
	}
	//
	virtual void flood_fill( std::function<bool(void *value_ptr)> inside_func, const parallel_driver &parallel ) override {
		//
		if( ! m_fill_mask ) m_fill_mask = new unsigned char [m_bit_mask_size];
		std::memset(m_fill_mask,0,m_bit_mask_size);
		//
		std::stack<vec2i> queue;
		auto markable = [&]( vec2i pi, bool default_result ) {
			if( ! shape2(m_nx,m_ny).out_of_bounds(pi)) {
				auto pass_m_fill_mask = [&]( size_t n ) {
					return ! ((*(m_fill_mask+n/8) >> n%8) & 1U);
				};
				const size_t n = encode(pi[0],pi[1]);
				if( (*(m_bit_mask+n/8) >> n%8) & 1U ) {
					return inside_func(m_buffer ? m_buffer+n*m_element_size : nullptr) && pass_m_fill_mask(n);
				} else {
					return default_result && pass_m_fill_mask(n);
				}
			} else {
				return false;
			}
		};
		auto mark = [&]( size_t n ) {
			*(m_fill_mask+n/8) |= 1UL << n%8;
		};
		size_t count = shape2(m_nx,m_ny).count();
		for( size_t n8=0; n8<m_bit_mask_size; ++n8 ) {
			if( *(m_bit_mask+n8) ) {
				for( size_t n=8*n8; n < 8*(n8+1); ++n ) if ( n < count ) {
					int i, j; decode(n,i,j);
					vec2i pi(i,j);
					if( markable(pi,false)) {
						if( (*(m_bit_mask+n8) >> n%8) & 1U ) {
							queue.push(pi);
							while(! queue.empty()) {
								vec2i qi = queue.top();
								mark(encode(qi[0],qi[1]));
								queue.pop();
								for( int dim : DIMS2 ) for( int dir=-1; dir<=1; dir+=2 ) {
									vec2i ni = qi+dir*vec2i(dim==0,dim==1);
									if( markable(ni,true)) queue.push(ni);
								}
							}
						}
					}
				}
			}
		};
	}
	//
	virtual void const_parallel_inside ( std::function<void(int i, int j, const void *value_ptr, const bool &active, int thread_index )> func, const parallel_driver &parallel ) const override {
		//
		if( m_fill_mask ) {
			size_t count = shape2(m_nx,m_ny).count();
			parallel.for_each(m_bit_mask_size,[&]( size_t n8, int thread_index ) {
				unsigned char &mask = *(m_fill_mask+n8);
				if( mask ) {
					for( size_t n=8*n8; n < 8*(n8+1); ++n ) if ( n < count ) {
						if( (mask >> n%8) & 1U ) {
							int i, j; decode(n,i,j);
							bool active = ((*(m_bit_mask+n/8)) >> n%8) & 1U;
							func(i,j, m_buffer ? m_buffer+n*m_element_size : nullptr,active,thread_index);
						}
					}
				}
			});
		}
	}
	virtual void const_serial_inside ( std::function<bool(int i, int j, const void *value_ptr, const bool &active )> func ) const override {
		//
		if( m_fill_mask ) {
			size_t count = shape2(m_nx,m_ny).count();
			for( size_t n8=0; n8<m_bit_mask_size; ++n8 ) {
				unsigned char &mask = *(m_fill_mask+n8);
				if( mask ) {
					bool do_break (false);
					for( size_t n=8*n8; n < 8*(n8+1); ++n ) if ( n < count ) {
						if( (mask >> n%8) & 1U ) {
							int i, j; decode(n,i,j);
							bool active = ((*(m_bit_mask+n/8)) >> n%8) & 1U;
							if(func(i,j,m_buffer ? m_buffer+n*m_element_size : nullptr,active)) {
								do_break = true;
								break;
							}
						}
					}
					if( do_break ) break;
				}
			}
		}
	}
	//
	void parallel_loop_actives_body ( int i, int j, std::function<void(int i, int j, void *value_ptr, bool &active, const bool &filled )> func ) {
		loop_actives_body(i,j,[&](int i, int j, void *value_ptr, bool &active, const bool &filled ) {
			func(i,j,value_ptr,active,filled);
			return false;
		});
	}
	bool loop_actives_body ( int i, int j, std::function<bool(int i, int j, void *value_ptr, bool &active, const bool &filled )> func ) {
		const size_t n = encode(i,j);
		unsigned char &mask = *(m_bit_mask+n/8);
		if( mask ) {
			bool active = (mask >> n%8) & 1U;
			bool filled = m_fill_mask ? (*(m_fill_mask+n/8) >> n%8) & 1U : false;
			if( active ) {
				bool result = func(i,j,m_buffer ? m_buffer+n*m_element_size : nullptr,active,filled);
				if( ! active ) mask &= ~(1UL << n%8);
				if( result ) return true;
			}
		}
		return false;
	}
	//
	void parallel_const_loop_actives_body ( int i, int j, std::function<void(int i, int j, const void *value_ptr, const bool &filled )> func ) const {
		const_loop_actives_body(i,j,[&](int i, int j, const void *value_ptr, const bool &filled ) {
			func(i,j,value_ptr,filled);
			return false;
		});
	}
	bool const_loop_actives_body ( int i, int j, std::function<bool(int i, int j, const void *value_ptr, const bool &filled )> func ) const {
		const size_t n = encode(i,j);
		const unsigned char &mask = *(m_bit_mask+n/8);
		if( mask ) {
			if( (mask >> n%8) & 1U ) {
				bool filled = m_fill_mask ? (*(m_fill_mask+n/8) >> n%8) & 1U : false;
				if(func(i,j,m_buffer ? m_buffer+n*m_element_size : nullptr,filled)) return true;
			}
		}
		return false;
	}
	//
	void parallel_loop_all_body ( int i, int j, std::function<void(int i, int j, void *value_ptr, bool &active, const bool &filled )> func ) {
		loop_all_body(i,j,[&](int i, int j, void *value_ptr, bool &active, const bool &filled) {
			func(i,j,value_ptr,active,filled);
			return false;
		});
	}
	bool loop_all_body ( int i, int j, std::function<bool(int i, int j, void *value_ptr, bool &active, const bool &filled )> func ) {
		const size_t n = encode(i,j);
		unsigned char &mask = *(m_bit_mask+n/8);
		bool active = (mask >> n%8) & 1U;
		bool new_active (active);
		bool filled = m_fill_mask ? (*(m_fill_mask+n/8) >> n%8) & 1U : false;
		bool result = func(i,j,m_buffer ? m_buffer+n*m_element_size : nullptr,new_active,filled);
		if( new_active != active ) {
			if( new_active ) mask |= 1UL << n%8;
			else mask &= ~(1UL << n%8);
		}
		return result;
	}
	void parallel_const_loop_all_body ( int i, int j, std::function<void(int i, int j, const void *value_ptr, const bool &active, const bool &filled )> func ) const {
		const_loop_all_body(i,j,[&](int i, int j, const void *value_ptr, const bool &active, const bool &filled) {
			func(i,j,value_ptr,active,filled);
			return false;
		});
	}
	bool const_loop_all_body ( int i, int j, std::function<bool(int i, int j, const void *value_ptr, const bool &active, const bool &filled )> func ) const {
		const size_t n = encode(i,j);
		const unsigned char &mask = *(m_bit_mask+n/8);
		bool active = (mask >> n%8) & 1U;
		bool filled = m_fill_mask ? (*(m_fill_mask+n/8) >> n%8) & 1U : false;
		return func(i,j,m_buffer ? m_buffer+n*m_element_size : nullptr,active,filled);
	}
	//
	virtual void parallel_actives ( std::function<void(int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) override {
		//
		size_t count = shape2(m_nx,m_ny).count();
		parallel.for_each(m_bit_mask_size,[&](size_t n8, int thread_index) {
			if( *(m_bit_mask+n8) ) {
				for( size_t n=8*n8; n<8*(n8+1); ++n ) if( n < count ) {
					int i, j; decode(n,i,j);
					parallel_loop_actives_body(i,j,[&](int i, int j, void *value_ptr, bool &active, const bool &filled) {
						return func(i,j,value_ptr,active,filled,thread_index);
					});
				}
			}
		});
	}
	virtual void serial_actives ( std::function<bool(int i, int j, void *value_ptr, bool &active, const bool &filled )> func ) override {
		for( int j=0; j<m_ny; ++j ) for( int i=0; i<m_nx; ++i ) if(loop_actives_body(i,j,func)) goto serial_actives_end;
serial_actives_end:	;
	}
	virtual void const_parallel_actives ( std::function<void(int i, int j, const void *value_ptr, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) const override {
		//
		size_t count = shape2(m_nx,m_ny).count();
		parallel.for_each(m_bit_mask_size,[&](size_t n8, int thread_index ) {
			if( *(m_bit_mask+n8) ) {
				for( size_t n=8*n8; n<8*(n8+1); ++n ) if( n < count ) {
					int i, j; decode(n,i,j);
					parallel_const_loop_actives_body(i,j,[&](int i, int j, const void *value_ptr, const bool &filled) {
						return func(i,j,value_ptr,filled,thread_index);
					});
				}
			}
		});
	}
	virtual void const_serial_actives ( std::function<bool(int i, int j, const void *value_ptr, const bool &filled )> func ) const override {
		for( int j=0; j<m_ny; ++j ) for( int i=0; i<m_nx; ++i ) if(const_loop_actives_body(i,j,func)) goto const_serial_actives_end;
const_serial_actives_end: ;
	}
	virtual void parallel_all ( std::function<void(int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) override {
		//
		size_t count = shape2(m_nx,m_ny).count();
		parallel.for_each(m_bit_mask_size,[&](size_t n8, int thread_index) {
			for( size_t n=8*n8; n<8*(n8+1); ++n ) if( n < count ) {
				int i, j; decode(n,i,j);
				parallel_loop_all_body(i,j,[&](int i, int j, void *value_ptr, bool &active, const bool &filled ) {
					func(i,j,value_ptr,active,filled,thread_index);
				});
			}
		});
	}
	virtual void serial_all ( std::function<bool(int i, int j, void *value_ptr, bool &active, const bool &filled )> func ) override {
		for( int j=0; j<m_ny; ++j ) for( int i=0; i<m_nx; ++i ) if(loop_all_body(i,j,func)) goto serial_all_end;
serial_all_end: ;
	}
	virtual void const_parallel_all ( std::function<void(int i, int j, const void *value_ptr, const bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) const override {
		//
		size_t count = shape2(m_nx,m_ny).count();
		parallel.for_each(m_bit_mask_size,[&](size_t n8, int thread_index) {
			for( size_t n=8*n8; n<8*(n8+1); ++n ) if( n < count ) {
				int i, j; decode(n,i,j);
				parallel_const_loop_all_body(i,j,[&](int i, int j, const void *value_ptr, const bool &active, const bool &filled) {
					func(i,j,value_ptr,active,filled,thread_index);
				});
			}
		});
	}
	virtual void const_serial_all ( std::function<bool(int i, int j, const void *value_ptr, const bool &active, const bool &filled )> func ) const override {
		for( int j=0; j<m_ny; ++j ) for( int i=0; i<m_nx; ++i ) if(const_loop_all_body(i,j,func)) goto const_serial_all_end;
const_serial_all_end: ;
	}
	//
	unsigned char *m_buffer {nullptr};
	unsigned char *m_bit_mask {nullptr};
	unsigned char *m_fill_mask {nullptr};
	unsigned m_nx {0}, m_ny {0}, m_element_size {0}, m_bit_mask_size {0};
	//
	size_t encode( int i, int j ) const { return i + j * m_nx; }
	void decode( size_t n, int &i, int &j) const { 
		i = n % m_nx;
		j = n / m_nx;
	}
};
//
extern "C" module * create_instance() {
	return new lineararray2();
}
//
extern "C" const char *license() {
	return "BSD-{2,3}-Clause";
}
//
SHKZ_END_NAMESPACE
//
#endif
