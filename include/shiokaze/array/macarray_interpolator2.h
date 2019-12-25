/*
**	macarray_interpolator2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 25, 2018.
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
#ifndef SHKZ_MACARRAY_INTERPOLATOR2_H
#define SHKZ_MACARRAY_INTERPOLATOR2_H
//
#include <shiokaze/math/vec.h>
#include "array_interpolator2.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that implements MAC array interpolation.
/// \~japanese @brief グリッドの補間を実装する名前空間。
namespace macarray_interpolator2 {
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array Grid.
	 @param[in] p Position in index space.
	 @param[in] only_actives Sample only active values.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッド。
	 @param[in] p インデックス空間での位置。
	 @param[in] only_actives アクティブな値だけサンプルするか。
	 @return 補間値。
	 */
	template<class T> static vec2<T> interpolate( const macarray2<T> &array, const vec2d &p, bool only_actives=false ) {
		vec2<T> result;
		for( int dim : DIMS2 ) {
			const vec2d pos = vec2d(p[0]-0.5*(dim!=0),p[1]-0.5*(dim!=1));
			result[dim] = array_interpolator2::interpolate<T>(array[dim],pos,only_actives);
		}
		return result;
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array Grid.
	 @param[in] origin Origin in physical space.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @param[in] only_actives Sample only active values.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッド。
	 @param[in] origin 物理空間での原点。
	 @param[in] dx グリッドセルの大きさ。
	 @param[in] p 物理空間の位置。
	 @param[in] only_actives アクティブな値だけサンプルするか。
	 @return 補間値。
	 */
	template<class T> static vec2<T> interpolate( const macarray2<T> &array, const vec2d &origin, double dx, const vec2d &p, bool only_actives=false ) {
		return interpolate<T>(array,(p-origin)/dx,only_actives);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//