/*
**	stdthread.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 5, 2018.
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
#ifndef SHKZ_STDTHREAD_H
#define SHKZ_STDTHREAD_H
//
#include <shiokaze/parallel/parallel_core.h>
#include <vector>
#include <thread>
//
SHKZ_BEGIN_NAMESPACE
//
class stdthread : public parallel_core {
public:
	//
	LONG_NAME("STD Thread")
	//
	virtual void for_each(
		std::function<void(size_t n, int thread_index)> func,
		std::function<size_t(int thread_index)> iterator_start,
		std::function<bool(size_t &n, int thread_index)> iterator_advance,
		int num_threads ) const override {
		//
		assert( num_threads );
		std::vector<std::thread> threads(num_threads);
		for( int q=0; q<num_threads; ++q ) {
			threads[q] = std::thread([&]( int q ) {
				size_t n = iterator_start(q);
				do { func(n,q); } while( iterator_advance(n,q));
			},q);
		}
		for( auto& thread : threads ) thread.join();
	}
	//
	virtual void run( const std::vector<std::function<void()> > &functions ) const override {
		//
		std::vector<std::thread> threads(functions.size());
		for( int q=0; q<threads.size(); ++q ) {
			threads[q] = std::thread([&](int q) { functions[q](); },q);
		}
		for( auto& thread : threads ) thread.join();
	}
};
//
extern "C" module * create_instance() {
	return new stdthread();
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