/*
**	array_utility3.h
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
#include "array3.h"
#include <cmath>
#include <cstdlib>
//
#ifndef SHKZ_ARRAY_UTILITY3_H
#define SHKZ_ARRAY_UTILITY3_H
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that provides various utility functions.
/// \~japanese @brief 様々なユーティリティ機能を提供する名前空間。
namespace array_utility3 {
	/**
	 \~english @brief Get if a grid is empty.
	 @param[in] array Array to examine.
	 @return \true if the grid is empty. \c false otherwise.
	 \~japanese @brief グリッドが空か取得する。
	 @param[in] array 調べるグリッド。
	 @return もし空なら \c true そうでなければ \c false を返す。
	 */
	template <class T> bool empty( const array3<T> &array ) {
		return array.shape().count() == 0;
	}
	/**
	 \~english @brief Get if a grid constain different values.
	 @param[in] array Array to examine.
	 @return \true if the grid has different values. \c false otherwise.
	 \~japanese @brief グリッドが異なる値を保持しているか取得する。
	 @param[in] array 調べるグリッド。
	 @return もし異なる値を持っていれば \c true そうでなければ \c false を返す。
	 */
	template <class T> bool has_different_values ( const array3<T> &array ) {
		bool result (false);
		bool assigned (false);
		T value;
		array.interruptible_const_serial_actives([&](int i, int j, int k, const auto &it) {
			if( ! assigned ) {
				value = array(i,j,k);
				assigned = true;
			} else if(array(i,j,k)!=value) {
				result = true;
				return true;
			}
			return false;
		});
		return result;
	}
	/**
	 \~english @brief Get if a grid constain a value that is different from an input value.
	 @param[in] array Array to examine.
	 @param[in] v Input value.
	 @return \true if the grid has different values other than the input of v. \c false otherwise.
	 \~japanese @brief グリッドが入力の値と異なる値を保持しているか取得する。
	 @param[in] array 調べるグリッド。
	 @return もし入力 v と異なる値を持っていれば \c true そうでなければ \c false を返す。
	 */
	template <class T> bool has_value_not( const array3<T> &array, const T &v ) {
		bool result (false);
		array.interruptible_const_serial_actives([&](int i, int j, int k, const auto &it) {
			if(it() != v) {
				result = true;
				return true;
			}
			return false;
		});
		return result;
	}
	/**
	 \~english @brief Get if a grid constain a value that is different from the background value.
	 @param[in] array Array to examine.
	 @return \true if the grid has value that is different from the background value. \c false otherwise.
	 \~japanese @brief グリッドがバックグラウンド値と異なる値を保持しているか取得する。
	 @param[in] array 調べるグリッド。
	 @return もしグリッドがバックグランド値と異なる値を持っていれば \c true そうでなければ \c false を返す。
	 */
	template <class T> bool value_exist( const array3<T> &array ) {
		return has_value_not(array,array.get_background_value());
	}
	/**
	 \~english @brief Get if a level set grid constain both negative and positive values.
	 @param[in] array Level set grid to examine.
	 @return \true if the grid has both negative and positive values. \c false otherwise.
	 \~japanese @brief レベルセットグリッドが負の値と正の値の両方を保持しているか取得する。
	 @param[in] array 調べるレベルセットグリッド。
	 @return もしグリッドが負の値と正の値の両方の値を持っていれば \c true そうでなければ \c false を返す。
	 */
	template <class T> bool levelset_exist( const array3<T> &levelset ) {
		//
		bool hasInside (false);
		levelset.interruptible_const_serial_actives([&](int i, int j, int k, const auto &it){
			if( it() < 0.0 ) {
				hasInside = true;
			}
			return hasInside;
		});
		return hasInside;
	}
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
//