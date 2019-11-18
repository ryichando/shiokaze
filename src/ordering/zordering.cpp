/*
**	zordering.cpp
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
#ifndef SHKZ_ZORDERING_H
#define SHKZ_ZORDERING_H
//
#include <shiokaze/ordering/ordering_core.h>
#include <cmath>
#include "libmorton/include/morton.h"
//
SHKZ_BEGIN_NAMESPACE
//
class zordering : public ordering_core {
protected:
	//
	LONG_NAME("Z-Curve Ordering Encoder/Decoder")
	//
	static int floor_power_of_two ( int x ) {
		return std::floor(std::log2(x));
	}
	//
	virtual const void* new_context( const shape2& shape ) const override {
		context *cx = new context;
		cx->nx = shape.w;
		cx->ny = shape.h;
		cx->n = floor_power_of_two(std::min(cx->nx,cx->ny));
		cx->width = 1UL << cx->n;
		cx->max_zn = cx->width * cx->width;
		cx->odd_w = cx->nx - cx->width;
		cx->odd_ww = cx->odd_w * cx->width;
		return cx;
	}
	//
	virtual const void* new_context( const shape3& shape ) const override {
		context *cx = new context;
		cx->nx = shape.w;
		cx->ny = shape.h;
		cx->nz = shape.d;
		cx->n = floor_power_of_two(std::min(std::min(cx->nx,cx->ny),cx->nz));
		cx->width = 1UL << cx->n;
		cx->max_zn = cx->width * cx->width * cx->width;
		cx->plane = cx->width * cx->width;
		cx->odd_plane = cx->nx * cx->ny - cx->plane;
		cx->odd_plane_w = cx->odd_plane * cx->width;
		cx->odd_w = cx->nx - cx->width;
		cx->odd_ww = cx->odd_w * cx->width;
		cx->full_plane = cx->nx * cx->ny;
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
			if( i < cx->width && j < cx->width ) {
				return (size_t) morton2D_32_encode(i,j);
			} else {
				if( j < cx->width ) {
					return cx->max_zn + j * cx->odd_w + (i - cx->width);
				} else {
					return (size_t) i + j * cx->nx;
				}
			}
			/* ------------------------------------------------------------------ */
		};
	}
	//
	virtual std::function<size_t(const void *context_ptr, int i, int j, int k)> get_encoder_func3( const void *context_ptr ) const override {
		return [](const void *context_ptr, int i, int j, int k) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			if( i < cx->width && j < cx->width && k < cx->width ) {
				return (size_t) morton3D_32_encode(i,j,k);
			} else {
				if( k < cx->width ) {
					size_t n = cx->max_zn + cx->odd_plane * k;
					if( j < cx->width ) {
						return n + j * cx->odd_w + (i - cx->width);
					} else {
						return n + cx->odd_ww + (j - cx->width) * cx->nx + i;
					}
				} else {
					return k * cx->full_plane + j * cx->nx + i;
				}
			}
			/* ------------------------------------------------------------------ */
		};
	}
	//
	virtual std::vector<decoder_func2> get_decoder_func2( const void *context_ptr ) const override {
		const context *cx = reinterpret_cast<const context *>(context_ptr);
		std::vector<decoder_func2> result;
		//
		decoder_func2 transform_func_0;
		transform_func_0.func = [](const void *context_ptr, size_t n, int &i, int &j) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			uint_fast16_t x, y;
			morton2D_32_decode(n,x,y); i = x; j = y;
			/* ------------------------------------------------------------------ */
		};
		transform_func_0.range[0] = 0;
		transform_func_0.range[1] = cx->max_zn;
		result.push_back(transform_func_0);
		//
		size_t end_n = cx->nx * cx->ny;
		if( cx->max_zn < end_n ) {
			decoder_func2 transform_func_1;
			transform_func_1.func = [](const void *context_ptr, size_t n, int &i, int &j) {
				/* ------------------------------------------------------------------ */
				const context *cx = reinterpret_cast<const context *>(context_ptr);
				n -= cx->max_zn;
				i = cx->width + n % cx->odd_w;
				j = n / cx->odd_w;
				/* ------------------------------------------------------------------ */
			};
			transform_func_1.range[0] = transform_func_0.range[1];
			transform_func_1.range[1] = cx->max_zn + cx->odd_ww;
			result.push_back(transform_func_1);
			//
			if ( transform_func_1.range[1] < end_n ) {
				decoder_func2 transform_func_2;
				transform_func_2.func = [](const void *context_ptr, size_t n, int &i, int &j) {
					/* ------------------------------------------------------------------ */
					const context *cx = reinterpret_cast<const context *>(context_ptr);
					n -= (cx->max_zn + cx->odd_ww);
					i = n % cx->nx;
					j = cx->width + n / cx->nx;
					/* ------------------------------------------------------------------ */
				};
				transform_func_2.range[0] = transform_func_1.range[1];
				transform_func_2.range[1] = end_n;
				result.push_back(transform_func_2);
			}
		}
		return result;
	}
	//
	virtual std::vector<decoder_func3> get_decoder_func3( const void *context_ptr ) const override {
		const context *cx = reinterpret_cast<const context *>(context_ptr);
		std::vector<decoder_func3> result;
		//
		decoder_func3 transform_func_0;
		transform_func_0.func = [](const void *context_ptr, size_t n, int &i, int &j, int &k) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			uint_fast16_t x, y, z;
			morton3D_32_decode(n,x,y,z); i = x; j = y; k = z;
			/* ------------------------------------------------------------------ */
		};
		transform_func_0.range[0] = 0;
		transform_func_0.range[1] = cx->max_zn;
		result.push_back(transform_func_0);
		//
		size_t end_n = cx->nx * cx->ny * cx->nz;
		if( cx->max_zn < end_n ) {
			decoder_func3 transform_func_1;
			transform_func_1.func = [](const void *context_ptr, size_t n, int &i, int &j, int &k) {
				/* ------------------------------------------------------------------ */
				const context *cx = reinterpret_cast<const context *>(context_ptr);
				n -= cx->max_zn;
				k = n / cx->odd_plane;
				size_t n_xy = n % cx->odd_plane;
				if( n_xy < cx->odd_ww ) {
					i = cx->width + n_xy % cx->odd_w;
					j = n_xy / cx->odd_w;
				} else {
					n_xy -= cx->odd_ww;
					i = n_xy % cx->nx;
					j = cx->width + n_xy / cx->nx;
				}
				/* ------------------------------------------------------------------ */
			};
			transform_func_1.range[0] = transform_func_0.range[1];
			transform_func_1.range[1] = cx->max_zn + cx->odd_plane_w;
			result.push_back(transform_func_1);
			//
			if( transform_func_1.range[1] < end_n ) {
				decoder_func3 transform_func_2;
				transform_func_2.func = [](const void *context_ptr, size_t n, int &i, int &j, int &k) {
					/* ------------------------------------------------------------------ */
					const context *cx = reinterpret_cast<const context *>(context_ptr);
					n -= (cx->max_zn + cx->odd_plane_w);
					i = (n % cx->full_plane) % cx->nx;
					j = (n % cx->full_plane) / cx->nx;
					k = cx->width + n / cx->full_plane;
					/* ------------------------------------------------------------------ */
				};
				transform_func_2.range[0] = transform_func_1.range[1];
				transform_func_2.range[1] = end_n;
				result.push_back(transform_func_2);
			}
		}
		//
		return result;
	}
	//
private:
	//
	struct context {
		int nx, ny, nz, n, width;
		size_t max_zn;
		size_t odd_plane, odd_plane_w, odd_w, odd_ww, plane, full_plane;
	};
};
//
extern "C" module * create_instance() {
	return new zordering();
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