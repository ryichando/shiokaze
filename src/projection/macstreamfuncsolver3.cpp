/*
**	macstreamfuncsolver3.cpp
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
#include <shiokaze/array/array3.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/math/RCMatrix_utility.h>
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/projection/macproject3_interface.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/utility/utility.h>
#include <numeric>
//
SHKZ_USING_NAMESPACE
//
template<class T> static const T& permute_clamp(typename macarray3<T>::const_accessor &acessor, int dim, unsigned i, unsigned j, unsigned k, const vec3i &permutation) {
	vec3i original_coord(i,j,k);
	i = original_coord[permutation[0]];
	j = original_coord[permutation[1]];
	k = original_coord[permutation[2]];
	return acessor(dim,acessor.shape().clamp(i,j,k));
}
//
static const double eps = 1e-8;
class macstreamfuncsolver3 : public macproject3_interface {
private:
	//
	LONG_NAME("MAC Streamfunction Solver 3D")
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
		timer.tick(); console::dump( ">>> Streamfunc projection started (%dx%dx%d)...\n", m_shape[0], m_shape[1], m_shape[2] );
		//
		shared_macarray3<double> areas(m_shape);
		shared_macarray3<double> rhos(m_shape);
		shared_macarray3<double> E_array(m_shape);
		shared_array3<char> visited(m_shape.nodal());
		shared_array3<char> corner_remap(m_shape.nodal());
		shared_array3<char> fixed(m_shape.nodal());
		std::vector<bool> solid_corner;
		std::vector<bool> solid_edge;
		//
		// Function to convert a coordinate into an index
		auto X = []( unsigned i, unsigned j, unsigned k, unsigned w, unsigned h ) {
			return i+j*w+(w*h)*k;
		};
		//
		// Make accessors
		auto fluid_accessors = fluid.get_const_accessors();
		auto solid_accessors = solid.get_const_accessors();
		auto rho_accessors = rhos().get_const_accessors();
		auto area_accessors = areas().get_const_accessors();
		auto velocity_accessors = velocity.get_const_accessors();
		auto E_acessors = E_array->get_const_accessors();
		//
		// Pre-compute solid cut "areas" and fluid density "rhos" for each cell facet
		timer.tick(); console::dump( "Precomputing solid and fluid fractions...");
		//
		m_macutility->compute_area_fraction(solid,areas());
		m_macutility->compute_fluid_fraction(fluid,rhos());
		//
		console::dump( "Done. Took %s\n", timer.stock("solid_fluid_fractions").c_str());
		//
		// Compute curvature and substitute to the right hand side for the surface tension force
		if( m_param.surftens_k ) {
			timer.tick(); console::dump( "Computing surface tension force (%.2e)...\n", m_param.surftens_k );
			double kappa = m_param.surftens_k;
			//
			// Compute curvature and store them into an array
			shared_array3<double> curvature(fluid.shape());
			curvature->parallel_op([&]( int i, int j, int k, auto &it, int tn ) {
				double value = (
					+fluid_accessors[tn](m_shape.clamp(i-1,j,k))+fluid_accessors[tn](m_shape.clamp(i+1,j,k))
					+fluid_accessors[tn](m_shape.clamp(i,j-1,k))+fluid_accessors[tn](m_shape.clamp(i,j+1,k))
					+fluid_accessors[tn](m_shape.clamp(i,j,k-1))+fluid_accessors[tn](m_shape.clamp(i,j,k+1))
					-6.0*fluid_accessors[tn](i,j,k)
				) / (m_dx*m_dx);
				it.set(value);
			});
			//
			auto curvature_acessors = curvature->get_const_accessors();
			velocity.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn ) {
				double rho = rho_accessors[tn](dim,i,j,k);
				// Embed ordinary 2nd order surface tension force
				if( rho && rho < 1.0 ) {
					double sgn = fluid_accessors[tn](m_shape.clamp(i,j,k)) < 0.0 ? -1.0 : 1.0;
					double theta = sgn < 0 ? 1.0-rho : rho;
					double face_c = 
						theta*curvature_acessors[tn](m_shape.clamp(i,j,k))
						+(1.0-theta)*curvature_acessors[tn](m_shape.clamp(i-(dim==0),j-(dim==1),k-(dim==2)));
					it.increment( -sgn * dt / (m_dx*rho) * kappa * face_c );
				}
			});
			console::dump( "Done. Took %s\n", timer.stock("surftension_force_add_to_velocity").c_str());
		}
		//
		// Allocate matrices and vectors
		unsigned Lhs_size = 0;
		unsigned face_size = 0;
		unsigned corner_size = m_shape.nodal().count();
		for( unsigned dim : DIMS3 ) {
			Lhs_size += (m_shape[0]+(dim!=0))*(m_shape[1]+(dim!=1))*(m_shape[2]+(dim!=2));
			face_size += (m_shape[0]+(dim==0))*(m_shape[1]+(dim==1))*(m_shape[2]+(dim==2));
		}
		//
		// Utility function to exchange coordinate by the permutation
		auto permute = [](unsigned &i, unsigned &j, unsigned &k, const vec3i &pm) {
			vec3i original_coord(i,j,k);
			i = original_coord[pm[0]];
			j = original_coord[pm[1]];
			k = original_coord[pm[2]];
		};
		//
		// Utility function to inverse permutation
		auto inv_permutation = [&]( vec3i pm ) {
			vec3i mp;
			mp[0] = pm[0]==0 ? 0 : (pm[1] == 0 ? 1 : 2);
			mp[1] = pm[0]==1 ? 0 : (pm[1] == 1 ? 1 : 2);
			mp[2] = pm[0]==2 ? 0 : (pm[1] == 2 ? 1 : 2);
			return mp;
		};
		//
		// Utility function to retrieve the row of a vector potential index
		auto Xp = [&](unsigned i, unsigned j, unsigned k, unsigned axis, const vec3i &pm) {
			permute(i,j,k,pm);
			unsigned index = 0;
			for( unsigned n=0; n<axis; n++ ) index += (m_shape[0]+(n!=0))*(m_shape[1]+(n!=1))*(m_shape[2]+(n!=2));
			if( axis == 0 ) index += X(i,j,k,m_shape[0],m_shape[1]+1);
			else if( axis == 1 ) index += X(i,j,k,m_shape[0]+1,m_shape[1]);
			else index += X(i,j,k,m_shape[0]+1,m_shape[1]+1);
			assert(index < Lhs_size);
			return index;
		};
		//
		// Utility function to retrieve the row of a facet index
		auto Xf = [&](unsigned i, unsigned j, unsigned k, unsigned axis, const vec3i &pm) {
			permute(i,j,k,pm);
			unsigned index = 0;
			for( unsigned n=0; n<axis; n++ ) index += (m_shape[0]+(n==0))*(m_shape[1]+(n==1))*(m_shape[2]+(n==2));
			if( axis == 0 ) index += X(i,j,k,m_shape[0]+1,m_shape[1]);
			else if( axis == 1 ) index += X(i,j,k,m_shape[0],m_shape[1]+1);
			else index += X(i,j,k,m_shape[0],m_shape[1]);
			assert(index < face_size);
			return index;
		};
		//
		// This function computes facet diagonal mass term [A]
		auto face_area_matrix = [&]() {
			std::vector<double> A(face_size,0.0);
			for( int dim : DIMS3 ) {
				m_parallel.for_each(areas()[dim].shape(),[&]( int i, int j, int k, int tn ) {
					unsigned row = Xf(i,j,k,dim,vec3i(0,1,2));
					double area = areas()[dim](i,j,k);
					A[row] = area;
				});
			}
			return A;
		};
		//
		// This function computes the inverse of facet diagonal mass term [iA]
		auto inverse_face_area_matrix = [&]( const std::vector<double> &A ) {
			std::vector<double> iA(face_size,0.0);
			m_parallel.for_each(face_size,[&]( size_t row ) {
				if( A[row] ) iA[row] = 1.0/A[row];
			});
			return iA;
		};
		//
		// This function computes facet diagonal mass term [F]
		auto face_mass_matrix = [&]() {
			std::vector<double> F(face_size,0.0);
			for( int dim : DIMS3 ) {
				m_parallel.for_each(areas()[dim].shape(),[&]( int i, int j, int k, int tn ) {
					unsigned row = Xf(i,j,k,dim,vec3i(0,1,2));
					 F[row] = rho_accessors[tn](dim,i,j,k);
				});
			};
			return F;
		};
		//
		// This function computes edge diagonal mass term [E]
		auto edge_mass_matrix = [&]( const std::vector<double> &F ) {
			E_array->parallel_all([&](int dim, int i, int j, int k, auto &it, int tn) {
				it.set(F[Xf(i,j,k,dim,vec3i(0,1,2))]);
			});
			//
			std::vector<double> E(Lhs_size,0.0);
			m_parallel.for_each(DIM3,[&]( size_t dim ) {
				vec3i mp((dim+1)%DIM3,(dim+2)%DIM3,dim); // Let's pretend that we are solving for z
				vec3i pm = inv_permutation(mp);
				m_parallel.for_each(shape3(m_shape[mp[0]]+1,m_shape[mp[1]]+1,m_shape[mp[2]]),[&]( int i, int j, int k, int tn ) {
					unsigned row = Xp(i,j,k,dim,pm);
					double rho_sum(0.0);
					double sum(0.0);
					for( int dir=-1; dir<=0; ++dir ) for( int dim=0; dim<2; ++dim ) {
						double rho = permute_clamp<double>(E_acessors[tn],mp[1-dim],i+dir*(dim==0),j+dir*(dim==1),k,pm);
						rho_sum += rho;
						sum ++;
					}
					if( sum ) {
						E[row] = rho_sum /= sum;
					}
				});
			});
			return E;
		};
		//
		// This function computes corner diagonal mass term [V]
		auto corner_mass_matrix = [&]( const std::vector<double> &E ) {
			std::vector<double> V(corner_size,0.0);
			m_parallel.for_each(m_shape.nodal(),[&](int i, int j, int k) {
				unsigned row = X(i,j,k,m_shape[0]+1,m_shape[1]+1);
				double rho_sum(0.0);
				double sum(0.0);
				for( unsigned dim : DIMS3 ) {
					vec3i pi(i,j,k);
					if( pi[dim] > 0 ) {
						rho_sum += E[Xp(i-(dim==0),j-(dim==1),k-(dim==2),dim,vec3i(0,1,2))];
						sum += 1.0;
					}
					if( pi[dim] < m_shape[dim] ) {
						rho_sum += E[Xp(i,j,k,dim,vec3i(0,1,2))];
						sum += 1.0;
					}
				}
				if( sum ) V[row] = rho_sum / sum;
			});
			return V;
		};
		//
		// This function computes the curl matrix [C]
		auto curl_matrix = [&]( const std::vector<double> &A ) {
			//
			auto C = m_factory->allocate_matrix(face_size,Lhs_size);
			m_parallel.for_each(DIM3,[&]( size_t dim ) {
				vec3i mp((dim+1)%DIM3,(dim+2)%DIM3,dim); // Let's pretend that we are solving for z
				vec3i pm = inv_permutation(mp);
				m_parallel.for_each(shape3(m_shape[mp[0]],m_shape[mp[1]],m_shape[mp[2]]+1),[&]( int i, int j, int k ) {
					unsigned row = Xf(i,j,k,dim,pm);
					if( A[row] ) {
						unsigned n_row;
						n_row=Xp(i+1,j,k,mp[1],pm); C->add_to_element(row,n_row,1.0);
						n_row=Xp(i,j,k,mp[1],pm); C->add_to_element(row,n_row,-1.0);
						n_row=Xp(i,j+1,k,mp[0],pm); C->add_to_element(row,n_row,-1.0);
						n_row=Xp(i,j,k,mp[0],pm); C->add_to_element(row,n_row,1.0);
					}
				});
			});
			return C;
		};
		//
		// This function computes the nullspace of vector potential [Z] = [Lhs_size] | [corner_size]
		auto nullspace_matrix = [&]( unsigned &num_topology, std::vector<bool> &solid_corner, std::vector<bool> &solid_edge ) {
			//
			auto Z = m_factory->allocate_matrix(Lhs_size,Lhs_size+m_shape.nodal().count());
			for( unsigned dim : DIMS3 ) {
				auto remappable = [&]( unsigned i, unsigned j, unsigned k ) {
					return ! corner_remap()(i,j,k) && solid(i,j,k) > -sqrt(DIM3)*m_dx;
				};
				areas().const_serial_all([&]( int dim, int i, int j, int k, const array3<double>::const_iterator &it ) {
					if( it() == 0.0 ) {
						if( dim == 0 ) {
							if( remappable(i,j,k)) corner_remap->set(i,j,k,1);
							if( remappable(i,j+1,k)) corner_remap->set(i,j+1,k,1);
							if( remappable(i,j+1,k+1)) corner_remap->set(i,j+1,k+1,1);
							if( remappable(i,j,k+1)) corner_remap->set(i,j,k+1,1);
						} else if( dim == 1 ) {
							if( remappable(i,j,k)) corner_remap->set(i,j,k,1);
							if( remappable(i+1,j,k)) corner_remap->set(i+1,j,k,1);
							if( remappable(i+1,j,k+1)) corner_remap->set(i+1,j,k+1,1);
							if( remappable(i,j,k+1)) corner_remap->set(i,j,k+1,1);
						} else if( dim == 2 ) {
							if( remappable(i,j,k)) corner_remap->set(i,j,k,1);
							if( remappable(i+1,j,k)) corner_remap->set(i+1,j,k,1);
							if( remappable(i+1,j+1,k)) corner_remap->set(i+1,j+1,k,1);
							if( remappable(i,j+1,k)) corner_remap->set(i,j+1,k,1);
						}
					}
				});
			}
			//
			// Mark solid corner
			solid_corner.resize(corner_size,0);
			m_parallel.for_each(corner_remap->shape(),[&](int i, int j, int k) {
				solid_corner[X(i,j,k,m_shape[0]+1,m_shape[1]+1)] = corner_remap()(i,j,k);
			});
			//
			// Mark solid edge
			solid_edge.resize(Lhs_size);
			for( int dim : DIMS3 ) {
				m_parallel.for_each(m_shape.edge(dim),[&]( int i, int j, int k ) {
					unsigned row = Xp(i,j,k,dim,vec3i(0,1,2));
					unsigned forward = solid_corner[X(i+(dim==0),j+(dim==1),k+(dim==2),m_shape[0]+1,m_shape[1]+1)];
					unsigned backward = solid_corner[X(i,j,k,m_shape[0]+1,m_shape[1]+1)];
					solid_edge[row] = forward && backward;
				});
			}
			//
			// Fix dirishlet value at the one of the arbitrary node in topological connected regions
			auto markable = [&]( const vec3i &q ) {
				return corner_remap()(q) && ! visited()(q);
			};
			auto recursive_mark = [&]( vec3i node ) {
				std::stack<vec3i> queue;
				queue.push(node);
				while(! queue.empty()) {
					vec3i q = queue.top();
					queue.pop();
					visited->set(q,1);
					vec3i nq;
					for( unsigned dim : DIMS3 ) {
						if( q[dim]<m_shape[dim] && markable(nq=(q+vec3i(dim==0,dim==1,dim==2)))) queue.push(nq);
						if( q[dim]>0 && markable(nq=(q-vec3i(dim==0,dim==1,dim==2)))) queue.push(nq);
					}
				}
			};
			//
			num_topology = 0;
			fixed->serial_op([&](int i, int j, int k, auto &it ) {
				if( markable(vec3i(i,j,k)) ) {
					it.set(1);
					++ num_topology;
					recursive_mark(vec3i(i,j,k));
				}
			});
			//
			for( int dim : DIMS3 ) {
				m_parallel.for_each(m_shape.edge(dim),[&]( int i, int j, int k ) {
					unsigned row = Xp(i,j,k,dim,vec3i(0,1,2));
					char nfixed[2] = { fixed()(i+(dim==0),j+(dim==1),k+(dim==2)), fixed()(i,j,k) };
					unsigned forward = corner_remap()(i+(dim==0),j+(dim==1),k+(dim==2));
					unsigned backward = corner_remap()(i,j,k);
					double solid_value = 0.5*(solid(i+(dim==0),j+(dim==1),k+(dim==2))+solid(i,j,k));
					if( forward && backward ) {
						if( ! nfixed[0] ) Z->add_to_element(row,Lhs_size+X(i+(dim==0),j+(dim==1),k+(dim==2),m_shape[0]+1,m_shape[1]+1),1.0);
						if( ! nfixed[1] ) Z->add_to_element(row,Lhs_size+X(i,j,k,m_shape[0]+1,m_shape[1]+1),-1.0);
					} else if( solid_value > -m_dx ) {
						Z->add_to_element(row,row,1.0);
					}
				});
			}
			return Z;
		};
		//
		// This function computes the divergence matrix [D]
		auto divergence_matrix = [&]( std::vector<bool> &solid_corner, std::vector<bool> &solid_edge, RCMatrix_interface<size_t,double> *D ) {
			//
			// Divergence operator for vector potential
			D->initialize(corner_size,Lhs_size);
			m_parallel.for_each(m_shape.nodal(),[&](int i, int j, int k) {
				unsigned row = X(i,j,k,m_shape[0]+1,m_shape[1]+1);
				if( ! solid_corner[row]) {
					vec3i pm(0,1,2);
					vec3i pi(i,j,k);
					for( unsigned dim : DIMS3 ) {
						unsigned column;
						if( pi[dim] > 0 ) {
							column = Xp(i-(dim==0),j-(dim==1),k-(dim==2),dim,pm);
							if( ! solid_edge[column] ) D->add_to_element(row,column,1.0);
						}
						if( pi[dim] < m_shape[dim] ) {
							column = Xp(i,j,k,dim,pm);
							if( ! solid_edge[column] ) D->add_to_element(row,column,-1.0);
						}
					}
				}
			});
		};
		//
		timer.tick(); console::dump( ">>> Building the linear system...\n");
		console::dump( "::: face_size = %d\n", face_size );
		console::dump( "::: vecpotential_size = %d\n", Lhs_size);
		console::dump( "::: corner_size = %d\n", corner_size);
		//
		// Build face area matrix
		timer.tick(); console::dump( "Computing [A] and [iA]...");
		//
		std::vector<double> A = face_area_matrix();
		std::vector<double> iA = inverse_face_area_matrix(A);
		//
		console::dump( "Done. Sum=%.4e. Took %s\n", std::accumulate(A.begin(),A.end(),0.0),
			timer.stock("buildmatrix_matrices_A_and_iA").c_str());
		//
		// Build face mass matrix
		timer.tick(); console::dump( "Computing [F]...");
		std::vector<double> F = face_mass_matrix();
		console::dump( "Done. Avge=%.4e. Took %s\n", std::accumulate(F.begin(),F.end(),0.0),
			timer.stock("buildmatrix_matrices_F").c_str());
		//
		// Build edge mass matrix
		timer.tick(); console::dump( "Computing [E]...");
		std::vector<double> E = edge_mass_matrix(F);
		console::dump( "Done. Avge=%.4e. Took %s\n", std::accumulate(E.begin(),E.end(),0.0),
			timer.stock("buildmatrix_matrices_E").c_str());
		//
		// Build corner mass matrix
		timer.tick(); console::dump( "Computing [V]...");
		std::vector<double> V = corner_mass_matrix(E);
		console::dump( "Done. Avge=%.4e. Took %s\n", std::accumulate(V.begin(),V.end(),0.0),
			timer.stock("buildmatrix_matrices_V").c_str());
		//
		// Precompute respective matrices
		if( ! m_C ) {
			timer.tick(); console::dump( ">>> Precomputing matrices...\n");
			timer.tick(); console::dump( "Computing [C], [C]^T and [Z]...");
			//
			{	std::vector<std::function<void()> > operations;
				operations.push_back([&](){
					// Build the full curl matrix
					m_C = curl_matrix(A);
					m_Ct = m_C->transpose();
				});
				operations.push_back([&](){
					// Precompute full nullspace matrix
					unsigned num_topology;
					m_Z = nullspace_matrix(num_topology,solid_corner,solid_edge);
				});
				m_parallel.run(operations);
			}
			console::dump( "Done. Took %s\n", timer.stock("precompute_C_and_Z").c_str());
			//
			timer.tick(); console::dump( "Computing [CZ] = [C][Z], [CZ]^T, [D], [DZ], [DZ]^T...");
			{	std::vector<std::function<void()> > operations;
				operations.push_back([&](){
					// Precompute [CZ], [CZ]^T
					m_CZ = m_C->multiply(m_Z.get());
					m_CZ_t = m_CZ->transpose();
				});
				operations.push_back([&](){
					// Precompute [DZ], [DZ]^T
					auto D = m_factory->allocate_matrix();
					divergence_matrix(solid_corner,solid_edge,D.get());
					m_DZ = D->multiply(m_Z.get());
					m_DZ_t = m_DZ->transpose();
				});
				m_parallel.run(operations);
			}
			console::dump( "Done. Took %s\n", timer.stock("precompute_matrices").c_str());
			//
			timer.tick(); console::dump( "Computing [P] = [CZ]^T[CZ]+[DZ]^T[DZ]...");
			auto CZtCZ = m_factory->allocate_matrix(), DZtDZ = m_factory->allocate_matrix();
			{	std::vector<std::function<void()> > operations;
				operations.push_back([&](){
					CZtCZ = m_CZ_t->multiply(m_CZ.get());
				});
				operations.push_back([&](){
					DZtDZ = m_DZ_t->multiply(m_DZ.get());
				});
				m_parallel.run(operations);
			}
			//
			m_P = CZtCZ->add(DZtDZ.get());
			//
			console::dump( "Done. Took %s\n", timer.stock("buildmatrix_P").c_str());
			console::dump( "<<< Done. Took %s\n", timer.stock("precompute_matrix").c_str());
		}
		//
		timer.tick(); console::dump( "Computing [iAF] and [iV]...");
		std::vector<double> iAF (face_size);
		std::vector<double> iV (corner_size);
		m_parallel.for_each(face_size,[&]( size_t row ) {
			iAF[row] = iA[row]*F[row]-1.0;
		});
		//
		m_parallel.for_each(corner_size, [&]( size_t row ) {
			iV[row] = V[row]-1.0;
		});
		console::dump("Done. Took %s\n", timer.stock("buildmatrix_iAF_iV").c_str());
		//
		// Hacked matrix operations
		auto hacked_multiply = [&]( const RCMatrix_ptr<size_t,double> At, const RCMatrix_ptr<size_t,double> A,
			const std::vector<double> &diag, const std::vector<char> &invalidated, RCMatrix_ptr<size_t,double> result ) {
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
		auto hacked_add = [&]( const RCMatrix_ptr<size_t,double> A, const RCMatrix_ptr<size_t,double> B, const RCMatrix_ptr<size_t,double> C, const std::vector<char> &invalidated, RCMatrix_ptr<size_t,double> result ) {
			//
			assert( A->rows() <= C->rows() && B->rows() <= C->rows() );
			result->copy(C.get());
			//
			m_parallel.for_each(result->rows(),[&]( size_t row ) {
				if( ! invalidated[row] ) {
					if( row < A->rows() ) A->for_each(row,[&]( unsigned index, double value ) {
						result->add_to_element(row,index,value);
					});
					if( row < B->rows() ) B->for_each(row,[&]( unsigned index, double value ) {
						result->add_to_element(row,index,value);
					});
				} else {
					result->clear(row);
				}
			});
		};
		//
		// Mark invalidated matrix rows (completely surrounded by air)
		timer.tick(); console::dump( "Invalidating edges...");
		std::vector<char> invalidated_edges(m_CZ_t->rows(),0);
		m_parallel.for_each(Lhs_size,[&]( size_t i ) {
			char invalid (1);
			if( m_P->get(i,i) != 6.0 ) {
				invalid = 0;
			} else {
				if( std::abs(E[i]) > eps ) invalid = 0;
			}
			invalidated_edges[i] = invalid;
		});
		//
		unsigned num_invalidated (0);
		for( char v : invalidated_edges ) if( v ) ++ num_invalidated;
		console::dump("Done. Invalidated %d edges. Took %s\n", num_invalidated, timer.stock("buildmatrix_invalidate").c_str());
		console::write(get_argument_name()+"_buildmatrix_invalidate_num",num_invalidated);
		//
		auto L = m_factory->allocate_matrix(), R = m_factory->allocate_matrix();
		//
		timer.tick(); console::dump( "Computing [L] and [R]...");
		std::vector<std::function<void()> > assemble_operations;
		assemble_operations.push_back([&](){
			hacked_multiply(m_CZ_t,m_CZ,iAF,invalidated_edges,L);
		});
		assemble_operations.push_back([&](){
			hacked_multiply(m_DZ_t,m_DZ,iV,invalidated_edges,R);
		});
		m_parallel.run(assemble_operations);
		console::dump("Done. Took %s\n", timer.stock("buildmatrix_L_and_R").c_str());
		//
		timer.tick(); console::dump( "Computing [Lhs] = [L]+[R]+[P]...");
		auto Lhs = m_factory->allocate_matrix();
		hacked_add(L,R,m_P,invalidated_edges,Lhs);
		console::dump("Done. Took %s\n", timer.stock("buildmatrix_Lhs").c_str());
		//
		// Compute intermediate full vorticity
		timer.tick(); console::dump( "Building [pu] = [rho][u]...");
		std::vector<double> pu_vector(m_CZ_t->columns());
		m_parallel.for_each(DIM3,[&]( size_t dim ) {
			 m_parallel.for_each(rhos()[dim].shape(),[&]( int i, int j, int k, int tn ) {
				unsigned row = Xf(i,j,k,dim,vec3i(0,1,2));
				double rho = rho_accessors[tn](dim,i,j,k);
				//
				// Add fluid momentum
				pu_vector[row] = velocity[dim](i,j,k) * rho;
			});
		});
		console::dump("Done. Took %s\n", timer.stock("pu").c_str());
		//
		// Build right hand side vector
		timer.tick(); console::dump( "Building right hand side [rhs~] = [CZ]^T[pu]...");
		std::vector<double> rhs = m_CZ_t->multiply(pu_vector);
		//
		console::dump("Done. Took %s\n", timer.stock("rhs_full").c_str());
		console::dump("<<< Done. Took %s\n", timer.stock("build_linsystem").c_str());
		//
		// Compute difference from previous call and add to the right-hand side
		if( m_param.diff_solve ) {
			if( m_vecpotential.size() != Lhs->rows() ) {
				timer.tick(); console::dump( "%s vectorpotential cache [~x] from (%d) to (%d). Diff=(%d)...",
							 m_vecpotential.size() < Lhs->rows() ? "Expanding" : "Shrinking", m_vecpotential.size(), Lhs->rows(), (int)Lhs->rows()-(int)m_vecpotential.size());
				m_vecpotential.resize(Lhs->rows());
				console::dump("Done. Took %s\n", timer.stock("cache_resize").c_str());
			}
			//
			// Clear out cache outside liquid
			timer.tick(); console::dump( "Clearing out cache outside fluid...");
			unsigned cleared[2] = { 0, 0 };
			for( unsigned n=0; n<Lhs_size; ++n ) {
				if( ! E[n] && m_vecpotential[n] ) {
					m_vecpotential[n] = 0.0;
					++ cleared[0];
				}
			}
			//
			m_parallel.for_each(m_shape.nodal(),[&]( int i, int j, int k ) {
				unsigned row = X(i,j,k,m_shape[0]+1,m_shape[1]+1);
				if( Lhs_size+row < Lhs->rows() ) {
					if( ! V.at(row) && m_vecpotential.at(Lhs_size+row) ) {
						m_vecpotential.at(Lhs_size+row) = 0.0;
						++ cleared[1];
					}
				}
			});
			//
			console::dump("Done. Cleared (%d) fluid and (%d) solid  Took %s\n", cleared[0], cleared[1], timer.stock("cache_clear").c_str());
			console::write(get_argument_name()+"_clear_cache_fluid", cleared[0]);
			console::write(get_argument_name()+"_clear_cache_solid", cleared[1]);
			//
			timer.tick(); console::dump( "Building the difference of the right hand side [rhs] = [rhs~]-[Lhs][x~]...");
			std::vector<double> rhs_diff = Lhs->multiply(m_vecpotential);
			m_parallel.for_each(Lhs->rows(),[&]( size_t row ) {
				rhs[row] -= rhs_diff[row];
			});
			console::dump("Done. Took %s\n", timer.stock("rhs_diff").c_str());
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
			RCMatrix_utility<size_t,double>::report(compressed_Lhs.get(),"Lhs");
			//
			timer.tick(); console::dump( "Solving the linear system...");
			std::vector<double> compressed_result;
			unsigned count = m_solver->solve(compressed_Lhs.get(),compressed_rhs,compressed_result);
			console::write(get_argument_name()+"_number_projection_iteration", count);
			console::dump( "Done. Took %d iterations. Took %s\n", count, timer.stock("linsolve").c_str());
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
			timer.tick(); console::dump( "Converting to the full solution [x] = [x^]+[x~]...");
			m_parallel.for_each(Lhs->rows(),[&]( size_t row ) {
				result[row] += m_vecpotential[row];
				m_vecpotential[row] = result[row];
			});
			console::dump( "Done. Took %s\n", timer.stock("extract_full_solution").c_str());
		}
		//
		// Extract velocity
		timer.tick(); console::dump( "Converting to velocity [u] = [iA][CZ][x]...");
		result.resize(m_CZ->columns());
		iA.resize(m_CZ->rows());
		std::vector<double> u_result = m_CZ->multiply(result);
		//
		velocity.parallel_actives([&]( int dim, int i, int j, int k, auto &it, int tn ) {
			if( area_accessors[tn](dim,i,j,k) && rho_accessors[tn](dim,i,j,k) ) {
				double area = area_accessors[tn](dim,i,j,k);
				unsigned row = Xf(i,j,k,dim,vec3i(0,1,2));
				double rho = F[row];
				if( area && rho ) it.set( iA[row] * u_result[row] );
				else it.set_off();
				//
				vec3i pi(i,j,k);
				if( pi[dim] == 0 || pi[dim] == velocity.shape()[dim] ) it.set(0.0);
			} else {
				it.set_off();
			}
		});
		//
		console::dump( "Done. Took %s\n", timer.stock("extract_velocity").c_str());
		console::dump( "<<< Projection done. Took %s.\n", timer.stock("projection").c_str());
		//
		if( m_target_volume ) {
			volume_correct(dt,velocity,solid,fluid,areas(),rhos());
		}
	}
	void volume_correct(double dt,
						macarray3<double> &velocity,
						const array3<double> &solid,
						const array3<double> &fluid,
						const macarray3<double> &areas,
						const macarray3<double> &rhos) {
		//
		//
		scoped_timer timer(this);
		//
		timer.tick(); console::dump( ">>> Volume Corrective Projection started...\n" );
		//
		// Make accessors
		shared_array3<double> pressure(m_shape);
		auto fluid_accessors = fluid.get_const_accessors();
		auto solid_accessors = solid.get_const_accessors();
		auto pressure_acessors = pressure->get_const_accessors();
		auto rho_accessors = rhos.get_const_accessors();
		auto area_accessors = areas.get_const_accessors();
		auto velocity_accessors = velocity.get_const_accessors();
		//
		// The target linear system to build
		timer.tick(); console::dump( "Building the high-res linear system [Lhs] and [rhs]..." );
		//
		// Label cell indices
		size_t index (0);
		shared_array3<size_t> index_map(fluid.shape());
		auto index_map_accessor = index_map->get_serial_accessor();
		const auto mark_body = [&]( int i, int j, int k ) {
			//
			bool inside (false);
			if( fluid_accessors[0](i,j,k) < 0.0 ) {
				//
				vec3i query[] = {vec3i(i+1,j,k),vec3i(i-1,j,k),vec3i(i,j+1,k),vec3i(i,j-1,k),vec3i(i,j,k+1),vec3i(i,j,k-1)};
				vec3i face[] = {vec3i(i+1,j,k),vec3i(i,j,k),vec3i(i,j+1,k),vec3i(i,j,k),vec3i(i,j,k+1),vec3i(i,j,k)};
				int direction[] = {0,0,1,1,2,2};
				//
				for( int nq=0; nq<6; nq++ ) {
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
				index_map_accessor.set(i,j,k,index++);
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
		auto index_map_accessors = index_map->get_const_accessors();
		//
		// Volume correction
		double rhs_correct = 0.0;
		if( m_param.gain && m_target_volume ) {
			timer.tick(); console::dump( "Computing volume correction...");
			double x = (m_current_volume-m_target_volume)/m_target_volume;
			double y = m_y_prev + x*dt; m_y_prev = y;
			double kp = m_param.gain * 2.3/(25.0*dt);
			double ki = kp*kp/16.0;
			rhs_correct = -(kp*x+ki*y)/(x+1.0);
			console::dump( "Done. Took %s\n", timer.stock("volume_correction").c_str());
			console::write(get_argument_name()+"_volume_correct_rhs", rhs_correct);
		}
		//
		index_map->const_parallel_actives([&]( int i, int j, int k, const auto &it, int tn ) {
			//
			size_t n_index = it();
			rhs->set(n_index,rhs_correct);
			//
			if( fluid_accessors[tn](i,j,k) < 0.0 ) {
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
							rhs->add(n_index,-sgn[nq]*area*velocity_accessors[tn](dim,face[nq])/m_dx);
						}
					}
				}
				Lhs->add_to_element(n_index,n_index,diagonal);
			}
		});
		//
		console::dump( "Done. Took %s\n", timer.stock("build_highres_linsystem").c_str());
		//
		RCMatrix_utility<size_t,double>::report(Lhs.get(),"Lhs");
		//
		// Solve the linear system
		timer.tick(); console::dump( "Solving the linear system...");
		auto result = m_factory->allocate_vector();
		unsigned count = m_solver->solve(Lhs.get(),rhs.get(),result.get());
		console::write(get_argument_name()+"_number_volume_correction_projection_iteration", count);
		console::dump( "Done. Took %d iterations. Took %s\n", count, timer.stock("linsolve").c_str());
		//
		// Re-arrange to the array
		auto pressure_acessor = pressure->get_serial_accessor();
		pressure->clear();
		index_map->const_serial_actives([&](int i, int j, int k, const auto& it) {
			pressure_acessor.set(i,j,k,result->at(it()));
		});
		//
		// Update the full velocity
		timer.tick(); console::dump( "Updating the velocity...");
		velocity.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn ) {
			double rho = rho_accessors[tn](dim,i,j,k);
			if( area_accessors[tn](dim,i,j,k) && rho ) {
				velocity[dim].subtract(i,j,k, dt * (
					+ pressure_acessors[tn](m_shape.clamp(i,j,k))
					- pressure_acessors[tn](m_shape.clamp(i-(dim==0),j-(dim==1),k-(dim==2)))
					) / (rho*m_dx));
				//
				vec3i pi(i,j,k);
				if( pi[dim] == 0 && it() < 0.0 ) it.set(0.0);
				else if( pi[dim] == velocity.shape()[dim] && it() > 0.0 ) it.set(0.0);
				//
			} else {
				it.set_off();
			}
		});
		console::dump( "Done. Took %s\n", timer.stock("update_velocity").c_str());
		//
		console::dump( "<<< Projection done. Took %s.\n", timer.stock("projection").c_str());
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_double("SurfaceTension",m_param.surftens_k,"Surface tension force coefficient");
		config.get_double("CorrectionGain",m_param.gain,"Volume correctino gain");
		config.get_bool("DiffSolve",m_param.diff_solve,"Whether we should perform difference-based linear system solve");
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	virtual void post_initialize() override {
		//
		m_C = nullptr;
		m_target_volume = m_current_volume = m_y_prev = 0.0;
	}
	//
	struct Parameters {
		//
		double surftens_k {0.0};
		double gain {1.0};
		bool diff_solve {true};
	};
	Parameters m_param;
	//
	RCMatrix_factory_driver<size_t,double> m_factory{this,"RCMatrix"};
	RCMatrix_solver_driver<size_t,double> m_solver{this,"pcg"};
	RCMatrix_ptr<size_t,double> m_C, m_Ct, m_Z, m_CZ, m_CZ_t, m_DZ, m_DZ_t, m_P;
	//
	shape3 m_shape;
	double m_dx {0.0};
	std::vector<double> m_vecpotential;
	//
	macutility3_driver m_macutility{this,"macutility3"};
	parallel_driver m_parallel{this};
	//
	double m_target_volume {0.0};
	double m_current_volume {0.0};
	double m_y_prev {0.0};
	//
};
//
extern "C" module * create_instance() {
	return new macstreamfuncsolver3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//