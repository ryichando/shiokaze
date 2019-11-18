/*
**	treearray3-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 2, 2018.
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
#include <shiokaze/core/runnable.h>
#include <shiokaze/core/console.h>
#include <shiokaze/array/array3.h>
//
SHKZ_USING_NAMESPACE
//
class treearray3_example : public runnable {
private:
	//
	LONG_NAME("Tree Array 3D")
	ARGUMENT_NAME("TreeArrayExample")
	//
	virtual void run_onetime() override {
		//
		console::dump( "Running example...\n");
		//
		m_array.initialize(shape3(2147483647,2147483647,2147483647));
		//
		for( int i=0; i<100; ++i ) for( int j=0; j<100; ++j ) for( int k=0; k<100; ++k ) {
			m_array.set(2147483647-1-i,143792334-1-j,143792334-1-k,i+j+k+2.0);
		}
		//
		Real error (0.0);
		for( int i=0; i<100; ++i ) for( int j=0; j<100; ++j ) for( int k=0; k<100; ++k ) {
			error = std::max(error,(Real)std::abs(m_array(2147483647-1-i,143792334-1-j,143792334-1-k)-(i+j+k+2.0)));
		}
		console::dump( "error = %f, count = %u\n", error, m_array.count());
		//
		m_array.const_parallel_actives([&]( int i, int j, int k, const auto &it ) {
			console::dump( "value(%d,%d,%d) = %f\n", i, j, k, it());
		});
		//
		m_array.parallel_actives([&]( int i, int j, int k, auto &it ) {
			it.set_off();
		});
		//
		console::dump( "new count = %u\n", m_array.count());
	}
	//
	array3<Real> m_array {this,"TargetArray:treearray3"};
};
//
extern "C" module * create_instance() {
	return new treearray3_example;
}
//
extern "C" const char *license() {
	return "MIT";
}
