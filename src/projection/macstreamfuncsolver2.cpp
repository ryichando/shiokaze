/*
**	macstreamfuncsolver2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017. 
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
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/array/array2.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/macarray2.h>
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/projection/macproject2_interface.h>
#include <shiokaze/utility/utility.h>
//
SHKZ_USING_NAMESPACE
//
class macstreamfuncsolver2 : public macproject2_interface {
protected:
	//
	LONG_NAME("MAC Streamfunction Solver 2D")
	MODULE_NAME("macstreamfuncsolver2")
	//
	virtual void set_target_volume( double current_volume, double target_volume ) override {
		m_current_volume = current_volume;
		m_target_volume = target_volume;
	}
	//
	virtual void project(double dt,
						macarray2<float> &velocity,
						const array2<float> &solid,
						const array2<float> &fluid,
						const std::vector<signed_rigidbody2_interface *> *rigidbodies ) override {
		//
		shared_macarray2<float> areas(m_shape);
		shared_macarray2<float> rhos(m_shape);
		shared_macarray2<float> E_array(m_shape);
		//
		shared_array2<char> corners(m_shape.nodal());
		shared_array2<char> visited(m_shape.nodal());
		shared_array2<unsigned> corner_remap(m_shape.nodal());
		//
		// Utility function to map X and its vector index
		const auto X = []( unsigned i, unsigned j, unsigned w ) {
			return i+j*w;
		};
		//
		// Compute fractions
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
		// Compute curvature and substitute to the right hand side for the surface tension force
		if( m_param.surftens_k ) {
			//
			// Compute curvature and store them into an array
			shared_array2<float> curvature(fluid.shape());
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
		// Build the linear system
		size_t Lhs_size = m_shape.nodal().count();
		size_t face_size = m_shape.face(0).count()+m_shape.face(1).count();
		//
		// Utility function to retrieve the row of a stream function index
		auto Xp = [&]( unsigned i, unsigned j ) {
			unsigned index = X(i,j,m_shape[0]+1);
			assert(index < Lhs_size);
			return index;
		};
		//
		// Utility function to retrieve the row of a facet index
		auto Xf = [&]( unsigned i, unsigned j, unsigned axis ) {
			//
			unsigned index(0);
			if( axis == 1 ) {
				index = m_shape.face(0).count()+X(i,j,m_shape[0]);
			} else {
				index = X(i,j,m_shape[0]+1);
			}
			assert(index < face_size);
			return index;
		};
		//
		// This function computes the face diagonal area term [A]
		auto face_area_matrix = [&]() {
			std::vector<float> A(face_size,0.0);
			for( int dim : DIMS2 ) {
				m_parallel.for_each(areas()[dim].shape(),[&]( int i, int j, int tn ) {
					unsigned row = Xf(i,j,dim);
					double area = areas()[dim](i,j);
					A[row] = area;
				});
			}
			return A;
		};
		//
		// This function computes the inverse of a face diagonal area term [iA]
		auto inverse_face_area_matrix = [&]( const std::vector<float> &A ) {
			std::vector<float> iA(face_size,0.0);
			m_parallel.for_each(face_size,[&]( unsigned row) {
				if( A[row] ) iA[row] = 1.0 / A[row];
			});
			return iA;
		};
		//
		// This function computes the diagonal face mass term [F]
		auto face_mass_matrix = [&]() {
			std::vector<float> F(face_size);
			for( int dim : DIMS2 ) {
				m_parallel.for_each(rhos()[dim].shape(),[&]( int i, int j, int tn ) {
					unsigned row = Xf(i,j,dim);
					F[row] = rhos()[dim](i,j);
				});
			}
			return F;
		};
		//
		// This function computes the diagonal vecpotential(nodal) mass term [E]
		auto edge_mass_matrix = [&]( const std::vector<float> &F ) {
			std::vector<float> E(Lhs_size,0.0);
			E_array->parallel_all([&](int dim, int i, int j, auto &it, int tn) {
				it.set(F[Xf(i,j,dim)]);
			});
			//
			m_parallel.for_each(m_shape.nodal(),[&]( int i, int j, int tn ) {
				unsigned row = Xp(i,j);
				double rho_sum(0.0);
				double sum(0.0);
				for( int dim : DIMS2 ) {
					rho_sum += E_array()[dim](E_array->shape(dim).clamp(i,j));
					rho_sum += E_array()[dim](E_array->shape(dim).clamp(i-(dim!=0),j-(dim!=1)));
					sum ++;
				}
				if( rho_sum ) E[row] = rho_sum / sum;
			});
			return E;
		};
		//
		// This function computes the curl matrix [C]
		auto curl_matrix = [&]( const std::vector<float> &A ) {
			//
			auto C = m_factory->allocate_matrix(face_size,Lhs_size);
			for( int dim : DIMS2 ) {
				double sign = dim == 0 ? 1.0 : -1.0;
				m_parallel.for_each(m_shape.face(dim),[&]( int i, int j, int tn ) {
					unsigned row = Xf(i,j,dim);
					if( A[row] ) {
						unsigned n_row;
						if( dim==0 ) {
							n_row=Xp(i,j+1);
						} else if( dim==1 ) {
							n_row=Xp(i+1,j);
						}
						C->add_to_element(row,n_row,+sign);
						n_row=Xp(i,j);
						C->add_to_element(row,n_row,-sign);
					}
				});
			};
			return C;
		};
		//
		// This function computes no-flux boundary condition [Z]: [Lhs_size] | [corner_indices]
		auto nullspace_matrix = [&]() {
			//
			// Mark incident corners as "unknown"
			corners->parallel_all([&](int i, int j, auto &it, int tn) {
				for( int dim : DIMS2 ) {
					shape2 face_shape(m_shape.face(dim));
					if( areas()[dim](face_shape.clamp(i,j)) == 0.0 ||
						areas()[dim](face_shape.clamp(i-(dim!=0),j-(dim!=1))) == 0.0 ) {
						it.set(1);
						break;
					}
				}
			});
			// Assign the degree of freedom there
			unsigned corner_indices = 0;
			visited->parallel_all([&](int i, int j, auto &it, int tn) {
				if( solid(i,j) < -sqrt(DIM2)*m_dx ) it.set(true);
			});
			//
			auto markable = [&]( const vec2i &q ) {
				return corners()(q) && ! visited()(q);
			};
			auto recursive_mark = [&]( vec2i node, unsigned corner_indices ) {
				std::stack<vec2i> queue;
				queue.push(node);
				while(! queue.empty()) {
					vec2i q = queue.top();
					queue.pop();
					visited->set(q,1);
					corner_remap->set(q,corner_indices);
					vec2i nq;
					for( unsigned dim : DIMS2 ) {
						if( q[dim]<m_shape[dim] && markable(nq=(q+vec2i(dim==0,dim==1)))) queue.push(nq);
						if( q[dim]>0 && markable(nq=(q-vec2i(dim==0,dim==1)))) queue.push(nq);
					}
				}
			};
			//
			m_shape.nodal().for_each([&]( int i, int j ) {
				if( markable(vec2i(i,j)) ) {
					recursive_mark(vec2i(i,j),++corner_indices);
				}
			});
			//
			// Fix the non-flux boundary there
			auto Z = m_factory->allocate_matrix(Lhs_size,Lhs_size+corner_indices);
			m_parallel.for_each(corner_remap->shape(),[&]( int i, int j, int tn ) {
				unsigned row = Xp(i,j);
				if( corner_remap()(i,j)) {
					Z->add_to_element(row,Lhs_size+corner_remap()(i,j)-1,1.0);
				} else if( solid(i,j) > -m_dx ) {
					Z->add_to_element(row,row,1.0);
				}
			});
			return Z;
		};
		//
		std::vector<float> A = face_area_matrix();
		std::vector<float> iA = inverse_face_area_matrix(A);
		std::vector<float> F = face_mass_matrix();
		std::vector<float> E = edge_mass_matrix(F);
		//
		// Precompute respective matrices
		if( ! m_C ) {
			//
			m_C = curl_matrix(A);
			m_Ct = m_C->transpose();
			m_Z = nullspace_matrix();
			m_CZ = m_C->multiply(m_Z.get());
			m_CZ_t = m_CZ->transpose();
			m_P = m_CZ_t->multiply(m_CZ.get());
		}
		//
		std::vector<float> iAF (face_size);
		m_parallel.for_each(face_size, [&]( size_t row ) {
			iAF[row] = iA[row]*F[row]-1.0;
		});
		//
		// Hacked matrix operations
		auto hacked_multiply = [&]( const RCMatrix_ptr<size_t,double> At, const RCMatrix_ptr<size_t,double> A,
			const std::vector<float> &diag, const std::vector<char> &invalidated, RCMatrix_ptr<size_t,double> result ) {
			//
			assert(diag.size()==A->rows());
			result->initialize(At->rows(),A->columns());
			//
			// Run in parallel
			m_parallel.for_each(At->rows(), [&]( size_t row ) {
				if( ! invalidated[row] ) {
					At->for_each(row,[&]( unsigned a_index, double a ) {
						A->for_each(a_index,[&]( unsigned b_index, double b ) {
							if( b_index < invalidated.size() && ! invalidated[b_index] ) {
								result->add_to_element(row,b_index,diag[a_index]*a*b);
							}
						});
					});
				}
			});
		};
		//
		auto hacked_add = [&]( const RCMatrix_ptr<size_t,double> A, const RCMatrix_ptr<size_t,double> B, 
			const std::vector<char> &invalidated, RCMatrix_ptr<size_t,double> result ) {
			//
			assert( A->rows() <= B->rows() );
			result->copy(B.get());
			//
			m_parallel.for_each(result->rows(),[&]( size_t row ) {
				if( ! invalidated[row] ) {
					if( row < A->rows() ) A->for_each(row,[&]( unsigned index, double value ) {
						result->add_to_element(row,index,value);
					});
				} else {
					result->clear(row);
				}
			});
		};
		//
		// Mark invalidated matrix rows (completely surrounded by air)
		std::vector<char> invalidated_edges(m_CZ_t->rows(),0);
		m_parallel.for_each(Lhs_size,[&]( size_t i ) {
			char invalid (1);
			if( m_P->get(i,i) != 4.0 ) {
				invalid = 0;
			} else {
				if( E[i] ) invalid = 0;
			}
			invalidated_edges[i] = invalid;
		});
		//
		unsigned num_invalidated (0);
		for( char v : invalidated_edges ) if( v ) ++ num_invalidated;
		//
		auto Lhs_A = m_factory->allocate_matrix();
		auto Lhs = m_factory->allocate_matrix();
		//
		hacked_multiply(m_CZ_t,m_CZ,iAF,invalidated_edges,Lhs_A);
		hacked_add(Lhs_A,m_P,invalidated_edges,Lhs);
		//
		// Compute right hand side
		std::vector<float> pu_vector(m_CZ_t->columns());
		rhos->const_parallel_all([&](int dim, int i, int j, auto &it, int tn){
			//
			unsigned row = Xf(i,j,dim);
			pu_vector[row] = velocity[dim](i,j) * it();
		});
		//
		// Assign to the vorticity
		std::vector<float> rhs = m_CZ_t->multiply(pu_vector);
		//
		// Compute difference from previous call and add to the right-hand side
		if( m_param.diff_solve ) {
			//
			if( m_vecpotential.size() != Lhs->rows() ) {
				m_vecpotential.resize(Lhs->rows());
			}
			//
			// Clear out cache outside liquid
			for( unsigned n=0; n<Lhs_size; ++n ) {
				if( ! E[n] && m_vecpotential[n] ) {
					m_vecpotential[n] = 0.0;
				}
			}
			//
			std::vector<float> rhs_diff = Lhs->multiply(m_vecpotential);
			m_parallel.for_each(Lhs->rows(),[&]( size_t row ) {
				rhs[row] -= rhs_diff[row];
			});
		}
		//
		// Solve the linear system
		std::vector<double> result(Lhs->rows());
		{
			std::vector<size_t> compressed_index_map(Lhs->rows());
			unsigned compressed_index (0);
			for( unsigned row=0; row<Lhs->rows(); ++row ) {
				if( ! Lhs->empty(row)) {
					compressed_index_map[row] = ++ compressed_index;
				}
			}
			auto compressed_Lhs = m_factory->allocate_matrix(compressed_index,compressed_index);
			std::vector<double> compressed_rhs(compressed_index);
			m_parallel.for_each(Lhs->rows(),[&]( size_t row ) {
				size_t remap_row = compressed_index_map[row];
				if( remap_row ) {
					Lhs->for_each(row,[&]( unsigned index, double value ) {
						unsigned remap_index = compressed_index_map[index];
						if( remap_index ) {
							compressed_Lhs->add_to_element(remap_row-1,remap_index-1,value);
						}
					});
					compressed_rhs[remap_row-1] = rhs[row];
				}
			});
			//
			std::vector<double> compressed_result;
			m_solver->solve(compressed_Lhs.get(),compressed_rhs,compressed_result);
			//
			if( m_param.diff_solve ) {
				m_parallel.for_each(Lhs->rows(),[&]( size_t row ) {
					size_t remap_row = compressed_index_map[row];
					if( remap_row ) result[row] = compressed_result[remap_row-1];
				});
			}
		}
		//
		// Revert back the final vector potential
		if( m_param.diff_solve ) {
			//
			m_parallel.for_each(Lhs->rows(),[&]( size_t row ) {
				result[row] += m_vecpotential[row];
				m_vecpotential[row] = result[row];
			});
		}
		//
		std::vector<double> vecpotential_result(m_Z->rows());
		m_Z->multiply(result,vecpotential_result);
		//
		// Assign to the vecpotential
		m_vecpotential_array.clear();
		m_vecpotential_array.parallel_all([&]( int i, int j, auto &it ) {
			size_t n = Xp(i,j);
			if( ! Lhs->empty(n) ) it.set(vecpotential_result[n]);
		});
		//
		// Extract velocity
		result.resize(m_CZ->columns());
		iA.resize(m_CZ->rows());
		std::vector<double> u_result = m_CZ->multiply(result);
		//
		velocity.parallel_actives([&]( int dim, int i, int j, auto &it, int tn ) {
			if( areas()[dim](i,j) && rhos()[dim](i,j) ) {
				double area = areas()[dim](i,j);
				unsigned row = Xf(i,j,dim);
				double rho = F[row];
				if( area && rho ) it.set( iA[row] * u_result[row]);
				else it.set(0.0);
				//
				vec2i pi(i,j);
				if( pi[dim] == 0 && it() < 0.0 ) it.set(0.0);
				else if( pi[dim] == velocity.shape()[dim] && it() > 0.0 ) it.set(0.0);
			} else {
				it.set_off();
			}
		});
		//
		if( m_target_volume ) {
			volume_correct(dt,velocity,solid,fluid,areas(),rhos());
		}
	}
	//
	void volume_correct(double dt,
						macarray2<float> &velocity,
						const array2<float> &solid,
						const array2<float> &fluid,
						const macarray2<float> &areas,
						const macarray2<float> &rhos) {
		//
		shared_array2<float> pressure(m_shape);
		//
		// Label cell indices
		size_t index (0);
		shared_array2<size_t> index_map(fluid.shape());
		const auto mark_body = [&]( int i, int j ) {
			//
			bool inside (false);
			if( fluid(i,j) < 0.0 ) {
				//
				vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
				vec2i face[] = {vec2i(i+1,j),vec2i(i,j),vec2i(i,j+1),vec2i(i,j)};
				int direction[] = {0,0,1,1};
				//
				for( int nq=0; nq<4; nq++ ) {
					if( ! m_shape.out_of_bounds(query[nq]) ) {
						if( fluid(query[nq]) < 0.0 ) {
							int dim = direction[nq];
							if( areas[dim](face[nq]) && rhos[dim](face[nq]) ) {
								inside = true;
								break;
							}
						}
					}
				}
			}
			if( inside ) {
				index_map->set(i,j,index++);
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
		// Volume correction
		double rhs_correct = 0.0;
		if( m_param.gain && m_target_volume ) {
			double x = (m_current_volume-m_target_volume)/m_target_volume;
			double y = m_y_prev + x*dt; m_y_prev = y;
			double kp = m_param.gain * 2.3/(25.0*dt);
			double ki = kp*kp/16.0;
			rhs_correct = -(kp*x+ki*y)/(x+1.0);
		}
		//
		index_map->const_parallel_actives([&]( int i, int j, const auto &it, int tn ) {
			//
			size_t n_index = it();
			rhs->set(n_index,rhs_correct);
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
					float area = areas[dim](face[nq]);
					if( area ) {
						float rho = rhos[dim](face[nq]);
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
				}
			}
			Lhs->add_to_element(n_index,n_index,diagonal);
		});
		//
		// Solve the linear system
		auto result = m_factory->allocate_vector();
		m_solver->solve(Lhs.get(),rhs.get(),result.get());
		//
		// Re-arrange to the array
		pressure->clear();
		index_map->const_serial_actives([&](int i, int j, const auto& it) {
			pressure->set(i,j,result->at(it()));
		});
		//
		// Update the full velocity
		velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
			float rho = rhos[dim](i,j);
			if( areas[dim](i,j) && rho ) {
				vec2i pi(i,j);
				if( pi[dim] == 0 || pi[dim] == velocity.shape()[dim] ) it.set(0.0);
				else {
					it.subtract( dt * (
						+ pressure()(m_shape.clamp(i,j))
						- pressure()(m_shape.clamp(i-(dim==0),j-(dim==1)))
					) / (rho*m_dx));
				}
			}
		});
	}
	//
	virtual const array2<float> * get_pressure() const override {
		return nullptr;
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		// Visualize pressure
		if( m_vecpotential_array.shape().count() ) {
			m_gridvisualizer->visualize_nodal_scalar(g,m_vecpotential_array);
		}
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_bool("SecondOrderAccurateFluid",m_param.second_order_accurate_fluid,"Whether to enforce second order accuracy");
		config.get_bool("SecondOrderAccurateSolid",m_param.second_order_accurate_solid,"Whether to enforce second order accuracy for solid surfaces");
		config.get_bool("DrawStreamfunc",m_param.draw_streamfunc);
		config.get_double("SurfaceTension",m_param.surftens_k,"Surface tension force coefficient");
		config.get_double("CorrectionGain",m_param.gain,"Volume correctino gain");
		config.get_bool("DiffSolve",m_param.diff_solve,"Whether we should perform difference-based linear system solve");
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
		m_vecpotential_array.initialize(m_shape.nodal());
		m_target_volume = m_current_volume = m_y_prev = 0.0;
		m_C = nullptr;
	}
	//
	struct Parameters {
		//
		double surftens_k {0.0};
		double gain {1.0};
		bool diff_solve {true};
		bool draw_streamfunc {true};
		bool second_order_accurate_fluid {true};
		bool second_order_accurate_solid {true};
	};
	Parameters m_param;
	//
	shape2 m_shape;
	double m_dx;
	//
	array2<float> m_vecpotential_array{this};
	std::vector<float> m_vecpotential;
	//
	RCMatrix_factory_driver<size_t,double> m_factory{this,"RCMatrix"};
	RCMatrix_solver_driver<size_t,double> m_solver{this,"pcg"};
	RCMatrix_ptr<size_t,double> m_C, m_Ct, m_Z, m_CZ, m_CZ_t, m_P;
	//
	macutility2_driver m_macutility{this,"macutility2"};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	parallel_driver m_parallel{this};
	//
	double m_target_volume {0.0};
	double m_current_volume {0.0};
	double m_y_prev {0.0};
};
//
extern "C" module * create_instance() {
	return new macstreamfuncsolver2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//