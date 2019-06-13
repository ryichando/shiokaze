/*
**	array_upsampler2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on May 30, 2019.
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
#ifndef SHKZ_ARRAY_UPSAMPLER2_H
#define SHKZ_ARRAY_UPSAMPLER2_H
//
#include <shiokaze/array/array_interpolator2.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namesampe for upsampling grids.
/// \~japanese @brief グリッドをアップサンプルする名前空間。
namespace array_upsampler2 {
	/**
	 \~english @brief Upsample a cell-centerd grid to double sized.
	 @param[in] array Input grid.
	 @param[out] doubled_array Doubled grid.
	 \~japanese @brief セルセンターのグリッドをアップサンプルして二倍の大きさにする。
	 @param[in] array 入力のグリッド。
	 @param[out] doubled_array 二倍にアップサンプルされたグリッド。
	 */
	template<class T> void upsample_to_double_cell( const array2<T> &array, double dx, array2<T> &doubled_array ) {
		//
		assert(doubled_array.shape() == 2*array.shape());
		//
		array.const_serial_actives([&]( int i, int j, auto &it ) {
			for( int ii=0; ii<2; ++ii ) for( int jj=0; jj<2; ++jj ) {
				vec2i pi (2*i+ii,2*j+jj);
				if( ! doubled_array.shape().out_of_bounds(pi)) {
					doubled_array.set(pi,0.0);
				}
			}
		});
		//
		doubled_array.parallel_actives([&]( int i, int j, auto &it, int tn) {
			vec2d p = 0.5*vec2i(i,j).cell()-vec2d(0.5,0.5);
			it.set(array_interpolator2::interpolate<double>(array,p));
		});
		//
		if( array.is_levelset()) {
			doubled_array.set_as_levelset(0.5*array.get_background_value());
			doubled_array.flood_fill();
		}
	}
	//
	/**
	 \~english @brief Upsample a nodal grid to double sized.
	 @param[in] array Input grid.
	 @param[out] doubled_array Doubled grid.
	 \~japanese @brief 節点ベースのグリッドをアップサンプルして二倍の大きさにする。
	 @param[in] array 入力のグリッド。
	 @param[out] doubled_array 二倍にアップサンプルされたグリッド。
	 */
	template<class T> void upsample_to_double_nodal( const array2<T> &array, double dx, array2<T> &doubled_array ) {
		//
		assert(doubled_array.shape() == 2*array.shape()-shape2(1,1));
		//
		array.const_serial_actives([&]( int i, int j, auto &it ) {
			for( int ii=0; ii<2; ++ii ) for( int jj=0; jj<2; ++jj ) {
				vec2i pi (2*i+ii,2*j+jj);
				if( ! doubled_array.shape().out_of_bounds(pi)) {
					doubled_array.set(pi,0.0);
				}
			}
		});
		//
		doubled_array.parallel_actives([&]( int i, int j, auto &it, int tn) {
			vec2d p = 0.5*vec2i(i,j).nodal();
			it.set(array_interpolator2::interpolate<double>(array,p));
		});
		//
		if( array.is_levelset()) {
			doubled_array.set_as_levelset(0.5*array.get_background_value());
			doubled_array.flood_fill();
		}
	}
};
//
SHKZ_END_NAMESPACE
//
#endif