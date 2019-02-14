/*
**	macutility2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 19, 2017. 
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
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/array/array_interpolator2.h>
#include <shiokaze/array/array_extrapolator2.h>
#include <shiokaze/array/array_derivative2.h>
#include <shiokaze/array/array_utility2.h>
#include <shiokaze/array/macarray_interpolator2.h>
#include <shiokaze/array/macarray_extrapolator2.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/math/WENO2.h>
#include <shiokaze/utility/utility.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
using namespace array_utility2;
using namespace array_interpolator2;
//
class macutility2 : public macutility2_interface {
private:
	//
	virtual double compute_max_u ( const macarray2<double> &velocity ) const override {
		//
		shared_array2<vec2d> cell_velocity(m_shape);
		velocity.convert_to_full(cell_velocity());
		//
		std::vector<double> max_u_t(cell_velocity->get_thread_num(),0.0);
		cell_velocity->parallel_actives([&]( int i, int j, auto &it, int tn ) {
			max_u_t[tn] = std::max(max_u_t[tn],it().len());
		});
		double max_u (0.0);
		for( double u : max_u_t ) max_u = std::max(max_u,u);
		return max_u;
	}
	virtual void constrain_velocity( const array2<double> &solid, macarray2<double> &velocity ) const override {
		//
		shared_macarray2<double> velocity_save(velocity);
		if( levelset_exist(solid) ) {
			//
			for( int dim : DIMS2 ) {
				velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
					vec2i pi(i,j);
					vec2d p(vec2i(i,j).face(dim));
					if( interpolate<double>(solid,p) < 0.0 ) {
						double derivative[DIM2];
						array_derivative2::derivative(solid,p,derivative);
						vec2d normal = vec2d(derivative)/m_dx;
						if( normal.norm2() ) {
							vec2d u = macarray_interpolator2::interpolate<double>(velocity_save(),p);
							if( u * normal < 0.0 ) {
								it.set((u-normal*(u*normal))[dim]);
							}
						}
					}
					if( pi[dim]==0 && it() < 0.0 ) it.set(0.0);
					if( pi[dim]==m_shape[dim] && it() > 0.0 ) it.set(0.0);
				});
			}
		}
	}
	virtual void extrapolate_and_constrain_velocity( const array2<double> &solid, macarray2<double> &velocity, int extrapolate_width ) const override {
		//
		macarray_extrapolator2::extrapolate(velocity,extrapolate_width);
		constrain_velocity(solid,velocity);
	}
	virtual void compute_area_fraction( const array2<double> &solid, macarray2<double> &areas ) const override {
		//
		if( levelset_exist(solid) ) {
			//
			areas.clear(0.0);
			m_parallel.for_each( DIM2, [&]( size_t dim ) {
				areas[dim].activate_as(solid);
				areas[dim].activate_as(solid,-vec2i(dim!=0,dim!=1));
				areas[dim].set_as_fillable(0.0,1.0);
			});
			//
			areas.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				double area;
				vec2i pi(i,j);
				if( pi[dim] == 0 || pi[dim] == solid.shape()[dim] ) area = 0.0;
				else area = 1.0-utility::fraction(solid(i,j),solid(i+(dim!=0),j+(dim!=1)));
				if( area && area < m_param.eps_solid ) area = m_param.eps_solid;
				it.set(area);
			});
			//
			m_parallel.for_each( DIM2, [&]( size_t dim ) {
				areas[dim].flood_fill();
			});
			//
		} else {
			//
			areas.clear(1.0);
			for( int i=0; i<m_shape.w; ++i ) {
				areas[1].set(i,0,0.0);
				areas[1].set(i,m_shape.h,0.0);
			}
			for( int j=0; j<m_shape.h; ++j ) {
				areas[0].set(0,j,0.0);
				areas[0].set(m_shape.w,j,0.0);
			}
		}
	}
	virtual void compute_fluid_fraction( const array2<double> &fluid, macarray2<double> &rhos ) const override {
		//
		if( levelset_exist(fluid)) {
			//
			rhos.clear(0.0);
			m_parallel.for_each( DIM2, [&]( size_t dim ) {
				rhos[dim].activate_as(fluid);
				rhos[dim].activate_as(fluid,vec2i(dim==0,dim==1));
				rhos[dim].set_as_fillable(0.0,1.0);
			});
			//
			rhos.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				//
				double rho = utility::fraction(
					fluid(m_shape.clamp(i,j)),
					fluid(m_shape.clamp(i-(dim==0),j-(dim==1)))
				);
				if( rho && rho < m_param.eps_fluid ) rho = m_param.eps_fluid;
				it.set(rho);
			});
			//
			m_parallel.for_each( DIM2, [&]( size_t dim ) {
				rhos[dim].flood_fill();
			});
			//
		} else {
			rhos.clear(1.0);
		}
	}
	virtual void compute_face_density( const array2<double> &solid, const array2<double> &fluid, macarray2<double> &density ) const override {
		//
		compute_fluid_fraction(fluid,density);
		if( levelset_exist(solid) ) {
			//
			shared_macarray2<double> tmp_areas(density.type());
			compute_area_fraction(solid,tmp_areas());
			density.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				it.multiply(tmp_areas()[dim](i,j));
			});
		}
	}
	virtual double get_kinetic_energy( const array2<double> &solid, const array2<double> &fluid, const macarray2<double> &velocity ) const override {
		//
		shared_macarray2<double> tmp_areas(velocity.type());
		shared_macarray2<double> tmp_rhos(velocity.type());
		//
		compute_area_fraction(solid,tmp_areas());
		compute_fluid_fraction(fluid,tmp_rhos());
		//
		std::vector<double> results (velocity.get_thread_num(),0.0);
		velocity.const_parallel_actives([&]( int dim, int i, int j, const auto &it, int tn ) {
			double area = tmp_areas()[dim](i,j);
			if( area ) {
				double rho = tmp_rhos()[dim](i,j);
				if( rho ) {
					double u = velocity[dim](i,j);
					double dA = (m_dx*m_dx) * (area*rho);
					results[tn] += 0.5*(u*u)*dA;
				}
			}
		});
		//
		double result (0.0);
		for( const auto &e : results ) result += e;
		return result;
	}
	virtual void get_velocity_jacobian( const vec2d &p, const macarray2<double> &velocity, vec2d jacobian[DIM2] ) const override {
		for( unsigned dim : DIMS2 ) {
			array_derivative2::derivative(velocity[dim],vec2d(p[0]/m_dx-0.5*(dim!=0),p[1]/m_dx-0.5*(dim!=1)),jacobian[dim].v);
			jacobian[dim] /= m_dx;
		}
	}
	virtual void assign_initial_variables(	const dylibloader &dylib, macarray2<double> &velocity,
									array2<double> *solid=nullptr, array2<double> *fluid=nullptr, array2<double> *density=nullptr ) const override {
		//
		// Assign initial velocity
		const double sqrt2 = sqrt(2.0);
		auto velocity_func = reinterpret_cast<vec2d(*)(const vec2d &)>(dylib.load_symbol("velocity"));
		if( velocity_func ) {
			auto fluid_func = reinterpret_cast<double(*)(const vec2d &)>(dylib.load_symbol("fluid"));
			velocity.parallel_all([&](int dim, int i, int j, auto &it) {
				bool skip (false);
				if( fluid_func ) {
					skip = (*fluid_func)(m_dx*vec2i(i,j).face(dim)) > sqrt2*m_dx;
				}
				if( ! skip ) {
					vec2d value = (*velocity_func)(m_dx*vec2i(i,j).face(dim));
					it.set(value[dim]);
				}
			});
		}
		//
		// Assign solid levelset
		if( solid ) {
			solid->set_as_levelset(m_dx);
			auto solid_func = reinterpret_cast<double(*)(const vec2d &)>(dylib.load_symbol("solid"));
			if( solid_func ) {
				solid->parallel_all([&](int i, int j, auto &it) {
					double value = (*solid_func)(m_dx*vec2i(i,j).nodal());
					if( std::abs(value) < sqrt2*m_dx ) it.set(value);
				});
			}
			solid->flood_fill();
		}
		//
		// Assign fluid levelset
		if( fluid ) {
			fluid->set_as_levelset(m_dx);
			auto fluid_func = reinterpret_cast<double(*)(const vec2d &)>(dylib.load_symbol("fluid"));
			auto solid_func = reinterpret_cast<double(*)(const vec2d &)>(dylib.load_symbol("solid"));
			if( fluid_func ) {
				if( solid_func ) {
					fluid->parallel_all([&](int i, int j, auto &it) {
						vec2d p = m_dx*vec2i(i,j).cell();
						double fluid_value = (*fluid_func)(p);
						double solid_value = (*solid_func)(p)+m_dx;
						double value = std::max(fluid_value,-solid_value);
						if( std::abs(value) < sqrt2*m_dx ) it.set(value);
					});
				} else {
					fluid->parallel_all([&](int i, int j, auto &it) {
						double value = (*fluid_func)(m_dx*vec2i(i,j).cell());
						if( std::abs(value) < sqrt2*m_dx ) it.set(value);
					});
				}
			}
			fluid->flood_fill();
		}
		//
		// Assign density
		if( density ) {
			auto density_func = reinterpret_cast<double(*)(const vec2d &)>(dylib.load_symbol("density"));
			if( density_func ) {
				density->parallel_all([&](int i, int j, auto &it) {
					it.set((*density_func)(m_dx*vec2i(i,j).cell()));
				});
			}
		}
	}
	virtual void add_force( vec2d p, vec2d f, macarray2<double> &external_force ) const override {
		for( unsigned dim : DIMS2 ) {
			vec2d index_coord = p/m_dx-vec2d(0.5,0.5);
			external_force[dim].set(m_shape.face(dim).clamp(index_coord),f[dim]);
		}
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	virtual void configure( configuration &config ) override {
		config.get_double("EpsFluid",m_param.eps_fluid,"Minimal bound for fluid fraction");
		config.get_double("EpsSolid",m_param.eps_solid,"Minimal bound for solid fraction");
		config.get_bool("WENO",m_param.weno_interpolation,"Whether to use WENO interpolation");
	}
	//
	struct Parameters {
		//
		double eps_fluid {1e-2};
		double eps_solid {1e-2};
		bool weno_interpolation {false};
	};
	//
	Parameters m_param;
	double m_dx;
	shape2 m_shape;
	parallel_driver m_parallel{this};
};
//
extern "C" module * create_instance() {
	return new macutility2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//