/*
**	macpressuresolver3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 27, 2017. 
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
#include <shiokaze/array/array3.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/math/RCMatrix_utility.h>
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/projection/macproject3_interface.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/utility/utility.h>
//
SHKZ_USING_NAMESPACE
//
class macpressuresolver3 : public macproject3_interface {
private:
	//
	LONG_NAME("MAC Pressure Solver 3D")
	//
	virtual void set_target_volume( double current_volume, double target_volume ) override {
		m_current_volume = current_volume;
		m_target_volume = target_volume;
	}
	//
	virtual void project( double dt,
				 macarray3<double> &velocity,
				 const array3<double> &solid,
				 const array3<double> &fluid) override {
		//
		scoped_timer timer(this);
		//
		timer.tick(); console::dump( ">>> Pressure Projection started...\n" );
		//
		shared_array3<double> pressure(m_shape);
		shared_macarray3<double> areas(velocity.shape());
		shared_macarray3<double> rhos(velocity.shape());
		//
		// Pre-compute solid cut "areas" and fluid density "rhos" for each cell face
		timer.tick(); console::dump( "Precomputing solid and fluid fractions...");
		m_macutility->compute_area_fraction(solid,areas());
		m_macutility->compute_fluid_fraction(fluid,rhos());
		//
		// Enforce first order accuracy
		if( ! m_param.second_order_accurate_fluid ) {
			rhos->parallel_actives([&]( auto &it ) {
				if( it() ) it.set(1.0);
			});
		}
		//
		if( ! m_param.second_order_accurate_solid ) {
			areas->parallel_actives([&]( auto &it ) {
				if( it() ) it.set(1.0);
			});
		}
		//
		console::dump( "Done. Took %s\n", timer.stock("solid_fluid_fractions").c_str());
		///
		// Compute curvature and substitute to the right hand side for the surface tension force
		if( m_param.surftens_k ) {
			//
			timer.tick(); console::dump( "Computing surface tension force...");
			double kappa = m_param.surftens_k;
			//
			// Compute curvature and store them into an array
			shared_array3<double> curvature(fluid.shape());
			curvature->parallel_op([&]( int i, int j, int k, auto &it, int tn ) {
				double value = (
					+fluid(m_shape.clamp(i-1,j,k))+fluid(m_shape.clamp(i+1,j,k))
					+fluid(m_shape.clamp(i,j-1,k))+fluid(m_shape.clamp(i,j+1,k))
					+fluid(m_shape.clamp(i,j,k-1))+fluid(m_shape.clamp(i,j,k+1))
					-6.0*fluid(i,j,k)
				) / (m_dx*m_dx);
				it.set(value);
			});
			//
			velocity.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn ) {
				double rho = rhos()[dim](i,j,k);
				// Embed ordinary 2nd order surface tension force
				if( rho && rho < 1.0 ) {
					double sgn = fluid(m_shape.clamp(i,j,k)) < 0.0 ? -1.0 : 1.0;
					double theta = sgn < 0 ? 1.0-rho : rho;
					double face_c = 
						theta*curvature()(m_shape.clamp(i,j,k))
						+(1.0-theta)*curvature()(m_shape.clamp(i-(dim==0),j-(dim==1),k-(dim==2)));
					it.increment( -sgn * dt / (m_dx*rho) * kappa * face_c );
				}
			});
			console::dump( "Done. Took %s\n", timer.stock("surftension_force").c_str());
		}
		//
		// The target linear system to build
		timer.tick(); console::dump( "Building the high-res linear system [Lhs] and [rhs]..." );
		//
		// Label cell indices
		size_t index (0);
		shared_array3<size_t> index_map(fluid.shape());
		const auto mark_body = [&]( int i, int j, int k ) {
			//
			bool inside (false);
			if( fluid(i,j,k) < 0.0 ) {
				//
				vec3i query[] = {vec3i(i+1,j,k),vec3i(i-1,j,k),vec3i(i,j+1,k),vec3i(i,j-1,k),vec3i(i,j,k+1),vec3i(i,j,k-1)};
				vec3i face[] = {vec3i(i+1,j,k),vec3i(i,j,k),vec3i(i,j+1,k),vec3i(i,j,k),vec3i(i,j,k+1),vec3i(i,j,k)};
				int direction[] = {0,0,1,1,2,2};
				//
				for( int nq=0; nq<6; nq++ ) {
					if( ! m_shape.out_of_bounds(query[nq]) ) {
						if( fluid(query[nq]) < 0.0 ) {
							int dim = direction[nq];
							if( areas()[dim](face[nq]) && rhos()[dim](face[nq]) ) {
								inside = true;
								break;
							}
						}
					}
				}
			}
			if( inside ) {
				index_map->set(i,j,k,index++);
			}
		};
		if( fluid.get_background_value() < 0.0 ) {
			fluid.const_serial_all([&]( int i, int j, int k, const auto &it) {
				mark_body(i,j,k);
			});
		} else {
			fluid.const_serial_inside([&]( int i, int j, int k, const auto &it) {
				mark_body(i,j,k);
			});
		}
		//
		// Assemble the linear system for the Poisson equations for pressure solve
		auto Lhs = m_factory->allocate_matrix(index,index);
		auto rhs = m_factory->allocate_vector(index);
		double assemble_time = utility::get_milliseconds();
		//
		index_map->const_parallel_actives([&]( int i, int j, int k, const auto &it, int tn ) {
			//
			size_t n_index = it();
			rhs->set(n_index,0.0);
			//
			if( fluid(i,j,k) < 0.0 ) {
				//
				vec3i query[] = {vec3i(i+1,j,k),vec3i(i-1,j,k),vec3i(i,j+1,k),vec3i(i,j-1,k),vec3i(i,j,k+1),vec3i(i,j,k-1)};
				vec3i face[] = {vec3i(i+1,j,k),vec3i(i,j,k),vec3i(i,j+1,k),vec3i(i,j,k),vec3i(i,j,k+1),vec3i(i,j,k)};
				int direction[] = {0,0,1,1,2,2};
				int sgn[] = {1,-1,1,-1,1,-1};
				//
				double diagonal = 0.0;
				for( int nq=0; nq<6; nq++ ) {
					int dim = direction[nq];
					int qi(query[nq][0]), qj(query[nq][1]), qk(query[nq][2]);
					int fi(face[nq][0]), fj(face[nq][1]), fk(face[nq][2]);
					if( ! m_shape.out_of_bounds(query[nq]) ) {
						double area = areas()[dim](face[nq]);
						if( area ) {
							double rho = rhos()[dim](face[nq]);
							if( rho ) {
								double value = dt*area/(m_dx*m_dx*rho);
								if( fluid(query[nq]) < 0.0 ) {
									assert(index_map->active(query[nq]));
									size_t m_index = index_map()(query[nq]);
									Lhs->add_to_element(n_index,m_index,-value);
								}
								diagonal += value;
							}
							rhs->add(n_index,-sgn[nq]*area*velocity[dim](face[nq])/m_dx);
						}
					}
				}
				Lhs->add_to_element(n_index,n_index,diagonal);
			}
		});
		//
		console::dump( "Done. Took %s\n", timer.stock("build_highres_linsystem").c_str());
		//
		// Volume correction
		double rhs_correct = 0.0;
		if( m_param.gain && m_target_volume ) {
			timer.tick(); console::dump( "Computing volume correction...");
			double x = (m_current_volume-m_target_volume)/m_target_volume;
			double y = m_y_prev + x*dt; m_y_prev = y;
			double kp = m_param.gain * 2.3/(25.0*0.01);
			double ki = kp*kp/16.0;
			rhs_correct = -(kp*x+ki*y)/(x+1.0);
			rhs->for_each([&]( unsigned row, double &value ) {
				value += rhs_correct;
			});
			console::dump( "Done. Took %s\n", timer.stock("volume_correction").c_str());
			console::write(get_argument_name()+"_volume_correct_rhs", rhs_correct);
		}
		//
		RCMatrix_utility<size_t,double>::report(Lhs.get(),"Lhs");
		//
		// Solve the linear system
		timer.tick(); console::dump( "Solving the linear system...");
		auto result = m_factory->allocate_vector(index);
		unsigned count = m_solver->solve(Lhs.get(),rhs.get(),result.get());
		console::write(get_argument_name()+"_number_projection_iteration", count);
		console::dump( "Done. Took %d iterations. Took %s\n", count, timer.stock("linsolve").c_str());
		//
		// Re-arrange to the array
		index_map->const_serial_actives([&](int i, int j, int k, const auto& it) {
			pressure->set(i,j,k,result->at(it()));
		});
		//
		// Update the full velocity
		timer.tick(); console::dump( "Updating the velocity...");
		velocity.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn ) {
			double rho = rhos()[dim](i,j,k);
			vec3i pi(i,j,k);
			if( areas()[dim](i,j,k) && rho ) {
				if( pi[dim] == 0 || pi[dim] == velocity.shape()[dim] ) it.set(0.0);
				else {
					velocity[dim].subtract(i,j,k, dt * (
						+ pressure()(i,j,k)
						- pressure()(i-(dim==0),j-(dim==1),k-(dim==2))
						) / (rho*m_dx));
				}
			} else {
				if( pi[dim] == 0 && fluid(pi) < 0.0 ) it.set(0.0);
				else if( pi[dim] == velocity.shape()[dim] && fluid(pi-vec3i(dim==0,dim==1,dim==2)) < 0.0 ) it.set(0.0);
				else it.set_off();
			}
		});
		console::dump( "Done. Took %s\n", timer.stock("update_velocity").c_str());
		//
		console::dump( "<<< Projection done. Took %s.\n", timer.stock("projection").c_str());
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("SecondOrderAccurateFluid",m_param.second_order_accurate_fluid,"Whether to enforce second order accuracy");
		config.get_bool("SecondOrderAccurateSolid",m_param.second_order_accurate_solid,"Whether to enforce second order accuracy for solid surfaces");
		config.get_double("SurfaceTension",m_param.surftens_k,"Surface tenstion coefficient");
		config.get_double("Gain",m_param.gain,"Rate for volume correction");
		config.set_default_bool("ReportProgress",false);
	}
	virtual void initialize( const shape3 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	virtual void post_initialize() override {
		//
		m_target_volume = m_current_volume = m_y_prev = 0.0;
	}
	//
	//
	struct Parameters {
		//
		double surftens_k {0.0};
		double gain {1.0};
		double eps_fluid {1e-2};
		double eps_solid {1e-2};
		bool ignore_solid {false};
		bool second_order_accurate_fluid {true};
		bool second_order_accurate_solid {true};
	};
	Parameters m_param;
	//
	shape3 m_shape;
	double m_dx {0.0};
	//
	macutility3_driver m_macutility{this,"macutility3"};
	RCMatrix_factory_driver<size_t,double> m_factory{this,"RCMatrix"};
	RCMatrix_solver_driver<size_t,double> m_solver{this,"pcg"};
	//
	double m_target_volume {0.0};
	double m_current_volume {0.0};
	double m_y_prev {0.0};
	//
};
//
extern "C" module * create_instance() {
	return new macpressuresolver3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//