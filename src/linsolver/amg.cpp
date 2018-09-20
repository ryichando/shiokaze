/*
**	amg.cpp
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
#include <amgcl/amg.hpp>
#include <amgcl/backend/builtin.hpp>
#include <amgcl/adapter/crs_builder.hpp>
#include <amgcl/make_solver.hpp>
#include <amgcl/adapter/crs_tuple.hpp>
#include <amgcl/coarsening/smoothed_aggregation.hpp>
#include <amgcl/relaxation/gauss_seidel.hpp>
#include <amgcl/solver/cg.hpp>
//
#include <shiokaze/linsolver/RCMatrix_solver.h>
//
SHKZ_USING_NAMESPACE
//
template <class N, class T> class amg_solver : public RCMatrix_solver_interface<N,T> {
private:
	//
	LONG_NAME("Algebraic Multigrid Solver")
	AUTHOR_NAME("Denis Demidov")
	ARGUMENT_NAME("AMG")
	//
	typedef amgcl::amg<
		amgcl::backend::builtin<T>,
		amgcl::coarsening::smoothed_aggregation,
		amgcl::relaxation::gauss_seidel
	> AMG;
	//
	virtual void configure( configuration &config ) override {
		config.get_double("Residual",m_param.residual,"Tolerable residual");
		config.get_unsigned("MaxIterations",m_param.max_iterations,"Maximal iteration count");
	}
	virtual unsigned solve( const RCMatrix_interface<N,T> *A, const RCMatrix_vector_interface<N,T> *b, RCMatrix_vector_interface<N,T> *x ) const override {
		//
		std::vector<N> rowstart;
		std::vector<N> index;
		std::vector<T> value;
		//
		size_t rows = A->rows();
		rowstart.resize(rows+1);
		rowstart[0] = 0;
		for(N i=0; i<rows; i++ ) {
			rowstart[i+1]=rowstart[i]+A->non_zeros(i);
		}
		value.resize(rowstart[rows]);
		index.resize(rowstart[rows]);
		N j (0);
		for( N i=0; i<rows; i++) {
			A->const_for_each(i,[&]( N column, T v ) {
				index[j] = column;
				value[j] = v;
				++ j;
			});
		}
		//
		AMG amg(std::tie(rows,rowstart,index,value));
		std::vector<T> rhs;
		std::vector<T> result(rows);
		b->convert_to(rhs);
		int iteration_count (0);
		//
		typedef amgcl::solver::cg<amgcl::backend::builtin<T> > Solver;
		typename Solver::params param;
		param.maxiter = m_param.max_iterations;
		param.tol = m_param.residual;
		Solver solve(rows,param);
		//
		// Solve the system. Returns number of iterations made and the achieved residual.
		double error;
		std::tie(iteration_count,error) = solve(amg,rhs,result);
		//
		x->convert_from(result);
		return iteration_count;
	}
	//
	struct Parameters {
		double residual {1e-4};
		unsigned max_iterations {300};
	};
	Parameters m_param;
};
//
extern "C" module * create_instance() {
	return new amg_solver<INDEX_TYPE,FLOAT_TYPE>();
}
//
extern "C" const char *license() {
	return "MIT";
}
//