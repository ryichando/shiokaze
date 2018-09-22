/*
**	array_gradient2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 22, 2018.
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
#ifndef SHKZ_ARRAY_GRADIENT2_H
#define SHKZ_ARRAY_GRADIENT2_H
//
#include "array2.h"
#include "macarray2.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that computes the gradient of physical quantities.
/// \~japanese @brief 物理量の勾配を計算するクラス。
namespace array_gradient2 {
	//
	template<class T> void compute_gradient( const array2<T> &array, macarray2<T> &gradient, double dx ) {
		for( int dim : DIMS2 ) {
			gradient[dim].clear();
			gradient[dim].activate_as(array,vec2i(0,0));
			gradient[dim].activate_as(array,vec2i(dim==0,dim==1));
		}
		auto array_accessors = array.get_const_accessors();
		shape2 shape = array.shape();
		gradient.parallel_actives([&]( int dim, int i, int j, auto &it, int tn ) {
			it.set(
				(array_accessors[tn](shape.clamp(i,j))
				-array_accessors[tn](shape.clamp(i-(dim==0),j-(dim==1)))
				)/dx);
		});
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//