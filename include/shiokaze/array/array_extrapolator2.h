/*
**	array_extrapolator2.h
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
#ifndef SHKZ_ARRAY_EXTRAPOLATOR2_H
#define SHKZ_ARRAY_EXTRAPOLATOR2_H
//
#include "array2.h"
#include "shared_array2.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that implements array extrapolation.
/// \~japanese @brief グリッドの外挿を実装する名前空間。
namespace array_extrapolator2 {
	/**
	 \~english @brief Extrapolate array where it passes the test function.
	 @param[in] array Grid to extrapolate.
	 @param[in] func Test function.
	 @param[in] count Extrapolation count.
	 \~japanese @brief テスト関数をパスする場所でグリッドを外挿する。
	 @param[in] array 外挿を行うグリッド。
	 @param[in] func テスト関数。
	 @param[in] count 外挿する回数。
	 */
	template<class T> unsigned extrapolate_if ( array2<T> &array, std::function<bool(int i, int j, int thread_index )> func, int count=1 ) {
		//
		std::vector<unsigned> counters(array.get_thread_num());
		const shape2 shape(array.shape());
		//
		array.dilate([&](int i, int j, auto &it, int tn ) {
			if( func(i,j,tn)) {
				T sum (0.0);
				int weight = 0;
				vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
				for( int nq=0; nq<4; nq++ ) {
					vec2i qi (query[nq]);
					if( ! shape.out_of_bounds(qi) ) {
						if( array.active(qi)) {
							sum += array(qi);
							weight ++;
						}
					}
				}
				if( weight ) {
					counters[tn] ++;
					it.set( sum / weight );
				}
			}
		},count);
		//
		unsigned sum (0);
		for( const auto &e : counters ) sum += e;
		return sum;
	}
	/**
	 \~english @brief Extrapolate array where it passes the test function.
	 @param[in] array Grid to extrapolate.
	 @param[in] count Extrapolation count.
	 \~japanese @brief テスト関数をパスする場所でグリッドを外挿する。
	 @param[in] array 外挿を行うグリッド。
	 @param[in] count 外挿する回数。
	 */
	template<class T> unsigned extrapolate ( array2<T> &array, int count=1 ) {
		return extrapolate_if(array,[&](int i, int j, int tn){ return true; },count);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//