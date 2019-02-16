/*
**	shared_array2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 15, 2018.
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
#ifndef SHKZ_SHARED_ARRAY_CORE2_H
#define SHKZ_SHARED_ARRAY_CORE2_H
//
#include <shiokaze/array/shape.h>
#include <functional>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Abstract storage class that enables sharing pre-allocated arrays.
/// \~japanese @brief 事前に確保されたビットグリッドの共有を可能にする抽象クラス。
class shared_array_core2 {
public:
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 @param[in] class_hash Hash indicator for the type of grid.
	 @param[in] core_name Core module name for the grid.
	 @param[in] alloc_func Grid allocation func.
	 @param[in] dealloc_func Grid deallocation func.
	 @return Pointer to a shared grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] class_hash Hash グリッドの種類を示すハッシュ値。
	 @param[in] core_name グリッドのコアモジュール名。
	 @param[in] alloc_func グリッドのメモリアロケーター関数。
	 @param[in] dealloc_func グリッドのメモリ解放関数。
	 @return 借用されたグリッドのポインタ。
	 */
	static void * borrow_shared( const shape2 &shape, size_t class_hash, std::string core_name, std::function<void *(const shape2 &shape, std::string core_name)> alloc_func, std::function<void( void *ptr )> dealloc_func );
	/**
	 \~english @brief Return a borrowed array.
	 @param[in] array Pointer to a borrowed array.
	 \~japanese @brief 借用されたグリッドを変換する。
	 @param[in] array 借用されたグリッドへのポインター。
	 */
	static void return_shared( void *array );
	/**
	 \~english @brief Clear grid storage.
	 \~japanese @brief グリッドの倉庫を空になる。
	 */
	static void clear();
};
//
SHKZ_END_NAMESPACE
//
#endif
//