/*
**	parallel_active_loop.h
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
#ifndef SHKZ_PARALLEL_ACTIVE_LOOP_H
#define SHKZ_PARALLEL_ACTIVE_LOOP_H
//
#include <shiokaze/parallel/parallel_driver.h>
#include <cstdint>
#include <limits>
#include <cmath>
//
SHKZ_BEGIN_NAMESPACE
//
class parallel_active_loop {
public:
	//
	static void run( size_t size, unsigned char *bit_mask, size_t bit_mask_size, std::function<bool( size_t n, bool &active, int thread_index )> body, const parallel_driver *parallel=nullptr ) {
		size_t bit_mask_size_64 = std::ceil(bit_mask_size / 8.0);
		//
		auto inner_body = [&]( size_t n64, int q ) {
		uint64_t mask64 = (n64+1)*8 < bit_mask_size ? *(uint64_t *)(bit_mask+n64*8) : std::numeric_limits<uint64_t>::max();
			if( mask64 ) {
				for( size_t nq=0; nq<8; ++nq ) {
					size_t n8 = n64*8+nq;
					uint64_t mask = *(bit_mask+n8);
					for( size_t n0=0; n0<8; ++n0 ) {
						size_t n = 8*n8+n0;
						if( n < size ) {
							if( (mask >> n0) & 1U ) {
								bool active (true);
								bool do_break (false);
								if(body(n,active,q)) {
									do_break = true;
								}
								mask &= ~(1UL << n0);
								mask64 &= ~(1ULL << (8*nq+n0));
								if( ! active ) *(bit_mask+n8) &= ~(1UL << n0);
								if( do_break || ! mask ) break;
							}
						}
					}
					if( ! mask64 ) break;
				}
			}
		};
		if( parallel ) {
			parallel->for_each(bit_mask_size_64,[&]( size_t n64, int q ) {
				inner_body(n64,q);
			});
		} else {
			for( size_t n64=0; n64<bit_mask_size_64; ++n64 ) {
				inner_body(n64,0);
			}
		}
	};
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
//