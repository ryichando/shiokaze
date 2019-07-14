/*
**	bitcount.cpp
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
#include <shiokaze/core/common.h>
#include <shiokaze/parallel/parallel_driver.h>
#include "bitcount.h"
//
SHKZ_USING_NAMESPACE
//
static inline uint64_t popcnt64(uint64_t x) {
	uint64_t m1 = 0x5555555555555555ll;
	uint64_t m2 = 0x3333333333333333ll;
	uint64_t m4 = 0x0F0F0F0F0F0F0F0Fll;
	uint64_t h01 = 0x0101010101010101ll;
	x -= (x >> 1) & m1;
	x = (x & m2) + ((x >> 2) & m2);
	x = (x + (x >> 4)) & m4;
	return (x * h01) >> 56;
}
//
static inline uint64_t popcnt64_unrolled(const uint64_t* data, uint64_t size) {
	uint64_t i = 0;
	uint64_t limit = size - size % 4;
	uint64_t cnt = 0;
	for (; i < limit; i += 4) {
		cnt += popcnt64(data[i+0]);
		cnt += popcnt64(data[i+1]);
		cnt += popcnt64(data[i+2]);
		cnt += popcnt64(data[i+3]);
	}
	for (; i < size; i++) cnt += popcnt64(data[i]);
	return cnt;
}
//
static inline void align_8(const uint8_t** p, uint64_t* size, uint64_t* cnt) {
	for (; *size > 0 && (uintptr_t) *p % 8; (*p)++) {
		*cnt += popcnt64(**p);
		*size -= 1;
	}
}
//
static inline uint64_t popcnt(const void* data, uint64_t size) {
	const uint8_t* ptr = (const uint8_t*) data;
	uint64_t cnt = 0;
	uint64_t i;
	align_8(&ptr, &size, &cnt);
	cnt += popcnt64_unrolled((const uint64_t*) ptr, size / 8);
	ptr += size - size % 8;
	size = size % 8;
	for (i = 0; i < size; i++) cnt += popcnt64(ptr[i]);
	return cnt;
}
//
size_t bitcount::count( const unsigned char *bit_mask, size_t bit_mask_size, const parallel_driver *parallel ) {
	//
	if( parallel ) {
		using T = uint64_t;
		size_t num_threads = parallel->get_thread_num();
		std::vector<size_t> total_slots(num_threads);
		size_t chunk_size = bit_mask_size / num_threads;
		size_t end_n (0);
		//
		if( num_threads > 1 && bit_mask_size > num_threads * sizeof(T)) {
			parallel->for_each(num_threads,[&](size_t n, int q) {
				size_t start (0), end (0), m (0);
				const size_t terminal (chunk_size-sizeof(T));
				while( m < terminal ) {
					while( m < terminal ) {
						if( *(T *)(bit_mask+m) ) {
							start = m;
							break;
						} else {
							m += sizeof(T);
						}
					}
					end = chunk_size;
					while( m < terminal ) {
						m += sizeof(T);
						if(! *(T *)(bit_mask+m) ) {
							end = m;
							break;
						}
					}
					total_slots[n] += popcnt(bit_mask+n*chunk_size+start,end);
				}
			});
			end_n = chunk_size * num_threads;
		}
		size_t total (0);
		if( end_n < bit_mask_size ) {
			total += popcnt(bit_mask+end_n,bit_mask_size-end_n);
		}
		for( const auto &e : total_slots ) total += e;
		return total;
	} else {
		return popcnt(bit_mask,bit_mask_size);
	}
}
//
