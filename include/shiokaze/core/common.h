/*
**	common.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 31, 2017. 
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
#ifndef SHKZ_COMMON_H
#define SHKZ_COMMON_H
//
#include <initializer_list>
/**
 \~english @brief Name space definition for shiokaze.
 \~japanese @brief shiokaze の名前空間の定義。
 */
#define SHKZ_NAMESPACE			shiokaze
/**
 \~english @brief Name space beggining definition for shiokaze.
 \~japanese @brief shiokaze の名前空間の開始の定義。
 */
#define SHKZ_BEGIN_NAMESPACE	namespace SHKZ_NAMESPACE {
/**
 \~english @brief Name space end definition for shiokaze.
 \~japanese @brief shiokaze の名前空間の終了の定義。
 */
#define SHKZ_END_NAMESPACE		};
/**
 \~english @brief Name space using definition for shiokaze.
 \~japanese @brief shiokaze の名前空間の使用の定義。
 */
#define SHKZ_USING_NAMESPACE	using namespace SHKZ_NAMESPACE;
//
SHKZ_BEGIN_NAMESPACE
/**
 \~english @brief Dimension list for two dimensions.
 \~japanese @brief 二次元のための次元リスト。
 */
const static int DIMS2[2] = {0,1};
/**
 \~english @brief Dimension list for three dimensions.
 \~japanese @brief 三次元のための次元リスト。
 */
const static int DIMS3[3] = {0,1,2};
/**
 \~english @brief Definition of dimension for two dimensions.
 \~japanese @brief 二次元のための次元の定義。
 */
const static int DIM2 (2);
/**
 \~english @brief Definition of dimension for three dimensions.
 \~japanese @brief 三次元のための次元の定義。
 */
const static int DIM3 (3);
//
SHKZ_END_NAMESPACE
//
#include "config.h"
//
#endif