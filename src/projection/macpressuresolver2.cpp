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
		dt = std::min(m_param.max_dt,std::max(m_param.min_dt,dt));
		//
		shared_macarray2<double> areas(velocity.shape());
		shared_macarray2<double> rhos(velocity.shape());
		//
		// Make accessors
		auto fluid_accessors = fluid.get_const_accessors();
		auto solid_accessors = solid.get_const_accessors();
		auto pressure_acessors = m_pressure.get_const_accessors();
		auto rho_accessors = rhos->get_const_accessors();
		auto area_accessors = areas->get_const_accessors();
		auto velocity_accessors = velocity.get_const_accessors();
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
					+ fluid_accessors[tn](m_shape.clamp(i-1,j))
					+ fluid_accessors[tn](m_shape.clamp(i+1,j))
					+ fluid_accessors[tn](m_shape.clamp(i,j-1))
					+ fluid_accessors[tn](m_shape.clamp(i,j+1))
					- 4.0*fluid_accessors[tn](i,j)
				) / (m_dx*m_dx);
				it.set(value);
			});
			//
			double kappa = m_param.surftens_k;
			auto curvature_acessors = curvature->get_const_accessors();
			velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
				double rho = rho_accessors[tn](dim,i,j);
				//
				// Embed ordinary 2nd order surface tension force
				if( rho && rho < 1.0 ) {
					double sgn = fluid_accessors[tn](m_shape.clamp(i,j)) < 0.0 ? -1.0 : 1.0;
					double theta = sgn < 0 ? 1.0-rho : rho;
					double face_c = 
						theta*curvature_acessors[tn](m_shape.clamp(i,j))
						+(1.0-theta)*curvature_acessors[tn](m_shape.clamp(i-(dim==0),j-(dim==1)));
					it.increment( -sgn * dt / (m_dx*rho) * kappa * face_c );
				}
			});
		}
		//
		// Label cell indices
		size_t index (0);
		shared_array2<size_t> index_map(fluid.shape());
		auto index_map_accessor = index_map->get_serial_accessor();
		const auto mark_body = [&]( int i, int j ) {
			//
			bool inside (false);
			vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
			vec2i face[] = {vec2i(i+1,j),vec2i(i,j),vec2i(i,j+1),vec2i(i,j)};
			int direction[] = {0,0,1,1};
			//
			if( fluid_accessors[0](i,j) < 0.0 ) {
				for( int nq=0; nq<4; nq++ ) {
					if( ! m_shape.out_of_bounds(query[nq]) ) {
						if( fluid_accessors[0](query[nq]) < 0.0 ) {
							int dim = direction[nq];
							if( area_accessors[0](dim,face[nq]) && rho_accessors[0](dim,face[nq]) ) {
								inside = true;
								break;
							}
						}
					}
				}
			}
			if( inside ) {
				index_map_accessor.set(i,j,index++);
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
		auto index_map_accessors = index_map->get_const_accessors();
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
					double area = area_accessors[tn](dim,face[nq]);
					if( area ) {
						double rho = rho_accessors[tn](dim,face[nq]);
						if( rho ) {
							double value = dt*area/(m_dx*m_dx*rho);
							if( fluid_accessors[tn](query[nq]) < 0.0 ) {
								assert(index_map_accessors[tn].active(query[nq]));
								size_t m_index = index_map_accessors[tn](query[nq]);
								Lhs->add_to_element(n_index,m_index,-value);
							}
							diagonal += value;
						}
					}
					rhs->add(n_index,-sgn[nq]*area*velocity_accessors[tn](dim,face[nq])/m_dx);
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
		auto pressure_acessor = m_pressure.get_serial_accessor();
		m_pressure.clear();
		index_map->const_serial_actives([&](int i, int j, const auto& it) {
			pressure_acessor.set(i,j,result->at(it()));
		});
		//
		// Update the full velocity
		velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
			double rho = rho_accessors[tn](dim,i,j);
			vec2i pi(i,j);
			if( area_accessors[tn](dim,i,j) && rho ) {
				if( pi[dim] == 0 || pi[dim] == velocity.shape()[dim] ) it.set(0.0);
				else {
					it.subtract(dt * (
						+ pressure_acessors[tn](i,j)
						- pressure_acessors[tn](i-(dim==0),j-(dim==1))
					) / (rho*m_dx));
				}
			} else {
				if( pi[dim] == 0 && fluid_accessors[tn](pi) < 0.0 ) it.set(0.0);
				else if( pi[dim] == velocity.shape()[dim] && fluid_accessors[tn](pi-vec2i(dim==0,dim==1)) < 0.0 ) it.set(0.0);
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
		config.get_double("MinDt",m_param.min_dt,"Minimal internal timestep");
		config.get_double("MaxDt",m_param.max_dt,"Maximal internal timestep");
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
		double min_dt {1e-5};
		double max_dt {1.0};
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
