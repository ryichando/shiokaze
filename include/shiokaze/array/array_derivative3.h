/*
**	array_derivative3.h
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
#ifndef SHKZ_ARRAY_DERIVATIVE3_H
#define SHKZ_ARRAY_DERIVATIVE3_H
//
#include "array3.h"
#include <cmath>
#include <cstdlib>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that computes the gradient of physical quantities.
/// \~japanese @brief 物理量の勾配を計算するクラス。
class array_derivative3 {
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
	static void derivative_interpolate_coef( const shape3 &shape, const vec3d &p, vec3i indices[8], double coef[DIM3][8] ) {
		//
		double x = std::max(0.0,std::min(shape.w-1.,p[0]));
		double y = std::max(0.0,std::min(shape.h-1.,p[1]));
		double z = std::max(0.0,std::min(shape.d-1.,p[2]));
		int i = std::min(x,shape.w-2.);
		int j = std::min(y,shape.h-2.);
		int k = std::min(z,shape.d-2.);
		//
		indices[0] = vec3i(i,j,k);
		indices[1] = vec3i(i+1,j,k);
		indices[2] = vec3i(i,j+1,k);
		indices[3] = vec3i(i+1,j+1,k);
		indices[4] = vec3i(i,j,k+1);
		indices[5] = vec3i(i+1,j,k+1);
		indices[6] = vec3i(i,j+1,k+1);
		indices[7] = vec3i(i+1,j+1,k+1);
		// x
		coef[0][0] = -(k+1-z)*(j+1-y);
		coef[0][1] = (k+1-z)*(j+1-y);
		coef[0][2] = -(k+1-z)*(y-j);
		coef[0][3] = (k+1-z)*(y-j);
		coef[0][4] = -(z-k)*(j+1-y);
		coef[0][5] = (z-k)*(j+1-y);
		coef[0][6] = -(z-k)*(y-j);
		coef[0][7] = (z-k)*(y-j);
		// y
		coef[1][0] = -(k+1-z)*(i+1-x);
		coef[1][1] = -(k+1-z)*(x-i);
		coef[1][2] = (k+1-z)*(i+1-x);
		coef[1][3] = (k+1-z)*(x-i);
		coef[1][4] = -(z-k)*(i+1-x);
		coef[1][5] = -(z-k)*(x-i);
		coef[1][6] = (z-k)*(i+1-x);
		coef[1][7] = (z-k)*(x-i);
		// z
		coef[2][0] = -(i+1-x)*(j+1-y);
		coef[2][1] = -(x-i)*(j+1-y);
		coef[2][2] = -(i+1-x)*(y-j);
		coef[2][3] = -(x-i)*(y-j);
		coef[2][4] = (i+1-x)*(j+1-y);
		coef[2][5] = (x-i)*(j+1-y);
		coef[2][6] = (i+1-x)*(y-j);
		coef[2][7] = (x-i)*(y-j);
	}
	/**
	 \~english @brief Computes the the gradient of a physical quantity.
	 @param[in] accessor Accessor of the grid.
	 @param[in] p Position in index space.
	 @param[out] result Gradient output.
	 \~japanese @brief 勾配を計算する。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] p インデックス空間での位置。
	 @param[out] result 勾配の出力。
	 */
	template<class T> static void derivative( typename array3<T>::const_accessor accessor, const vec3d &p, T result[DIM3] ) {
		//
		T values[8]; vec3i indices[8]; double coef[DIM3][8];
		for( unsigned dim : DIMS3 ) result[dim] = T();
		derivative_interpolate_coef(accessor.shape(),p,indices,coef);
		for( unsigned dim : DIMS3 ) {
			for( unsigned n=0; n<8; ++n ) {
				result[dim] += coef[dim][n] * accessor(indices[n][0],indices[n][1],indices[n][2]);
			}
		}
	}
	/**
	 \~english @brief Computes the the gradient of a physical quantity.
	 @param[in] accessor Accessor of the grid.
	 @param[in] origin Origin.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @return Gradient output.
	 \~japanese @brief 勾配を計算する。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] origin 原点.
	 @param[in] dx グリッドセルの大きさ.
	 @param[in] p 物理空間の位置。
	 @return 勾配の出力。
	 */
	template<class T> static void derivative( const array3<T> &array, const vec3d &p, T result[DIM3] ) {
		auto accessor = array.get_const_accessor();
		derivative<T>(accessor,p,result);
	}
	/**
	 \~english @brief Computes the the gradient of a physical quantity.
	 @param[in] accessor Accessor of the grid.
	 @param[in] origin Origin.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @return Gradient output.
	 \~japanese @brief 勾配を計算する。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] origin 原点.
	 @param[in] dx グリッドセルの大きさ.
	 @param[in] p 物理空間の位置。
	 @return 勾配の出力。
	 */
	template<class T> static vec3<T> derivative( typename macarray3<T>::const_accessor &accessor, const vec3d &origin, double dx, const vec3d &p ) {
		return derivative<T>(accessor,(p-origin)/dx);
	}
	/**
	 \~english @brief Computes the the gradient of a physical quantity.
	 @param[in] array Reference to a grid.
	 @param[in] origin Origin.
	 @param[in] dx Grid cell size.
	 @param[in] p Position in physical space.
	 @return Gradient output.
	 \~japanese @brief 勾配を計算する。
	 @param[in] array グリッドの参照。
	 @param[in] origin 原点.
	 @param[in] dx グリッドセルの大きさ.
	 @param[in] p 物理空間の位置。
	 @return 勾配の出力。
	 */
	template<class T> static vec3<T> derivative( const macarray3<T> &array, const vec3d &origin, double dx, const vec3d &p ) {
		auto accessor = array.get_const_accessor();
		return derivative<T>(accessor,origin,dx,p);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//