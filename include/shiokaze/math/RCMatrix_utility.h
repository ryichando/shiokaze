/*
**	RCMatrix_utility.h
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
#ifndef SHKZ_RCMATRIX_UTILITY_H
#define SHKZ_RCMATRIX_UTILITY_H
//
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/core/console.h>
#include <cmath>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that provides utility functions for RCMatrix_interface.
/// \~japanese @brief RCMatrix_interface のためのユーティリティ関数を提供するクラス。
template <class N, class T> class RCMatrix_utility {
public:
	/**
	 \~english @brief Generate a diagonal matrix.
	 @param[out] matrix Resulting diagonal matrix.
	 @param[in] diag Diagonal entries.
	 \~japanese @brief 対角行列を生成する。
	 @param[out] matrix 結果の対角行列。
	 @param[in] diag 対角行列の成分。
	 */
	static void diag( const RCMatrix_interface<N,T> *matrix, const std::vector<T> &diag ) {
		matrix->initialize(diag.size(),diag.size());
		for( N row=0; row<diag.size(); ++row ) matrix->add_to_element(row,row,diag[row]);
	}
	/**
	 \~english @brief Measure the uniform norm of the symmetericity error.
	 @param[in] matrix Matrix to measure the error.
	 @return The uniform norm.
	 \~japanese @brief 行列が対称行列からどれくらい離れているかの一様ノルムを計算する。
	 @param[in] matrix 検証する行列。
	 @return 一様ノルム。
	 */
	static T symmetricity_error ( const RCMatrix_interface<N,T> *matrix ) {
		T max_error (0.0);
		for( N row=0; row<matrix->rows(); ++row ) {
			matrix->const_for_each(row,[&]( N column, T value ) {
				if( column < matrix->rows()) {
					max_error = std::max(max_error,std::abs(value-matrix->get(column,row)));
				} else {
					max_error = std::max(max_error,std::abs(value));
				}
			});
		}
		return max_error;
	}
	/**
	 \~english @brief Report matrix properties.
	 @param[in] matrix Matrix to examine.
	 @param[in] name Name of the matrix used in the reporting text.
	 @return \c if the matrix is symmetric and diagonals are all positive, \c false otherwise.
	 \~japanese @brief 行列の性質について調べて報告する。
	 @param[in] matrix 調べる行列。
	 @param[in] name 報告文で使われる行列の名前。
	 @return もし行列が対称行列で対角項が全て正なら、\c true を、そうでなければ \c false を返す。
	 */
	static bool report( const RCMatrix_interface<N,T> *matrix, std::string name ) {
		//
		N active(0), max_row(0), min_row(std::numeric_limits<N>::max()), active_rows(0);
		T avg_row(0.0), max_diag(0.0), min_diag(std::numeric_limits<T>::max());
		T diag_ratio(0.0);
		bool symm_postive_diag (true);
		bool has_nan (false);
		for( N i=0; i<matrix->rows(); i++ ) {
			if( ! matrix->empty(i) ) {
				N row_nonzero = matrix->non_zeros(i);
				active += row_nonzero;
				max_row = std::max(max_row,row_nonzero);
				min_row = std::min(min_row,row_nonzero);
				avg_row += row_nonzero;
				T diag (0.0);
				T max_nondiag (0.0);
				matrix->const_for_each(i,[&]( N column, T value ) {
					if( column == i ) diag = value;
					else max_nondiag = std::max(max_nondiag,std::abs(value));
					if( value!=value ) has_nan = true;
				});
				max_diag = std::max(max_diag,diag);
				min_diag = std::min(min_diag,diag);
				if( diag ) diag_ratio = std::max( diag_ratio, max_nondiag / diag );
				active_rows ++;
			}
		}
		if( active_rows ) avg_row /= (T)active_rows;
		double symmetric_error = symmetricity_error(matrix);
		console::dump( ">>> ==== Matrix [%s] analysis ====\n", name.c_str());
		console::dump( "Matrix dimension = %dx%d\n", matrix->rows(), matrix->columns() );
		console::dump( "Matrix active row size = %d\n", active_rows );
		console::dump( "Matrix nonzero entries = %d\n", active );
		console::dump( "Matrix maximal row = %d\n", max_row );
		console::dump( "Matrix minimal row = %d\n", min_row );
		console::dump( "Matrix row average = %.2f\n", avg_row );
		console::dump( "Matrix max diag = %.2e\n", max_diag );
		console::dump( "Matrix min diag = %.2e\n", min_diag );
		console::dump( "Matrix worst max(non_diag) / diag = %.2e\n", diag_ratio );
		console::dump( "Matrix max(symmetricity error) = %.2e\n", symmetric_error );
		console::dump( "Matrix has_NaN = %s\n", has_nan ? "Yes" : "No");
		console::dump( "<<< =========================\n" );
		if( min_diag < 0.0 ) {
			console::dump( "WARNING: min_diag < 0.0\n");
			symm_postive_diag = false;
		}
		if( symmetric_error ) symm_postive_diag = false;
		return symm_postive_diag;
	}
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
//