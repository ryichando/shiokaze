/*
**	macarray_interpolator3.h
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
#ifndef SHKZ_MACARRAY_INTERPOLATOR3_H
#define SHKZ_MACARRAY_INTERPOLATOR3_H
//
#include <shiokaze/math/vec.h>
#include "array_interpolator3.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that implements MAC array interpolation.
/// \~japanese @brief グリッドの補間を実装する名前空間。
namespace macarray_interpolator3 {
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] accessor Accessor of the grid.
	 @param[in] p Position in index space.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] p インデックス空間での位置。
	 @return 補間値。
	 */
	template<class T> vec3<T> interpolate( typename macarray3<T>::const_accessor &accessor, const vec3d &p ) {
		vec3<T> result;
		for( int dim : DIMS3 ) {
			const vec3d pos = vec3d(p[0]-0.5*(dim!=0),p[1]-0.5*(dim!=1),p[2]-0.5*(dim!=2));
			result[dim] = array_interpolator3::interpolate<T>(accessor.get(dim),pos);
		}
		return result;
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array Grid to interpolate.
	 @param[in] p Position in index space.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array 補間するグリッド。
	 @param[in] p インデックス空間での位置。
	 @return 補間値。
	 */
	template<class T> vec3<T> interpolate( const macarray3<T> &array, const vec3d &p ) {
		auto accessor = array.get_const_accessor();
		return interpolate<T>(accessor,p);
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] accessor Accessor of the grid.
	 @param[in] origin Origin in physical space.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] origin 物理空間での原点.
	 @param[in] dx グリッドセルの大きさ.
	 @param[in] p 物理空間の位置。
	 @return 補間値。
	 */
	template<class T> vec3<T> interpolate( typename macarray3<T>::const_accessor &accessor, const vec3d &origin, double dx, const vec3d &p ) {
		return interpolate<T>(accessor,(p-origin)/dx);
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array Reference to a grid.
	 @param[in] origin Origin in physical space.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッドの参照。
	 @param[in] origin 物理空間での原点.
	 @param[in] dx グリッドセルの大きさ.
	 @param[in] p 物理空間の位置。
	 @return 補間値。
	 */
	template<class T> vec3<T> interpolate( const macarray3<T> &array, const vec3d &origin, double dx, const vec3d &p ) {
		auto accessor = array.get_const_accessor();
		return interpolate<T>(accessor,origin,dx,p);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//