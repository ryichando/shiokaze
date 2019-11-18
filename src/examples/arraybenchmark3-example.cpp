/*
**	arraybenchmark3-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 13, 2019.
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
#include <shiokaze/array/array3.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/console.h>
#include <cmath>
#include <thread>
//
SHKZ_USING_NAMESPACE
//
class arraybenchmark3 : public runnable {
private:
	//
	LONG_NAME("Array Benchmark 3D")
	ARGUMENT_NAME("ArrayBenchmarkExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
		config.get_unsigned("ResolutionZ",m_shape[2],"Resolution towards Z axis");
		//
		double resolution_scale (1.0);
		config.get_double("ResolutionScale",resolution_scale,"Resolution doubling scale");
		//
		m_shape *= resolution_scale;
		m_dx = m_shape.dx();
	}
	//
	virtual void run_onetime() override {
		//
		scoped_timer timer(this);
		//
		timer.tick(); console::dump( "Performing initialization..." );
		m_array.initialize(m_shape);
		m_array.set_as_levelset(2.0*m_dx);
		console::dump( "Done. Took %s\n", timer.stock("initialization").c_str());
		//
		auto levelset_func = [&]( const vec3d &p ) {
			double r (0.225);
			double w (0.25);
			vec3r center(0.5,0.5,0.5);
			return (p-center).len()-r;
		};
		//
		timer.tick(); console::dump( "Performing parallel_all..." );
		m_array.parallel_all([&](int i, int j, int k, auto &it) {
			double d = levelset_func(m_dx*vec3i(i,j,k).cell());
			if( std::abs(d) < 2.0*m_dx) it.set(d);
			else it.set_off();
		});
		console::dump( "Done. Took %s\n", timer.stock("parallel_all").c_str());
		//
		timer.tick(); console::dump( "Performing dilation..." );
		m_array.dilate(5);
		console::dump( "Done. Took %s\n", timer.stock("dilation").c_str());
		//
		timer.tick(); console::dump( "Performing serial_all..." );
		m_array.serial_all([&](int i, int j, int k, auto &it) {
			it();
		});
		console::dump( "Done. Took %s\n", timer.stock("serial_all").c_str());
		//
		timer.tick(); console::dump( "Performing flood fill..." );
		m_array.flood_fill();
		console::dump( "Done. Took %s\n", timer.stock("flood_fill").c_str());
		//
		timer.tick(); console::dump( "Performing paralle_actives..." );
		m_array.parallel_actives([&](int i, int j, int k, auto &it) {
			it.set(2.0 * it());
		});
		console::dump( "Done. Took %s\n", timer.stock("parallel_actives").c_str());
		//
		timer.tick(); console::dump( "Performing const_parallel_inside..." );
		m_array.const_parallel_inside([&](int i, int j, int k, const auto &it) {
			it();
		});
		console::dump( "Done. Took %s\n", timer.stock("const_parallel_inside").c_str());
		//
		timer.tick(); console::dump( "Performing serial_actives..." );
		m_array.serial_actives([&](int i, int j, int k, auto &it) {
			it.set(2.0 * it());
		});
		console::dump( "Done. Took %s\n", timer.stock("serial_actives").c_str());
		//
		timer.tick(); console::dump( "Performing parallel read..." );
		m_parallel.for_each(m_array.shape(),[&]( int i, int j, int k, int tid ) {
			m_array(i,j,k);
		});
		console::dump( "Done. Took %s\n", timer.stock("parallel_read").c_str());
		//
		timer.tick(); console::dump( "Performing sequential read..." );
		m_array.shape().for_each([&]( int i, int j, int k ) {
			m_array(i,j,k);
		});
		console::dump( "Done. Took %s\n", timer.stock("sequential_read").c_str());
		//
		timer.tick(); console::dump( "Performing sequntial write..." );
		m_shape.for_each([&]( int i, int j, int k ) {
			m_array.set(i,j,k,i+j+k);
		});
		console::dump( "Done. Took %s\n", timer.stock("sequntial_write").c_str());
	}
	//
	array3<Real> m_array {this,"treearray3"};
	parallel_driver m_parallel{this};
	shape3 m_shape {256,256,256};
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new arraybenchmark3;
}
//
extern "C" const char *license() {
	return "MIT";
}