/*
**	pcg.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 1, 2018. 
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
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include <cstdint>
#include <cmath>
#include "pcgsolver/pcg_solver.h"
//
SHKZ_USING_NAMESPACE
//
template <class N, class T> class pcg_solver : public RCMatrix_solver_interface<N,T> {
protected:
	//
	LONG_NAME("Preconditioned Conjugate Gradient Solver")
	AUTHOR_NAME("Robert Bridson")
	ARGUMENT_NAME("PCG")
	MODULE_NAME("pcg_solver")
	//
	virtual void configure( configuration &config ) override {
		config.get_double("Residual",m_param.residual,"Tolerable residual");
		config.get_double("ModifiedIC",m_param.modified_incomplete_cholesky_parameter,"Modified incomplete cholesky");
		config.get_double("MinDiagRatio",m_param.min_diagonal_ratio,"Minimal diagonal ratio");
		config.get_unsigned("MaxIterations",m_param.max_iterations,"Maximal iteration count");
	}
	virtual typename RCMatrix_solver_interface<N,T>::Result solve( const RCMatrix_interface<N,T> *A, const RCMatrix_vector_interface<N,T> *b, RCMatrix_vector_interface<N,T> *x ) const override {
		//
		SparseMatrix<T> matrix(A->rows());
		for( N row=0; row<matrix.n; ++row ) {
			A->const_for_each(row,[&]( N column, T value ) {
				matrix.add_to_element(row,column,value);
			});
		}
		//
		std::vector<T> rhs;
		b->convert_to(rhs);
		//
		std::vector<T> result(matrix.n);
		T residual_out;
		int iterations_out;
		PCGSolver<T> solver;
		//
		solver.set_solver_parameters(
			m_param.residual,
			m_param.max_iterations,
			m_param.modified_incomplete_cholesky_parameter,
			m_param.min_diagonal_ratio
		);
		//
		solver.solve(matrix,rhs,result,residual_out,iterations_out);
		x->convert_from(result);
		//
		return {(N)iterations_out,(T)residual_out};
	}
	//
	struct Parameters {
		double residual {1e-4};
		unsigned max_iterations {30000};
		double modified_incomplete_cholesky_parameter {0.97};
		double min_diagonal_ratio {0.25};
	};
	Parameters m_param;
};
//
extern "C" module * create_instance() {
	return new pcg_solver<INDEX_TYPE,FLOAT_TYPE>();
}
//
extern "C" const char *license() {
	return "Public domain";
}
//