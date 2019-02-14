/*
**	macbackwardflip3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 9, 2017. 
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
#include "macbackwardflip3.h"
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <numeric>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
void macbackwardflip3::configure( configuration &config ) {
	//
	config.get_unsigned("BFMaxLayer",m_param.max_layers,"Maximal backstep count");
	m_param.max_velocity_layers = m_param.max_layers;
	config.get_unsigned("BFMaxVelLayer",m_param.max_velocity_layers,"Maximal backstep count for velocity");
	config.get_unsigned("BFNumSample",m_param.r_sample,"Subsampling number for integration per dimension divided by 2");
	config.get_double("BFDecayRate",m_param.decay_rate,"Weighting decay rate");
	config.get_double("BFDecayTruncate",m_param.decay_truncate,"Weighting truncate threshold");
	config.get_bool("BFUseHachisuka",m_param.use_hachisuka,"Whether to use the method of Hachisuka");
	if( m_param.use_hachisuka ) m_param.use_temporal_adaptivity = false;
	config.get_bool("BFUseTemporalAdaptivity",m_param.use_temporal_adaptivity,"Whether to use temporal adaptive method");
	config.get_bool("BFUseSpatialAdaptivity",m_param.use_spatial_adaptivity,"Whether to use spatial adaptive method");
	config.get_unsigned("BFMaxTemporalAdaptivityLevel",m_param.max_temporal_adaptivity_level,"Maximal temporal adaptivity level");
	config.get_double("BFTemporalAdaptiveRate",m_param.temporal_adaptive_rate,"Temporal adaptivity rate");
	config.get_double("BFSpatialAdaptiveRate",m_param.spatial_adaptive_rate,"Spatial adaptivity rate");
	config.get_double("BFSpatialDensityThreshold",m_param.spatial_density_threshold,"Density cutoff for spatial adaptivity");
	config.get_double("BFInjectDiff",m_param.inject_diff,"Whether to inject velocity differences");
	//
	if( ! m_param.use_temporal_adaptivity ) m_param.use_accumulative_buffer = false;
	config.get_bool("BFUseAccumulativeBuffer",m_param.use_accumulative_buffer,"Whether to use accumulative buffer");
	//
	if( m_param.use_temporal_adaptivity && m_param.use_hachisuka ) {
		console::dump( "Adaptivity is not supported when the method of Hachisuka is specificed.\n");
		exit(0);
	}
	if( ! m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
		console::dump( "Accumulative buffer is not supported when temporal adaptivity is turned off.\n");
		exit(0);
	}
}
//
void macbackwardflip3::initialize ( const shape3 &shape, double dx ) {
	m_shape = shape;
	m_dx = dx;
}
//
void macbackwardflip3::post_initialize () {
	//
	scoped_timer timer{this};
	m_exist_gradient = false;
	m_exist_density = false;
	m_step = 0;
	//
	m_density_reconstructed.initialize(m_shape);
	m_density.initialize(m_shape);
	m_u_reconstructed.initialize(m_shape);
	if( m_param.inject_diff ) m_u_diff.initialize(m_shape);
	//
	// Seed sample particles
	timer.tick(); console::dump( "Seeding sample points..." );
	m_original_seed_vector.clear();
	m_original_seed_mass.clear();
	m_seed_cell.initialize(m_shape);
	m_seed_face.initialize(m_shape);
	//
	m_seed_cell.parallel_all([&](int i, int j, int k, auto &it) {
		it.set(std::vector<unsigned>());
	});
	m_seed_face.parallel_all([&](int dim, int i, int j, int k, auto &it) {
		it.set(std::vector<unsigned>());
	});
	//
	unsigned seed_index (0);
	int r_sample = (int) m_param.r_sample;
	double space = 1.0 / r_sample;
	double mass = pow(space,DIM3);
	//
	m_seed_cell.serial_all([&](int i, int j, int k, auto &it) {
		//
		// Seed tracking points
		if( m_param.use_spatial_adaptivity || r_sample == 1 ) {
			vec3d pos = m_dx*vec3i(i,j,k).cell();
			m_original_seed_vector.push_back(pos);
			m_original_seed_mass.push_back(0.5);
			it.ptr()->push_back(seed_index);
			for( int dim : DIMS3 ) {
				m_seed_face[dim].ptr(i,j,k)->push_back(seed_index);
				m_seed_face[dim].ptr(i+(dim==0),j+(dim==1),k+(dim==2))->push_back(seed_index);
			}
			seed_index ++;
		}
		//
		if( ! m_param.use_spatial_adaptivity || r_sample > 1 ) {
			//
			// Add seed points
			for( int pii=0; pii<r_sample; ++pii ) for( int pjj=0; pjj<r_sample; ++pjj ) for( int kk=0; kk<r_sample; ++kk ) {
				int ii = kk % 2 == 0 ? pii : r_sample-pii-1;
				int jj = pii % 2 == 0 ? pjj : r_sample-pjj-1;
				vec3d unit_pos = 0.5*vec3d(space,space,space)+vec3d(ii*space,jj*space,kk*space);
				vec3d pos = m_dx*(unit_pos+vec3d(i,j,k));
				m_original_seed_vector.push_back(pos);
				m_original_seed_mass.push_back(mass);
				it.ptr()->push_back(seed_index);
				for( int dim : DIMS3 ) {
					if( unit_pos[dim] < 0.5 ) {
						m_seed_face[dim].ptr(i,j,k)->push_back(seed_index);
					} else {
						m_seed_face[dim].ptr(i+(dim==0),j+(dim==1),k+(dim==2))->push_back(seed_index);
					}
				}
				seed_index ++;
			}
		}
	});
	//
	if( m_param.use_hachisuka ) {
		shape3 shape = m_param.r_sample * m_shape;
		m_forward_tracers.initialize(shape);
		m_g_integrated.initialize(shape,vec3d());
		reset_forward_tracers();
	}
	console::dump( "Done. Took %s\n", timer.stock("seed_points").c_str());
	//
	m_buffers.clear();
	if( m_param.use_temporal_adaptivity ) {
		if( m_param.use_accumulative_buffer ) {
			m_back_buffer = layer3 ();
			m_back_buffer.allocate();
			m_back_buffer.u->initialize(m_shape);
			m_back_buffer.u_reconstructed->initialize(m_shape);
			m_back_buffer.g->initialize(m_shape);
			m_back_buffer.d->initialize(m_shape);
			m_back_buffer.d_added->initialize(m_shape);
			m_back_buffer.dt = 0.0;
			m_back_buffer.time = 0.0;
		} else {
			m_coarse_buffers.resize(m_param.max_temporal_adaptivity_level);
			for( unsigned n=0; n<m_param.max_temporal_adaptivity_level; ++n ) m_coarse_buffers[n].clear();
		}
		m_level_stored.resize(m_param.max_temporal_adaptivity_level);
		for( unsigned n=0; n<m_level_stored.size(); ++n ) m_level_stored[n] = pow(2,n+1);
		m_tracer.adaptivity_rate.resize(seed_index);
		for( auto &v : m_tracer.adaptivity_rate ) v.resize(m_param.max_temporal_adaptivity_level+1);
	}
	if( m_param.use_spatial_adaptivity ) m_spatial_adaptivity.initialize(m_shape);
	//
	m_tracer.p.resize(seed_index);
	m_tracer.u.resize(seed_index);
	m_tracer.mass.resize(seed_index);
	m_tracer.s.resize(seed_index);
	//
	m_accumulator.wsum.resize(seed_index);
	m_accumulator.vel.resize(seed_index);
	m_accumulator.g.resize(seed_index);
}
//
void macbackwardflip3::reset_forward_tracers () {
	//
	m_step_back_limit = 0;
	m_g_integrated.clear();
	m_forward_tracers.parallel_all([&](int i, int j, int k, auto &it) {
		it.set(m_dx * vec3i(i,j,k).cell() / m_param.r_sample );
	});
}
//
void macbackwardflip3::integrate_forward_tracers ( const macarray3<double> &velocity0, const macarray3<double> &velocity1, const macarray3<double> &g, double dt ) {
	//
	auto getVector = [&]( const vec3d &p, const macarray3<double> &u ) {
		vec3d new_u;
		for( unsigned dim : DIMS3 ) new_u[dim] = array_interpolator3::interpolate(u[dim],p/m_dx-0.5*vec3d(dim!=0,dim!=1,dim!=2));
		return new_u;
	};
	//
	scoped_timer timer{this};
	timer.tick(); console::dump("Advancing forward tracers...");
	//
	shared_array3<vec3d> m_forward_tracers_save(m_forward_tracers);
	m_forward_tracers.parallel_all([&](int i, int j, int k, auto &it) {
		vec3d p = it();
		const vec3d u0 = getVector(p,velocity0);
		const vec3d u1 = getVector(p+dt*u0,velocity1);
		const vec3d p0 (p);
		p = p + 0.5 * dt * (u0+u1);
		for( unsigned dim : DIMS3 ) {
			if( p[dim] < 0.0 ) p[dim] = 0.0;
			if( p[dim] > m_dx*m_shape[dim] ) p[dim] = m_dx*m_shape[dim];
		}
		it.set(p);
	});
	//
	m_g_integrated.parallel_all([&](int i, int j, int k, auto &it, int tn) {
		it.increment(getVector(0.5*(
			m_forward_tracers_save()(i,j,k)+
			m_forward_tracers(i,j,k)
		),g));
	});
	//
	console::dump( "Done. Took %s\n", timer.stock("FLIP_forward_trace").c_str());
}
//
static vec3d get_velocity ( const vec3d &p, double dx, const macarray3<double> &velocity ) {
	vec3d new_u;
	for( int dim : DIMS3 ) new_u[dim] = array_interpolator3::interpolate(velocity[dim],p/dx-0.5*vec3d(dim!=0,dim!=1,dim!=2));
	return new_u;
}
//
void macbackwardflip3::backtrace(std::vector<vec3d> &p, std::vector<vec3d> &u, const std::vector<double> &mass, std::vector<std::vector<double> > &adaptivity_rate, std::vector<double> *d ) {
	//
	assert(p.size()==u.size());
	if( m_exist_density ) std::fill(d->begin(),d->end(),0.0);
	//
	auto backtrace = [&]( const vec3d &p, vec3d &u, double dt, double dx, const macarray3<double> &velocity0, const macarray3<double> &velocity1 ) {
		vec3d u0 = get_velocity(p,dx,velocity0);
		vec3d u1 = get_velocity(p-dt*u0,dx,velocity1);
		u = 0.5 * (u0+u1);
		return p - dt * u;
	};

	auto sqr = []( const double &x ) { return x*x; };
	m_parallel.for_each(p.size(),[&]( size_t n ) {
		//
		m_accumulator.vel[n] = m_accumulator.g[n] = vec3d();
		m_accumulator.wsum[n] = 0.0;
		//
		if( mass[n]) {
			unsigned buffer_size = m_buffers.size();
			unsigned maximal_backtrace_count = buffer_size;
			if( ! m_exist_density ) {
				maximal_backtrace_count = std::min(maximal_backtrace_count,m_param.max_velocity_layers);
				if( m_param.use_hachisuka ) maximal_backtrace_count = std::min(maximal_backtrace_count,m_step_back_limit);
			}
			//
			unsigned all_count (0), single_count (0);
			std::vector<unsigned> adaptive_count(m_param.max_temporal_adaptivity_level,0);
			//
			const macarray3<double> *prev_u = &m_velocity;
			const layer3 *last_layer (nullptr);
			vec3d u_passive = get_velocity(m_tracer.p[n],m_dx,m_velocity);
			for( unsigned k=0; k<maximal_backtrace_count; ) {
				//
				unsigned adaptivity_level (0);
				unsigned advance_step (1);
				unsigned coarse_k (0);
				double dt (0.0);
				//
				double u_passive_len2 = u_passive.norm2();
				if( m_param.use_temporal_adaptivity ) {
					if( m_param.use_accumulative_buffer ) {
						double tmp_dt0 = m_buffers[k].time;
						double target = sqr(m_param.temporal_adaptive_rate*m_dx);
						for( unsigned level=0; level < m_param.max_temporal_adaptivity_level; ++level ) {
							const unsigned num = std::min(m_level_stored[level],maximal_backtrace_count-k);
							if( k >= num-1 ) {
								double tmp_dt (tmp_dt0);
								if( k+num < m_buffers.size() ) tmp_dt -= m_buffers[k+num].time;
								else tmp_dt -= m_back_buffer.time;
								//
								if( sqr(tmp_dt)*u_passive_len2 < target/sqr(num) ) {
									adaptivity_level = level+1;
									advance_step = num;
									dt = tmp_dt;
								}
							}
						}
					} else {
						for( unsigned level=0; level < m_param.max_temporal_adaptivity_level; ++level ) {
							const unsigned &num = m_level_stored[level];
							if( k >= num-1 && (k-(m_step%num)) % num == 0 ) {
								unsigned tmp_coarse_k = (m_step%num!=0) + (k-(m_step%num)) / num;
								if( tmp_coarse_k >= m_coarse_buffers[level].size() ) {
									printf( "WARNING: Something is wrong with the way coarse_k computed.\n");
									exit(0);
								} else {
									if( sqr(m_coarse_buffers[level][tmp_coarse_k].dt)*u_passive_len2 < sqr(m_param.temporal_adaptive_rate*m_dx/num)) {
										adaptivity_level = level+1;
										coarse_k = tmp_coarse_k;
										advance_step = num;
									}
								}
							}
						}
					}
				} else {
					adaptivity_level = 0;
					advance_step = 1;
				}
				//
				const layer3 *layer (nullptr);
				if( adaptivity_level ) {
					if( m_param.use_accumulative_buffer ) {
						assert( k+advance_step-1 < buffer_size );
						layer = &m_buffers[k+advance_step-1];
					} else {
						layer = &m_coarse_buffers[adaptivity_level-1][coarse_k];
						dt = layer->dt;
					}
				} else {
					layer = &m_buffers[k];
					dt = layer->dt;
				}
				//
				vec3d p0 (p[n]);
				p[n] = backtrace(p0,u_passive,dt,m_dx,*prev_u,*layer->u);
				//
				if( m_param.use_hachisuka ) {
					if( k == m_step_back_limit-1 ) {
						m_accumulator.vel[n] = get_velocity(p[n],m_dx,*layer->u)+array_interpolator3::interpolate(m_g_integrated,m_param.r_sample*p[n]/m_dx-vec3d(0.5,0.5,0.5));
						m_accumulator.wsum[n] = 1.0;
						u[n] = m_accumulator.vel[n] / m_accumulator.wsum[n];
					}
				} else {
					if( m_exist_gradient && k < m_param.max_velocity_layers ) {
						vec3d mid_pos = 0.5*(p0+p[n]);
						if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
							m_accumulator.g[n] += get_velocity(mid_pos,m_dx,*m_buffers[k].g);
							if( k+advance_step < buffer_size ) m_accumulator.g[n] -= get_velocity(mid_pos,m_dx,*m_buffers[k+advance_step].g);
							else m_accumulator.g[n] -= get_velocity(mid_pos,m_dx,*m_back_buffer.g);
						} else {
							m_accumulator.g[n] += get_velocity(mid_pos,m_dx,*layer->g);
						}
						const double w = advance_step * std::pow(m_param.decay_rate,maximal_backtrace_count-k-1-0.5*advance_step);
						if( w > m_param.decay_truncate ) {
							m_accumulator.vel[n] += w*(get_velocity(p[n],m_dx,*layer->u_reconstructed)+m_accumulator.g[n]); m_accumulator.wsum[n] += w;
							u[n] = m_accumulator.vel[n] / m_accumulator.wsum[n];
						}
					}
				}
				//
				if( m_exist_density ) {
					vec3d mid_pos = 0.5*(p0+p[n]);
					if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
						(*d)[n] += array_interpolator3::interpolate(*m_buffers[k].d_added,mid_pos/m_dx-vec3d(0.5,0.5,0.5));
						if( k+advance_step < buffer_size ) (*d)[n] -= array_interpolator3::interpolate(*m_buffers[k+advance_step].d_added,mid_pos/m_dx-vec3d(0.5,0.5,0.5));
						else (*d)[n] -= array_interpolator3::interpolate(*m_back_buffer.d_added,mid_pos/m_dx-vec3d(0.5,0.5,0.5));
					} else {
						(*d)[n] += array_interpolator3::interpolate(*layer->d_added,mid_pos/m_dx-vec3d(0.5,0.5,0.5));
					}
				}
				//
				prev_u = layer->u.get();
				last_layer = layer;
				//
				if( adaptivity_level ) k += advance_step;
				else k ++;
				//
				all_count ++;
				if( m_param.use_temporal_adaptivity && adaptivity_level ) adaptive_count[adaptivity_level-1] ++;
				else single_count ++;
			}
			//
			// Assign m_density if requested
			if( last_layer && m_exist_density ) {
				(*d)[n] += array_interpolator3::interpolate(*last_layer->d,p[n]/m_dx-vec3d(0.5,0.5,0.5));
			}
			//
			if( m_param.use_temporal_adaptivity ) {
				adaptivity_rate[n][0] = single_count / (double) all_count;
				for( unsigned level=0; level < m_param.max_temporal_adaptivity_level; ++level ) {
					adaptivity_rate[n][level+1] = adaptive_count[level] / (double) all_count;
				}
			}
		}
	});
}
//
bool macbackwardflip3::backtrace( const array3<double> &solid, const array3<double> &fluid ) {
	//
	scoped_timer timer(this);
	if( m_buffers.empty()) {
		return false;
	} else {
		timer.tick(); console::dump( ">>> Backward FLIP started (depth=%d)...\n", m_buffers.size());
		//
		m_tracer.p = m_original_seed_vector;
		m_tracer.mass = m_original_seed_mass;
		//
		// Set spatial adaptivity
		if( m_param.use_spatial_adaptivity ) {
			timer.tick(); console::dump( "Setting spatial adaptivity..." );
			//
			m_spatial_adaptivity.parallel_all([&]( int i, int j, int k, auto &it, int tn ) {
				vec3d cell_u;
				for( int dim : DIMS3 ) cell_u[dim] = 0.5*(m_velocity[dim](i,j,k)+m_velocity[dim](i+(dim==0),j+(dim==1),k+(dim==2)));
				it.set( cell_u.norm2() > m_param.spatial_adaptive_rate*m_param.spatial_adaptive_rate || m_density(i,j,k) > m_param.spatial_density_threshold);
			});
			//
			m_spatial_adaptivity.const_parallel_all([&](int i, int j, int k, auto &it, int tn ) {
				if( m_spatial_adaptivity(i,j,k) ) {
					for( const unsigned &n : m_seed_cell(i,j,k) ) if(m_tracer.mass[n]==0.5) m_tracer.mass[n] = 0.0;
				} else {
					for( const unsigned &n : m_seed_cell(i,j,k) ) if(m_tracer.mass[n]<0.5) m_tracer.mass[n] = 0.0;
				}
			});
			console::dump( "Done. Took %s\n", timer.stock("set_spatial_adaptivity").c_str());
		}
		//
		// Skip tracers outside the domain
		timer.tick(); console::dump( "Setting mass zero for tracers outside the domain..." );
		bool has_solid = array_utility3::has_different_values(solid);
		bool has_fluid = array_utility3::has_different_values(fluid);
		m_parallel.for_each(m_tracer.p.size(),[&]( size_t n ) {
			if( has_solid && array_interpolator3::interpolate(solid,m_tracer.p[n]/m_dx) < 0.0 ) m_tracer.mass[n] = 0.0;
			if( has_fluid && array_interpolator3::interpolate(fluid,m_tracer.p[n]/m_dx-vec3d(0.5,0.5,0.5)) > 0.0 ) m_tracer.mass[n] = 0.0;
		});
		console::dump( "Done. Took %s\n", timer.stock("set_zero_mass").c_str());
		//
		auto compute_face_velocity = [&]( macarray3<double> &u_array ) {
			u_array.parallel_all([&]( int dim, int i, int j, int k, auto &it, int tn ) {
				double usum (0.0);
				double wsum (0.0);
				for( const unsigned &n : m_seed_face[dim](i,j,k) ) {
					double m = m_tracer.mass[n];
					usum += m*m_tracer.u[n][dim];
					wsum += m;
				}
				if( wsum ) {
					it.set( usum / wsum );
				} else {
					it.set(0.0);
				}
			});
		};
		//
		// Compute difference
		if( m_param.inject_diff && m_exist_gradient ) {
			//
			timer.tick(); console::dump( "Computing velocity differences..." );
			m_parallel.for_each(m_tracer.u.size(),[&]( size_t n) {
				m_tracer.u[n] = get_velocity(m_tracer.p[n],m_dx,m_velocity);
			});
			compute_face_velocity(m_u_diff);
			m_u_diff -= m_velocity;
			console::dump( "Done. Took %s\n", timer.stock("compute_diff").c_str());
		}
		//
		timer.tick(); console::dump( "Backtracing points..." );
		backtrace(m_tracer.p, m_tracer.u, m_tracer.mass, m_tracer.adaptivity_rate, m_exist_density ? &m_tracer.s : nullptr);
		console::dump( "Done. Took %s\n", timer.stock("macbackward_FLIP_internal_backtrace").c_str());
		//
		global_timer::pause();
		if( m_param.use_temporal_adaptivity ) {
			for( unsigned level=0; level<m_param.max_temporal_adaptivity_level+1; ++level ) {
				double temporal_adaptivity_sum (0.0), temporal_adaptivity_weight (0.0);
				for( unsigned n=0; n<m_tracer.adaptivity_rate.size(); ++n ) {
					if( m_tracer.mass[n] ) {
						temporal_adaptivity_sum += m_tracer.adaptivity_rate[n][level];
						temporal_adaptivity_weight += 1.0;
					}
				}
				double temporal_average = temporal_adaptivity_weight ? temporal_adaptivity_sum / temporal_adaptivity_weight : 0.0;
				console::dump( "Report: Temporal adaptivity (level %d) = %.2f%%.\n", level, 100.0*temporal_average );
				console::write((std::string("_temporal_adaptivity_level")+std::to_string(level+1)).c_str(),temporal_average);
			}
			//
			if( m_param.use_spatial_adaptivity ) {
				unsigned spatial_sum (0);
				unsigned spatial_count (0);
				m_spatial_adaptivity.const_serial_all([&](int i, int j, int k, auto &it) {
					++ spatial_sum;
					if( it()) ++ spatial_count;
				});
				if( m_param.use_spatial_adaptivity && spatial_sum ) {
					double spatial_average = 1.0 - spatial_count / (double) spatial_sum;
					console::dump( "Report: Spatial adaptivity = %.2f%%.\n", 100.0*spatial_average );
					console::write("spatial_adaptivity", spatial_average);
				}
			}
		}
		global_timer::resume();
		//
		// Assign m_density if requested
		if( m_exist_density ) {
			timer.tick(); console::dump( "Reconstructing m_density..." );
			m_density_reconstructed.parallel_all([&](int i, int j, int k, auto &it, int tn) {
				double dsum (0.0);
				double wsum (0.0);
				for( const unsigned &n : m_seed_cell(i,j,k) ) {
					double w = m_tracer.mass[n];
					if( w ) {
						dsum += w*m_tracer.s[n];
						wsum += w;
					}
				}
				// Assign the new m_density
				if( wsum ) {
					it.set(dsum/wsum);
				} else {
					it.set(0.0);
				}
			});
			console::dump( "Done. Took %s\n", timer.stock("density").c_str());
		}
		//
		// Assign the reconstructed velocity
		if( m_exist_gradient ) {
			timer.tick(); 
			if( m_param.inject_diff ) console::dump( "Reconstructing velocity (%.2f)...", m_param.inject_diff );
			else console::dump( "Reconstructing velocity..." );
			compute_face_velocity(m_u_reconstructed);
			if( m_param.inject_diff ) {
				m_u_diff *= m_param.inject_diff;
				m_u_reconstructed -= m_u_diff;
			}
			console::dump( "Done. Took %s\n", timer.stock("reconstruct").c_str());
		}
		//
		console::dump( "<<< Done. Took %s\n", timer.stock("complete_backtrace").c_str());
		return true;
	}
}
//
bool macbackwardflip3::fetch( macarray3<double> &u_reconstructed ) const {
	//
	if( m_buffers.size() && m_exist_gradient ) {
		u_reconstructed.copy(m_u_reconstructed);
		return true;
	} else {
		return false;
	}
}
//
bool macbackwardflip3::fetch( array3<double> &density_reconstructed ) const {
	//
	if( m_buffers.size() && m_exist_density ) {
		density_reconstructed.copy(m_density_reconstructed);
		return true;
	} else {
		return false;
	}
}
//
void macbackwardflip3::register_buffer(const macarray3<double> &u1,
									const macarray3<double> &u0,
									const macarray3<double> *m_u_reconstructed,
									const macarray3<double> *g,
									const array3<double> *d1,
									const array3<double> *d0,
									const array3<double> *d_added,
									double dt ) {
	layer3 layer;
	layer.allocate();
	layer.dt = dt;
	layer.time = dt;
	if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer && m_buffers.size() ) layer.time += m_buffers.front().time;
	//
	if( d0 ) {
		layer.d->copy(*d0);
		if( d_added ) {
			layer.d_added->copy(*d_added);
			if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
				if( m_buffers.size()) *(layer.d_added) += *(m_buffers.front().d_added);
			}
		}
		m_exist_density = true;
	}
	if( d1 ) {
		m_density.copy(*d1);
	}
	//
	m_exist_gradient = m_u_reconstructed != nullptr && g != nullptr;
	m_velocity.copy(u1);
	layer.u->copy(u0);
	if( g ) layer.g->copy(*g);
	if( m_u_reconstructed ) layer.u_reconstructed->copy(*m_u_reconstructed);
	if( ! m_param.use_hachisuka && g ) {
		if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
			if( m_buffers.size()) *(layer.g) += *(m_buffers.front().g);
		}
	}
	//
	if( m_param.use_hachisuka ) {
		if( m_step_back_limit >= m_param.max_velocity_layers ) {
			reset_forward_tracers();
		}
		m_step_back_limit ++;
		m_buffers.push_front(layer);
		if( g ) integrate_forward_tracers(u0,u1,*g,dt);
		if( m_buffers.size() > m_param.max_layers ) {
			if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
				m_back_buffer = m_buffers.back();
			}
			m_buffers.pop_back();
		}
		//
	} else {
		m_buffers.push_front(layer);
		if( m_buffers.size() > m_param.max_layers ) {
			if( m_param.use_temporal_adaptivity && m_param.use_accumulative_buffer ) {
				m_back_buffer = m_buffers.back();
			}
			m_buffers.pop_back();
		}
	}
	//
	if( m_param.use_temporal_adaptivity && ! m_param.use_accumulative_buffer ) {
		for( unsigned level=0; level < m_param.max_temporal_adaptivity_level; ++level ) {
			const unsigned &num = m_level_stored[level];
			if( m_step % num == 0 ) {
				m_coarse_buffers[level].push_front(layer);
				if( m_coarse_buffers[level].size() > 2+m_buffers.size()/num ) {
					m_coarse_buffers[level].pop_back();
				}
			} else {
				layer3 &prev_layer = m_coarse_buffers[level].front();
				*(prev_layer.g) += *(layer.g);
				*(prev_layer.d_added) += *(layer.d_added);
				prev_layer.dt += layer.dt;
			}
		}
	}
	//
	m_step ++;
}
//
void macbackwardflip3::draw( graphics_engine &g ) const {
	if( m_buffers.size() && m_exist_gradient ) {
		//
		// Draw velocity field back in time
		g.color4(1.0,0.3,0.3,0.5);
		const layer3 &layer = m_buffers[std::min(m_buffers.size(),(size_t)m_param.max_velocity_layers)-1];
		g.begin(graphics_engine::MODE::LINES);
		m_shape.for_each([&]( int i, int j, int k ) {
			vec3d u;
			for( unsigned dim : DIMS3 ) {
				u[dim] = 0.5 * ((*layer.u_reconstructed)[dim](i,j,k)+(*layer.u_reconstructed)[dim](i+(dim==0),j+(dim==1),k+(dim==2)));
			}
			vec3d p0 = m_dx*vec3d(i+0.5,j+0.5,k+0.5);
			vec3d p1 = p0+m_dx*u;
			g.vertex3v(p0.v);
			g.vertex3v(p1.v);
		});
		g.end();
	}
}
//
extern "C" module * create_instance() {
	return new macbackwardflip3;
}
//
