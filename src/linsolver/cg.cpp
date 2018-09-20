/*
**	cg.cpp
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
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
template <class N, class T> class cg_solver : public RCMatrix_solver_interface<N,T> {
private:
	//
	LONG_NAME("Conjugate Gradient Solver")
	ARGUMENT_NAME("CG")
	//
	virtual void configure( configuration &config ) override {
		config.get_double("Residual",m_param.residual,"Tolerable residual");
		config.get_unsigned("MaxIterations",m_param.max_iterations,"Maximal iteration count");
	}
	virtual unsigned solve( const RCMatrix_ptr<N,T> &A, const RCMatrix_vector_ptr<N,T> b, RCMatrix_vector_ptr<N,T> x ) const override {
		//
		auto cg = []( std::function<void( RCMatrix_vector_ptr<N,T> x, RCMatrix_vector_ptr<N,T> result)> A,
					  RCMatrix_vector_ptr<N,T> b, RCMatrix_vector_ptr<N,T> x, unsigned max_iterations, T tolerance_factor ) {
			//
			size_t n (b->size()), iterations_out(0);
			T residual_0, residual_1, delta;
			auto r = b->allocate_vector(n), z = b->allocate_vector(n), p = b->allocate_vector(n);
			//
			r->copy(b.get());
			residual_0 = r->abs_max();
			z->copy(r.get()); p->copy(z.get()); delta = r->dot(z.get());
			if(delta < std::numeric_limits<T>::epsilon()) return N();
			//
			N iteration (0);
			for( ; iteration<max_iterations; ++iteration ) {
				//
				A(p,z); // z = A * p
				T alpha = delta / p->dot(z.get());
				x->add_scaled(alpha,p.get()); // x += alpha * p;
				r->add_scaled(-alpha,z.get()); // r -= alpha * z;
				residual_1 = r->abs_max();
				T relative_residual_out = residual_1 / residual_0;
				if( relative_residual_out <= tolerance_factor ) {
					iterations_out = iteration+1;
					break;
				}
				z->copy(r.get());
				T beta = r->dot(z.get());
				z->add_scaled(beta/delta,p.get()); p.swap(z); // p = z + ( beta / delta ) * p;
				delta = beta;
			}
			return iteration;
		};
		//
		const auto A_fixed = A->make_fixed();
		return cg([&]( RCMatrix_vector_ptr<N,T> x, RCMatrix_vector_ptr<N,T> result ) {
			A_fixed->multiply(x.get(),result.get());
		},b,x,m_param.max_iterations,m_param.residual);
	}
	//
	struct Parameters {
		double residual {1e-4};
		unsigned max_iterations {30000};
	};
	Parameters m_param;
};
//
extern "C" module * create_instance() {
	return new cg_solver<INDEX_TYPE,FLOAT_TYPE>();
}
//
extern "C" const char *license() {
	return "MIT";
}
//