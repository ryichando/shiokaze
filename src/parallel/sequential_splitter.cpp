/*
**	sequential_splitter.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 8, 2018.
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
#ifndef SHKZ_SEQUENCE_SPLITTER_H
#define SHKZ_SEQUENCE_SPLITTER_H
//
#include <shiokaze/parallel/loop_splitter.h>
#include <cmath>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
class sequential_splitter : public loop_splitter {
protected:
	//
	LONG_NAME("Sequential Loop Splitter")
	MODULE_NAME("sequential_splitter")
	//
	virtual const void* new_context ( size_t size, int num_threads ) const override {
		//
		assert( num_threads <= size );
		context *cx = new context;
		size_t chunk_size = std::ceil(size/(double)num_threads);
		cx->start.resize(num_threads);
		cx->end.resize(num_threads);
		if( num_threads*chunk_size != size ) {
			// * * * * * * * * * * * * * * * *
			//
			// C ... Chunk size
			// n ... num threads of size C
			// m ... num threads of size C-1
			// T ... total threads count
			// S ... Target size
			//
			// Derivation:
			//
			// Cn + (C-1)m = S 				(1)
			// n + m = T 					(2)
			//
			// Cn + CT - Cn - T + n = S 	(3)
			// CT - T + n = S 				(4)
			// n = S + T - CT 				(5)
			// n = S + T(1-C)
			//	 = S + T - TC 				(6)
			// m = T - n 
			//	 = T - (S + T(1-C))
			//	 = T - S - T(1-C)
			//	 = T - S - T + TC
			//	 = TC - S 					(7)
			//
			// * * * * * * * * * * * * * * * *
			size_t end_n = size+num_threads-num_threads*chunk_size;
			for( size_t n=0; n<end_n; ++n ) {
				cx->start[n] = n*chunk_size;
				cx->end[n] = cx->start[n] + chunk_size;
				assert( cx->end[n] <= size );
			}
			size_t end_m = num_threads*chunk_size-size;
			for( size_t m=0; m<end_m; ++m ) {
				size_t k = m+end_n;
				cx->start[k] = cx->end[k-1];
				cx->end[k] = cx->start[k] + chunk_size-1;
				assert( cx->end[k] <= size );
			}
			//
		} else {
			for( size_t n=0; n<num_threads; ++n ) {
				cx->start[n] = n*chunk_size;
				cx->end[n] = cx->start[n] + chunk_size;
			}
		}
		assert( cx->end[num_threads-1] == size );
		return cx;
	}
	virtual std::function<size_t(const void *context_ptr, int thread_index)> get_start_func(const void *context_ptr) const override {
		/* --------------------------------------------- */
		return [&](const void *context_ptr, int thread_index) {
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			return cx->start[thread_index];
		};
		/* --------------------------------------------- */
	}
	virtual std::function<bool(const void *context_ptr, size_t &n, int thread_index)> get_advance_func(const void *context_ptr) const override {
		/* --------------------------------------------- */
		return [&](const void *context_ptr, size_t &n, int thread_index) {
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			++ n; return n < cx->end[thread_index];
		};
		/* --------------------------------------------- */
	}
	virtual void delete_context( const void *context_ptr ) const override {
		delete reinterpret_cast<const context *>(context_ptr);
	}
	//
	struct context {
		std::vector<size_t> start;
		std::vector<size_t> end;
	};
};
//
extern "C" module * create_instance() {
	return new sequential_splitter();
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