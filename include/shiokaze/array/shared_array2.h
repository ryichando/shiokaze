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
#ifndef SHKZ_SHARED_ARRAY2_H
#define SHKZ_SHARED_ARRAY2_H
//
#include <typeinfo>
#include <cassert>
//
#include "shared_array_core2.h"
#include "array2.h"
#include "macarray2.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Storage class that enables sharing pre-allocated arrays.
/// \~japanese @brief 事前に確保されたグリッドの共有を可能にするクラス。
template<class T> class shared_array2 {
public:
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 @param[in] initial_value Initial value.
	 @param[in] core_name Core module name for the grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] initial_value 初期値。
	 @param[in] core_name グリッドのコアモジュール名。
	 */
	shared_array2( const shape2 &shape, T initial_value=T(), std::string core_name="" ) {
		m_array = reinterpret_cast<array2<T> *>(shared_array_core2::borrow_shared(shape,typeid(array2<T>).hash_code(),core_name,
			[]( const shape2 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared Array 2D","SharedArray"));
				return (void *)(new array2<T>(shape,T(),core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<array2<T> *>(ptr);
			}));
		assert( m_array->shape() == shape );
		m_array->clear(initial_value);
	}
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 @param[in] initial_value Initial value.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] initial_value 初期値。
	 */
	shared_array2( const typename array2<T>::type2 &type, T initial_value=T() ) : shared_array2(type.shape,initial_value,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_array2( const array2<T> &array ) : shared_array2(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_array2.
	 \~japanese @brief shared_array2 のデストラクタ。
	 */
	~shared_array2() { 
		m_array->clear();
		shared_array_core2::return_shared(m_array);
	}
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const array2<T>& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	array2<T>& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	array2<T>* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const array2<T>* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	array2<T>* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const array2<T>* get() const { return m_array; }
	//
private:
	array2<T> *m_array;
};
//
/// \~english @brief Storage class that enables sharing pre-allocated arrays for MAC grid.
/// \~japanese @brief MAC グリッドのための、事前に確保されたグリッドの共有を可能にするクラス。
template<class T> class shared_macarray2 {
public:
	/**
	 \~english @brief Borrow a shared MAC array.
	 @param[in] shape Shape of the grid.
	 @param[in] initial_value Initial value.
	 @param[in] core_name Core module name for the grid.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] initial_value 初期値。
	 @param[in] core_name グリッドのコアモジュール名。
	 */
	shared_macarray2( const shape2 &shape, vec2<T> initial_value=vec2<T>(), std::string core_name="" ) {
		m_array = reinterpret_cast<macarray2<T> *>(shared_array_core2::borrow_shared(shape,typeid(macarray2<T>).hash_code(),core_name,
			[]( const shape2 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared MAC Array 2D","SharedMACArray"));
				return (void *)(new macarray2<T>(shape,T(),core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<macarray2<T> *>(ptr);
			}));
		assert( m_array->shape() == shape );
		m_array->clear(initial_value);
	}
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 @param[in] initial_value Initial value.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] initial_value 初期値。
	 */
	shared_macarray2( const typename macarray2<T>::type2 &type, vec2<T> initial_value=vec2<T>() ) : shared_macarray2(type.shape,initial_value,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_macarray2( const macarray2<T> &array ) : shared_macarray2(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_macarray2.
	 \~japanese @brief shared_macarray2 のデストラクタ。
	 */
	~shared_macarray2() {
		m_array->clear();
		shared_array_core2::return_shared(m_array);
	}
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const macarray2<T>& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	macarray2<T>& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	macarray2<T>* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const macarray2<T>* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	macarray2<T>* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const macarray2<T>* get() const { return m_array; }
	//
private:
	macarray2<T> *m_array;
};
//
SHKZ_END_NAMESPACE
//
#endif
//