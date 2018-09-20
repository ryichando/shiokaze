/*
**	RCMatrix_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Inspired by Robert Bridson's matrix code, re-written by Ryoichi Ando on Feb 7, 2017.
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
#ifndef SHKZ_RCMATRIX_INTERFACE_H
#define SHKZ_RCMATRIX_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/core/console.h>
#include <functional>
#include <vector>
#include <memory>
//
SHKZ_BEGIN_NAMESPACE
//
template <class N, class T> class RCMatrix_vector_interface;
template <class N, class T> class RCFixedMatrix_interface;
template <class N, class T> class RCMatrix_interface;
//
template <class N, class T> using RCMatrix_vector_ptr = std::shared_ptr<RCMatrix_vector_interface<N,T> >;
template <class N, class T> using RCFixedMatrix_ptr = std::shared_ptr<RCFixedMatrix_interface<N,T> >;
template <class N, class T> using RCMatrix_ptr = std::shared_ptr<RCMatrix_interface<N,T> >;
//
/** @file */
/// \~english @brief Interface to provide allocators for Row Compressed Matrix and vector instances.
/// \~japanese @brief 行圧縮の疎行列とベクトルを生成するインターフェース。
template <class N, class T> class RCMatrix_allocator_interface {
public:
	/**
	 \~english @brief Allocate a vector.
	 @param[in] size Dimemsion of size.
	 \~japanese @brief ベクトルを生成する。
	 @param[in] size ベクトルの次元。
	 */
	virtual RCMatrix_vector_ptr<N,T> allocate_vector( N size=0 ) const = 0;
	/**
	 \~english @brief Allocate a matrix.
	 @param[in] rows Number of rows.
	 @param[in] columns Number of columns.
	 \~japanese @brief 行列を生成する。
	 @param[in] rows 行の数。
	 @param[in] columns 列の数。
	 */
	virtual RCMatrix_ptr<N,T> allocate_matrix( N rows=0, N columns=0 ) const = 0;
};
//
/// \~english @brief Interface to provide vector calculations.
/// \~japanese @brief ベクトル計算を提供するインターフェース。
template <class N, class T> class RCMatrix_vector_interface : public RCMatrix_allocator_interface<N,T> {
public:
	/**
	 \~english @brief Copy the vector.
	 @param[in] x Vector from which to copy.
	 \~japanese @brief ベクトルをコピーする。
	 @param[in] x コピーするベクトル。
	 */
	virtual void copy( const RCMatrix_vector_interface<N,T> *x ) {
		resize(x->size());
		x->const_for_each([&]( N row, T value ) {
			set(row,value);
		});
	}
	/**
	 \~english @brief Resize the dimension.
	 @param[in] size New size of dimension.
	 \~japanese @brief 次元をリサイズする。
	 @param[in] size 新しい次元の数。
	 */
	virtual void resize( N size ) = 0;
	/**
	 \~english @brief Get the size of dimension.
	 @return Size of dimension.
	 \~japanese @brief 次元の数を取得する。
	 @return 次元の数。
	 */
	virtual N size() const = 0;
	/**
	 \~english @brief Clear out all the value with the input value. Note that the dimension size of vector remain intact.
	 @param[in] value Value with which to initialize.
	 \~japanese @brief 入力値で全ての要素を初期化する。ベクトルの次元の数は変更されない。
	 @param[in] value 初期化する値。
	 */
	virtual void clear( T value=T() ) = 0;
	/**
	 \~english @brief Get an element value at an input index position.
	 @return Element value.
	 \~japanese @brief 入力のインデックスの位置での要素の値を取得する。
	 @return 要素の値。
	 */
	virtual T at( N index ) const = 0;
	/**
	 \~english @brief Set an element value at an input index position.
	 @param[in] index Index position.
	 @param[in] value Value to set.
	 \~japanese @brief 入力のインデックスの位置での要素の値を設定する。
	 @param[in] index インデックスの位置。
	 @param[in] value セットする値。
	 */
	virtual void set( N index, T value ) = 0;
	/**
	 \~english @brief Add an element value at an input index position.
	 @param[in] index Index position.
	 @param[in] value Value to add.
	 \~japanese @brief 入力のインデックスの位置で要素の値を加算する。
	 @param[in] index インデックスの位置。
	 @param[in] value 加算する値。
	 */
	virtual void add( N index, T value ) = 0;
	/**
	 \~english @brief Subtract an element value at an input index position.
	 @param[in] index Index position.
	 @param[in] value Value to subtract.
	 \~japanese @brief 入力のインデックスの位置で要素の値を減算する。
	 @param[in] index インデックスの位置。
	 @param[in] value 減算する値。
	 */
	virtual void subtract( N index, T value ) = 0;
	/**
	 \~english @brief Multiply an element value at an input index position.
	 @param[in] index Index position.
	 @param[in] value Value to multiply.
	 \~japanese @brief 入力のインデックスの位置で要素の値を乗算する。
	 @param[in] index インデックスの位置。
	 @param[in] value 乗算する値。
	 */
	virtual void multiply( N index, T value ) = 0;
	/**
	 \~english @brief Divide an element value at an input index position.
	 @param[in] index Index position.
	 @param[in] value Value by which to divide.
	 \~japanese @brief 入力のインデックスの位置で要素の値を入力値で割り算する。
	 @param[in] index インデックスの位置。
	 @param[in] value 割り算する値。
	 */
	virtual void divide( N index, T value ) = 0;
	/**
	 \~english @brief Manipulate values in parallel.
	 @param[in] func Manipulation function.
	 \~japanese @brief 並列に要素の値を操作する。
	 @param[in] func 操作を行う関数。
	 */
	virtual void parallel_for_each( std::function<void( N row, T& value )> func) = 0;
	/**
	 \~english @brief Read values in parallel.
	 @param[in] func Function that reads values.
	 \~japanese @brief 要素の値を並列に読み込む。
	 @param[in] func 読み込みを行う関数。
	 */
	virtual void const_parallel_for_each( std::function<void( N row, T value )> func) const = 0;
	/**
	 \~english @brief Manipulate values in serial order.
	 @param[in] func Manipulation function. Return \c true to stop the iteration.
	 \~japanese @brief 逐次的に要素の値を操作する。
	 @param[in] func 操作を行う関数。\c true を返すと、操作を中止する。
	 */
	virtual void interruptible_for_each( std::function<bool( N row, T& value )> func) = 0;
	/**
	 \~english @brief Read values in serial order.
	 @param[in] func Function that reads values. Return \c true to stop the iteration.
	 \~japanese @brief 逐次的に要素の値を読み込む。
	 @param[in] func 読み込みを行う関数。\c true を返すと、操作を中止する。
	 */
	virtual void const_interruptible_for_each( std::function<bool( N row, T value )> func) const = 0;
	/**
	 \~english @brief Compute the uniform norm.
	 @return The uniform norm.
	 \~japanese @brief 一様ノルムを計算する。
	 @return 一様ノルム。
	 */
	virtual T abs_max() const = 0;
	/**
	 \~english @brief Compute the dot product.
	 @param[in] x Input vector.
	 @return Dot product result.
	 \~japanese @brief 内積を計算する。
	 @param[in] x 入力のベクトル。
	 @return 内積の結果。
	 */
	virtual T dot( const RCMatrix_vector_interface<N,T> *x ) const = 0;
	/**
	 \~english @brief Add alpha * x.
	 @param[in] alpha alpha.
	 @param[in] x x.
	 \~japanese @brief alpha * x を加算する。
	 @param[in] alpha alpha.
	 @param[in] x x.
	 */
	virtual void add_scaled( T alpha, const RCMatrix_vector_interface<N,T> *x ) = 0;
	/**
	 \~english @brief Duplicate this vector.
	 @return Duplicated vector.
	 \~japanese @brief ベクトルを複製する。
	 @return 複製されたベクトル。
	 */
	RCMatrix_vector_ptr<N,T> duplicate() const {
		auto result = this->allocate_vector();
		result->copy(this);
		return result;
	}
	/**
	 \~english @brief Manipulate values in serial order.
	 @param[in] func Manipulation function.
	 \~japanese @brief 逐次的に要素の値を操作する。
	 @param[in] func 操作を行う関数。
	 */
	void for_each( std::function<void( N row, T& value )> func) {
		interruptible_for_each([&]( N row, T& value ) {
			func(row,value);
			return false;
		});
	}
	/**
	 \~english @brief Read values in serial order.
	 @param[in] func Function that reads values.
	 \~japanese @brief 要素の値を逐次的に読み込む。
	 @param[in] func 読み込みを行う関数。
	 */
	void const_for_each( std::function<void( N row, T value )> func) const {
		const_interruptible_for_each([&]( N row, T value ) {
			func(row,value);
			return false;
		});
	}
	/**
	 \~english @brief Convert input std::vector to the vector of this type.
	 @param[in] v Input vector.
	 \~japanese @brief 入力の std::vector ベクトルをこの種類のベクトルに変換する。
	 @param[in] v 入力のベクトル。
	 */
	void convert_from( const std::vector<T> &v ) {
		resize(v.size());
		for( N i=0; i<v.size(); ++i ) set(i,v[i]);
	}
	/**
	 \~english @brief Convert to std::vector.
	 @param[out] v Output vector.
	 \~japanese @brief std::vector へ変換する。
	 @param[out] v 出力のベクトル。
	 */
	void convert_to( std::vector<T> &v ) {
		v.resize(size());
		for( N i=0; i<v.size(); ++i ) v[i] = at(i);
	}
};
//
/// \~english @brief Specialized Row Compressed Matrix that efficiently performs matrix-vector calculations.
/// \~japanese @brief 行列とベクトルのかけ算に特化しか行圧縮の行列クラス。
template <class N, class T> class RCFixedMatrix_interface : public RCMatrix_allocator_interface<N,T> {
public:
	/**
	 \~english @brief Apply multiplication to an input vector and substitute to a result vector.
	 @param[in] rhs Input vector to apply.
	 @param[out] result Result vector.
	 \~japanese @brief 入力のベクトルにかけ算を行い、結果を結果ベクトルに代入する。
	 @param[in] rhs 入力のベクトル。
	 @param[out] result 結果のベクトル。
	 */
	virtual void multiply( const RCMatrix_vector_interface<N,T> *rhs, RCMatrix_vector_interface<N,T> *result ) const = 0;
	/**
	 \~english @brief Apply multiplication to an input vector.
	 @param[in-out] rhs Input vector to apply.
	 \~japanese @brief 入力のベクトルにかけ算を行う。
	 @param[in-out] rhs 入力のベクトル。
	 */
	void apply( RCMatrix_vector_interface<N,T> *x ) const {
		auto x_save = allocate_vector(x->size());
		x_save->copy(x);
		multiply(x_save,x);
	}
	/**
	 \~english @brief Apply multiplication to an input vector and substitute to a result vector. Provided to preserve std::vector compatibility.
	 @param[in] rhs Input vector to apply.
	 @param[out] result Result vector.
	 \~japanese @brief 入力のベクトルにかけ算を行い、結果を結果ベクトルに代入する。std::vector との互換性を保つために提供される。
	 @param[in] rhs 入力のベクトル。
	 @param[out] result 結果のベクトル。
	 */
	void multiply(const std::vector<T> &rhs, std::vector<T> &result ) const {
		auto _rhs = allocate_vector(rhs.size()); _rhs->convert_from(rhs);
		auto _result = allocate_vector(rhs.size());
		multiply(_rhs,_result);
		_result->convert_to(result);
	}
};
//
/// \~english @brief Interface for Row Compressed Matrix.
/// \~japanese @brief 行圧縮の疎行列のインターフェース。
template <class N, class T> class RCMatrix_interface : public RCMatrix_allocator_interface<N,T> {
public:
	/**
	 \~english @brief Initialize matrix with rows and columns.
	 @param[in] rows Size of rows.
	 @param[in] columns Size of columns.
	 \~japanese @brief 指定された行と列の大きさと行列を初期化する。
	 @param[in] rows 行の大きさ。
	 @param[in] columns 列の大きさ。
	 */
	virtual void initialize( N rows, N columns ) = 0;
	/**
	 \~english @brief Copy the input matrix.
	 @param[in] m Matrix from which to copy.
	 \~japanese @brief 行列をコピーする。
	 @param[in] m コピーする行列。
	 */
	virtual void copy( const RCMatrix_interface<N,T> *m ) = 0;
	/**
	 \~english @brief Clear the entire row with zeros.
	 @param[in] row Row index position.
	 \~japanese @brief 行をゼロで初期化する。
	 @param[in] row 行のインデックス番号。
	 */
	virtual void clear( N row ) = 0;
	/**
	 \~english @brief Get the element value at the row and the column.
	 @param[in] row Row index position.
	 @param[in] column Column index position.
	 @return Value of the element.
	 \~japanese @brief 指定された行と列の行列を取得する。
	 @param[in] row 行のインデックス番号。
	 @param[in] column 列のインデックス番号。
	 @return 要素の値。
	 */
	virtual T get( N row, N column ) const = 0;
	/**
	 \~english @brief Add a value to an element.
	 @param[in] row Row index position.
	 @param[in] column Column index position.
	 @param[in] increment_value Incremental value.
	 \~japanese @brief 要素に値を加算する。
	 @param[in] row 行のインデックス番号。
	 @param[in] column 列のインデックス番号。
	 @param[in] increment_value 加算する値。
	 */
	virtual void add_to_element( N row, N column, T increment_value ) = 0;
	/**
	 \~english @brief Clear out an element with zero.
	 @param[in] row Row index position.
	 @param[in] column Column index position.
	 \~japanese @brief 要素の値をゼロにする。
	 @param[in] row 行のインデックス番号。
	 @param[in] column 列のインデックス番号。
	 */
	virtual void clear_element( N row, N column ) = 0;
	/**
	 \~english @brief Manipulate values in serial order.
	 @param[in] row Row index position.
	 @param[in] func Manipulation function. Return \c true to stop the iteration.
	 \~japanese @brief 逐次的に要素の値を操作する。
	 @param[in] row 行のインデックス番号。
	 @param[in] func 操作を行う関数。\c true を返すと、操作を中止する。
	 */
	virtual void interruptible_for_each( N row, std::function<bool( N column, T& value )> func) = 0;
	/**
	 \~english @brief Read values in serial order.
	 @param[in] row Row index position.
	 @param[in] func Function that reads values. Return \c true to stop the iteration.
	 \~japanese @brief 逐次的に要素の値を読み込む。
	 @param[in] row 行のインデックス番号。
	 @param[in] func 読み込みを行う関数。\c true を返すと、操作を中止する。
	 */
	virtual void const_interruptible_for_each( N row, std::function<bool( N column, T value )> func) const = 0;
	/**
	 \~english @brief Get the size of rows.
	 @return Size of rows.
	 \~japanese @brief 行の数を取得する。
	 @return 行の数。
	 */
	virtual N rows() const = 0;
	/**
	 \~english @brief Get the size of columns.
	 @return Size of columns.
	 \~japanese @brief 列の数を取得する。
	 @return 列の数。
	 */
	virtual N columns() const = 0;
	/**
	 \~english @brief Get the size of non zero entries in a row.
	 @return Size of non zero entries.
	 \~japanese @brief 行の非ゼロ成分の数を取得する。
	 @return 行の非ゼロ成分の数。
	 */
	virtual N non_zeros( N row ) const = 0;
	/**
	 \~english @brief Duplicate matrix.
	 @return Duplicated matrix.
	 \~japanese @brief 行列を複製する。
	 @return 複製された行列。
	 */
	RCMatrix_ptr<N,T> duplicate() const {
		auto result = this->allocate_matrix();
		result->copy(this);
		return result;
	}
	/**
	 \~english @brief Reset all the elements with zeros.
	 \~japanese @brief 要素を全てゼロで初期化する。
	 */
	void clear() {
		initialize(rows(),columns());
	}
	/**
	 \~english @brief Get the number of all the non-zero entries.
	 @return Number of all the non-zero entries.
	 \~japanese @brief 全ての非ゼロ要素の数を得る。
	 @return 全ての非ゼロ要素の数。
	 */
	N non_zeros() const {
		N sum (0);
		for( N row=0; row<rows(); ++row ) sum += non_zeros(row);
		return sum;
	}
	/**
	 \~english @brief Get if the matrix is empty.
	 @return \c true if the matrix is empty. \c false otherwise.
	 \~japanese @brief 行列が空か取得する。
	 @return もし空なら \c true を、そうでなければ \c false を返す。
	 */
	bool empty() const {
		return non_zeros() == 0;
	}
	/**
	 \~english @brief Get if a row in the matrix is empty.
	 @param[in] row Row index position.
	 @return \c true if the row is empty. \c false otherwise.
	 \~japanese @brief 行列のある行が空か取得する。
	 @param[in] row 列のインデックス番号。
	 @return もし空なら \c true を、そうでなければ \c false を返す。
	 */
	bool empty( N row ) const {
		return non_zeros(row) == 0;
	}
	/**
	 \~english @brief Manipulate elements in serial order.
	 @param[in] row Row index position.
	 @param[in] func Manipulation function.
	 \~japanese @brief 逐次的に要素の値を操作する。
	 @param[in] row 列のインデックス番号。
	 @param[in] func 操作を行う関数。
	 */
	void for_each( N row, std::function<void( N column, T& value )> func) {
		interruptible_for_each(row,[&]( N column, T& value ) {
			func(column,value);
			return false;
		});
	}
	/**
	 \~english @brief Read elements in serial order.
	 @param[in] row Row index position.
	 @param[in] func Function that reads values.
	 \~japanese @brief 要素の値を逐次的に読み込む。
	 @param[in] row 列のインデックス番号。
	 @param[in] func 読み込みを行う関数。
	 */
	void const_for_each( N row, std::function<void( N column, T value )> func) const {
		const_interruptible_for_each(row,[&]( N column, T value ) {
			func(column,value);
			return false;
		});
	}
	/**
	 \~english @brief Multiply a value to all the elements.
	 @param[in] value Value to multiply.
	 \~japanese @brief 全ての要素に値を乗算する。
	 @param[in] value 乗算する値。
	 */
	virtual void multiply(T value) = 0;
	/**
	 \~english @brief Apply multiplication to an input vector and substitute to a result vector.
	 @param[in] rhs Input vector to apply.
	 @param[out] result Result vector.
	 \~japanese @brief 入力のベクトルにかけ算を行い、結果を結果ベクトルに代入する。
	 @param[in] rhs 入力のベクトル。
	 @param[out] result 結果のベクトル。
	 */
	virtual void multiply( const RCMatrix_vector_interface<N,T> *rhs, RCMatrix_vector_interface<N,T> *result ) const = 0;
	/**
	 \~english @brief Apply multiplication to an input matrix of the form: [result] = [self][m].
	 @param[in] rhs Input matrix to apply.
	 @param[out] result Result matrix.
	 \~japanese @brief [result] = [self][m] のように入力の行列にかけ算を行う。
	 @param[in] rhs 入力の行列。
	 @param[out] result 結果の行列。
	 */
	virtual void multiply( const RCMatrix_interface<N,T> *m, RCMatrix_interface<N,T> *result ) const = 0;
	/**
	 \~english @brief Add a matrix.
	 @param[in] m Matrix to add.
	 @param[out] result Result.
	 \~japanese @brief 行列を加算する。
	 @param[in] m 加算する行列。
	 @param[out] result 結果。
	 */
	virtual void add( const RCMatrix_interface<N,T> *m, RCMatrix_interface<N,T> *result ) const = 0;
	/**
	 \~english @brief Transpose this matrix.
	 @param[out] result Result.
	 \~japanese @brief 転置行列を計算する。
	 @param[out] result 結果。
	 */
	virtual void transpose( RCMatrix_interface<N,T> *result ) const = 0;
	/**
	 \~english @brief Make a fixed matrix.
	 @return Generated fixed matrix.
	 \~japanese @brief 固定行列を生成する。
	 @return 生成された固定行列。
	 */
	virtual RCFixedMatrix_ptr<N,T> make_fixed() const = 0;
	/**
	 \~english @brief Apply multiplication to an input vector.
	 @param[in-out] rhs Input vector to apply.
	 \~japanese @brief 入力のベクトルにかけ算を行う。
	 @param[in-out] rhs 入力のベクトル。
	 */
	RCMatrix_vector_ptr<N,T> multiply( const RCMatrix_vector_interface<N,T> *rhs ) const {
		assert( rhs->size() == columns());
		auto result = this->allocate_vector();
		multiply(rhs,result.get());
		return result;
	}
	/**
	 \~english @brief Apply multiplication to an input matrix of the form: [result] = [self][m].
	 @param[in-out] rhs Input matrix to apply.
	 @retrun Resulting matrix.
	 \~japanese @brief [result] = [self][m] のように入力の行列にかけ算を行う。
	 @param[in-out] rhs 入力の行列。
	 @return 結果の行列。
	 */
	RCMatrix_ptr<N,T> multiply( RCMatrix_interface<N,T> *m ) const {
		auto result = this->allocate_matrix();
		multiply(m,result.get());
		return result;
	}
	/**
	 \~english @brief Add to an input matrix.
	 @param[in-out] rhs Input matrix to add.
	 @retrun Resulting matrix.
	 \~japanese @brief 入力の行列を加算する。
	 @param[in-out] rhs 入力の行列。
	 @return 結果の行列。
	 */
	RCMatrix_ptr<N,T> add( RCMatrix_interface<N,T> *m ) const {
		auto result = this->allocate_matrix();
		add(m,result.get());
		return result;
	}
	/**
	 \~english @brief Transpose this matrix.
	 @return result Result.
	 \~japanese @brief 転置行列を計算する。
	 @return result 結果。
	 */
	RCMatrix_ptr<N,T> transpose() const {
		auto result = this->allocate_matrix();
		transpose(result.get());
		return result;
	}
	/**
	 \~english @brief Apply multiplication to an input vector and substitute to a result vector. Provided to preserve std::vector compatibility.
	 @param[in] rhs Input vector to apply.
	 @param[out] result Result vector.
	 \~japanese @brief 入力のベクトルにかけ算を行い、結果を結果ベクトルに代入する。std::vector との互換性を保つために提供される。
	 @param[in] rhs 入力のベクトル。
	 @param[out] result 結果のベクトル。
	 */
	virtual void multiply( const std::vector<T> &rhs, std::vector<T> &result ) const {
		assert( rhs.size() == columns());
		auto _rhs = this->allocate_vector(rows()); _rhs->convert_from(rhs);
		auto _result = this->allocate_vector(rows());
		multiply(_rhs.get(),_result.get());
		_result->convert_to(result);
	}
	/**
	 \~english @brief Apply multiplication to an input vector. Provided to preserve std::vector compatibility.
	 @param[in-out] rhs Input vector to apply.
	 \~japanese @brief 入力のベクトルにかけ算を行う。std::vector との互換性を保つために提供される。
	 @param[in-out] rhs 入力のベクトル。
	 */
	std::vector<T> multiply( const std::vector<T> &rhs ) const {
		assert( rhs.size() == columns());
		std::vector<T> result;
		multiply(rhs,result);
		return result;
	}
};
//
/// \~english @brief Interface for creating Row Compressed Matrix and vector instances. "RCMatrix" is provided as implementation for the type of T=double, N=size_t.
/// \~japanese @brief 行圧縮の疎行列とベクトルを生成するインターフェース。
template <class N, class T> class RCMatrix_factory_interface : public recursive_configurable_module, public RCMatrix_allocator_interface<N,T> {
public:
	//
	DEFINE_MODULE(RCMatrix_factory_interface,"Row Compressed Matrix Factory","RCMatrix","Row compressed matrix module")
	//
};
//
template <class N, class T> using RCMatrix_factory_driver = recursive_configurable_driver<RCMatrix_factory_interface<N,T> >;
//
SHKZ_END_NAMESPACE
//
#endif
//
