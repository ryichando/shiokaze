/*
**	WENO2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 7, 2017. 
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
#ifndef SHKZ_WENO2_H
#define SHKZ_WENO2_H
//
#include "WENO.h"
#include <shiokaze/math/vec.h>
#include <shiokaze/array/array2.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that provides two dimensional WENO interpolations.
/// \~japanese @brief 2次元の WENO 補間を提供するインターフェース。
class WENO2 {
public:
	/**
	 \~english @brief Interpolate using WENO interpolation.
	 @param[in] accessor Grid accessor.
	 @param[in] p Position in index coordinate.
	 @param[in] order Order of WENO interpolation. Either 6 or 4 is available.
	 @return Interpolated value.
	 \~japanese @brief WENO 補間を行う。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] p インデックス空間の位置。
	 @param[in] order WENO の次数。4 あるいは 6 が指定可能。
	 @return 補間結果。
	 */
	static vec2d interpolate( array2<vec2d>::const_accessor &accessor, const vec2d &p, unsigned order=6 ) {
		//
		const auto shape = accessor.shape();
		double x = std::max(0.0,std::min(shape.w-1.,p[0]));
		double y = std::max(0.0,std::min(shape.h-1.,p[1]));
		//
		int i = std::min(x,shape.w-2.);
		int j = std::min(y,shape.h-2.);
		//
		vec2d result;
		for( int dim : DIMS2 ) {
			if( order == 6 ) {
				double vv[6];
				for( int jj=0; jj<6; ++jj ) {
					double v[6];
					for( int ii=0; ii<6; ++ii ) v[ii] = accessor(shape.clamp(i+ii-2,j+jj-2))[dim];
					vv[jj] = WENO::interp6(x-i,v);
				}
				result[dim] = WENO::interp6(y-j,vv);
			} else if( order == 4 ) {
				double vv[4];
				for( int jj=0; jj<4; ++jj ) {
					double hv[4];
					for( int ii=0; ii<4; ++ii ) hv[ii] = accessor(shape.clamp(i+ii-1,j+jj-1))[dim];
					vv[jj] = WENO::interp4(x-i,hv);
				}
				result[dim] = WENO::interp4(y-j,vv);
			} else {
				printf( "Unsupported order (order=%d)\n", order );
				exit(0);
			}
		}
		return result;
	}
	/**
	 \~english @brief Interpolate using WENO interpolation.
	 @param[in] array Grid to interpolate.
	 @param[in] p Position in index coordinate.
	 @param[in] order Order of WENO interpolation. Either 6 or 4 is available.
	 @return Interpolated value.
	 \~japanese @brief WENO 補間を行う。
	 @param[in] array 補間を行うグリッド。
	 @param[in] p インデックス空間の位置。
	 @param[in] order WENO の次数。4 あるいは 6 が指定可能。
	 @return 補間結果。
	 */
	static vec2d interpolate( const array2<vec2d> &array, const vec2d &p, unsigned order=6 ) {
		auto accessor = array.get_const_accessor();
		return interpolate(accessor,p,order);
	}
	/**
	 \~english @brief Interpolate using WENO interpolation.
	 @param[in] accessor Grid accessor.
	 @param[in] p Position in index coordinate.
	 @param[in] order Order of WENO interpolation. Either 6 or 4 is available.
	 @return Interpolated value.
	 \~japanese @brief WENO 補間を行う。
	 @param[in] accessor グリッドのアクセッサー。
	 @param[in] p インデックス空間の位置。
	 @param[in] order WENO の次数。4 あるいは 6 が指定可能。
	 @return 補間結果。
	 */
	static double interpolate( array2<double>::const_accessor &accessor, const vec2d &p, unsigned order=6 ) {
		//
		const auto shape = accessor.shape();
		double x = std::max(0.0,std::min(shape.w-1.,p[0]));
		double y = std::max(0.0,std::min(shape.h-1.,p[1]));
		//
		int i = std::min(x,shape.w-2.);
		int j = std::min(y,shape.h-2.);
		//
		if( order == 6 ) {
			double vv[6];
			for( int jj=0; jj<6; ++jj ) {
				double v[6];
				for( int ii=0; ii<6; ++ii ) v[ii] = accessor(shape.clamp(i+ii-2,j+jj-2));
				vv[jj] = WENO::interp6(x-i,v);
			}
			return WENO::interp6(y-j,vv);
		} else if( order == 4 ) {
			double vv[4];
			for( int jj=0; jj<4; ++jj ) {
				double v[4];
				for( int ii=0; ii<4; ++ii ) v[ii] = accessor(shape.clamp(i+ii-1,j+jj-1));
				vv[jj] = WENO::interp4(x-i,v);
			}
			return WENO::interp4(y-j,vv);
		} else {
			printf( "Unsupported order (order=%d)\n", order );
			exit(0);
		}
	}
	/**
	 \~english @brief Interpolate using WENO interpolation.
	 @param[in] array Grid to interpolate.
	 @param[in] p Position in index coordinate.
	 @param[in] order Order of WENO interpolation. Either 6 or 4 is available.
	 @return Interpolated value.
	 \~japanese @brief WENO 補間を行う。
	 @param[in] array 補間を行うグリッド。
	 @param[in] p インデックス空間の位置。
	 @param[in] order WENO の次数。4 あるいは 6 が指定可能。
	 @return 補間結果。
	 */
	static double interpolate( const array2<double> &array, const vec2d &p, unsigned order=6 ) {
		auto accessor = array.get_const_accessor();
		return interpolate(accessor,p,order);
	}
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
