/*
**	macpressuresolver2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 24, 2017. 
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
#include <shiokaze/array/array2.h>
#include <shiokaze/array/macarray2.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/projection/macproject2_interface.h>
#include <memory>
//
SHKZ_USING_NAMESPACE
//
class macpressuresolver2 : public macproject2_interface {
private:
	//
	LONG_NAME("MAC Pressure Solver 2D")
	//
	virtual void set_target_volume( double current_volume, double target_volume ) override {
		m_current_volume = current_volume;
		m_target_volume = target_volume;
	}
	//
	virtual void project(double dt,
						macarray2<double> &velocity,
						const array2<double> &solid,
						const array2<double> &fluid) override {
		//
		shared_macarray2<double> areas(velocity.shape());
		shared_macarray2<double> rhos(velocity.shape());
		//
		// Compute fractions
		m_macutility->compute_area_fraction(solid,areas());
		m_macutility->compute_fluid_fraction(fluid,rhos());
		//
		// Compute curvature and substitute to the right hand side for the surface tension force
		if( m_param.surftens_k ) {
			//
			// Compute curvature and store them into an array
			shared_array2<double> curvature(fluid.shape());
			curvature->activate_as(fluid);
	 		curvature->parallel_actives([&](int i, int j, auto &it, int tn ) {
				double value = (
					+ fluid(m_shape.clamp(i-1,j))
					+ fluid(m_shape.clamp(i+1,j))
					+ fluid(m_shape.clamp(i,j-1))
					+ fluid(m_shape.clamp(i,j+1))
					- 4.0*fluid(i,j)
				) / (m_dx*m_dx);
				it.set(value);
			});
			//
			double kappa = m_param.surftens_k;
			velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
				double rho = rhos()[dim](i,j);
				//
				// Embed ordinary 2nd order surface tension force
				if( rho && rho < 1.0 ) {
					double sgn = fluid(m_shape.clamp(i,j)) < 0.0 ? -1.0 : 1.0;
					double theta = sgn < 0 ? 1.0-rho : rho;
					double face_c = 
						theta*curvature()(m_shape.clamp(i,j))
						+(1.0-theta)*curvature()(m_shape.clamp(i-(dim==0),j-(dim==1)));
					it.increment( -sgn * dt / (m_dx*rho) * kappa * face_c );
				}
			});
		}
		//
		// Label cell indices
		size_t index (0);
		shared_array2<size_t> index_map(fluid.shape());
		const auto mark_body = [&]( int i, int j ) {
			//
			bool inside (false);
			vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
			vec2i face[] = {vec2i(i+1,j),vec2i(i,j),vec2i(i,j+1),vec2i(i,j)};
			int direction[] = {0,0,1,1};
			//
			if( fluid(i,j) < 0.0 ) {
				for( int nq=0; nq<4; nq++ ) {
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
				index_map().set(i,j,index++);
			}
		};
		if( fluid.get_background_value() < 0.0 ) {
			fluid.const_serial_all([&]( int i, int j, const auto &it) {
				mark_body(i,j);
			});
		} else {
			fluid.const_serial_inside([&]( int i, int j, const auto &it) {
				mark_body(i,j);
			});
		}
		//
		auto Lhs = m_factory->allocate_matrix(index,index);
		auto rhs = m_factory->allocate_vector(index);
		//
		index_map->const_parallel_actives([&]( int i, int j, const auto &it, int tn ) {
			//
			size_t n_index = it();
			rhs->set(n_index,0.0);
			//
			vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
			vec2i face[] = {vec2i(i+1,j),vec2i(i,j),vec2i(i,j+1),vec2i(i,j)};
			int direction[] = {0,0,1,1};
			int sgn[] = {1,-1,1,-1};
			//
			double diagonal (0.0);
			for( int nq=0; nq<4; nq++ ) {
				int dim = direction[nq];
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
					}
					rhs->add(n_index,-sgn[nq]*area*velocity[dim](face[nq])/m_dx);
				}
			}
			Lhs->add_to_element(n_index,n_index,diagonal);
		});
		//
		// Volume correction
		double rhs_correct = 0.0;
		if( m_param.gain && m_target_volume ) {
			double x = (m_current_volume-m_target_volume)/m_target_volume;
			double y = m_y_prev + x*dt; m_y_prev = y;
			double kp = m_param.gain * 2.3/(25.0*dt);
			double ki = kp*kp/16.0;
			rhs_correct = -(kp*x+ki*y)/(x+1.0);
			rhs->for_each([&]( unsigned row, double &value ) {
				value += rhs_correct;
			});
		}
		//
		// Solve the linear system
		auto result = m_factory->allocate_vector(index);
		m_solver->solve(Lhs.get(),rhs.get(),result.get());
		//
		// Re-arrange to the array
		m_pressure.clear();
		index_map->const_serial_actives([&](int i, int j, const auto& it) {
			m_pressure.set(i,j,result->at(it()));
		});
		//
		// Update the full velocity
		velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
			double rho = rhos()[dim](i,j);
			vec2i pi(i,j);
			if( areas()[dim](i,j) && rho ) {
				if( pi[dim] == 0 || pi[dim] == velocity.shape()[dim] ) it.set(0.0);
				else {
					it.subtract(dt * (
						+ m_pressure(i,j)
						- m_pressure(i-(dim==0),j-(dim==1))
					) / (rho*m_dx));
				}
			} else {
				if( pi[dim] == 0 && fluid(pi) < 0.0 && it() < 0.0 ) it.set(0.0);
				else if( pi[dim] == velocity.shape()[dim] && fluid(pi-vec2i(dim==0,dim==1)) < 0.0 && it() > 0.0 ) it.set(0.0);
				else it.set_off();
			}
		});
	}
	//
	virtual void draw( const graphics_engine &g ) const override {
		if( m_param.draw_pressure ) {
			//
			// Visualize pressure
			m_gridvisualizer->visualize_cell_scalar(g,m_pressure);
		}
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("DrawPressure",m_param.draw_pressure,"Whether to draw pressure");
		config.get_double("SurfaceTension",m_param.surftens_k,"Surface tenstion coefficient");
		config.get_double("Gain",m_param.gain,"Rate for volume correction");
		config.set_default_bool("ReportProgress",false);
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	virtual void post_initialize() override {
		//
		m_pressure.initialize(m_shape);
		m_target_volume = m_current_volume = m_y_prev = 0.0;
	}
	//
	struct Parameters {
		//
		double surftens_k {0.0};
		double gain {1.0};
		bool ignore_solid {false};
		bool draw_pressure {true};
	};
	Parameters m_param;
	//
	shape2 m_shape;
	double m_dx {0.0};
	array2<double> m_pressure{this};
	//
	macutility2_driver m_macutility{this,"macutility2"};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	RCMatrix_factory_driver<size_t,double> m_factory{this,"RCMatrix"};
	RCMatrix_solver_driver<size_t,double> m_solver{this,"pcg"};
	//
	double m_assemble_time {0.0};
	double m_target_volume {0.0};
	double m_current_volume {0.0};
	double m_y_prev {0.0};
	//
};
//
extern "C" module * create_instance() {
	return new macpressuresolver2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
