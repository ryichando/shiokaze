/*
**	blockordering.cpp
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
#ifndef SHKZ_BLOCKORDERING_H
#define SHKZ_BLOCKORDERING_H
//
#include <shiokaze/ordering/ordering_core.h>
#include "libmorton/include/morton.h"
//
SHKZ_BEGIN_NAMESPACE
//
class blockordering : public ordering_core {
public:
	//
	LONG_NAME("Block Ordering Encoder/Decoder")
	MODULE_NAME("blockordering")
	//
	blockordering() {
		block_size = 8;
		use_zordering = true;
	}
	//
protected:
	//
	static inline bool is_power_of_two( unsigned n ) {
		// http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
		return (n & (n - 1)) == 0;
	}
	//
	virtual const void* new_context( const shape2& shape ) const override {
		context *cx = new context;
		cx->nx = shape.w;
		cx->ny = shape.h;
		cx->nz = 0;
		cx->block_size = block_size;
		cx->chunk_x = cx->nx * cx->block_size;
		cx->chunk_xy = 0;
		cx->use_zordering = use_zordering;
		return cx;
	}
	//
	virtual const void* new_context( const shape3& shape ) const override {
		context *cx = new context;
		cx->nx = shape.w;
		cx->ny = shape.h;
		cx->nz = shape.d;
		cx->block_size = block_size;
		cx->chunk_x = cx->nx * cx->block_size;
		cx->chunk_xy = cx->nx * cx->ny * cx->block_size;
		cx->use_zordering = use_zordering;
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
			int bi = i / cx->block_size;
			int bj = j / cx->block_size;
			int ii = i - bi * cx->block_size;
			int jj = j - bj * cx->block_size;
			int bw = std::min( cx->block_size, cx->nx - bi * cx->block_size );
			int bh = std::min( cx->block_size, cx->ny - bj * cx->block_size );
			size_t bn = bj * cx->chunk_x + bh * cx->block_size * bi;
			if( cx->use_zordering && bw == bh && is_power_of_two(bw)) {
				return bn + morton2D_32_encode(ii,jj);
			} else {
				return bn + bw * jj + ii;
			}
			/* ------------------------------------------------------------------ */
		};
	}
	//
	virtual std::function<size_t(const void *context_ptr, int i, int j, int k)> get_encoder_func3( const void *context_ptr ) const override {
		return [](const void *context_ptr, int i, int j, int k) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			int bi = i / cx->block_size;
			int bj = j / cx->block_size;
			int bk = k / cx->block_size;
			int ii = i - bi * cx->block_size;
			int jj = j - bj * cx->block_size;
			int kk = k - bk * cx->block_size;
			int bw = std::min( cx->block_size, cx->nx - bi * cx->block_size );
			int bh = std::min( cx->block_size, cx->ny - bj * cx->block_size );
			int bd = std::min( cx->block_size, cx->nz - bk * cx->block_size );
			size_t bn = bk * cx->chunk_xy + bj * cx->chunk_x * bd + bd * bh * bi * cx->block_size;
			if( cx->use_zordering && bw == bh && bh == bd && is_power_of_two(bw)) {
				return bn + morton3D_32_encode(ii,jj,kk);
			} else {
				return bn + (bw*bh) * kk + bw * jj + ii;
			}
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
			//
			unsigned block_j = n / cx->chunk_x;
			unsigned block_height = std::min( (unsigned) cx->block_size, (unsigned) cx->ny - block_j * cx->block_size );
			unsigned block_i = (n % cx->chunk_x) / (block_height * cx->block_size);
			unsigned block_width = std::min( (unsigned) cx->block_size, (unsigned) cx->nx - block_i * cx->block_size );
			unsigned odd = n - block_j * cx->chunk_x - block_height * cx->block_size * block_i;
			//
			if( cx->use_zordering && block_width == block_height && is_power_of_two(block_width)) {
				uint_fast16_t x, y;
				morton2D_32_decode(odd,x,y);
				i = block_i * cx->block_size + x;
				j = block_j * cx->block_size + y;
			} else {
				i = block_i * cx->block_size + odd % block_width;
				j = block_j * cx->block_size + odd / block_width;
			}
			/* ------------------------------------------------------------------ */
		};
		transform_func.range[0] = 0;
		transform_func.range[1] = cx->nx * cx->ny;
		//
		std::vector<decoder_func2> result;
		result.push_back(transform_func);
		return result;
	}
	//
	virtual std::vector<decoder_func3> get_decoder_func3( const void *context_ptr ) const override {
		const context *cx = reinterpret_cast<const context *>(context_ptr);
		//
		decoder_func3 transform_func;
		transform_func.func = [](const void *context_ptr, size_t n, int &i, int &j, int &k) {
			/* ------------------------------------------------------------------ */
			const context *cx = reinterpret_cast<const context *>(context_ptr);
			//
			unsigned block_k = n / cx->chunk_xy;
			unsigned block_depth = std::min( (unsigned) cx->block_size, (unsigned) cx->nz - block_k * cx->block_size );
			unsigned block_j = (n % cx->chunk_xy) / (block_depth * cx->chunk_x);
			unsigned block_height = std::min( (unsigned) cx->block_size, (unsigned) cx->ny - block_j * cx->block_size );
			unsigned block_i = ((n % cx->chunk_xy) % (block_depth * cx->chunk_x)) / (block_height * block_depth * cx->block_size);
			unsigned block_width = std::min( (unsigned) cx->block_size, (unsigned) cx->nx - block_i * cx->block_size );
			unsigned odd = n - block_k * cx->chunk_xy - block_j * block_depth * cx->chunk_x - block_depth * block_height * cx->block_size * block_i;
			unsigned planar_chunk = block_width * block_height;
			unsigned odd_xy = odd % planar_chunk;
			//
			if( cx->use_zordering && block_width == block_height && block_height == block_depth && is_power_of_two(block_width)) {
				uint_fast16_t x, y, z;
				morton3D_32_decode(odd,x,y,z);
				i = block_i * cx->block_size + x;
				j = block_j * cx->block_size + y;
				k = block_k * cx->block_size + z;
			} else {
				i = block_i * cx->block_size + odd_xy % block_width;
				j = block_j * cx->block_size + odd_xy / block_width;
				k = block_k * cx->block_size + odd / planar_chunk;
			}
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
protected:
	//
	virtual void configure( configuration &config ) override {
		configuration::auto_group group(config,*this);
		config.get_integer("BlockSize",block_size,"Block size of chunk per dimension");
		config.get_bool("UseZOrdering",use_zordering,"Whether to use zordering for internal loop");
	}
	//
private:
	//
	struct context {
		int nx, ny, nz, block_size, chunk_x, chunk_xy;
		bool use_zordering;
	};
	int block_size;
	bool use_zordering;
};
//
extern "C" module * create_instance() {
	return new blockordering();
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