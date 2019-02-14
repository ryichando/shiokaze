/*
**	array_interpolator3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 14, 2018.
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
#ifndef SHKZ_ARRAY_INTERPOLATOR3_H
#define SHKZ_ARRAY_INTERPOLATOR3_H
//
#include "array3.h"
#include <cmath>
#include <cstdlib>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that implements array interpolation.
/// \~japanese @brief グリッドの補間を実装する名前空間。
namespace array_interpolator3 {
	/**
	 \~english @brief Get the coefficients for the interpolation.
	 @param[in] shape Shape of the grid.
	 @param[in] p Position in index space.
	 @param[out] indices Indices output.
	 @param[out] coef Corresponding coefficents output.
	 \~japanese @brief 補間に関して格子点とその係数を取得する。
	 @param[in] shape 格子の形。
	 @param[in] p インデックス空間での位置。
	 @param[out] indices 格子インデックス番号の出力。
	 @param[out] coef 該当する係数の出力。
	 */
	static void interpolate_coef( const shape3 &shape, const vec3d &p, vec3i indices[8], double coef[8] ) {
		double x = std::max(0.0,std::min(shape.w-1.,p[0]));
		double y = std::max(0.0,std::min(shape.h-1.,p[1]));
		double z = std::max(0.0,std::min(shape.d-1.,p[2]));
		int i = std::min(x,shape.w-2.);
		int j = std::min(y,shape.h-2.);
		int k = std::min(z,shape.d-2.);
		indices[0] = vec3i(i,j,k);
		indices[1] = vec3i(i+1,j,k);
		indices[2] = vec3i(i,j+1,k);
		indices[3] = vec3i(i+1,j+1,k);
		indices[4] = vec3i(i,j,k+1);
		indices[5] = vec3i(i+1,j,k+1);
		indices[6] = vec3i(i,j+1,k+1);
		indices[7] = vec3i(i+1,j+1,k+1);
		coef[0] = (k+1-z)*(i+1-x)*(j+1-y);
		coef[1] = (k+1-z)*(x-i)*(j+1-y);
		coef[2] = (k+1-z)*(i+1-x)*(y-j);
		coef[3] = (k+1-z)*(x-i)*(y-j);
		coef[4] = (z-k)*(i+1-x)*(j+1-y);
		coef[5] = (z-k)*(x-i)*(j+1-y);
		coef[6] = (z-k)*(i+1-x)*(y-j);
		coef[7] = (z-k)*(x-i)*(y-j);
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array grid.
	 @param[in] p Position in index space.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッド。
	 @param[in] p インデックス空間での位置。
	 @return 補間値。
	 */
	template<class T> T interpolate( const array3<T> &array, const vec3d &p ) {
		T values[8]; vec3i indices[8]; double coef[8];
		interpolate_coef(array.shape(),p,indices,coef);
		for( int n=0; n<8; ++n ) values[n] = array(indices[n][0],indices[n][1],indices[n][2]);
		T value = T();
		for( unsigned n=0; n<8; ++n ) if( coef[n] ) value += values[n] * coef[n];
		return value;
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array grid.
	 @param[in] origin Origin.
	 @param[in] dx Grid cell size
	 @param[in] p Position in physical space.
	 @param[out] result Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッド。
	 @param[in] origin 原点。
	 @param[in] dx Grid 格子セルの大きさ。
	 @param[in] p 物理空間での位置。
	 @param[out] 補間値。
	 */
	template<class T> T interpolate( const array3<T> &array, const vec3d &origin, double dx, const vec3d &p ) {
		return interpolate<T>(array,(p-origin)/dx);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//