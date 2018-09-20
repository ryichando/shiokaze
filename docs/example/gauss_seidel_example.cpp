//! [code]
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include <cmath>
#include <cassert>
//
SHKZ_USING_NAMESPACE
//
template <class N, class T> class gause_seidel_solver : public RCMatrix_solver_interface<N,T> {
private:
	//
	LONG_NAME("Gauss Seidel Solver")
	ARGUMENT_NAME("GaussSeidel")
	//
	virtual void configure( configuration &config ) override {
		config.get_double("Residual",m_param.residual,"Tolerable residual");
		config.get_unsigned("MaxIterations",m_param.max_iterations,"Maximal iteration count");
	}
	virtual unsigned solve( const RCMatrix_interface<N,T> *A, const RCMatrix_vector_interface<N,T> *b, RCMatrix_vector_interface<N,T> *x ) const override {
		//
		T relative_error (1.0), initial_error (0.0);
		unsigned iteration_count (0);
		do {
			T error (0.0);
			++ iteration_count;
			for( N row=0; row<A->rows(); ++row ) {
				T diag (0.0), rhs (0.0), bi (b->at(row));
				A->const_for_each(row,[&]( N column, T value ) {
					if( row == column ) {
						diag = value;
					} else {
						rhs += value * x->at(column);
					}
				});
				error = std::max(error,std::abs(rhs+diag*x->at(row)-bi));
				x->set(row,(bi-rhs)/diag);
			}
			if( ! error ) break;
			else {
				if( ! initial_error ) initial_error = error;
				relative_error = error / initial_error;
			}
		} while( relative_error > m_param.residual && iteration_count < m_param.max_iterations );
		return iteration_count;
	}
	//
	struct Parameters {
		double residual {1e-4};
		unsigned max_iterations {30000};
	};
	//
	Parameters m_param;
};
//
extern "C" module * create_instance() {
	return new gause_seidel_solver<INDEX_TYPE,FLOAT_TYPE>();
}
//
extern "C" const char *license() {
	return "MIT";
}
//! [code]
//! [wscript]
bld.shlib(source = 'gauss_seidel.cpp',
			cxxflags = ['-DINDEX_TYPE=size_t','-DFLOAT_TYPE=double'],
			target = bld.get_target_name(bld,'gauss_seidel'),
			use = bld.get_target_name(bld,'core'))
//! [wscript]