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
#include <amgcl/solver/bicgstab.hpp>
#include <amgcl/solver/bicgstabl.hpp>
#include <amgcl/solver/gmres.hpp>
#include <amgcl/solver/lgmres.hpp>
#include <amgcl/solver/fgmres.hpp>
#include <shiokaze/linsolver/RCMatrix_solver.h>
//
SHKZ_USING_NAMESPACE
//
template <class N, class T> class amg_solver : public RCMatrix_solver_interface<N,T> {
protected:
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
		config.get_double("AbsResidual",m_param.abs_residual,"Absolute tolerable residual");
		config.get_unsigned("MaxIterations",m_param.max_iterations,"Maximal iteration count");
		config.get_string("Solver",m_param.method,"Solver name");
		config.get_bool("ForceGlobalResidual",m_param.force_global_residual,"Force using the global residual");
	}
	virtual void register_vector_norm_kind( const std::vector<unsigned char> &kind ) override {
		m_kind = &kind;
	}
	virtual typename RCMatrix_solver_interface<N,T>::Result solve( const RCMatrix_interface<N,T> *A, const RCMatrix_vector_interface<N,T> *b, RCMatrix_vector_interface<N,T> *x ) const override {
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
		unsigned iteration_count (0);
		double reresid;
		std::vector<T> vector_reresid, vector_absresid;
		//
		auto set_param = [&]( auto &param ) {
			param.maxiter = m_param.max_iterations;
			param.tol = m_param.residual;
			param.abstol = m_param.abs_residual;
		};
		//
		if( m_param.method == "CG") {
			typedef amgcl::solver::cg<amgcl::backend::builtin<T> > Solver;
			typename Solver::params param; set_param(param);
			Solver solve(rows,param);
			std::tie(iteration_count,reresid) = solve(amg,rhs,result);
		} else if( m_param.method == "BICGSTAB") {
			typedef amgcl::solver::bicgstab<amgcl::backend::builtin<T> > Solver;
			typename Solver::params param; set_param(param);
			Solver solve(rows,param);
			std::tie(iteration_count,reresid) = solve(amg,rhs,result);
		} else if( m_param.method == "BICGSTABL") {
			typedef amgcl::solver::bicgstabl<amgcl::backend::builtin<T> > Solver;
			typename Solver::params param; set_param(param);
			param.force_global_residual = m_param.force_global_residual;
			Solver solve(rows,param);
			if( m_kind ) solve.kind = m_kind;
			std::tie(iteration_count,reresid) = solve(amg,rhs,result);
			if( m_kind ) {
				vector_reresid = solve.vector_reresid;
				vector_absresid = solve.vector_absresid;
			}
		} else if( m_param.method == "GMRES") {
			typedef amgcl::solver::gmres<amgcl::backend::builtin<T> > Solver;
			typename Solver::params param; set_param(param);
			Solver solve(rows,param);
			std::tie(iteration_count,reresid) = solve(amg,rhs,result);
		} else if( m_param.method == "FGMRES") {
			typedef amgcl::solver::fgmres<amgcl::backend::builtin<T> > Solver;
			typename Solver::params param; set_param(param);
			Solver solve(rows,param);
			std::tie(iteration_count,reresid) = solve(amg,rhs,result);
		} else if( m_param.method == "LGMRES") {
			typedef amgcl::solver::lgmres<amgcl::backend::builtin<T> > Solver;
			typename Solver::params param; set_param(param);
			Solver solve(rows,param);
			std::tie(iteration_count,reresid) = solve(amg,rhs,result);
		} else {
			printf( "Unknown solver %s\n", m_param.method.c_str());
			exit(0);
		}
		//
		x->convert_from(result);
		return {(N)iteration_count,(T)reresid,vector_reresid,vector_absresid};
	}
	//
	struct Parameters {
		double residual {1e-4};
		double abs_residual {1e-20};
		unsigned max_iterations {300};
		std::string method {"CG"};
		bool force_global_residual;
	};
	Parameters m_param;
	const std::vector<unsigned char> *m_kind {nullptr};
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