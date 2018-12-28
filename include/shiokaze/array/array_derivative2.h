/*
**	array_derivative2.h
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
#ifndef SHKZ_ARRAY_DERIVATIVE2_H
#define SHKZ_ARRAY_DERIVATIVE2_H
//
#include "array2.h"
#include <cmath>
#include <cstdlib>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that computes the derivative of physical quantities at arbitrary position.
/// \~japanese @brief 物理量の任意の位置で勾配を計算するクラス。
class array_derivative2 {
public:
	/**
	 \~english @brief Get the coefficients for the gradient.
	 @param[in] shape Shape of the grid.
	 @param[in] p Position in index space.
	 @param[out] indices Indices output.
	 @param[out] coef Corresponding coefficents output.
	 \~japanese @brief 勾配に関して格子点とその係数を取得する。
	 @param[in] shape 格子の形。
	 @param[in] p インデックス空間での位置。
	 @param[out] indices 格子インデックス番号の出力。
	 @param[out] coef 該当する係数の出力。
	 */
	static void derivative_interpolate_coef( const shape2 &shape, const vec2d &p, vec2i indices[4], double coef[DIM2][4] ) {
		//
		double x = std::max(0.0,std::min(shape.w-1.,p[0]));
		double y = std::max(0.0,std::min(shape.h-1.,p[1]));
		int i = std::min(x,shape.w-2.);
		int j = std::min(y,shape.h-2.);
		//
		indices[0] = vec2i(i,j);
		indices[1] = vec2i(i+1,j);
		indices[2] = vec2i(i,j+1);
		indices[3] = vec2i(i+1,j+1);
		// x
		coef[0][0] = -(j+1-y);
		coef[0][1] = (j+1-y);
		coef[0][2] = -(y-j);
		coef[0][3] = (y-j);
		// y
		coef[1][0] = -(i+1-x);
		coef[1][1] = -(x-i);
		coef[1][2] = (i+1-x);
		coef[1][3] = (x-i);
	}
	/**
	 \~english @brief Computes the the gradient of a physical quantity.
	 @param[in] array Grid.
	 @param[in] p Position in index space.
	 @param[out] result Gradient output.
	 \~japanese @brief 勾配を計算する。
	 @param[in] array グリッド。
	 @param[in] p インデックス空間での位置。
	 @param[out] result 勾配の出力。
	 */
	template<class T>
	static void derivative( const array2<T> &array, const vec2d &p, T result[DIM2] ) {
		//
		T values[4]; vec2i indices[4]; double coef[DIM2][4];
		for( unsigned dim : DIMS2 ) result[dim] = T();
		derivative_interpolate_coef(array.shape(),p,indices,coef);
		for( unsigned dim : DIMS2 ) {
			for( unsigned n=0; n<4; ++n ) {
				result[dim] += coef[dim][n] * array(indices[n][0],indices[n][1]);
			}
		}
	}
	/**
	 \~english @brief Computes the the gradient of a physical quantity.
	 @param[in] array Grid.
	 @param[in] origin Origin.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @return Gradient output.
	 \~japanese @brief 勾配を計算する。
	 @param[in] array グリッド。
	 @param[in] origin 原点.
	 @param[in] dx グリッドセルの大きさ.
	 @param[in] p 物理空間の位置。
	 @return 勾配の出力。
	 */
	template<class T> static vec2<T> derivative( const macarray2<T> &array, const vec2d &origin, double dx, const vec2d &p ) {
		return derivative<T>(array,(p-origin)/dx);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//