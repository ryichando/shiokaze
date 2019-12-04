/*
**	macnbflip3.cpp
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
#include "macnbflip3.h"
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/shared_bitarray3.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/array_upsampler3.h>
#include <shiokaze/array/macarray_interpolator3.h>
#include <shiokaze/array/array_derivative3.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
//
static const double default_mass = 1.0 / 8.0;
//
double macnbflip3::grid_kernel( const vec3d &r, double dx ) {
	double x = std::abs(r[0]) / dx;
	double y = std::abs(r[1]) / dx;
	double z = std::abs(r[2]) / dx;
	return std::max(0.0,1.0-x) * std::max(0.0,1.0-y) * std::max(0.0,1.0-z);
}
//
vec3d macnbflip3::grid_gradient_kernel( const vec3d &r, double dx ) {
	double x = std::abs(r[0]) / dx;
	double y = std::abs(r[1]) / dx;
	double z = std::abs(r[2]) / dx;
	if( x <= 1.0 && y <= 1.0 && z <= 1.0 ) {
		double u = std::copysign((y-1.0)*(z-1.0),r[0]);
		double v = std::copysign((x-1.0)*(z-1.0),r[1]);
		double w = std::copysign((x-1.0)*(y-1.0),r[2]);
		return vec3d(u,v,w) / dx;
	} else {
		return vec3d();
	}
}
//
void macnbflip3::configure( configuration &config ) {
	//
	config.get_bool("APIC",m_param.use_apic,"Whether to use APIC");
	config.get_unsigned("Narrowband",m_param.narrowband,"Narrowband bandwidth");
	config.get_double("FitParticleDist",m_param.fit_particle_dist,"FLIP particle fitting threshold");
	config.get_integer("RK_Order",m_param.RK_order,"Order of accuracy for Runge-kutta integration");
	config.get_double("Erosion",m_param.erosion,"Rate of erosion for internal levelset");
	config.get_unsigned("MinParticlesPerCell",m_param.min_particles_per_cell,"Minimal target number of particles per cell");
	config.get_unsigned("MaxParticlesPerCell",m_param.max_particles_per_cell,"Maximal target number of particles per cell");
	config.get_unsigned("MiminalLiveCount",m_param.minimal_live_count,"Minimal step of particles to stay alive");
	config.get_double("CorrectStiff",m_param.stiff,"Position correction strength");
	config.get_bool("VelocityCorrection",m_param.velocity_correction,"Should do velocity correction");
	config.get_double("BulletMaximalTime",m_param.bullet_maximal_time,"Maximal time for bullet particles to survive");
	config.get_bool("DrawFLIPParticles",m_param.draw_particles,"Whether to draw FLIP particles.");
	config.get_double("DecayRate",m_param.decay_rate,"Decay rate for tracer particles");
	//
}
//
void macnbflip3::initialize( const shape3 &shape, double dx ) {
	//
	m_shape = shape;
	m_dx = dx;
}
//
void macnbflip3::post_initialize() {
	//
	m_particles.resize(0);
	sort_particles();
}
//
size_t macnbflip3::mark_bullet( double time, std::function<double(const vec3d &p)> fluid, std::function<vec3d(const vec3d &p)> velocity) {
	//
	if( m_particles.size()) {
		//
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			char new_status (0);
			if( fluid(particle.p) > 0.0 ) {
				new_status = 1;
				for( int dim : DIMS3 ) particle.c[dim] = vec3d();
			}
			if( new_status != particle.bullet ) {
				particle.bullet = new_status;
				particle.bullet_time = new_status ? time : 0.0;
				if( ! particle.bullet ) {
					particle.mass = default_mass;
					particle.r = 0.25 * m_dx;
					particle.velocity = velocity(particle.p);
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
size_t macnbflip3::remove_bullet( double time ) {
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
	m_particles.shrink_to_fit();
	//
	// Update hash table
	sort_particles();
	return removed_total;
}
//
typedef struct {
	Real mass;
	Real momentum;
} mass_momentum3;
//
void macnbflip3::splat( macarray3<macflip3_interface::mass_momentum3> &mass_and_momentum ) const {
	scoped_timer timer(this);
	if( m_particles.size()) {
		timer.tick(); console::dump( ">>> Splatting FLIP particles...\n");
		//
		// Splat
		timer.tick(); console::dump( "Splatting momentum...");
		//
		mass_and_momentum.clear();
		shared_bitarray3 cell_mask(m_shape);
		for( size_t n=0; n<m_particles.size(); ++n ) {
			cell_mask().set(m_shape.clamp(m_particles[n].p/m_dx));
		}
		cell_mask->const_serial_actives([&]( int i, int j, int k ) {
			const vec3i pi(i,j,k);
			for( int dim : DIMS3 ) {
				mass_and_momentum[dim].set(pi,{0.,0.});
				mass_and_momentum[dim].set(pi+vec3i(dim==0,dim==1,dim==2),{0.,0.});
			}
		});
		//
		mass_and_momentum.dilate();
		mass_and_momentum.parallel_actives([&]( int dim, int i, int j, int k, auto &it, int tn ) {
			//
			Real mom (0.0), m (0.0);
			vec3d pos = m_dx*vec3i(i,j,k).face(dim);
			std::vector<size_t> neighbors = m_pointgridhash->get_face_neighbors(vec3i(i,j,k),dim);
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
		console::dump( "Done. Took %s\n", timer.stock("splat_momentum").c_str());
		//
		if( m_param.use_apic ) {
			timer.tick(); console::dump( "Additionally applying velocity derivative...");
			additionally_apply_velocity_derivative(mass_and_momentum);
			console::dump( "Done. Took %s\n", timer.stock("splat_velocity_derivative").c_str());
		}
		console::dump( "<<< Done. Took %s\n", timer.stock("splat_particles").c_str());
	} else {
		mass_and_momentum.clear();
	}
}
//
void macnbflip3::advect( std::function<double(const vec3d &p)> solid,
						 std::function<vec3d(const vec3d &p)> velocity,
						 double time, double dt ) {
	//
	if( m_particles.size()) {
		//
		scoped_timer timer(this);
		std::string order_str = "RK"+ std::to_string(m_param.RK_order);
		timer.tick(); console::dump( "Advecting %d particles (%s)...", m_particles.size(), order_str.c_str());
		//
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			bool bullet = particle.bullet;
			const vec3d &u = particle.velocity;
			const vec3d &p = particle.p;
			if( bullet ) {
				particle.p += dt*u;
			} else {
				vec3d u1 = velocity(p);
				if( u1.norm2()) {
					if( m_param.RK_order==4 ) {
						vec3d u2 = velocity(p+0.5*dt*u1);
						vec3d u3 = velocity(p+0.5*dt*u2);
						vec3d u4 = velocity(p+dt*u3);
						particle.p += dt*(u1+2.0*u2+2.0*u3+u4)/6.0;
					} else if( m_param.RK_order==2 ) {
						vec3d u2 = velocity(p+dt*u1);
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
			// Decay particle sizing value
			m_particles[n].sizing_value -= m_param.decay_rate * dt;
		});
		//
		sort_particles();
		console::dump( "Done. Took %s\n", timer.stock("particles_advection").c_str());
	}
	//
	// Perform collision
	collision(solid);
}
//
void macnbflip3::mark_bullet( std::function<double(const vec3d &p)> fluid, std::function<vec3d(const vec3d &p)> velocity, double time ) {
	//
	if( m_particles.size()) {
		//
		// Mark bullet
		scoped_timer timer(this);
		timer.tick(); console::dump( "Marking bullet particles...");
		size_t num_bullet = mark_bullet(time,fluid,velocity);
		console::write("number_bullet",num_bullet);
		console::dump( "Done. Bullet count = %u, Took %s.\n", num_bullet, timer.stock("mark_bullet").c_str());
		//
		// Remove long-term ballistic particles
		timer.tick(); console::dump( "Removing bullet particles...");
		size_t removed_total = remove_bullet(time);
		num_bullet = 0;
		for( size_t n=0; n<m_particles.size(); ++n ) if( m_particles[n].bullet ) ++ num_bullet;
		console::dump( "Done. Removed = %u. Bullet count = %u. Took %s.\n", removed_total,num_bullet, timer.stock("mark_bullet").c_str());
	}
}
//
void macnbflip3::update( std::function<double(const vec3d &p)> solid, array3<Real> &fluid ) {
	//
	if( m_particles.size()) {
		//
		scoped_timer timer(this);
		timer.tick(); console::dump( "Correcting levelset...");
		//
		shared_array3<Real> save_fluid (fluid);
		fluid.parallel_actives([&]( int i, int j, int k, auto &it ) {
			if( solid(m_dx*vec3d(i,j,k)) > m_dx ) {
				it.increment(m_param.erosion*m_dx);
			}
		});
		//
		shared_bitarray3 mask(fluid.shape());
		std::vector<particlerasterizer3_interface::Particle3> points(m_particles.size());
		for( int n=0; n<m_particles.size(); ++n ) {
			vec3d p = m_particles[n].p;
			points[n].p = p;
			points[n].r = m_particles[n].r;
			mask->set(mask->shape().clamp(p/m_dx));
		}
		//
		mask->dilate(2);
		shared_array3<Real> particle_levelset(m_shape,1.0);
		m_particlerasterizer->build_levelset(particle_levelset(),mask(),points);
		//
		fluid.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			double sizing_sum (0.0);
			double sizing_weight (0.0);
			std::vector<size_t> list = m_pointgridhash->get_points_in_cell(vec3i(i,j,k));
			for( const auto &n : list ) {
				double w (m_particles[n].mass);
				sizing_sum += w*m_particles[n].sizing_value;
				sizing_weight += w;
			}
			if( sizing_weight ) sizing_sum /= sizing_weight;
			//
			double value = sizing_sum * std::min(fluid(i,j,k),particle_levelset()(i,j,k)) + (1.0-sizing_sum) * save_fluid()(i,j,k);
			it.set(value);
		});
		//
		console::dump("Done. Took %s.\n", timer.stock("levelset_correction").c_str());
	}
}
//
void macnbflip3::collision( std::function<double(const vec3d &p)> solid ) {
	//
	if( m_particles.size()) {
		//
		m_parallel.for_each( m_particles.size(), [&]( size_t pindex ) {
			Particle &particle = m_particles[pindex];
			vec3r &p = particle.p;
			vec3r &u = particle.velocity;
			const double &r = particle.r;
			double phi = solid(p)-r;
			if( phi < 0.0 ) {
				vec3d gradient = interpolate_solid_gradient(solid,p);
				p = p - phi*gradient;
				double dot = gradient * u;
				if( dot < 0.0 ) u = u - gradient*dot;
			}
			for( unsigned dim : DIMS3 ) {
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
}
//
void macnbflip3::correct( std::function<double(const vec3d &p)> fluid, const macarray3<Real> &velocity ) {
	//
	// Compute correction displacement
	if( m_particles.size()) {
		scoped_timer timer(this);
		timer.tick(); console::dump( "Performing position correction...");
		//
		std::vector<vec3d> displacements(m_particles.size());
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			const Particle &pi = m_particles[n];
			vec3d displacement;
			std::vector<size_t> neighbors = m_pointgridhash->get_cell_neighbors(m_shape.find_cell(pi.p/m_dx),pointgridhash3_interface::USE_NODAL);
			for( const size_t &j : neighbors ) {
				if( n != j ) {
					const Particle &pj = m_particles[j];
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
			displacements[n] = displacement;
		});
		//
		// Kill the normal components to prevent volume gain
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			if( ! displacements[n].empty()) {
				const Particle &p = m_particles[n];
				vec3d new_pos = p.p+displacements[n];
				vec3d normal = this->interpolate_fluid_gradient(fluid,new_pos);
				double dot = displacements[n] * normal;
				if( dot > 0.0 ) displacements[n] -= dot * normal;
			}
		});
		//
		// Apply velocity correction
		if( m_param.velocity_correction ) {
			m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
				vec3d mid_p = 0.5 * (m_particles[n].p+displacements[n]);
				for( int dim : DIMS3 ) {
					vec3d gradient = array_derivative3::derivative(velocity[dim],m_dx*vec3i().face(dim),m_dx,mid_p);
					m_particles[n].velocity[dim] += gradient * displacements[n];
				}
			});
		}
		//
		// Apply displacement
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			m_particles[n].p += displacements[n];
		});
		//
		// Update hash table
		sort_particles();
		//
		console::dump( "Done. Took %s\n", timer.stock("possition_correction").c_str());
	}
}
//
void macnbflip3::fit_particle( std::function<double(const vec3d &p)> fluid, Particle &particle, const vec3d &gradient ) const {
	//
	if( std::abs(fluid(particle.p)) < m_param.fit_particle_dist * particle.r ) {
		for( unsigned n=0; n<3; ++n ) {
			particle.p -= 0.5 * (fluid(particle.p)+particle.r) * gradient;
		}
	}
}
//
size_t macnbflip3::seed(const array3<Real> &fluid,
						std::function<double(const vec3d &p)> solid,
						const macarray3<Real> &velocity ) {
	//
	scoped_timer timer(this);
	timer.tick(); console::dump(">>> Reseeding particles...\n");
	timer.tick(); console::dump( "Computing narrowband (%d cells wide)...", m_param.narrowband);
	//
	// Compute narrowband
	shared_bitarray3 narrowband_mask(m_shape);
	bool smoke_simulation_flag (true);
	fluid.const_serial_actives([&](int i, int j, int k, auto &it) {
		if( it() > 0 && solid(m_dx*vec3i(i,j,k).cell()) > 0 ) {
			narrowband_mask->set(i,j,k);
		}
		if( it() < 0.0 ) smoke_simulation_flag = false;
	});
	//
	if( smoke_simulation_flag ) {
		narrowband_mask->activate_all();
	} else {
		narrowband_mask->dilate([&](int i, int j, int k, auto& it, int tn ) {
			if(fluid.active(i,j,k) && fluid(i,j,k) < 0 && solid(m_dx*vec3i(i,j,k).cell()) > 0.125*m_dx ) it.set();
		},m_param.narrowband);
		//
		fluid.const_serial_actives([&](int i, int j, int k, auto &it) {
			if( it() > m_dx ) narrowband_mask->set_off(i,j,k);
		});
	}
	//
	size_t count = narrowband_mask->count();
	console::dump( "Done. Found %d cells. Took %s\n", count, timer.stock("compute_narrowband").c_str());
	//
	timer.tick(); console::dump( "Computing sizing function...");
	//
	// Compute sizing function
	shared_array3<Real> sizing_array(m_shape);
	compute_sizing_func(fluid,narrowband_mask(),velocity,sizing_array());
	//
	// Update sizing value on particles
	if( m_particles.size()) {
		m_parallel.for_each(m_particles.size(),[&]( size_t n, int tn ) {
			Particle &p = m_particles[n];
			p.sizing_value = std::max((Real)p.sizing_value,sizing_array()(m_shape.find_cell(p.p/m_dx)));
		});
	}
	//
	console::dump( "Done. Took %s\n", timer.stock("compute_sizing_func").c_str());
	timer.tick(); console::dump( "Reseeding particles...");
	//
	std::vector<std::vector<Particle> > new_particles_t(m_parallel.get_thread_num());
	std::vector<char> remove_particles(m_particles.size(),0);
	//
	// Bucket cell method to remove too dense particles
	shared_array3<char> cell_bucket(m_shape);
	if( m_particles.size()) {
		for( size_t n=0; n<m_particles.size(); ++n ) {
			const Particle &p = m_particles[n];
			vec3i pi = m_shape.clamp(p.p/m_dx);
			int i (pi[0]), j (pi[1]), k (pi[2]);
			m_shape.clamp(i,j,k);
			if( ! p.bullet ) {
				if( ! narrowband_mask()(i,j,k) || ! sizing_array()(i,j,k) || cell_bucket()(i,j,k) >= m_param.max_particles_per_cell || p.sizing_value < 0.0 ) {
					if( p.live_count > m_param.minimal_live_count ) {
						remove_particles[n] = 1;
					}
				}
			}
			if( ! remove_particles[n] && solid(p.p) < -p.r ) {
				remove_particles[n] = 1;
			}
			if( utility::box(p.p,vec3d(0.0,0.0,0.0),m_dx*vec3d(m_shape[0],m_shape[1],m_shape[2])) > -p.r ) {
				remove_particles[n] = 1;
			}
			if( ! remove_particles[n] ) {
				cell_bucket().increment(i,j,k,1);
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
	narrowband_mask->const_parallel_actives([&]( int i, int j, int k, int tn ) {
		//
		size_t num_added (0);
		double sizing_value = sizing_array()(i,j,k);
		auto attempt_reseed = [&]( const vec3d &p ) {
			//
			double r = 0.25 * m_dx;
			if( cell_bucket()(i,j,k)+num_added < m_param.min_particles_per_cell ) {
				//
				if( smoke_simulation_flag || this->interpolate_fluid(fluid,p) < -r ) {
					//
					// Put a FLIP particle here...
					Particle new_particle;
					new_particle.p = p;
					bool sparse (true);
					const std::vector<size_t> &indices = m_pointgridhash->get_points_in_cell(vec3i(i,j,k));
					for( auto it=indices.begin(); it!=indices.end(); ++it ) {
						if( (m_particles[*it].p-p).len() <= 2.0*r ) {
							sparse = false;
							break;
						}
					}
					if( sparse && solid(new_particle.p) > r ) {
						new_particle.mass = default_mass;
						new_particle.velocity = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p,true);
						new_particle.r = r;
						new_particle.bullet = 0;
						new_particle.bullet_time = 0.0;
						new_particle.live_count = 0;
						new_particle.sizing_value = sizing_value;
						if( m_param.use_apic ) this->update_velocity_derivative(new_particle,velocity);
						this->fit_particle([&](const vec3d &p) {
							return this->interpolate_fluid(fluid,p);
						},new_particle,this->interpolate_fluid_gradient(fluid,new_particle.p));
						new_particles_t[tn].push_back(new_particle);
						num_added ++;
					}
				}
			}
		};
		//
		if( sizing_value ) {
			if( ! smoke_simulation_flag && fluid(i,j,k) < -1.25*m_dx ) {
				attempt_reseed(m_dx*vec3i(i,j,k).cell());
			} else {
				for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) for( unsigned kk=0; kk<2; kk++ ) {
					vec3d p = m_dx*vec3d(i,j,k)+0.25*m_dx*vec3d(1,1,1)+0.5*m_dx*vec3d(ii,jj,kk);
					attempt_reseed(p);
				}
			}
		}
	});
	//
	// Reconstruct a new particle array
	std::vector<Particle> old_particles (m_particles);
	m_particles.clear();
	size_t reseeded (0);
	for( size_t t=0; t<new_particles_t.size(); ++t ) {
		m_particles.insert(m_particles.end(),new_particles_t[t].begin(),new_particles_t[t].end());
		reseeded += new_particles_t[t].size();
	}
	size_t removed (0);
	for( size_t i=0; i<remove_particles.size(); i++) {
		if( ! remove_particles[i] ) m_particles.push_back(old_particles[i]);
		else ++ removed;
	}
	m_particles.shrink_to_fit();
	//
	console::write("number_particles",m_particles.size());
	console::write("number_removed",removed);
	console::write("number_reseeded",reseeded);
	//
	// Update hash table
	sort_particles();
	console::dump( "Done. Reseed=%u Removed=%u. Took %s\n", reseeded, removed, timer.stock("compute_reseed").c_str());
	console::dump( "<<< Done. Took %s\n",  timer.stock("seed").c_str());
	//
	return reseeded;
}
//
size_t macnbflip3::remove(std::function<double(const vec3r &p, bool bullet)> test_function ) {
	//
	size_t removed_count (0);
	if( m_particles.size()) {
		//
		scoped_timer timer(this);
		timer.tick(); console::dump( "Removing FLIP particles...");
		//
		std::vector<Particle> old_particles (m_particles);
		std::vector<char> remove_flag (m_particles.size(),0);
		//
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			remove_flag[n] = test_function(old_particles[n].p,old_particles[n].bullet);
		});
		//
		m_particles.clear();
		for( size_t i=0; i<old_particles.size(); ++i) {
			if( remove_flag[i] ) ++ removed_count;
			else m_particles.push_back(old_particles[i]);
		}
		//
		if( removed_count ) {
			m_particles.shrink_to_fit();
			sort_particles();
		}
		console::dump( "Done. Removed = %u. Took %s.\n", removed_count, timer.stock("remove").c_str());
	}
	return removed_count;
}
//
void macnbflip3::update( const macarray3<Real> &prev_velocity, const macarray3<Real> &new_velocity,
						 double dt, vec3d gravity, double PICFLIP ) {
	//
	if(m_particles.size()) {
		//
		scoped_timer timer(this);
		timer.tick(); console::dump( "Updating FLIP velocities...");
		//
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
					particle.velocity = macarray_interpolator3::interpolate(new_velocity,vec3d(),m_dx,particle.p,true);
					//
					// Update particle velocity derivative (APIC)
					update_velocity_derivative(particle,new_velocity);
					//
				} else {
					//
					// Fetch grid velocity
					vec3d new_grid_velocity = macarray_interpolator3::interpolate(new_velocity,vec3d(),m_dx,particle.p,true);
					vec3d old_grid_velocity = macarray_interpolator3::interpolate(prev_velocity,vec3d(),m_dx,particle.p,true);
					//
					// Compute pure FLIP velocity
					vec3d FLIP_velocity = particle.velocity + (new_grid_velocity-old_grid_velocity);
					//
					// Compute PICFLIP velocity
					vec3d PICFLIP_velocity = PICFLIP * FLIP_velocity + (1.0-PICFLIP) * new_grid_velocity;
					//
					// Compute the final velocity of FLIP
					particle.velocity = PICFLIP_velocity;
				}
			}
		});
		//
		console::dump( "Done. Took %s\n", timer.stock("update").c_str());
	}
}
//
void macnbflip3::update( std::function<void(const vec3r &p, vec3r &velocity, Real &mass, bool bullet )> func ) {
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		Particle &particle = m_particles[n];
		func(particle.p,particle.velocity,particle.mass,particle.bullet);
	});
}
//
std::vector<macflip3_interface::particle3> macnbflip3::get_particles() const {
	std::vector<macflip3_interface::particle3> result;
	for( size_t n=0; n<m_particles.size(); ++n ) {
		macflip3_interface::particle3 particle;
		particle.p = m_particles[n].p;
		particle.r = m_particles[n].r;
		particle.sizing_value = m_particles[n].sizing_value;
		particle.bullet = m_particles[n].bullet;
		result.push_back(particle);
	}
	//
	size_t num_bullet (0);
	for( size_t n=0; n<m_particles.size(); ++n ) if( m_particles[n].bullet ) ++ num_bullet;
	//
	return result;
}
//
void macnbflip3::sort_particles() {
	//
	if( m_particles.size()) {
		std::vector<vec3r> points(m_particles.size());
		m_parallel.for_each(m_particles.size(),[&]( size_t n) {
			points[n] = m_particles[n].p;
		});
		m_pointgridhash->sort_points(points);
	} else {
		m_pointgridhash->clear();
	}
}
//
void macnbflip3::update_velocity_derivative( Particle& particle, const macarray3<Real> &velocity ) {
	//
	// Written by Takahiro Sato
	for (int dim : DIMS3) {
		vec3r &c = particle.c[dim];
		c = vec3d();
		//
		// particle position
		const vec3d p_pos = particle.p;
		vec3d offset = 0.5*vec3d(dim!=0,dim!=1,dim!=2);
		///
		// cell index
		int i = std::floor(p_pos[0]/m_dx-offset[0]);
		int j = std::floor(p_pos[1]/m_dx-offset[1]);
		int k = std::floor(p_pos[2]/m_dx-offset[2]);
		//
		// cell position
		// 0: (i,j,k), 1: (i+1,j,k), 2: (i,j+1,k), 3: (i+1,j+1,k),
		// 4: (i,j,k+1), 5: (i+1,j,k+1), 6: (i,j+1,k+1), 7: (i+1,j+1,k+1)
		vec3d cell_pos[8];
		cell_pos[0] = m_dx*(vec3d(i,j,k)+offset);
		cell_pos[1] = m_dx*(vec3d(i+1,j,k)+offset);
		cell_pos[2] = m_dx*(vec3d(i,j+1,k)+offset);
		cell_pos[3] = m_dx*(vec3d(i+1,j+1,k)+offset);
		cell_pos[4] = m_dx*(vec3d(i,j,k+1)+offset);
		cell_pos[5] = m_dx*(vec3d(i+1,j,k+1)+offset);
		cell_pos[6] = m_dx*(vec3d(i,j+1,k+1)+offset);
		cell_pos[7] = m_dx*(vec3d(i+1,j+1,k+1)+offset);
		//
		vec3d dw[8];
		dw[0] = grid_gradient_kernel(cell_pos[0]-p_pos,m_dx);
		dw[1] = grid_gradient_kernel(cell_pos[1]-p_pos,m_dx);
		dw[2] = grid_gradient_kernel(cell_pos[2]-p_pos,m_dx);
		dw[3] = grid_gradient_kernel(cell_pos[3]-p_pos,m_dx);
		dw[4] = grid_gradient_kernel(cell_pos[4]-p_pos,m_dx);
		dw[5] = grid_gradient_kernel(cell_pos[5]-p_pos,m_dx);
		dw[6] = grid_gradient_kernel(cell_pos[6]-p_pos,m_dx);
		dw[7] = grid_gradient_kernel(cell_pos[7]-p_pos,m_dx);
		//
		auto v_shape = velocity[dim].shape();
		//
		c += dw[0] * velocity[dim](v_shape.clamp(i,j,k));
		c += dw[1] * velocity[dim](v_shape.clamp(i+1,j,k));
		c += dw[2] * velocity[dim](v_shape.clamp(i,j+1,k));
		c += dw[3] * velocity[dim](v_shape.clamp(i+1,j+1,k));
		c += dw[4] * velocity[dim](v_shape.clamp(i,j,k+1));
		c += dw[5] * velocity[dim](v_shape.clamp(i+1,j,k+1));
		c += dw[6] * velocity[dim](v_shape.clamp(i,j+1,k+1));
		c += dw[7] * velocity[dim](v_shape.clamp(i+1,j+1,k+1));
	}
}
//
void macnbflip3::additionally_apply_velocity_derivative( macarray3<macflip3_interface::mass_momentum3> &mass_and_momentum ) const {
	//
	// Written by Takahiro Sato
	mass_and_momentum.parallel_actives([&]( int dim, int i, int j, int k, auto &it ) {
		vec3d pos = m_dx*vec3d(i+0.5*(dim!=0),j+0.5*(dim!=1),k+0.5*(dim!=2));
		std::vector<size_t> neighbors = m_pointgridhash->get_face_neighbors(vec3i(i,j,k),dim);
		double mom (0.0);
		for( size_t k : neighbors ) {
			const Particle &p = m_particles[k];
			const vec3d &c = (p.c)[dim];
			const vec3d r = pos-p.p;
			double w = grid_kernel(r,m_dx);
			if( w ) {
				mom += w*p.mass*(c*r);
			}
		}
		const auto value = it();
		it.set({value.mass,(Real)(value.momentum+mom)});
	});
}
//
double macnbflip3::interpolate_fluid( const array3<Real> &fluid, const vec3d &p ) const {
	return array_interpolator3::interpolate(fluid,p/m_dx-vec3d(0.5,0.5,0.5),true);
}
//
vec3d macnbflip3::interpolate_fluid_gradient( std::function<double(const vec3d &p)> fluid, const vec3d &p ) const {
	vec3d result;
	for( int dim : DIMS3 ) result[dim] = fluid(p+0.25*m_dx*vec3d(dim==0,dim==1,dim==2))-fluid(p-0.25*m_dx*vec3d(dim==0,dim==1,dim==2));
	return result.normal();
}
//
vec3d macnbflip3::interpolate_fluid_gradient( const array3<Real> &fluid, const vec3d &p ) const {
	//
	Real derivative[DIM3];
	array_derivative3::derivative(fluid,p/m_dx-vec3d(0.5,0.5,0.5),derivative);
	return vec3d(derivative).normal();
}
//
vec3d macnbflip3::interpolate_solid_gradient( std::function<double(const vec3d &p)> solid, const vec3d &p ) const {
	vec3d result;
	for( int dim : DIMS3 ) result[dim] = solid(p+0.5*m_dx*vec3d(dim==0,dim==1,dim==2))-solid(p-0.5*m_dx*vec3d(dim==0,dim==1,dim==2));
	return result.normal();
}
//
void macnbflip3::draw( graphics_engine &g, double time ) const {
	//
	if( m_param.draw_particles ) {
		//
		g.point_size(2.0);
		g.begin(graphics_engine::MODE::POINTS);
		for( const Particle &particle : m_particles ) {
			const vec3d &p = particle.p;
			double alpha = particle.sizing_value;
			if( particle.bullet ) g.color4(1.0,0.5,0.5,alpha);
			else g.color4(0.5,0.5,1.0,alpha);
			g.vertex3v(p.v);
		}
		g.end();
	}
}
//
extern "C" module * create_instance() {
	return new macnbflip3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
