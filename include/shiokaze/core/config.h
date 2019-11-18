/*
**	config.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 17, 2019.
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
/** @file */
//
#ifndef SHKZ_CONFIG_H
#define SHKZ_CONFIG_H
//
SHKZ_BEGIN_NAMESPACE
/**
 \~english @brief Precision of the Realing point.
 \~japanese @brief 浮動小数点の精度。
 */
using Real = float;
/**
 \~english @brief Default 2D array class.
 \~japanese @brief デフォルトの2次元配列クラス。
 */
const static char *shkz_default_array_core2 = "tiledarray2";
/**
 \~english @brief Default 3D array class.
 \~japanese @brief デフォルトの3次元配列クラス。
 */
const static char *shkz_default_array_core3 = "tiledarray3";
//
SHKZ_END_NAMESPACE
//
#endif