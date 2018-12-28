/*
**	array_core3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 8, 2018.
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
#ifndef SHKZ_ARRAY_CORE3_H
#define SHKZ_ARRAY_CORE3_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <functional>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Core module class for three dimensional array designed to be used in array3 class.
/// \~japanese @brief array3 クラスで使われる三次元配列クラスのコアモジュールクラス。
class array_core3 : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(array_core3,"Array Core 3D","Array","Array core module")
	//
	array_core3() = default;
	virtual ~array_core3() = default;
	/**
	 \~english @brief Allocate grid memory with value.
	 @param[in] nx Grid width.
	 @param[in] ny Grid height.
	 @param[in] nz Grid depth.
	 @param[in] element_size bytes per element
	 \~japanese @brief グリッドを値でメモリに展開する。
	 @param[in] nx グリッドの幅。
	 @param[in] ny グリッドの高さ。
	 @param[in] nz グリッドの奥行き。
	 @param[in] element_size 要素のバイト数。
	 */
	virtual void initialize( unsigned nx, unsigned ny, unsigned nz, unsigned element_size ) = 0;
	/**
	 \~english @brief Get grid information.
	 @param[out] nx Grid width.
	 @param[out] ny Grid height.
	 @param[out] nz Grid depth.
	 @param[out] element_size bytes per element
	 \~japanese @brief グリッドの情報を得る。
	 @param[out] nx グリッドの幅。
	 @param[out] ny グリッドの高さ。
	 @param[out] nz グリッドの奥行き。
	 @param[out] element_size 要素のバイト数。
	 */
	virtual void get( unsigned &nx, unsigned &ny, unsigned &nz, unsigned &element_size ) const = 0;
	/**
	 \~english @brief Count the number of active cells.
	 @param[in] parallel Instance to a parallel driver.
	 @return Count of active cells.
	 \~japanese @brief アクティブなセルの数を数える。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 @return アクティブセルの数。
	 */
	virtual size_t count( const parallel_driver &parallel ) const = 0;
	/**
	 \~english @brief Copy grid.
	 @param[in] array Source grid to copy.
	 @param[in] copy_func Function that performs copy of an element.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief グリッドをコピーする。
	 @param[in] array コピー元のグリッド。
	 @param[in] copy_func 要素のコピーを実行する関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void copy( const array_core3 &array, std::function<void(void *target, const void *src)> copy_func, const parallel_driver *parallel ) = 0;
	/**
	 \~english @brief Set a value of a cell.
	 @param[in] i Position on x coordinate
	 @param[in] j Position on y coordinate
	 @param[in] k Position on z coordinate
	 @param[in] func Function that sets a value.
	 @param[in] cache Pointer to a cache if available.
	 \~japanese @brief セルに値を設定する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[in] func 値の設定を代行する関数。
	 */
	virtual void set( int i, int j, int k, std::function<void(void *value_ptr, bool &active)> func ) = 0;
	/**
	 \~english @brief Get a value of a cell.
	 @param[in] i Position on x coordinate
	 @param[in] j Position on y coordinate
	 @param[in] k Position on z coordinate
	 @param[out] filled Whether the position is filled.
	 @param[in] cache Pointer to a cache if available.
	 @return Pointer to the value of the cell.
	 \~japanese @brief セルの値を得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[out] filled グリッドの位置が塗りつぶされているか。
	 @return セルの値へのポインター。
	 */
	virtual const void * operator()( int i, int j, int k, bool &filled ) const = 0;
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that processes a cell.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 全てのアクティブなセルを並列に処理する。
	 @param[in] func セルの処理を行う関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void parallel_actives ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) = 0;
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that processes a cell.
	 \~japanese @brief 全てのアクティブなセルをシリアルに処理する。
	 @param[in] func セルの処理を行う関数。
	 */
	virtual void serial_actives ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) = 0;
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that processes a cell.
	 \~japanese @brief 全てのアクティブなセルを読み込み可能に限定して並列に処理する。
	 @param[in] func セルの処理を行う関数。
	 */
	virtual void const_parallel_actives ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) const = 0;
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that processes a cell.
	 \~japanese @brief 全てのアクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func セルの処理を行う関数。
	 */
	virtual void const_serial_actives ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &filled )> func ) const = 0;
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that processes a cell.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func セルの処理を行う関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void parallel_all ( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) = 0;
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that processes a cell.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func セルの処理を行う関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void serial_all ( std::function<bool(int i, int j, int k, void *value_ptr, bool &active, const bool &filled )> func ) = 0;
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that processes a cell.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 全てのセルを読み込み可能に限定して並列に処理する。
	 @param[in] func セルの処理を行う関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void const_parallel_all ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_index )> func, const parallel_driver &parallel ) const = 0;
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that processes a cell.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func セルの処理を行う関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void const_serial_all ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled )> func ) const = 0;
	/**
	 \~english @brief Dilate cells.
	 @param[in] func Function that specifies what value to assign on dilated cells.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 拡張する。
	 @param[in] func 拡張されたセルにどのような値を与えるか指定する関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void dilate( std::function<void(int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index)> func, const parallel_driver &parallel ) = 0;
	/**
	 \~english @brief Perform flood fill.
	 @param[in] inside_func Function that determines if the cell is inside.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 塗りつぶし処理を行う。
	 @param[in] inside_func 内側かどうかを判定する関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void flood_fill( std::function<bool(void *value_ptr)> inside_func, const parallel_driver &parallel ) = 0;
	/**
	 \~english @brief Loop over all the filled cells in parallel by read-only fashion.
	 @param[in] func Function that processes a cell.
	 @param[in] parallel Instance to a parallel driver.
	 \~japanese @brief 全ての塗りつぶされたセルを読み込み可能に限定して並列に処理する。
	 @param[in] func セルの処理を行う関数。
	 @param[in] parallel 並列化ドライバーのインスタンス。
	 */
	virtual void const_parallel_inside ( std::function<void(int i, int j, int k, const void *value_ptr, const bool &active, int thread_index )> func, const parallel_driver &parallel ) const = 0;
	/**
	 \~english @brief Loop over all the filled cells in serial order by read-only fashion.
	 @param[in] func Function that processes a cell.
	 \~japanese @brief 全ての塗りつぶされたセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func セルの処理を行う関数。
	 */
	virtual void const_serial_inside ( std::function<bool(int i, int j, int k, const void *value_ptr, const bool &active)> func ) const = 0;
	//
protected:
	//
	array_core3( const array_core3 & ) = delete;
	void operator=( const array_core3 & ) = delete;
	//
};
//
using array3_ptr = std::unique_ptr<array_core3>;
//
SHKZ_END_NAMESPACE
//
#endif
//