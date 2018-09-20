/*
**	macutility3.cpp
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
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/array_extrapolator3.h>
#include <shiokaze/array/macarray_interpolator3.h>
#include <shiokaze/array/macarray_extrapolator3.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_derivative3.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/scoped_timer.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/math/WENO3.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
using namespace array_utility3;
using namespace array_interpolator3;
//
class macutility3 : public macutility3_interface {
private:
	//
	virtual double compute_max_u ( const macarray3<double> &velocity ) const override {
		//
		shared_array3<vec3d> cell_velocity(m_shape);
		velocity.convert_to_full(cell_velocity());
		//
		std::vector<double> max_u_t(cell_velocity->get_thread_num(),0.0);
		cell_velocity->parallel_actives([&]( int i, int j, int k, auto &it, int tn ) {
			max_u_t[tn] = std::max(max_u_t[tn],it().len());
		});
		double max_u (0.0);
		for( double u : max_u_t ) max_u = std::max(max_u,u);
		return max_u;
	}
	virtual void constrain_velocity( const array3<double> &solid, macarray3<double> &velocity ) const override {
		//
		shared_macarray3<double> velocity_save = shared_macarray3<double>(velocity.type());
		velocity_save->copy(velocity);
		//
		if( levelset_exist(solid) ) {
			//
			auto solid_accessors = solid.get_const_accessors();
			auto velocity_save_accessors = velocity_save->get_const_accessors();
			//
			for( int dim : DIMS3 ) {
				velocity.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
					vec3i pi(i,j,k);
					vec3d p(vec3i(i,j,k).face(dim));
					if( interpolate<double>(solid_accessors[tn],p) < 0.0 ) {
						double derivative[DIM3];
						array_derivative3::derivative(solid,p,derivative);
						vec3d normal = vec3d(derivative)/m_dx;
						if( normal.norm2() ) {
							vec3d u = macarray_interpolator3::interpolate<double>(velocity_save_accessors[tn],vec3i(i,j,k).cell());
							it.set((u-normal*(u*normal))[dim]);
						}
					}
					if( pi[dim]==0 || pi[dim]==m_shape[dim] ) it.set(0.0);
				});
			}
		}
	}
	virtual void extrapolate_and_constrain_velocity( const array3<double> &solid, macarray3<double> &velocity, int extrapolate_width ) const override {
		//
		macarray_extrapolator3::extrapolate(velocity,extrapolate_width);
		constrain_velocity(solid,velocity);
	}
	virtual void compute_area_fraction( const array3<double> &solid, macarray3<double> &areas ) const override {
		//
		if( levelset_exist(solid) ) {
			//
			areas.clear(0.0);
			m_parallel.for_each( DIM3, [&]( size_t dim ) {
				areas[dim].activate_as(solid);
				if( dim == 0 ) {
					for( int jj=-1; jj<=0; ++jj ) for( int kk=-1; kk<=0; ++kk ) {
						areas[dim].activate_as(solid,vec3i(0,jj,kk));
					}
				} else if( dim == 1 ) {
					for( int ii=-1; ii<=0; ++ii ) for( int kk=-1; kk<=0; ++kk ) {
						areas[dim].activate_as(solid,vec3i(ii,0,kk));
					}
				} else if( dim == 2 ) {
					for( int ii=-1; ii<=0; ++ii ) for( int jj=-1; jj<=0; ++jj ) {
						areas[dim].activate_as(solid,vec3i(ii,jj,0));
					}
				}
				areas[dim].set_as_fillable(0.0,1.0);
			});
			//
			auto solid_accessors = solid.get_const_accessors();
			areas.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				double area;
				vec3i pi(i,j,k);
				if( pi[dim] == 0 || pi[dim] == solid.shape()[dim] ) area = 0.0;
				else {
					double quadsolid[2][2];
					if( dim == 0 ) {
						quadsolid[0][0] = solid_accessors[tn](i,j,k);
						quadsolid[1][0] = solid_accessors[tn](i,j+1,k);
						quadsolid[1][1] = solid_accessors[tn](i,j+1,k+1);
						quadsolid[0][1] = solid_accessors[tn](i,j,k+1);
					} else if( dim == 1 ) {
						quadsolid[0][0] = solid_accessors[tn](i,j,k);
						quadsolid[1][0] = solid_accessors[tn](i+1,j,k);
						quadsolid[1][1] = solid_accessors[tn](i+1,j,k+1);
						quadsolid[0][1] = solid_accessors[tn](i,j,k+1);
					} else if( dim == 2 ) {
						quadsolid[0][0] = solid_accessors[tn](i,j,k);
						quadsolid[1][0] = solid_accessors[tn](i+1,j,k);
						quadsolid[1][1] = solid_accessors[tn](i+1,j+1,k);
						quadsolid[0][1] = solid_accessors[tn](i,j+1,k);
					}
					area = 1.0-utility::get_area(quadsolid);
				}
				if( area && area < m_param.eps_solid ) area = m_param.eps_solid;
				it.set(area);
			});
			//
			m_parallel.for_each( DIM3, [&]( size_t dim ) {
				areas[dim].flood_fill();
			});
			//
		} else {
			//
			areas.clear(1.0);
			auto accessor = areas.get_serial_accessor();
			for( int j=0; j<m_shape.h; ++j ) for( int k=0; k<m_shape.d; ++k ) {
				accessor.set(0,0,j,k,0.0);
				accessor.set(0,m_shape.w,j,k,0.0);
			}
			for( int i=0; i<m_shape.w; ++i ) for( int k=0; k<m_shape.d; ++k ) {
				accessor.set(1,i,0,k,0.0);
				accessor.set(1,i,m_shape.h,k,0.0);
			}
			for( int i=0; i<m_shape.w; ++i ) for( int j=0; j<m_shape.h; ++j ) {
				accessor.set(2,i,j,0,0.0);
				accessor.set(2,i,j,m_shape.d,0.0);
			}
		}
	}
	virtual void compute_fluid_fraction( const array3<double> &fluid, macarray3<double> &rhos ) const override {
		//
		if( levelset_exist(fluid)) {
			//
			rhos.clear(0.0);
			m_parallel.for_each( DIM3, [&]( size_t dim ) {
				rhos[dim].activate_as(fluid);
				rhos[dim].activate_as(fluid,vec3i(dim==0,dim==1,dim==2));
				rhos[dim].set_as_fillable(0.0,1.0);
			});
			//
			auto fluid_accessors = fluid.get_const_accessors();
			rhos.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				//
				double rho = utility::fraction(
					fluid_accessors[tn](m_shape.clamp(i,j,k)),
					fluid_accessors[tn](m_shape.clamp(i-(dim==0),j-(dim==1),k-(dim==2)))
				);
				if( rho && rho < m_param.eps_fluid ) rho = m_param.eps_fluid;
				it.set(rho);
			});
			//
			m_parallel.for_each( DIM3, [&]( size_t dim ) {
				rhos[dim].flood_fill();
			});
			//
		} else {
			rhos.clear(1.0);
		}
	}
	virtual void compute_face_density( const array3<double> &solid, const array3<double> &fluid, macarray3<double> &density ) const override {
		//
		compute_fluid_fraction(fluid,density);
		if( levelset_exist(solid) ) {
			//
			shared_macarray3<double> tmp_areas(density.type());
			auto tmp_areas_accessors = tmp_areas->get_const_accessors();
			compute_area_fraction(solid,tmp_areas());
			density.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				it.multiply(tmp_areas_accessors[tn](dim,i,j,k));
			});
		}
	}
	virtual double get_kinetic_energy( const array3<double> &solid, const array3<double> &fluid, const macarray3<double> &velocity ) const override {
		//
		shared_macarray3<double> tmp_areas(velocity.type());
		shared_macarray3<double> tmp_rhos(velocity.type());
		//
		compute_area_fraction(solid,tmp_areas());
		compute_fluid_fraction(fluid,tmp_rhos());
		//
		auto area_accessors = tmp_areas->get_const_accessors();
		auto fluid_accessors = tmp_rhos->get_const_accessors();
		auto velocity_accessors = velocity.get_const_accessors();
		//
		std::vector<double> results (velocity_accessors.size(),0.0);
		velocity.const_parallel_actives([&]( int dim, int i, int j, int k, const auto &it, int tn ) {
			double area = area_accessors[tn](dim,i,j,k);
			if( area ) {
				double rho = fluid_accessors[tn](dim,i,j,k);
				if( rho ) {
					double u = velocity_accessors[tn](dim,i,j,k);
					double dA = (m_dx*m_dx*m_dx) * (area*rho);
					results[tn] += 0.5*(u*u)*dA;
				}
			}
		});
		//
		double result (0.0);
		for( const auto &e : results ) result += e;
		return result;
	}
	virtual void get_velocity_jacobian( const vec3d &p, const macarray3<double> &velocity, vec3d jacobian[DIM3] ) const override {
		for( unsigned dim : DIMS3 ) {
			array_derivative3::derivative(velocity[dim],vec3d(p[0]/m_dx-0.5*(dim!=0),p[1]/m_dx-0.5*(dim!=1),p[2]/m_dx-0.5*(dim!=2)),jacobian[dim].v);
			jacobian[dim] /= m_dx;
		}
	}
	virtual void assign_initial_variables( const dylibloader &dylib, macarray3<double> &velocity,
									array3<double> *solid=nullptr, array3<double> *fluid=nullptr,array3<double> *density=nullptr ) const override {
		//
		// Scoped timer
		scoped_timer timer(this,"assign_initial_variables");
		//
		// Assign velocity
		const double sqrt3 = sqrt(3.0);
		auto velocity_func = reinterpret_cast<vec3d(*)(const vec3d &)>(dylib.load_symbol("velocity"));
		if( velocity_func ) {
			timer.tick(); console::dump( "Assigining velocity..." );
			auto fluid_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("fluid"));
			velocity.parallel_all([&](int dim, int i, int j, int k, auto &it) {
				bool skip (false);
				if( fluid_func ) {
					skip = (*fluid_func)(m_dx*vec3i(i,j,k).face(dim)) > sqrt3*m_dx;
				}
				if( ! skip ) {
					vec3d value = (*velocity_func)(m_dx*vec3i(i,j,k).face(dim));
					it.set(value[dim]);
				}
			});
			console::dump( "Done. Took %s.\n", timer.stock("assign_velocity").c_str());
		}
		//
		// Assign solid levelset
		if( solid ) {
			solid->set_as_levelset(m_dx);
			auto solid_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("solid"));
			if( solid_func ) {
				timer.tick(); console::dump( "Assigning solid levelset..." );
				solid->parallel_all([&](int i, int j, int k, auto &it) {
					double value = (*solid_func)(m_dx*vec3i(i,j,k).nodal());
					if( std::abs(value) < sqrt3*m_dx ) it.set(value);
				});
				console::dump( "Done. Took %s.\n", timer.stock("evaluate_solid").c_str());
			}
			solid->flood_fill();
		}
		//
		// Assign fluid levelsets
		if( fluid ) {
			fluid->set_as_levelset(m_dx);
			auto fluid_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("fluid"));
			auto solid_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("solid"));
			if( fluid_func ) {
				if( solid_func ) {
					fluid->parallel_all([&](int i, int j, int k, auto &it) {
						vec3d p = m_dx*vec3i(i,j,k).cell();
						double fluid_value = (*fluid_func)(p);
						double solid_value = (*solid_func)(p)+m_dx;
						double value = std::max(fluid_value,-solid_value);
						if( std::abs(value) < sqrt3*m_dx ) it.set(value);
					});
				} else {
					fluid->parallel_all([&](int i, int j, int k, auto &it) {
						double value = (*fluid_func)(m_dx*vec3i(i,j,k).cell());
						if( std::abs(value) < sqrt3*m_dx ) it.set(value);
					});
				}
			}
			fluid->flood_fill();
		}
		//
		// Assign density
		if( density ) {
			auto density_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("density"));
			if( density_func ) {
				timer.tick(); console::dump( "Assigning initial density..." );
				density->parallel_all([&](int i, int j, int k, auto &it) {
					it.set((*density_func)(m_dx*vec3i(i,j,k).cell()));
				});
				console::dump( "Done. Took %s.\n", timer.stock("evaluate_density").c_str());
			}
		}
	}
	virtual void add_force( vec3d p, vec3d f, macarray3<double> &external_force ) const override {
		for( unsigned dim : DIMS3 ) {
			vec3d index_coord = p/m_dx-vec3d(0.5,0.5,0.5);
			external_force[dim].set(m_shape.face(dim).clamp(index_coord),f[dim]);
		}
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_double("EpsFluid",m_param.eps_fluid,"Minimal bound for fluid fraction");
		config.get_double("EpsSolid",m_param.eps_solid,"Minimal bound for solid fraction");
		config.get_bool("WENO",m_param.weno_interpolation,"Whether to use WENO interpolation");
	}
	virtual void initialize( const shape3 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	struct Parameters {
		//
		double eps_fluid {1e-2};
		double eps_solid {1e-2};
		bool weno_interpolation {false};
	};
	Parameters m_param;
	//
	double m_dx;
	shape3 m_shape;
	parallel_driver m_parallel{this};
};
//
extern "C" module * create_instance() {
	return new macutility3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//