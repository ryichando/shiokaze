/*
**	shared_bitarray3.h
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
#ifndef SHKZ_SHARED_BITARRAY3_H
#define SHKZ_SHARED_BITARRAY3_H
//
#include <typeinfo>
#include <functional>
#include <cassert>
//
#include "bitarray3.h"
#include "bitmacarray3.h"
//
SHKZ_BEGIN_NAMESPACE
//
/// \~english @brief Storage class that enables sharing pre-allocated bit arrays.
/// \~japanese @brief 事前に確保されたビットグリッドの共有を可能にするクラス。
class shared_bitarray3 {
public:
	/**
	 \~english @brief Borrow a shared bit array.
	 @param[in] shape Shape of the grid.
	 @param[in] core_name Core module name for the grid.
	 \~japanese @brief 共有されたビットグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] core_name グリッドのコアモジュール名。
	 */
	shared_bitarray3( const shape3 &shape, std::string core_name="" ) {
		m_array = reinterpret_cast<bitarray3 *>(shared_array_core3::borrow_shared(shape,typeid(bitarray3).hash_code(),core_name,
			[]( const shape3 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared Bit Array 3D","SharedBitArray"));
				return (void *)(new bitarray3(shape,core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<bitarray3 *>(ptr);
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
	shared_bitarray3( const typename bitarray3::type3 &type ) : shared_bitarray3(type.shape,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_bitarray3( const bitarray3 &array ) : shared_bitarray3(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_bitarray3.
	 \~japanese @brief shared_bitarray3 のデストラクタ。
	 */
	~shared_bitarray3() { shared_array_core3::return_shared(m_array); }
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const bitarray3& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	bitarray3& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitarray3* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitarray3* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitarray3* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitarray3* get() const { return m_array; }
	//
private:
	bitarray3 *m_array;
};
//
/// \~english @brief Storage class that enables sharing pre-allocated bit arrays for MAC grid.
/// \~japanese @brief MAC グリッドのための、事前に確保されたビットグリッドの共有を可能にするクラス。
class shared_bitmacarray3 {
public:
	/**
	 \~english @brief Borrow a shared MAC array.
	 @param[in] shape Shape of the grid.
	 @param[in] core_name Core module name for the grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] core_name グリッドのコアモジュール名。
	 */
	shared_bitmacarray3( const shape3 &shape, std::string core_name="" ) {
		m_array = reinterpret_cast<bitmacarray3 *>(shared_array_core3::borrow_shared(shape,typeid(bitmacarray3).hash_code(),core_name,
			[]( const shape3 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared Bit MAC Array 3D","SharedBitMACArray"));
				return (void *)(new bitmacarray3(shape,core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<bitmacarray3 *>(ptr);
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
	shared_bitmacarray3( const typename bitmacarray3::type3 &type ) : shared_bitmacarray3(type.shape,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_bitmacarray3( const bitmacarray3 &array ) : shared_bitmacarray3(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_bitmacarray3.
	 \~japanese @brief shared_bitmacarray3 のデストラクタ。
	 */
	~shared_bitmacarray3() { shared_array_core3::return_shared(m_array); }
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const bitmacarray3& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	bitmacarray3& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitmacarray3* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitmacarray3* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	bitmacarray3* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const bitmacarray3* get() const { return m_array; }
	//
private:
	bitmacarray3 *m_array;
};
//
SHKZ_END_NAMESPACE
//
#endif
//