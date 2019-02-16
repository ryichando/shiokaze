/*
**	macnbflip2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 29, 2017.
**	APIC extension by Takahiro Sato.
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
#include "macnbflip2.h"
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/shared_bitarray2.h>
#include <shiokaze/array/array_utility2.h>
#include <shiokaze/array/array_interpolator2.h>
#include <shiokaze/array/macarray_interpolator2.h>
#include <shiokaze/array/array_derivative2.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/array/shared_bitarray2.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
//
static const double default_mass = 1.0 / 4.0;
//
double macnbflip2::grid_kernel( const vec2d &r, double dx ) {
	//
	double x = ( r[0] > 0.0 ? r[0] : -r[0] ) / dx;
	double y = ( r[1] > 0.0 ? r[1] : -r[1] ) / dx;
	return std::max(0.0,1.0-x) * std::max(0.0,1.0-y);
}
//
vec2d macnbflip2::grid_gradient_kernel( const vec2d &r, double dx ) {
	//
	double x = ( r[0] > 0.0 ? r[0] : -r[0] ) / dx;
	double y = ( r[1] > 0.0 ? r[1] : -r[1] ) / dx;
	if( x <= 1.0 && y <= 1.0 ) {
		double x_sgn = r[0] <= 0.0 ? 1.0 : -1.0;
		double y_sgn = r[1] <= 0.0 ? 1.0 : -1.0;
		return vec2d(x_sgn*(y-1.0),y_sgn*(x-1.0)) / dx;
	} else {
		return vec2d();
	}
}
//
macnbflip2::macnbflip2() {
	//
	m_macadvection.set_name("Levelset Advection for FLIP 2D","LevelsetAdvectionFLIP");
}
//
void macnbflip2::configure( configuration &config ) {
	//
	config.get_bool("APIC",m_param.use_apic,"Whether to use APIC");
	config.get_unsigned("LevelsetHalfwidth",m_param.levelset_half_bandwidth_count,"Level set half bandwidth");
	config.get_unsigned("Narrowband",m_param.narrowband,"Narrowband bandwidth");
	config.get_unsigned("CorrectDepth",m_param.correct_depth,"Position correction depth");
	config.get_double("FitParticleDist",m_param.fit_particle_dist,"FLIP particle fitting threshold");
	config.get_integer("RK_Order",m_param.RK_order,"Order of accuracy for Runge-kutta integration");
	config.get_double("Erosion",m_param.erosion,"Rate of erosion for internal levelset");
	config.get_unsigned("MinParticlesPerCell",m_param.min_particles_per_cell,"Minimal target number of particles per cell");
	config.get_unsigned("MaxParticlesPerCell",m_param.max_particles_per_cell,"Maximal target number of particles per cell");
	config.get_unsigned("MiminalLiveCount",m_param.minimal_live_count,"Minimal step of particles to stay alive");
	config.get_double("CorrectStiff",m_param.stiff,"Position correction strength");
	config.get_bool("VelocityCorrection",m_param.velocity_correction,"Should perform velocity correction");
	config.get_double("BulletMaximalTime",m_param.bullet_maximal_time,"Maximal time for bullet particles to survive");
	config.get_double("SizingEps",m_param.sizing_eps,"Minimal sizing function value to be considered");
	config.get_bool("LooseInterior",m_param.loose_interior,"Whether to seed sparsely particles at deep cells");
	config.get_bool("DrawFLIPParticles",m_param.draw_particles,"Whether to draw FLIP particles.");
	config.get_bool("DrawFLIPLevelset",m_param.draw_levelset,"Whether to draw FLIP levelset.");
	//
}
//
void macnbflip2::initialize( const shape2 &shape, double dx ) {
	//
	m_shape = shape;
	m_dx = dx;
}
//
void macnbflip2::post_initialize() {
	//
	m_fluid_filled = false;
	m_solid_exit = false;
	//
	initialize_fluid();
	initialize_solid();
	//
	m_sizing_array.initialize(m_shape,1.0);
	m_narrowband_mask.initialize(m_shape);
	m_particles.resize(0);
	sort_particles();
}
//
void macnbflip2::assign_solid( const array2<double> &solid ) {
	m_solid.copy(solid);
	m_solid_exit = array_utility2::levelset_exist(solid);
}
//
size_t macnbflip2::seed( const array2<double> &fluid, const macarray2<double> &velocity ) {
	//
	seed_set_fluid(fluid);
	m_fluid_filled = true;
	fluid.interruptible_const_serial_actives([&]( int i, int j, const auto &it ) {
		if( it() > 0.0 ) {
			m_fluid_filled = false;
			return true;
		}
		return false;
	});
	compute_narrowband();
	sizing_func(m_sizing_array,m_narrowband_mask,velocity,0.0);
	size_t count = reseed(velocity,m_param.loose_interior);
	return count;
}
//
size_t macnbflip2::mark_bullet( double time, const macarray2<double> &velocity ) {
	//
	if( m_particles.size()) {
		//
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			char new_status (0);
			if(	interpolate_fluid(particle.p) > 0.0 ) {
				new_status = 1;
				for( int dim : DIMS2 ) particle.c[dim] = vec2d();
			}
			if( new_status != particle.bullet ) {
				particle.bullet = new_status;
				particle.bullet_sizing_value = std::min(1.0,particle.sizing_value);
				particle.bullet_time = new_status ? time : 0.0;
				if( ! particle.bullet ) {
					particle.mass = default_mass;
					particle.r = 0.25 * m_dx;
					particle.velocity = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,particle.p);
					update_velocity_derivative(particle,velocity);
				}
			}
		});
		//
		size_t num_bullet(0);
		for( size_t n=0; n<m_particles.size(); ++n ) if( m_particles[n].bullet ) ++ num_bullet;
		return num_bullet;
	} else {
		return 0;
	}
}
//
size_t macnbflip2::remove_bullet( double time ) {
	//
	if( ! m_param.bullet_maximal_time || m_particles.empty()) return 0;
	std::vector<char> remove_flag(m_particles.size(),0);
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		Particle &particle = m_particles[n];
		if( particle.bullet ) {
			if( time-particle.bullet_time > m_param.bullet_maximal_time ) {
				remove_flag[n] = 1;
			} else {
				double scale = std::max(0.01,1.0 - std::max(0.0,time-particle.bullet_time) / m_param.bullet_maximal_time);
				particle.r = 0.25 * m_dx * scale;
				particle.mass = scale * default_mass;
			}
		}
	});
	//
	// Reconstruct a new particle array
	std::vector<Particle> old_particles (m_particles);
	m_particles.clear();
	size_t removed_total (0);
	for( size_t i=0; i<old_particles.size(); ++i) {
		if( ! remove_flag[i] ) m_particles.push_back(old_particles[i]);
		else ++ removed_total;
	}
	//
	// Update hash table
	sort_particles();
	return removed_total;
}
//
typedef struct {
	double mass;
	double momentum;
} mass_momentum2;
//
void macnbflip2::splat( macarray2<double> &momentum, macarray2<double> &mass ) const {
	//
	if( m_particles.size()) {
		//
		shared_macarray2<mass_momentum2> mass_and_momentum(momentum.shape());
		shared_bitarray2 cell_mask(m_shape);
		for( size_t n=0; n<m_particles.size(); ++n ) {
			cell_mask().set(m_shape.clamp(m_particles[n].p/m_dx));
		}
		cell_mask->const_serial_actives([&]( int i, int j, auto &it ) {
			const vec2i pi(i,j);
			for( int dim : DIMS2 ) {
				mass_and_momentum()[dim].set(pi,{0.,0.});
				mass_and_momentum()[dim].set(pi+vec2i(dim==0,dim==1),{0.,0.});
			}
		});
		mass_and_momentum().dilate();
		mass_and_momentum->parallel_actives([&]( int dim, int i, int j, auto &it, int tn ) {
			//
			double mom (0.0), m (0.0);
			vec2d pos = m_dx*vec2i(i,j).face(dim);
			std::vector<size_t> neighbors = m_pointgridhash->get_face_neighbors(vec2i(i,j),dim);
			for( size_t k : neighbors ) {
				const Particle &p = m_particles[k];
				double w = grid_kernel(p.p-pos,m_dx);
				if( w ) {
					mom += w*p.mass*p.velocity[dim];
					m += w*p.mass;
				}
			}
			if( m ) it.set({m,mom});
			else it.set_off();
		});
		//
		mass.clear(0.0);
		mass.activate_as(mass_and_momentum());
		mass.parallel_actives([&]( int dim, int i, int j, auto &it, int tn ) {
			it.set(mass_and_momentum()[dim](i,j).mass);
		});
		//
		momentum.clear(0.0);
		momentum.activate_as(mass_and_momentum());
		momentum.parallel_actives([&]( int dim, int i, int j, auto &it, int tn ) {
			it.set(mass_and_momentum()[dim](i,j).momentum);
		});
		//
		if( m_param.use_apic ) additionally_apply_velocity_derivative(momentum);
		//
	} else {
		momentum.clear();
		mass.clear();
	}
}
//
void macnbflip2::advect( const macarray2<double> &velocity, double time, double dt ) {
	//
	if( m_particles.size()) {
		//
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			bool bullet = particle.bullet;
			const vec2d &u = particle.velocity;
			const vec2d &p = particle.p;
			if( bullet ) {
				particle.p += dt*u;
			} else {
				vec2d u1 = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,p);
				if( u1.norm2()) {
					if( m_param.RK_order==4 ) {
						vec2d u2 = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,p+0.5*dt*u1);
						vec2d u3 = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,p+0.5*dt*u2);
						vec2d u4 = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,p+dt*u3);
						particle.p += dt*(u1+2.0*u2+2.0*u3+u4)/6.0;
					} else if( m_param.RK_order==2 ) {
						vec2d u2 = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,p+dt*u1);
						particle.p += dt*0.5*(u1+u2);
					} else if( m_param.RK_order==1 ) {
						particle.p += dt*(u1);
					} else {
						printf( "Unsupported RK order (%d)\n", m_param.RK_order );
						exit(0);
					}
				} else {
					particle.p += dt*u;
				}
			}
		});
		sort_particles();
		//
		if( m_param.stiff ) {
			if( m_fluid_filled ) {
				correct(velocity,nullptr);
			} else {
				correct(velocity,&m_fluid);
			}
		}
	}
	//
	// Perform collision
	collision();
	//
	// Advect levelset
	if( ! m_fluid_filled ) {
		advect_levelset(velocity,dt,m_param.erosion);
		compute_narrowband();
	}
	//
	// Recompute sizing function
	sizing_func(m_sizing_array,m_narrowband_mask,velocity,dt);
	//
	// Reseed particles
	reseed(velocity,false);
	//
	if( m_particles.size()) {
		//
		// Mark bullet
		mark_bullet(time,velocity);
		//
		// Remove long-term ballistic particles
		remove_bullet(time);
	}
}
//
void macnbflip2::compute_narrowband() {
	//
	if( m_param.narrowband && ! m_fluid_filled ) {
		//
		m_narrowband_mask.clear();
		m_fluid.const_serial_actives([&](int i, int j, auto &it) {
			if( it() > 0 && this->interpolate_solid(m_dx*vec2i(i,j).cell()) > 0 ) {
				m_narrowband_mask.set(i,j);
			}
		});
		for( int n=0; n<m_param.narrowband; ++n ) {
			m_narrowband_mask.dilate([&](int i, int j, auto& it, int tn ) {
				if(m_fluid(i,j) < 0 && this->interpolate_solid(m_dx*vec2i(i,j).cell()) > 0.125*m_dx ) it.set();
			});
		}
		m_fluid.const_serial_actives([&](int i, int j, auto &it) {
			if( it() > m_dx ) m_narrowband_mask.set_off(i,j);
		});
	} else {
		m_narrowband_mask.clear();
	}
}
//
void macnbflip2::collision() {
	//
	if( m_particles.size()) {
		//
		m_parallel.for_each( m_particles.size(), [&]( size_t pindex ) {
			Particle &particle = m_particles[pindex];
			vec2d &p = particle.p;
			vec2d &u = particle.velocity;
			const double &r = particle.r;
			double phi = interpolate_solid(p)-r;
			if( phi < 0.0 ) {
				vec2d gradient = interpolate_solid_gradient(p);
				p = p - phi*gradient;
				double dot = gradient * u;
				if( dot < 0.0 ) u = u - gradient*dot;
			}
			for( unsigned dim : DIMS2 ) {
				if( p[dim] < r ) {
					p[dim] = r;
					if( u[dim] < 0.0 ) u[dim] = 0.0;
				}
				if( p[dim] > m_dx*m_shape[dim]-r ) {
					p[dim] = m_dx*m_shape[dim]-r;
					if( u[dim] > 0.0 ) u[dim] = 0.0;
				}
			}
		});
	}
	//
	sort_particles();
	collision_levelset([&]( const vec2d &p ) {
		return interpolate_solid(p);
	});
}
//
size_t macnbflip2::correct( const macarray2<double> &velocity, const array2<double> *mask ) {
	//
	if( m_particles.empty()) return 0;
	//
	// Compute correction displacement
	std::vector<vec2d> displacements(m_particles.size());
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		bool skip (false);
		const Particle &pi = m_particles[n];
		if( mask ) {
			if( ! (*mask).active(m_shape.find_cell(pi.p/m_dx))) skip = true;
		}
		vec2d displacement;
		if( ! skip ) {
			std::vector<size_t> neighbors = m_pointgridhash->get_cell_neighbors(m_shape.find_cell(pi.p/m_dx),pointgridhash2_interface::USE_NODAL);
			for( const size_t &j : neighbors ) {
				if( n != j ) {
					const Particle &pj = m_particles[j];
					if( mask ) {
						if( ! (*mask).active(m_shape.find_cell(pj.p/m_dx))) skip = true;
					}
					if( ! skip ) {
						double dist2 = (pi.p-pj.p).norm2();
						double target = pi.r+pj.r;
						if( dist2 < target*target ) {
							double diff = target-sqrt(dist2);
							const double &mi = pi.mass;
							const double &mj = pj.mass;
							displacement += m_param.stiff * diff * (pi.p-pj.p).normal() * mj / (mi+mj);
						}
					}
				}
			}
		}
		displacements[n] = displacement;
	});
	//
	// Kill the normal components to prevent volume gain
	if( ! m_fluid_filled ) {
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			if( ! displacements[n].empty()) {
				const Particle &p = m_particles[n];
				vec2d new_pos = p.p+displacements[n];
				vec2d normal = interpolate_fluid_gradient(new_pos);
				double dot = displacements[n] * normal;
				if( dot > 0.0 ) displacements[n] -= dot * normal;
			}
		});
	}
	//
	// Apply displacement
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		if( ! displacements[n].empty()) {
			bool skip (false);
			const vec2d &pos = m_particles[n].p;
			for( int dim : DIMS2 ) {
				if( pos[dim] < 0.0 || pos[dim] > m_dx*m_shape[dim] ) skip = true;
			}
			if( ! skip ) {
				if( m_param.velocity_correction ) {
					vec2d jacobian[DIM2], incr;
					m_macutility->get_velocity_jacobian(pos,velocity,jacobian);
					for ( int dim : DIMS2 ) {
						incr[dim] = jacobian[dim] * displacements[n];
					}
					m_particles[n].velocity += incr;
				}
				m_particles[n].p += displacements[n];
			}
		}
	});
	//
	// Update hash table
	sort_particles();
	//
	size_t correct_count (0);
	for( size_t n=0; n<displacements.size(); ++n ) {
		if( ! displacements[n].empty()) ++ correct_count;
	}
	return correct_count;
}
//
void macnbflip2::fit_particle( std::function<double(const vec2d &p)> fluid_func, Particle &particle, const vec2d &gradient ) const {
	//
	if( std::abs(fluid_func(particle.p)) < m_param.fit_particle_dist * particle.r ) {
		for( unsigned n=0; n<3; ++n ) {
			double signed_dist = fluid_func(particle.p);
			double gap = signed_dist < 0.0 ? signed_dist+particle.r : signed_dist-particle.r;
			particle.p -= 0.5 * gap * gradient;
		}
	}
}
//
size_t macnbflip2::reseed( const macarray2<double> &velocity, bool loose_interior ) {
	//
	std::vector<std::vector<Particle> > new_particles_t(m_parallel.get_maximal_threads());
	std::vector<char> remove_particles(m_particles.size(),0);
	//
	// Bucket cell method to remove too dense particles
	shared_array2<char> cell_bucket(m_shape);
	if( m_particles.size()) {
		for( size_t n=0; n<m_particles.size(); ++n ) {
			const Particle &p = m_particles[n];
			vec2i pi = m_shape.clamp(p.p/m_dx);
			int i (pi[0]), j (pi[1]);
			m_shape.clamp(i,j);
			if( ! p.bullet ) {
				if( ! m_sizing_array(i,j) || cell_bucket()(i,j) >= m_param.max_particles_per_cell || 
					(! m_fluid_filled && ! m_narrowband_mask(i,j)) || p.sizing_value < 0.0 ) {
					if( p.live_count > m_param.minimal_live_count ) {
						remove_particles[n] = 1;
					}
				}
			}
			if( ! remove_particles[n] && interpolate_solid(p.p) < -p.r ) {
				remove_particles[n] = 1;
			}
			if( ! remove_particles[n] ) {
				cell_bucket().increment(i,j,1);
			}
		}
		//
		// Increment live count
		for( size_t n=0; n<m_particles.size(); ++n ) {
			m_particles[n].live_count ++;
		}
	}
	//
	// Particle reseeding...
	m_parallel.for_each(m_shape,[&]( int i, int j, int tn ) {
		//
		size_t num_added (0);
		auto attempt_reseed = [&]( const vec2d &p ) {
			if( cell_bucket()(i,j)+num_added < m_param.min_particles_per_cell ) {
				//
				double r = 0.25 * m_dx;
				if( interpolate_fluid(p) < -r ) {
					//
					// Put a FLIP particle here...
					Particle new_particle;
					new_particle.p = p;
					bool sparse (true);
					const std::vector<size_t> &indices = m_pointgridhash->get_points_in_cell(vec2i(i,j));
					for( auto it=indices.begin(); it!=indices.end(); ++it ) {
						if( (m_particles[*it].p-p).len() <= 2.0*r ) {
							sparse = false;
							break;
						}
					}
					if( sparse && interpolate_solid(new_particle.p) > r ) {
						double sizing_value = m_sizing_array(m_sizing_array.shape().clamp(p/m_dx));
						new_particle.mass = default_mass;
						new_particle.velocity = macarray_interpolator2::interpolate(velocity,vec2d(),m_dx,p);
						new_particle.r = r;
						new_particle.bullet = 0;
						new_particle.bullet_time = 0.0;
						new_particle.bullet_sizing_value = 0.0;
						new_particle.live_count = 0;
						new_particle.sizing_value = sizing_value;
						new_particle.gen_p = new_particle.p;
						update_velocity_derivative(new_particle,velocity);
						fit_particle([&](const vec2d &p) {
							return interpolate_fluid(p);
						},new_particle,interpolate_fluid_gradient(new_particle.p));
						new_particles_t[tn].push_back(new_particle);
						num_added ++;
					}
				}
			}
		};
		//
		if( m_fluid_filled || (m_narrowband_mask(i,j) && m_sizing_array(i,j))) {
			if( loose_interior && m_fluid(i,j) < -1.25*m_dx ) {
				attempt_reseed(m_dx*vec2i(i,j).cell());
			} else {
				for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) {
					vec2d p = m_dx*vec2d(i,j)+0.25*m_dx*vec2d(1,1)+0.5*m_dx*vec2d(ii,jj);
					attempt_reseed(p);
				}
			}
		}
		//
	});
	//
	// Reconstruct a new particle array
	std::vector<Particle> old_particles(m_particles);
	m_particles.clear();
	size_t reseeded (0);
	for( size_t t=0; t<new_particles_t.size(); ++t ) {
		m_particles.insert(m_particles.end(),new_particles_t[t].begin(),new_particles_t[t].end());
		reseeded += new_particles_t[t].size();
	}
	for( size_t i=0; i<remove_particles.size(); i++) {
		if( ! remove_particles[i] ) m_particles.push_back(old_particles[i]);
	}
	//
	// Update hash table
	sort_particles();
	return reseeded;
}
//
void macnbflip2::update( const macarray2<double> &prev_velocity, const macarray2<double> &new_velocity,
						 double dt, vec2d gravity, double PICFLIP ) {
	//
	if(m_particles.size()) {
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			if( particle.bullet ) {
				//
				// If the particle is a ballistic particle, just add gravity force
				particle.velocity += dt * gravity;
				//
			} else {
				if( m_param.use_apic ) {
					//
					// Fetch grid velocity
					particle.velocity = macarray_interpolator2::interpolate(new_velocity,vec2d(),m_dx,particle.p);
					//
					// Update particle velocity derivative (APIC)
					update_velocity_derivative(particle,new_velocity);
					//
				} else {
					//
					// Fetch grid velocity
					vec2d new_grid_velocity = macarray_interpolator2::interpolate(new_velocity,vec2d(),m_dx,particle.p);
					vec2d old_grid_velocity = macarray_interpolator2::interpolate(prev_velocity,vec2d(),m_dx,particle.p);
					//
					// Compute pure FLIP velocity
					vec2d FLIP_velocity = particle.velocity + (new_grid_velocity-old_grid_velocity);
					//
					// Compute PICFLIP velocity
					vec2d PICFLIP_velocity = PICFLIP * FLIP_velocity + (1.0-PICFLIP) * new_grid_velocity;
					//
					// Compute the final velocity of FLIP
					particle.velocity = PICFLIP_velocity;
				}
			}
		});
	}
}
//
void macnbflip2::update( std::function<void(const vec2d &p, vec2d &velocity, double &mass, bool bullet )> func ) {
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		Particle &particle = m_particles[n];
		func(particle.p,particle.velocity,particle.mass,particle.bullet);
	});
}
//
void macnbflip2::get_levelset( array2<double> &fluid ) const {
	fluid.copy(m_fluid);
}
//
std::vector<macflip2_interface::particle2> macnbflip2::get_particles() const {
	std::vector<macflip2_interface::particle2> result;
	for( size_t n=0; n<m_particles.size(); ++n ) {
		macflip2_interface::particle2 particle;
		particle.p = m_particles[n].p;
		particle.r = m_particles[n].r;
		particle.bullet = m_particles[n].bullet;
		particle.bullet_time = m_particles[n].bullet_time;
		result.push_back(particle);
	}
	return result;
}
//
void macnbflip2::sort_particles() {
	//
	if( m_particles.size()) {
		std::vector<vec2d> points(m_particles.size());
		m_parallel.for_each(m_particles.size(),[&]( size_t n) {
			points[n] = m_particles[n].p;
		});
		m_pointgridhash->sort_points(points);
	} else {
		m_pointgridhash->clear();
	}
}
//
void macnbflip2::update_velocity_derivative( Particle& particle, const macarray2<double> &velocity ) {
	//
	if( m_param.use_apic ) {
		// Written by Takahiro Sato
		for( int dim : DIMS2 ) {
			vec2d &c = particle.c[dim];
			c = vec2d();
			//
			// particle position
			const vec2d p_pos = particle.p;
			//
			// cell index
			int i = std::floor((p_pos[0]-0.5*m_dx*(dim==1))/m_dx);
			int j = std::floor((p_pos[1]-0.5*m_dx*(dim==0))/m_dx);
			//
			// cell position
			vec2d cell_pos[4]; // 0: lower left, 1: lower right, 2: upper left, 3: upper right
			cell_pos[0] = m_dx*vec2d(i+0.5*(dim!=0),j+0.5*(dim!=1));
			cell_pos[1] = m_dx*vec2d(i+1+0.5*(dim!=0),j+0.5*(dim!=1));
			cell_pos[2] = m_dx*vec2d(i+0.5*(dim!=0),j+1+0.5*(dim!=1));
			cell_pos[3] = m_dx*vec2d(i+1+0.5*(dim!=0),j+1+0.5*(dim!=1));
			//
			vec2d dw[4];
			dw[0] = grid_gradient_kernel(cell_pos[0]-p_pos, m_dx);
			dw[1] = grid_gradient_kernel(cell_pos[1]-p_pos, m_dx);
			dw[2] = grid_gradient_kernel(cell_pos[2]-p_pos, m_dx);
			dw[3] = grid_gradient_kernel(cell_pos[3]-p_pos, m_dx);
			//
			auto v_shape = velocity[dim].shape();
			c += dw[0] * velocity[dim](v_shape.clamp(i,j));
			c += dw[1] * velocity[dim](v_shape.clamp(i+1,j));
			c += dw[2] * velocity[dim](v_shape.clamp(i,j+1));
			c += dw[3] * velocity[dim](v_shape.clamp(i+1,j+1));
		}
	}
}
//
void macnbflip2::additionally_apply_velocity_derivative( macarray2<double> &momentum ) const {
	//
	// Written by Takahiro Sato
	momentum.parallel_actives([&]( int dim, int i, int j, auto &it ) {
		vec2d pos = m_dx*vec2d(i+0.5*(dim!=0),j+0.5*(dim!=1));
		std::vector<size_t> neighbors = m_pointgridhash->get_face_neighbors(vec2i(i,j),dim);
		double mom (0.0);
		for( unsigned k : neighbors ) {
			const Particle &p = m_particles[k];
			const vec2d &c = (p.c)[dim];
			const vec2d r = pos-p.p;
			double w = grid_kernel(r,m_dx);
			if( w ) {
				mom += w*p.mass*(c*r);
			}
		}
		it.increment(mom);
	});
}
//
void macnbflip2::initialize_fluid() {
	//
	m_fluid.initialize(m_shape);
	m_fluid.set_as_levelset(m_dx*(double)m_param.levelset_half_bandwidth_count);
}
//
void macnbflip2::initialize_solid() {
	//
	m_solid.initialize(m_shape.nodal());
	m_solid.set_as_levelset(m_dx*(double)m_param.levelset_half_bandwidth_count);
}
//
void macnbflip2::seed_set_fluid( const array2<double> &fluid ) {
	//
	m_fluid.activate_as(fluid);
	m_fluid.parallel_actives([&]( int i, int j, auto &it, int tn ) {
		it.set(std::min(fluid(i,j),it()));
	});
	m_fluid.flood_fill();
}
//
double macnbflip2::interpolate_fluid( const vec2d &p ) const {
	return m_fluid_filled ? -1.0 : array_interpolator2::interpolate(m_fluid,p/m_dx-vec2d(0.5,0.5));
}
//
double macnbflip2::interpolate_solid( const vec2d &p ) const {
	return array_interpolator2::interpolate(m_solid,p/m_dx);
}
//
vec2d macnbflip2::interpolate_fluid_gradient( const vec2d &p ) const {
	//
	double derivative[DIM2];
	array_derivative2::derivative(m_fluid,p/m_dx-vec2d(0.5,0.5),derivative);
	return vec2d(derivative).normal();
}
//
vec2d macnbflip2::interpolate_solid_gradient( const vec2d &p ) const {
	//
	double derivative[DIM2];
	array_derivative2::derivative(m_solid,p/m_dx,derivative);
	return vec2d(derivative).normal();
}
//
void macnbflip2::advect_levelset( const macarray2<double> &velocity, double dt, double erosion ) {
	//
	// Advect levelset
	if( ! m_fluid_filled ) {
		//
		unsigned dilate_width = m_param.levelset_half_bandwidth_count;
		m_fluid.dilate(dilate_width);
		shared_array2<double> fluid_save(m_fluid);
		m_macadvection->advect_scalar(m_fluid,velocity,fluid_save(),dt);
		//
		if( m_particles.size()) {
			//
			m_redistancer->redistance(m_fluid,dilate_width);
			bool solid_exist = array_utility2::levelset_exist(m_solid);
			//
			// Erosion
			shared_array2<double> save_fluid (m_fluid);
			m_fluid.parallel_actives([&]( int i, int j, auto &it ) {
				if( solid_exist ) {
					if( this->interpolate_solid(m_dx*vec2i(i,j).cell()) > 0.5*m_dx ) {
						it.increment(erosion*m_dx);
					}
				} else {
					it.increment(erosion*m_dx);
				}
			});
			//
			shared_bitarray2 mask(m_fluid.shape());
			std::vector<particlerasterizer2_interface::Particle2> points(m_particles.size());
			for( int n=0; n<m_particles.size(); ++n ) {
				vec2d p = m_particles[n].p;
				points[n].p = p;
				points[n].r = m_particles[n].r;
				mask->set(mask->shape().clamp(p/m_dx));
			}
			//
			mask().dilate(2);
			mask->parallel_actives([&](int i, int j, auto &it, int tn) {
				if( m_fluid(i,j) < -m_dx ) it.set_off();
			});
			m_fluid.activate_as(mask());
			//
			shared_array2<double> particle_levelset(m_shape,0.125*m_dx);
			m_particlerasterizer->build_levelset(particle_levelset(),mask(),points);
			//
			m_fluid.dilate(3);
			m_fluid.parallel_actives([&](int i, int j, auto &it, int tn) {
				double rate = m_sizing_array(i,j);
				double value = rate * std::min(m_fluid(i,j),particle_levelset()(i,j)) + (1.0-rate) * save_fluid()(i,j);
				it.set(value);
			});
		}
		//
		// Re-initialize levelset
		m_redistancer->redistance(m_fluid,dilate_width);
		m_gridutility->extrapolate_levelset(m_solid,m_fluid);
	}
}
//
void macnbflip2::sizing_func( array2<double> &sizing_array, const bitarray2 &mask, const macarray2<double> &velocity, double dt ) {
	//
	sizing_array.clear(1.0);
}
//
void macnbflip2::collision_levelset( std::function<double(const vec2d& p)> levelset ) {
	//
	const double sqrt_DIM = sqrt(DIM2);
	//
	m_fluid.parallel_actives([&]( int i, int j, auto &it, int tn ) {
		vec2d cell_p = m_dx*vec2i(i,j).cell();
		it.set(std::max(m_fluid(i,j),-levelset(cell_p)-sqrt_DIM*m_dx));
	});
}
//
void macnbflip2::draw_flip_circle ( graphics_engine &g, const vec2d &p, double r, bool bullet, double sizing_value ) const {
	const unsigned num_v = 20;
	double alpha = m_fluid_filled ? 0.25 : 0.75;
	if( bullet ) {
		g.color4(1.0,0.5,0.5,alpha);
	} else {
		g.color4(0.5,0.5,1.0,alpha*sizing_value);
	}
	//
	g.begin(graphics_engine::MODE::TRIANGLE_FAN);
	for( unsigned t=0; t<num_v; t++ ) {
		double theta = 2.0 * M_PI * t / (double)num_v;
		g.vertex2v((p+r*vec2d(cos(theta),sin(theta))).v);
	}
	g.end();
	g.color4(1.0,1.0,1.0,0.5);
	g.begin(graphics_engine::MODE::LINE_LOOP);
	for( int t=0; t<num_v; t++ ) {
		double theta = 2.0 * M_PI * t / (double)num_v;
		g.vertex2v((p+r*vec2d(cos(theta),sin(theta))).v);
	}
	g.end();
}
//
void macnbflip2::draw( graphics_engine &g, double time ) const {
	//
	if( ! m_fluid_filled ) {
		if( m_param.draw_levelset ) {
			g.color4(0.5,0.6,1.0,0.5);
			m_gridvisualizer->draw_levelset(g,m_fluid);
		}
	}
	if( m_param.draw_particles ) {
		for( const Particle &particle : m_particles ) {
			const vec2d &p = particle.p;
			draw_flip_circle(g,p,particle.r,particle.bullet,
				array_interpolator2::interpolate(m_sizing_array,p/m_dx-vec2d(0.5,0.5)));
		}
	}
	//
	m_gridvisualizer->draw_active(g,m_fluid);
}
//
extern "C" module * create_instance() {
	return new macnbflip2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
