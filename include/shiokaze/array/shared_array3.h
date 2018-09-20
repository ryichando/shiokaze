/*
**	shared_array3.h
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
#ifndef SHKZ_SHARED_ARRAY3_H
#define SHKZ_SHARED_ARRAY3_H
//
#include <typeinfo>
#include <functional>
//
#include "array3.h"
#include "macarray3.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Abstract storage class that enables sharing pre-allocated arrays.
/// \~japanese @brief 事前に確保されたグリッドの共有を可能にする抽象クラス。
class shared_array_core3 {
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
	static void * borrow_shared( const shape3 &shape, size_t class_hash, std::string core_name, std::function<void *(const shape3 &shape, std::string core_name)> alloc_func, std::function<void( void *ptr )> dealloc_func );
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
/// \~english @brief Storage class that enables sharing pre-allocated arrays.
/// \~japanese @brief 事前に確保されたグリッドの共有を可能にするクラス。
template<class T> class shared_array3 {
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
	shared_array3( const shape3 &shape, T initial_value=T(), std::string core_name="" ) {
		m_array = reinterpret_cast<array3<T> *>(shared_array_core3::borrow_shared(shape,typeid(array3<T>).hash_code(),core_name,
			[]( const shape3 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared Array 3D","SharedArray"));
				return (void *)(new array3<T>(shape,T(),core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<array3<T> *>(ptr);
			}));
		m_array->clear(initial_value);
		assert( m_array->shape() == shape );
	}
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 @param[in] initial_value Initial value.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] initial_value 初期値。
	 */
	shared_array3( const typename array3<T>::type3 &type, T initial_value=T() ) : shared_array3(type.shape,initial_value,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_array3( const array3<T> &array ) : shared_array3(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_array2.
	 \~japanese @brief shared_array2 のデストラクタ。
	 */
	~shared_array3() {
		m_array->clear();
		shared_array_core3::return_shared(m_array);
	}
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const array3<T>& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	array3<T>& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	array3<T>* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const array3<T>* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	array3<T>* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const array3<T>* get() const { return m_array; }
	//
private:
	array3<T> *m_array;
};
//
/// \~english @brief Storage class that enables sharing pre-allocated arrays for MAC grid.
/// \~japanese @brief MAC グリッドのための、事前に確保されたグリッドの共有を可能にするクラス。
template<class T> class shared_macarray3 {
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
	shared_macarray3( const shape3 &shape, vec3<T> initial_value=vec3<T>(), std::string core_name="" ) {
		m_array = reinterpret_cast<macarray3<T> *>(shared_array_core3::borrow_shared(shape,typeid(macarray3<T>).hash_code(),core_name,
			[]( const shape3 &shape, std::string core_name ) {
				configuration::auto_group group(configurable::get_global_configuration(),
					credit("Shared MAC Array 3D","SharedMACArray"));
				return (void *)(new macarray3<T>(shape,T(),core_name));
			},
			[]( void *ptr ) {
				delete reinterpret_cast<macarray3<T> *>(ptr);
			}));
		m_array->clear(initial_value);
		assert( m_array->shape() == shape );
	}
	/**
	 \~english @brief Borrow a shared array.
	 @param[in] shape Shape of the grid.
	 @param[in] initial_value Initial value.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] shape グリッドの形。
	 @param[in] initial_value 初期値。
	 */
	shared_macarray3( const typename macarray3<T>::type3 &type, vec3<T> initial_value=vec3<T>() ) : shared_macarray3(type.shape,initial_value,type.core_name) {
		m_array->set_type(type);
	}
	/**
	 \~english @brief Borrow a shared array and copy the input.
	 @param[in] array Array to copy from.
	 \~japanese @brief 共有されたグリッドを借用する。
	 @param[in] array コピー元のグリッド。
	 */
	shared_macarray3( const macarray3<T> &array ) : shared_macarray3(array.type()) {
		m_array->copy(array);
	}
	/**
	 \~english @brief Destructor for shared_macarray2.
	 \~japanese @brief shared_macarray2 のデストラクタ。
	 */
	~shared_macarray3() {
		m_array->clear();
		shared_array_core3::return_shared(m_array);
	}
	/**
	 \~english @brief Get the const reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なリファレンスを得る。
	 */
	const macarray3<T>& operator()() const { return *m_array; }
	/**
	 \~english @brief Get the reference to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのリファレンスを得る。
	 */
	macarray3<T>& operator()() { return *m_array; }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	macarray3<T>* operator->() { return get(); }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const macarray3<T>* operator->() const { return get(); }
	/**
	 \~english @brief Get the pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドのポインターを得る。
	 */
	macarray3<T>* get() { return m_array; }
	/**
	 \~english @brief Get the const pointer to the internal borrowed array.
	 \~japanese @brief 内部の借用されたグリッドの const なポインターを得る。
	 */
	const macarray3<T>* get() const { return m_array; }
	//
private:
	macarray3<T> *m_array;
};
//
SHKZ_END_NAMESPACE
//
#endif
//