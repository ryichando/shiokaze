/*
**	RCMatrix_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on May 22, 2018.
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
#ifndef SHKZ_RCMATRIX_SOLVER_INETERFACE_H
#define SHKZ_RCMATRIX_SOLVER_INETERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/RCMatrix_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that solves a sparse linear system. "cg", "pcg" and "amg" are provided as implementations for the type of T=double and N=size_t.
/// \~japanese @brief 疎行列で構成される線形一次方程式を解くためのインターフェース。"cg" と "pcg"、"amg" が T=double and N=size_t の実装として提供される。
template <class N, class T> class RCMatrix_solver_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(RCMatrix_solver_interface,"Linear System Solver","LinSolver","Linear system solver engine")
	/**
	 \~english @brief Solve a linear system of the form: Ax = b.
	 @param[in] A Sparse Row Compressed Matrix.
	 @param[in] b Right hand side vector.
	 @param[in] x Solution vector.
	 \~japanese @brief Ax = b で表される線形一次方程式を解く。
	 @param[in] A 行圧縮の疎行列。
	 @param[in] b 右側のベクトル。
	 @param[in] x 解となるベクトル。
	 */
	virtual unsigned solve( const RCMatrix_ptr<N,T> &A, const RCMatrix_vector_ptr<N,T> b, RCMatrix_vector_ptr<N,T> x ) const = 0;
	/**
	 \~english @brief Solve a linear system of the form: Ax = b. Provided to preserve std::vector compatibility.
	 @param[in] A Sparse Row Compressed Matrix.
	 @param[in] b Right hand side vector.
	 @param[in] x Solution vector.
	 \~japanese @brief Ax = b で表される線形一次方程式を解く。std::vector の互換性のため用意される。
	 @param[in] A 行圧縮の疎行列。
	 @param[in] b 右側のベクトル。
	 @param[in] x 解となるベクトル。
	 */
	inline unsigned solve( const RCMatrix_ptr<N,T> &A, const std::vector<T> &b, std::vector<T> &x ) const {
		//
		auto _b = A->allocate_vector(); _b->convert_from(b);
		auto _x = A->allocate_vector(b.size());
		unsigned count = solve(A,_b,_x);
		_x->convert_to(x);
		return count;
	}
};
//
template <class N, class T> using RCMatrix_solver_driver = recursive_configurable_driver<RCMatrix_solver_interface<N,T> >;
//
SHKZ_END_NAMESPACE
//
#endif
//