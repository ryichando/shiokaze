/*
**	macarray_extrapolator3.h
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
#ifndef SHKZ_MACARRAY_EXTRAPOLATOR3_H
#define SHKZ_MACARRAY_EXTRAPOLATOR3_H
//
#include "array_extrapolator3.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that implements MAC array extrapolation.
/// \~japanese @brief MAC グリッドの外挿を実装する名前空間。
namespace macarray_extrapolator3 {
	/**
	 \~english @brief Extrapolate MAC array.
	 @param[in] array Grid to extrapolate.
	 @param[in] width Extrapolation count.
	 \~japanese @brief MAC グリッドを外挿する。
	 @param[in] array 外挿を行うグリッド。
	 @param[in] width 外挿する回数。
	 */
	template<class T> void extrapolate ( macarray3<T> &array, int width ) {
		for( int dim : DIMS3 ) {
			array_extrapolator3::extrapolate<T>(array[dim],width);
		}
	};
};
//
SHKZ_END_NAMESPACE
//
#endif
//