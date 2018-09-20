/*
**	dispersed_splitter.cpp
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
#ifndef SHKZ_DISPERSED_SPLITTER_H
#define SHKZ_DISPERSED_SPLITTER_H
//
#include <shiokaze/parallel/loop_splitter.h>
//
SHKZ_BEGIN_NAMESPACE
//
class dispersed_splitter : public loop_splitter {
public:
	//
	LONG_NAME("Dispersed Loop Splitter")
	//
	virtual const void* new_context ( size_t size, int num_threads ) const override {
		assert( num_threads <= size );
		context *cx = new context;
		cx->size = size;
		cx->num_threads = num_threads;
		return cx;
	}
	virtual std::function<size_t(const void *context_ptr, int thread_index)> get_start_func(const void *context_ptr) const override {
		/* --------------------------------------------- */
		return [&](const void *context_ptr, int thread_index) {
			return thread_index;
		};
		/* --------------------------------------------- */
	}
	virtual std::function<bool(const void *context_ptr, size_t &n, int thread_index)> get_advance_func(const void *context_ptr) const override {
		/* --------------------------------------------- */
		return [&](const void *context_ptr, size_t &n, int thread_index) {
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			n += cx->num_threads; return n < cx->size;
		};
		/* --------------------------------------------- */
	}
	virtual void delete_context( const void *context_ptr ) const override {
		delete reinterpret_cast<const context *>(context_ptr);
	}
private:
	struct context {
		size_t size;
		int num_threads;
	};
};
//
extern "C" module * create_instance() {
	return new dispersed_splitter();
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