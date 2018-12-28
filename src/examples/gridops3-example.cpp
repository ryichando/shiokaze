/*
**	gridops3-example.cpp
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
#include <shiokaze/ui/drawable.h>
#include <shiokaze/core/console.h>
#include <shiokaze/array/shared_array3.h>
//
SHKZ_USING_NAMESPACE
//
class gridops3 : public runnable {
private:
	//
	LONG_NAME("Grid Operations 3D")
	ARGUMENT_NAME("GridOpsExample")
	//
	virtual void run_onetime() override {
		//
		shape3 shape {81,62,42};
		//
		shared_array3<double> array0(shape);
		shared_array3<double> array1(shape);
		//
		array0() = 4.0;
		array1() = 3.0;
		array0() -= array1();
	}
	//
};
//
extern "C" module * create_instance() {
	return new gridops3;
}
//
extern "C" const char *license() {
	return "MIT";
}
