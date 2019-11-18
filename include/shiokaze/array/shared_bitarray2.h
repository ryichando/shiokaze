/*
**	shared_bitarray2.h
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
#ifndef SHKZ_SHARED_BITARRAY2_H
#define SHKZ_SHARED_BITARRAY2_H
//
#include <typeinfo>
#include <functional>
#include <cassert>
//
#include "bitarray2.h"
#include "bitmacarray2.h"
#include "shared_array_core2.h"
//
SHKZ_BEGIN_NAMESPACE
//
/// \~english @brief Storage class that enables sharing pre-allocated bit arrays.
/// \~japanese @brief 事前に確保されたビットグリッドの共有を可能にするクラス。
class shared_bitarray2 {
public:
	/**
	 \~english @brief Borrow a shared bit array.
	 @param[in] shape Shape of the grid.
	 @param[in] core_name Core module name for the grid.
	 \~japanese @brief 共有されたビットグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] core_name グリッドのコアモジュール名。
	 */
	shared_bitarray2( const shape2 &shape, std::string core_name="" ) {
		m_array = reinterpret_cast<bitarray2 *>(shared_array_core2::borrow_shared(shape,typeid(bitarray2).hash_code(),core_name,
			[]( const shape2 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared Bit Array 2D","SharedBitArray"));
				return (void *)(new bitarray2(shape,core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<bitarray2 *>(ptr);
			}));
		assert( m_array->shape() == shape );
		m_array->clear();
	}
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 */
	shared_bitarray2( const typename bitarray2::type2 &type ) : shared_bitarray2(type.shape,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_bitarray2( const bitarray2 &array ) : shared_bitarray2(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_bitarray2.
	 \~japanese @brief shared_bitarray2 のデストラクタ。
	 */
	~shared_bitarray2() { shared_array_core2::return_shared(m_array); }
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const bitarray2& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	bitarray2& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitarray2* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitarray2* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitarray2* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitarray2* get() const { return m_array; }
	//
private:
	bitarray2 *m_array;
};
//
/// \~english @brief Storage class that enables sharing pre-allocated bit arrays for MAC grid.
/// \~japanese @brief MAC グリッドのための、事前に確保されたビットグリッドの共有を可能にするクラス。
class shared_bitmacarray2 {
public:
	/**
	 \~english @brief Borrow a shared MAC array.
	 @param[in] shape Shape of the grid.
	 @param[in] core_name Core module name for the grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] core_name グリッドのコアモジュール名。
	 */
	shared_bitmacarray2( const shape2 &shape, std::string core_name="" ) {
		m_array = reinterpret_cast<bitmacarray2 *>(shared_array_core2::borrow_shared(shape,typeid(bitmacarray2).hash_code(),core_name,
			[]( const shape2 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared Bit MAC Array 2D","SharedBitMACArray"));
				return (void *)(new bitmacarray2(shape,core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<bitmacarray2 *>(ptr);
			}));
		assert( m_array->shape() == shape );
		m_array->clear();
	}
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 */
	shared_bitmacarray2( const typename bitmacarray2::type2 &type ) : shared_bitmacarray2(type.shape,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_bitmacarray2( const bitmacarray2 &array ) : shared_bitmacarray2(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_bitmacarray2.
	 \~japanese @brief shared_bitmacarray2 のデストラクタ。
	 */
	~shared_bitmacarray2() { shared_array_core2::return_shared(m_array); }
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const bitmacarray2& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	bitmacarray2& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitmacarray2* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitmacarray2* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitmacarray2* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitmacarray2* get() const { return m_array; }
	//
private:
	bitmacarray2 *m_array;
};
//
SHKZ_END_NAMESPACE
//
#endif
//