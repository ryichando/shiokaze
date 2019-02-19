/*
**	dilate2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 12, 2018.
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
#include "dilate2.h"
#include "parallel_active_loop.h"
#include <sparsepp/spp.h>
//
SHKZ_USING_NAMESPACE
//
template <class T> using hash_type = spp::sparse_hash_set<T>;
//
std::vector<size_t> dilate2::dilate( const shape2 &shape, unsigned char *bit_mask, size_t bit_mask_size, const parallel_driver *parallel ) {
	//
	const size_t nx(shape.w), ny(shape.h);
	const size_t size = nx*ny;
	const size_t thread_size = parallel ? parallel->get_thread_num() : 1;
	std::vector<std::vector<size_t> > dilate_coords(thread_size);
	//
	// Do upward
	parallel_active_loop::run(size,bit_mask,bit_mask_size,[&]( size_t n, bool &active, int q ) {
		if( n < size-nx ) {
			n += nx;
			const unsigned char &mask = *(bit_mask+n/8);
			if( mask == 0xFF ) {
				return true;
			} else if( ! ((mask >> n%8) & 1U)) {
				dilate_coords[q].push_back(n);
			}
		}
		return false;
	},parallel);
	//
	// Do downward
	parallel_active_loop::run(size,bit_mask,bit_mask_size,[&]( size_t n, bool &active, int q ) {
		if( n >= nx ) {
			n -= nx;
			const unsigned char &mask = *(bit_mask+n/8);
			if( mask == 0xFF ) {
				return true;
			} else if( ! ((mask >> n%8) & 1U)) {
				dilate_coords[q].push_back(n);
			}
		}
		return false;
	},parallel);
	//
	// Do left edge
	auto left_edge_do = [&]( size_t n8, int q ) {
		if( (*(bit_mask+n8) & 1U ) && n8 > 0 && 8*n8 % nx ) {
			const unsigned char &mask = *(bit_mask+n8-1);
			if( ! mask ) {
				dilate_coords[q].push_back(8*n8-1);
			} else {
				bool active_right = (mask >> 7) & 1U;
				if( ! active_right ) {
					dilate_coords[q].push_back(8*n8-1);
				}
			}
		}
	};
	if( parallel ) {
		parallel->for_each(bit_mask_size,[&]( size_t n8, int q ) {
			left_edge_do(n8,q);
		});
	} else {
		for( size_t n8=0; n8<bit_mask_size; ++n8 ) {
			left_edge_do(n8,0);
		}
	}
	//
	// Do right edge
	auto right_edge_do = [&]( size_t n8, int q ) {
		if( ((*(bit_mask+n8) >> 7) & 1U ) && n8 < bit_mask_size-1 && 8*(n8+1) % nx ) {
			const unsigned char &mask = *(bit_mask+n8+1);
			if( ! mask ) {
				dilate_coords[q].push_back(8*(n8+1));
			} else {
				bool active_left = mask & 1U;
				if( ! active_left ) {
					dilate_coords[q].push_back(8*(n8+1));
				}
			}
		}
	};
	if( parallel ) {
		parallel->for_each(bit_mask_size,[&]( size_t n8, int q ) {
			right_edge_do(n8,q);
		});
	} else {
		for( size_t n8=0; n8<bit_mask_size; ++n8 ) {
			right_edge_do(n8,0);
		}
	}
	//
	// Do internal
	auto do_internal = [&]( size_t n8, int q ) {
		const unsigned char &mask = *(bit_mask+n8);
		if( mask && mask != 0xFF ) {
			unsigned char dilate_mask;
			size_t r = nx - (8*n8) % nx;
			if( r < 8 ) {
				dilate_mask = ~mask & (((mask << 1) & ~(0xFF << r)) | ((mask & (0xFF << r)) << 1) | ((mask >> 1) & (0xFF << r )) | ((mask & ~(0xFF << r)) >> 1));
			} else {
				dilate_mask = (~mask) & ((mask >> 1) | (mask << 1));
			}
			for( size_t n0=0; n0<8; ++n0 ) {
				if( (dilate_mask >> n0 ) & 1U ) {
					dilate_coords[q].push_back(8*n8+n0);
				}
			}
		}
	};
	if( parallel ) {
		parallel->for_each(bit_mask_size,[&]( size_t n8, int q ) {
			do_internal(n8,q);
		});
	} else {
		for( size_t n8=0; n8<bit_mask_size; ++n8 ) {
			do_internal(n8,0);
		}
	}
	//
	hash_type<size_t> assembled;
	for( const auto &e : dilate_coords ) {
		for( const auto &it : e ) { assembled.insert(it); }
	}
	std::vector<size_t> result;
	result.assign(assembled.begin(),assembled.end());
	return result;
}
//
std::vector<size_t> dilate2::dilate( const shape2 &shape, const ordering_core *ordering, const void *context, unsigned char *bit_mask, size_t bit_mask_size, const parallel_driver *parallel ) {
	//
	const size_t nx(shape.w), ny(shape.h);
	const size_t thread_size = parallel ? parallel->get_thread_num() : 1;
	//
	auto encoder = ordering->get_encoder_func2(context);
	auto decoders = ordering->get_decoder_func2(context);
	std::vector<std::vector<size_t> > dilate_coords(thread_size);
	//
	const vec2i query[] = {vec2i(+1,0),vec2i(-1,0),vec2i(0,+1),vec2i(0,-1)};
	for( int nq=0; nq<4; nq++ ) {
		parallel_active_loop::run(shape.count(),bit_mask,bit_mask_size,[&]( size_t n, bool &active, int q ) {
			const unsigned char &mask = *(bit_mask+n/8);
			if( ((mask >> n%8) & 1U)) {
				int i, j;
				ordering->decode(decoders,context,n,i,j);
				i += query[nq][0]; j += query[nq][1];
				if( ! shape.out_of_bounds(i,j)) {
					size_t m = encoder(context,i,j);
					const unsigned char &qmask = *(bit_mask+m/8);
					if( ! ((qmask >> m%8) & 1U)) {
						dilate_coords[q].push_back(m);
					}
				}
			}
			return false;
		},parallel);
	}
	//
	hash_type<size_t> assembled;
	for( const auto &e : dilate_coords ) {
		for( const auto &it : e ) { assembled.insert(it); }
	}
	std::vector<size_t> result;
	result.assign(assembled.begin(),assembled.end());
	return result;
}
//