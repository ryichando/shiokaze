/*
**	array_interpolator2.h
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
#ifndef SHKZ_ARRAY_INTERPOLATOR2_H
#define SHKZ_ARRAY_INTERPOLATOR2_H
//
#include "array2.h"
#include <cmath>
#include <cstdlib>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that implements array interpolation.
/// \~japanese @brief グリッドの補間を実装する名前空間。
namespace array_interpolator2 {
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
	static void interpolate_coef( const shape2 &shape, const vec2d &p, vec2i indices[4], double coef[4] ) {
		double x = std::max(0.0,std::min(shape.w-1.,p[0]));
		double y = std::max(0.0,std::min(shape.h-1.,p[1]));
		int i = std::min(x,shape.w-2.);
		int j = std::min(y,shape.h-2.);
		indices[0] = vec2i(i,j);
		indices[1] = vec2i(i+1,j);
		indices[2] = vec2i(i,j+1);
		indices[3] = vec2i(i+1,j+1);
		coef[0] = (i+1-x)*(j+1-y);
		coef[1] = (x-i)*(j+1-y);
		coef[2] = (i+1-x)*(y-j);
		coef[3] = (x-i)*(y-j);
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array grid.
	 @param[in] p Position in index space.
	 @param[in] only_actives Sample only active values.
	 @return Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッド。
	 @param[in] p インデックス空間での位置。
	 @param[in] only_actives アクティブな値だけサンプルするか。
	 @return 補間値。
	 */
	template<class T> T static interpolate( const array2<T> &array, const vec2d &p, bool only_actives=false ) {
		T values[4]; vec2i indices[4]; double coef[4];
		interpolate_coef(array.shape(),p,indices,coef);
		T value = T();
		if( only_actives ) {
			double w[4];
			double sum (0.0);
			for( int n=0; n<4; ++n ) w[n] = array.active(indices[n]) ? coef[n] : 0.0;
			for( int n=0; n<4; ++n ) sum += w[n];
			if( sum ) {
				for( int n=0; n<4; ++n ) w[n] /= sum;
				for( unsigned n=0; n<4; ++n ) if( w[n] ) value += array(indices[n]) * w[n];
			}
		} else {
			for( unsigned n=0; n<4; ++n ) if( coef[n] ) value += array(indices[n]) * coef[n];
		}
		return value;
	}
	/**
	 \~english @brief Interpolate a physical quantity.
	 @param[in] array grid.
	 @param[in] origin Origin.
	 @param[in] dx Grid cell size
	 @param[in] p Position in physical space.
	 @param[in] only_actives Sample only active values.
	 @param[out] result Interpolation result.
	 \~japanese @brief 補間を計算する。
	 @param[in] array グリッド。
	 @param[in] origin 原点。
	 @param[in] dx Grid 格子セルの大きさ。
	 @param[in] p 物理空間での位置。
	 @param[in] only_actives アクティブな値だけサンプルするか。
	 @param[out] 補間値。
	 */
	template<class T> T static interpolate( const array2<T> &array, const vec2d &origin, double dx, const vec2d &p, bool only_actives=false ) {
		return interpolate<T>(array,(p-origin)/dx,only_actives);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//