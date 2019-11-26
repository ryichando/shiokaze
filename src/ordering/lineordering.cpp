/*
**	lineordering.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 1, 2017.
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
#ifndef SHKZ_LINEORDERING_H
#define SHKZ_LINEORDERING_H
//
#include <shiokaze/ordering/ordering_core.h>
//
SHKZ_BEGIN_NAMESPACE
//
class lineordering : public ordering_core {
protected:
	//
	LONG_NAME("Line Ordering Encoder/Decoder")
	//
	virtual const void* new_context( const shape2& shape ) const override {
		context *cx = new context;
		cx->nx = shape.w;
		cx->ny = shape.h;
		cx->nz = 0;
		cx->plane = 0;
		return cx;
	}
	//
	virtual const void* new_context( const shape3& shape ) const override {
		context *cx = new context;
		cx->nx = shape.w;
		cx->ny = shape.h;
		cx->nz = shape.d;
		cx->plane = shape.w * shape.h;
		return cx;
	}
	//
	virtual void delete_context( const void *ptr ) const override {
		delete reinterpret_cast<const context *>(ptr);
	}
	//
	virtual std::function<size_t(const void *context_ptr, int i, int j)> get_encoder_func2( const void *context_ptr ) const override {
		return [](const void *context_ptr, int i, int j) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			return i+j*cx->nx;
			/* ------------------------------------------------------------------ */
		};
	}
	//
	virtual std::function<size_t(const void *context_ptr, int i, int j, int k)> get_encoder_func3( const void *context_ptr ) const override {
		return [](const void *context_ptr, int i, int j, int k) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			return i+j*cx->nx+k*cx->plane;
			/* ------------------------------------------------------------------ */
		};
	}
	//
	virtual std::vector<decoder_func2> get_decoder_func2( const void *context_ptr ) const override {
		const context *cx = reinterpret_cast<const context *>(context_ptr);
		//
		decoder_func2 transform_func;
		transform_func.func = [](const void *context_ptr, size_t n, int &i, int &j) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			i = n % cx->nx;
			j = n / cx->nx;
			/* ------------------------------------------------------------------ */
		};
		transform_func.range[0] = 0;
		transform_func.range[1] = cx->nx * cx->ny;
		//
		std::vector<decoder_func2> result;
		result.push_back(transform_func);
		return result;
	}
	virtual std::vector<decoder_func3> get_decoder_func3( const void *context_ptr ) const override {
		const context *cx = reinterpret_cast<const context *>(context_ptr);
		//
		decoder_func3 transform_func;
		transform_func.func = [](const void *context_ptr, size_t n, int &i, int &j, int &k) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			const size_t &plane = cx->plane;
			size_t plane_idx = n % plane;
			i = plane_idx % cx->nx;
			j = plane_idx / cx->nx;
			k = n / plane;
			/* ------------------------------------------------------------------ */
		};
		transform_func.range[0] = 0;
		transform_func.range[1] = cx->nx * cx->ny * cx->nz;
		//
		std::vector<decoder_func3> result;
		result.push_back(transform_func);
		return result;
	}
	//
private:
	//
	struct context {
		int nx, ny, nz;
		size_t plane;
	};
};
//
extern "C" module * create_instance() {
	return new lineordering();
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