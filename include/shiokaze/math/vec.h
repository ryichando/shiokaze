/*
**	vec.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Dec 29, 2017. 
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
#ifndef SHKZ_VEC_H
#define SHKZ_VEC_H
//
#include <cmath>
#include <ostream>
#include <cassert>
#include <shiokaze/core/common.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Fixed sized vector structure.
/// \~japanese @brief 固定サイズのベクトル構造体。
template <class T, unsigned D> struct vec {
	//
	/**
	 \~english @brief Vector value array.
	 \~japanese @brief ベクトルの値の配列。
	 */
	T v[D];
	/**
	 \~english @brief Default constructor.
	 \~japanese @brief デフォルトコンストラクタ。
	 */
	vec() {
		for( unsigned dim=0; dim<D; ++dim ) this->v[dim] = T();
	}
	/**
	 \~english @brief Copy constructor.
	 @param[in] v C array to the vector values from which to copy.
	 \~japanese @brief コピーコンストラクタ。
	 @param[in] v コピーするベクトルの C 配列の値。
	 */
	template<class Y>  vec( const Y *v ) {
		for( unsigned dim=0; dim<D; ++dim ) this->v[dim] = v[dim];
	}
	/**
	 \~english @brief Copy constructor.
	 @param[in] v A vector from which to copy.
	 \~japanese @brief コピーコンストラクタ。
	 @param[in] v コピーするベクトル。
	 */
	template<class Y> vec( const vec<Y,D> &v) {
		for( unsigned dim=0; dim<D; ++dim ) this->v[dim] = v.v[dim];
	}
	/**
	 \~english @brief Constructor.
	 @param[in] v Value with which to initialize.
	 \~japanese @brief コンストラクタ。
	 @param[in] v 初期化する値。
	 */
	vec( T v ) {
		for( unsigned dim=0; dim<D; ++dim ) this->v[dim] = v;
	}
	/**
	 \~english @brief Constructor for two dimensional vector.
	 @param[in] x Value for the first dimension.
	 @param[in] y Value for the second dimension.
	 \~japanese @brief 2次元ベクトルのコンストラクタ。
	 @param[in] x 1番目の次元の値。
	 @param[in] y 2番目の次元の値。
	 */
	vec( T x, T y ) {
		assert( D == 2 );
		v[0] = x; v[1] = y;
	}
	/**
	 \~english @brief Constructor for three dimensional vector.
	 @param[in] x Value for the first dimension.
	 @param[in] y Value for the second dimension.
	 @param[in] z Value for the third dimension.
	 \~japanese @brief 3次元ベクトルのコンストラクタ。
	 @param[in] x 1番目の次元の値。
	 @param[in] y 2番目の次元の値。
	 @param[in] z 3番目の次元の値。
	 */
	vec( T x, T y, T z ) {
		assert( D == 3 );
		v[0] = x; v[1] = y;	v[2] = z;
	}
	/**
	 \~english @brief Get the value at a specified dimension.
	 @param[in] idx Index position.
	 @return Value at the position.
	 \~japanese @brief 指定された次元の値を取得する。
	 @param[in] idx インデックス番号。
	 @return 入力インデックス位置での値。
	 */
	const T &operator[](unsigned idx) const {
		return v[idx];
	}
	/**
	 \~english @brief Get the reference to the value at a specified dimension.
	 @param[in] idx Index position.
	 @return Reference of the value at the position.
	 \~japanese @brief 指定された次元の値の参照を取得する。
	 @param[in] idx インデックス番号。
	 @return 入力インデックス位置での値の参照。
	 */
	T &operator[](unsigned idx) {
		return v[idx];
	}
	/**
	 \~english @brief Get if all the elements in the vector is empty.
	 @return \c true if all the elements in the vector is zero. \c false otherwise.
	 \~japanese @brief ベクトルの全ての要素がゼロか取得する。
	 @return もしベクトルの全ての要素がゼロなら \c true を、そうでなければ \c false を返す。
	 */
	bool empty() const {
		for( unsigned dim=0; dim<D; ++dim ) if( v[dim] ) return false;
		return true;
	}
	/**
	 \~english @brief Get if the vector is equal to the input vector.
	 @return \c true if the vector is equal to the input vector. \c false otherwise.
	 \~japanese @brief ベクトルが入力のベクトルと同じか取得する。
	 @return もしベクトルが入力のベクトルと同じなら \c true を、そうでなければ \c false を返す。
	 */
	bool operator==(const vec &v) const {
		for( unsigned dim=0; dim<D; ++dim ) if( this->v[dim] != v[dim]) return false;
		return true;
	}
	/**
	 \~english @brief Get if the vector is not equal to the input vector.
	 @return \c true if the vector is not equal to the input vector. \c false otherwise.
	 \~japanese @brief ベクトルが入力のベクトルと違うか取得する。
	 @return もしベクトルが入力のベクトルと違うなら \c true を、そうでなければ \c false を返す。
	 */
	bool operator!=(const vec &v) const {
		return ! (*this == v);
	}
	/**
	 \~english @brief Add a vector.
	 @param[in] v Vector to add.
	 @return Added vector.
	 \~japanese @brief ベクトルを加算する。
	 @param[in] v 加算するベクトル。
	 @return 加算されたベクトル。
	 */
	template<class Y> vec<T,D> operator+(const vec<Y,D> &v) const {
		struct vec result(*this);
		for( unsigned dim=0; dim<D; ++dim ) result[dim] += v[dim];
		return result;
	}
	/**
	 \~english @brief Subtract a vector.
	 @param[in] v Vector to subtract.
	 @return Subtracted vector.
	 \~japanese @brief ベクトルを減算する。
	 @param[in] v 減算するベクトル。
	 @return 減算されたベクトル。
	 */
	template<class Y> vec<T,D> operator-(const vec<Y,D> &v) const {
		struct vec result(*this);
		for( unsigned dim=0; dim<D; ++dim ) result[dim] -= v[dim];
		return result;
	}
	/**
	 \~english @brief Add a vector.
	 @param[in] v Vector to add.
	 \~japanese @brief ベクトルを加算する。
	 @param[in] v 加算するベクトル。
	 */
	template<class Y> void operator+=(const vec<Y,D> &v) {
		for( unsigned dim=0; dim<D; ++dim ) this->v[dim] += v[dim];
	}
	/**
	 \~english @brief Subtract a vector.
	 @param[in] v Vector to subtract.
	 \~japanese @brief ベクトルを減算する。
	 @param[in] v 減算するベクトル。
	 */
	template<class Y> void operator-=(const vec<Y,D> &v) {
		for( unsigned dim=0; dim<D; ++dim ) this->v[dim] -= v[dim];
	}
	/**
	 \~english @brief Multiply a value.
	 @param[in] s Value to multiply.
	 @return Result.
	 \~japanese @brief 値を乗算する。
	 @param[in] s 乗算する値。
	 @return 結果。
	 */
	template<class Y> vec operator*(Y s) const {
		struct vec result(*this);
		for( unsigned dim=0; dim<D; ++dim ) result[dim] *= s;
		return result;
	}
	/**
	 \~english @brief Divide by a value.
	 @param[in] s Value to divide.
	 @return Result.
	 \~japanese @brief 値で割り算をする。
	 @param[in] s 割り算をする値。
	 @return 結果。
	 */
	template<class Y> vec operator/(Y s) const {
		struct vec result(*this);
		for( unsigned dim=0; dim<D; ++dim ) result[dim] /= s;
		return result;
	}
	/**
	 \~english @brief Multiply a value.
	 @param[in] s Value to multiply.
	 \~japanese @brief 値を乗算する。
	 @param[in] s 乗算する値。
	 */
	template<class Y> void operator*=(Y s) {
		for( unsigned dim=0; dim<D; ++dim ) v[dim] *= s;
	}
	/**
	 \~english @brief Divide by a value.
	 @param[in] s Value to divide.
	 \~japanese @brief 値で割り算をする。
	 @param[in] s 割り算をする値。
	 */
	template<class Y> void operator/=(Y s) {
		for( unsigned dim=0; dim<D; ++dim ) v[dim] /= s;
	}
	/**
	 \~english @brief Compute the dot product.
	 @param[in] v Input vector.
	 @return Dot product result.
	 \~japanese @brief 内積を計算する。
	 @param[in] v 入力のベクトル。
	 @return 内積の結果。
	 */
	template<class Y> T operator*(vec<Y,D> v) const {
		T result = T();
		for( unsigned dim=0; dim<D; ++dim ) result += this->v[dim]*v[dim];
		return result;
	}
	/**
	 \~english @brief Compute L2 norm.
	 @return L2 norm.
	 \~japanese @brief L2 ノルムを計算する。
	 @return L2 ノルム。
	 */
	double norm2() const {
		double result = T();
		for( unsigned dim=0; dim<D; ++dim ) result += this->v[dim]*this->v[dim];
		return result;
	}
	/**
	 \~english @brief Compute L-inf norm.
	 @return L-inf norm.
	 \~japanese @brief L-inf ノルムを計算する。
	 @return L-inf ノルム。
	 */
	double norm_inf() const {
		double result = T();
		for( unsigned dim=0; dim<D; ++dim ) result = std::max(result,(double)std::abs(this->v[dim]));
		return result;
	}
	/**
	 \~english @brief Compute L1 norm.
	 @return L1 norm.
	 \~japanese @brief L1 ノルムを計算する。
	 @return L1 ノルム。
	 */
	double len() const {
		return sqrt(norm2());
	}
	/**
	 \~english @brief Compute normalized vector.
	 @return Normalized vector.
	 \~japanese @brief 正規化ベクトルを計算する。
	 @return 正規化されたベクトル。
	 */
	vec normal() const {
		vec copy = *this;
		copy.normalize();
		return copy;
	}
	/**
	 \~english @brief Normaliz vector.
	 \~japanese @brief ベクトルを正規化する。
	 */
	bool normalize() {
		T length = len();
		if( length ) {
			for( unsigned dim=0; dim<D; ++dim ) v[dim] /= length;
		}
		return length > 0.0;
	}
	/**
	 \~english @brief Rotate vector 90 degrees counterclockwise.
	 @return Rotated vector.
	 \~japanese @brief 反時計回りにベクトルを90度回転させる。
	 @return 回転されたベクトル。
	 */
	vec rotate90() const {
		assert( D == 2 );
		struct vec result;
		result[0] = -v[1];
		result[1] = v[0];
		return result;
	}
	/**
	 \~english @brief Compute the cross product for two dimensions.
	 @param[in] r Two dimensoinal vector.
	 @return Result.
	 \~japanese @brief 2次元ベクトル同士の外積を計算する。
	 @param[in] r 2次元ベクトル。
	 @return 結果。
	 */
	template<class Y=Real> T cross2(const vec<Y,2> &r) const {
		assert( D == 2 );
		return v[0]*r[1]-v[1]*r[0];
	}
	/**
	 \~english @brief Compute the cross product for three dimensions.
	 @param[in] r Three dimensoinal vector.
	 @return Result.
	 \~japanese @brief 3次元ベクトルの外積を計算する。
	 @param[in] r 2次元ベクトル。
	 @return 結果。
	 */
	template<class Y=Real> vec<T,3> cross(const vec<Y,3> &r) const {
		assert( D == 3 );
		return vec<T,3>(v[1]*r[2]-v[2]*r[1],v[2]*r[0]-v[0]*r[2],v[0]*r[1]-v[1]*r[0]);
	}
	/**
	 \~english @brief Compute the position of the center of a cell from the index space.
	 @return Cell center position.
	 \~japanese @brief インデックス位置からセルの中心位置を計算する。
	 @return セルの中心の位置。
	 */
	template<class Y=Real> vec<Y,D> cell() const {
		vec<Y,D> result;
		for( int dim=0; dim<D; ++dim ) result[dim] = v[dim]+0.5;
		return result;
	}
	/**
	 \~english @brief Compute the position of the nodal position from the index space.
	 @return Nodal position.
	 \~japanese @brief インデックス位置からセルの節点位置を計算する。
	 @return セルの節点の位置。
	 */
	template<class Y=Real> vec<Y,D> nodal() const {
		vec<Y,D> result;
		for( int dim=0; dim<D; ++dim ) result[dim] = v[dim];
		return result;
	}
	/**
	 \~english @brief Compute the position of the center of face position from the index space.
	 @param[in] dim Dimension of the face.
	 @return Face center position.
	 \~japanese @brief インデックス位置からセルの面の中心位置を計算する。
	 @param[in] dim 面の向きの次元番号。
	 @return セルの面の中心位置。
	 */
	template<class Y=Real> vec<Y,D> face( int dim ) const {
		vec<Y,D> result;
		for( int _dim=0; _dim<D; ++_dim ) result[_dim] = v[_dim]+0.5*(dim!=_dim);
		return result;
	}
	/**
	 \~english @brief Compute the position of the center of edge position from the index space.
	 @param[in] dim Dimension of the edge.
	 @return Edge center position.
	 \~japanese @brief インデックス位置からエッジの中心位置を計算する。
	 @param[in] dim エッジの向きの次元番号。
	 @return エッジの中心位置。
	 */
	template<class Y=Real> vec<Y,D> edge( int dim ) const {
		assert( D == 3 );
		return vec<Y,3>(v[0]+0.5*(dim==0),v[1]+0.5*(dim==1),v[2]+0.5*(dim==2));
	}
};
template <class T, class Y> static inline T operator^(const vec<T,2> &l, const vec<Y,2> &r) {
	return l.cross2(r);
}
template <class T, class Y> static inline vec<T,3> operator^(const vec<T,3> &l, const vec<Y,3> &r) {
	return l.cross(r);
}
template <class Y, class T, unsigned D> static inline vec<T,D> operator*(Y s, const vec<T,D> &vec) {
	return vec*s;
}
template <class T, unsigned D> static inline vec<T,D> operator-(const vec<T,D> &v) {
	return -1.0 * v;
}
//
template <class T> using vec2 = vec<T,2>;
template <class T> using vec3 = vec<T,3>;
//
using vec2r = vec2<Real>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;
using vec2i = vec2<int>;
using vec3r = vec3<Real>;
using vec3f = vec3<float>;
using vec3d = vec3<double>;
using vec3i = vec3<int>;
//
SHKZ_END_NAMESPACE
#endif