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
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <cstdio>
#include <cstdlib>
//
SHKZ_USING_NAMESPACE
//
static const double default_mass = 1.0 / 8.0;
//
double macnbflip3::grid_kernel( const vec3d &r, double dx ) {
	double x = ( r[0] > 0.0 ? r[0] : -r[0] ) / dx;
	double y = ( r[1] > 0.0 ? r[1] : -r[1] ) / dx;
	double z = ( r[2] > 0.0 ? r[2] : -r[2] ) / dx;
	return std::max(0.0,1.0-x) * std::max(0.0,1.0-y) * std::max(0.0,1.0-z);
}
//
vec3d macnbflip3::grid_gradient_kernel( const vec3d &r, double dx ) {
	double x = ( r[0] > 0.0 ? r[0] : -r[0] ) / dx;
	double y = ( r[1] > 0.0 ? r[1] : -r[1] ) / dx;
	double z = ( r[2] > 0.0 ? r[2] : -r[2] ) / dx;
	if( x <= 1.0 && y <= 1.0 && z <= 1.0 ) {
		double x_sgn = r[0] <= 0.0 ? -1.0 : 1.0;
		double y_sgn = r[1] <= 0.0 ? -1.0 : 1.0;
		double z_sgn = r[2] <= 0.0 ? -1.0 : 1.0;
		return vec3d(x_sgn*(y-1.0)*(z-1.0),y_sgn*(x-1.0)*(z-1.0),z_sgn*(x-1.0)*(y-1.0)) / dx;
	} else {
		return vec3d();
	}
}
//
macnbflip3::macnbflip3() {
	//
	m_macadvection.set_name("Levelset Advection 3D for FLIP","LevelsetAdvectionFLIP");
}
//
void macnbflip3::configure( configuration &config ) {
	//
	config.set_default_double("HighresRasterizer.RadiusFactor",1.0);
	config.set_default_double("HighresRasterizer.WeightFactor",2.0);
	config.set_default_unsigned("HighresRasterizer.NeighborLookUpCells",2);
	//
	config.get_bool("APIC",m_param.use_apic,"Whether to use APIC");
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
}
//
void macnbflip3::initialize( const shape3 &shape, double dx ) {
	//
	m_shape = shape;
	m_dx = dx;
	//
	m_double_shape = 2 * m_shape;
	m_half_dx = 0.5 * m_dx;
	//
	m_highres_particlerasterizer->set_environment("shape",&m_double_shape);
	m_highres_particlerasterizer->set_environment("dx",&m_half_dx);
	//
	m_highres_macsurfacetracker->set_environment("shape",&m_double_shape);
	m_highres_macsurfacetracker->set_environment("dx",&m_half_dx);
}
void macnbflip3::post_initialize() {
	//
	// Copy the grid size
	m_fluid_filled = false;
	m_solid_exit = false;
	//
	// Initialize fluid levelset
	initialize_fluid();
	initialize_solid();
	//
	m_sizing_array.initialize(m_shape,1.0);
	m_narrowband_mask.initialize(m_shape);
	m_particles.resize(0);
	sort_particles();
}
//
void macnbflip3::assign_solid( const array3<double> &solid ) {
	m_solid.copy(solid);
	m_solid_exit = array_utility3::levelset_exist(solid);
}
//
size_t macnbflip3::seed( const array3<double> &fluid, const macarray3<double> &velocity ) {
	//
	scoped_timer timer(this);
	seed_set_fluid(fluid);
	m_fluid_filled = true;
	fluid.interruptible_const_serial_actives([&]( int i, int j, int k, const auto &it ) {
		if( it() > 0.0 ) {
			m_fluid_filled = false;
			return true;
		}
		return false;
	});
	//
	size_t count = compute_narrowband();
	sizing_func(m_sizing_array,m_narrowband_mask,velocity,0.0);
	timer.tick(); console::dump( "Seeding FLIP particles...");
	size_t seeded, removed;
	reseed(velocity,seeded,removed,m_param.loose_interior);
	//
	console::dump( "Done. Seed=%d. Took %s\n", seeded, timer.stock("splat_particles").c_str());
	console::write("number_seed",seeded);
	console::write("number_seed_narrowband_cells",count);
	//
	return seeded;
}
//
size_t macnbflip3::mark_bullet( double time, const macarray3<double> &velocity ) {
	//
	if( m_particles.size()) {
		//
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			char new_status (0);
			if( interpolate_fluid(particle.p) > 0.0 ) {
				new_status = 1;
				for( int dim : DIMS3 ) particle.c[dim] = vec3d();
			}
			if( new_status != particle.bullet ) {
				particle.bullet = new_status;
				particle.bullet_sizing_value = std::min(1.0,particle.sizing_value);
				particle.bullet_time = new_status ? time : 0.0;
				if( ! particle.bullet ) {
					particle.mass = default_mass;
					particle.r = 0.25 * m_dx;
					particle.velocity = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,particle.p);
					update_velocity_derivative(particle,velocity);
				}
			}
		});
		//
		size_t num_bullet(0);
		for( size_t n=0; n<m_particles.size(); ++n ) if( m_particles[n].bullet ) ++ num_bullet;
		//
		console::write("number_bullet",num_bullet);
		return num_bullet;
	} else {
		return 0;
	}
}
//
size_t macnbflip3::remove_bullet( double time ) {
	//
	if( ! m_param.bullet_maximal_time || m_particles.empty() ) return 0;
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
	std::vector<Particle> old_particles = m_particles;
	m_particles.clear();
	size_t removed_total (0);
	for( size_t i=0; i<old_particles.size(); i++) {
		if( ! remove_flag[i] ) m_particles.push_back(old_particles[i]);
		else ++ removed_total;
	}
	// Update hash table
	sort_particles();
	//
	console::write("number_remove_bullet",removed_total);
	return removed_total;
}
//
typedef struct {
	double mass;
	double momentum;
} mass_momentum3;
//
void macnbflip3::splat( macarray3<double> &momentum, macarray3<double> &mass ) const {
	//
	scoped_timer timer(this);
	if( m_particles.size()) {
		timer.tick(); console::dump( ">>> Splatting FLIP particles...\n");
		//
		// Splat
		timer.tick(); console::dump( "Splatting momentum...");
		//
		shared_macarray3<mass_momentum3> mass_and_momentum(momentum.shape());
		//
		shared_bitarray3 cell_mask(m_shape);
		for( size_t n=0; n<m_particles.size(); ++n ) {
			cell_mask().set(m_shape.clamp(m_particles[n].p/m_dx));
		}
		cell_mask->const_serial_actives([&]( int i, int j, int k, auto &it ) {
			const vec3i pi(i,j,k);
			for( int dim : DIMS3 ) {
				mass_and_momentum()[dim].set(pi,{0.,0.});
				mass_and_momentum()[dim].set(pi+vec3i(dim==0,dim==1,dim==2),{0.,0.});
			}
		});
		//
		mass_and_momentum().dilate();
		mass_and_momentum->parallel_actives([&]( int dim, int i, int j, int k, auto &it, int tn ) {
			//
			double mom (0.0), m (0.0);
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
		mass.clear();
		mass.activate_as(mass_and_momentum());
		mass.parallel_actives([&]( int dim, int i, int j, int k, auto &it, int tn ) {
			it.set(mass_and_momentum()[dim](i,j,k).mass);
		});
		//
		momentum.clear();
		momentum.activate_as(mass_and_momentum());
		momentum.parallel_actives([&]( int dim, int i, int j, int k, auto &it, int tn ) {
			it.set(mass_and_momentum()[dim](i,j,k).momentum);
		});
		console::dump( "Done. Took %s\n", timer.stock("splat_momentum").c_str());
		//
		if( m_param.use_apic ) {
			timer.tick(); console::dump( "Additionally applying velocity derivative...");
			additionally_apply_velocity_derivative(momentum);
			console::dump( "Done. Took %s\n", timer.stock("splat_velocity_derivative").c_str());
		}
		console::dump( "<<< Done. Took %s\n", timer.stock("splat_particles").c_str());
	} else {
		momentum.clear();
		mass.clear();
	}
	//
}
//
void macnbflip3::advect( const macarray3<double> &velocity, double time, double dt ) {
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( ">>> Performing advection\n");
	//
	// Advect particles
	if( m_particles.size()) {
		//
		std::string order_str = "RK"+ std::to_string(m_param.RK_order);
		timer.tick(); console::dump( "Advecting %d particles (%s)...", m_particles.size(), order_str.c_str());
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			bool bullet = particle.bullet;
			const vec3d &u = particle.velocity;
			const vec3d &p = particle.p;
			if( bullet ) {
				particle.p += dt*u;
			} else {
				vec3d u1 = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p);
				if( u1.norm2()) {
					if( m_param.RK_order==4 ) {
						vec3d u2 = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p+0.5*dt*u1);
						vec3d u3 = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p+0.5*dt*u2);
						vec3d u4 = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p+dt*u3);
						particle.p += dt*(u1+2.0*u2+2.0*u3+u4)/6.0;
					} else if( m_param.RK_order==2 ) {
						vec3d u2 = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p+dt*u1);
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
		console::dump( "Done. Took %s\n", timer.stock("particles_advection").c_str());
		//
		// Correct particle position
		if( m_param.stiff ) {
			timer.tick(); console::dump( "Performing position correction");
			if( m_fluid_filled==false && m_param.correct_depth && m_param.correct_depth != m_param.narrowband ) console::dump(" (depth=%d)...", m_param.correct_depth );
			else console::dump("...");
			size_t correct_count (0);
			if( m_fluid_filled ) {
				correct_count = correct(velocity,nullptr);
			} else {
				correct_count = correct(velocity,&m_fluid);
			}
			console::dump( "Done. Corrected %d particles. Took %s\n", correct_count, timer.stock("possition_correction").c_str());
			console::write("number_position_correction",correct_count);
		}
		//
	}
	//
	// Perform collision
	timer.tick(); console::dump( "Performing collision correction...");
	collision();
	console::dump( "Done. Took %s\n", timer.stock("collision_correction").c_str());
	//
	// Levelset advection
	if( ! m_fluid_filled ) {
		advect_levelset(velocity,dt,m_param.erosion);
		size_t narrowband_cell_count = compute_narrowband();
		console::write("number_narrowband_cells",narrowband_cell_count);
	}
	//
	// Recompute sizing function
	timer.tick(); console::dump( "Computing sizing function...");
	sizing_func(m_sizing_array,m_narrowband_mask,velocity,dt);
	console::dump( "Done. Took %s\n", timer.stock("sizing_function").c_str());
	//
	// Reseed particles
	timer.tick(); console::dump( "Performing particle reseeding...");
	size_t reseeded, removed;
	reseed(velocity,reseeded,removed,false);
	console::dump( "Done. Seed=%d Remove=%d Total=%d. Took %s\n", reseeded, removed, m_particles.size(), timer.stock("particle_reseeding").c_str());
	//
	// Mark bullet
	if( m_particles.size()) {
		timer.tick(); console::dump( "Marking and removing bullet particles...");
		size_t num_bullets = mark_bullet(time,velocity);
		size_t removed_num = remove_bullet(time);
		console::dump( "Done. Marked=%d. Removed=%d. Took %s\n", num_bullets, removed_num, timer.stock("bullet_particles").c_str());
	}
	//
	console::dump( "<<< Done. Took %s\n", timer.stock("advection").c_str());
}
//
size_t macnbflip3::compute_narrowband() {
	size_t count (0);
	scoped_timer timer(this);
	if( m_param.narrowband && ! m_fluid_filled ) {
		timer.tick(); console::dump( "Computing narrowband (%d cells wide)...", m_param.narrowband);
		//
		m_narrowband_mask.clear();
		m_fluid.const_serial_actives([&](int i, int j, int k, auto &it) {
			if( it() > 0 && this->interpolate_solid(m_dx*vec3i(i,j,k).cell()) > 0 ) {
				m_narrowband_mask.set(i,j,k);
			}
		});
		for( int n=0; n<m_param.narrowband; ++n ) {
			m_narrowband_mask.dilate([&](int i, int j, int k, auto& it, int tn ) {
				if(m_fluid(i,j,k) < 0 && this->interpolate_solid(m_dx*vec3i(i,j,k).cell()) > 0.125*m_dx ) it.set();
			});
		}
		m_fluid.const_serial_actives([&](int i, int j, int k, auto &it) {
			if( it() > m_dx ) m_narrowband_mask.set_off(i,j,k);
		});
		count = m_narrowband_mask.count();
		console::dump( "Done. Found %d cells. Took %s\n", count, timer.stock("compute_narrowband").c_str());
	} else {
		m_narrowband_mask.clear();
	}
	return count;
}
//
void macnbflip3::collision() {
	//
	if( m_particles.size()) {
		m_parallel.for_each( m_particles.size(), [&]( size_t pindex ) {
			Particle &particle = m_particles[pindex];
			vec3d &p = particle.p;
			vec3d &u = particle.velocity;
			const double &r = particle.r;
			double phi = interpolate_solid(p)-r;
			if( phi < 0.0 ) {
				vec3d gradient = interpolate_solid_gradient(p);
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
		//
	}
	sort_particles();
	collision_levelset([&]( const vec3d &p ) {
		return interpolate_solid(p);
	});
}
//
size_t macnbflip3::correct( const macarray3<double> &velocity, const array3<double> *mask ) {
	//
	if( m_particles.empty()) return 0;
	//
	// Compute correction displacement
	std::vector<vec3d> displacements(m_particles.size());
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		bool skip (false);
		const Particle &pi = m_particles[n];
		if( mask ) {
			if( ! (*mask).active(m_shape.find_cell(pi.p/m_dx))) skip = true;
		}
		vec3d displacement;
		if( ! skip ) {
			std::vector<size_t> neighbors = m_pointgridhash->get_cell_neighbors(m_shape.find_cell(pi.p/m_dx),pointgridhash3_interface::USE_NODAL);
			for( const size_t &j : neighbors ) {
				if( n != j ) {
					const Particle &pj = m_particles[j];
					if( mask ) {
						if( ! (*mask).active(m_shape.find_cell(pj.p/m_dx)) ) skip = true;
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
				vec3d new_pos = p.p+displacements[n];
				vec3d normal = interpolate_fluid_gradient(new_pos);
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
			const vec3d &pos = m_particles[n].p;
			for( int dim : DIMS3 ) {
				if( pos[dim] < 0.0 || pos[dim] > m_dx*m_shape[dim] ) skip = true;
			}
			if( ! skip ) {
				if( m_param.velocity_correction ) {
					vec3d jacobian[DIM3], incr;
					m_macutility->get_velocity_jacobian(pos,velocity,jacobian);
					for ( int dim : DIMS3 ) {
						incr[dim] = jacobian[dim] * displacements[n];
					}
					m_particles[n].velocity += incr;
				}
				m_particles[n].p += displacements[n];
			}
		}
		m_particles[n].p += displacements[n];
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
void macnbflip3::fit_particle( std::function<double(const vec3d &p)> fluid, Particle &particle, const vec3d &gradient ) const {
	if( std::abs(fluid(particle.p)) < m_param.fit_particle_dist * particle.r ) {
		for( size_t n=0; n<3; ++n ) {
			double signed_dist = fluid(particle.p);
			double gap = signed_dist < 0.0 ? signed_dist+particle.r : signed_dist-particle.r;
			particle.p -= 0.5 * gap * gradient;
		}
	}
}
//
void macnbflip3::reseed( const macarray3<double> &velocity, size_t &reseeded, size_t &removed, bool loose_interior ) {
	//
	std::vector<std::vector<Particle> > new_particles_t(m_parallel.get_maximal_threads());
	std::vector<char> remove_particles(m_particles.size(),0);
	//
	// Bucket cell method to remove too dense particles
	shared_array3<char> cell_bucket(m_shape);
	if( m_particles.size()) {
		cell_bucket->initialize(m_shape,0);
		for( size_t n=0; n<m_particles.size(); ++n ) {
			const Particle &p = m_particles[n];
			vec3i pi = m_shape.clamp(p.p/m_dx);
			int i (pi[0]), j (pi[1]), k(pi[2]);
			m_shape.clamp(i,j,k);
			if( ! p.bullet ) {
				if( ! m_sizing_array(i,j,k) || cell_bucket()(i,j,k) >= m_param.max_particles_per_cell || 
					(! m_fluid_filled && ! m_narrowband_mask(i,j,k)) || p.sizing_value < 0.0 ) {
					if( p.live_count > m_param.minimal_live_count ) {
						remove_particles[n] = 1;
					}
				}
			}
			if( ! remove_particles[n] && interpolate_solid(p.p) < -p.r ) {
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
	m_parallel.for_each(m_shape,[&]( int i, int j, int k, int tn ) {
		//
		size_t num_added (0);
		auto attempt_reseed = [&]( const vec3d &p ) {
			if( cell_bucket()(i,j,k)+num_added < m_param.min_particles_per_cell ) {
				//
				double r = 0.25 * m_dx;
				if( interpolate_fluid(p) < -r ) {
					//
					// Put a FLIP particle here...
					double r = 0.25 * m_dx;
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
					if( sparse && interpolate_solid(new_particle.p) > r ) {
						double sizing_value = m_sizing_array(m_sizing_array.shape().clamp(p/m_dx));
						new_particle.mass = default_mass;
						new_particle.velocity = macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p);
						new_particle.r = r;
						new_particle.bullet = 0;
						new_particle.bullet_time = 0.0;
						new_particle.bullet_sizing_value = 0.0;
						new_particle.sizing_value = sizing_value;
						new_particle.live_count = 0;
						new_particle.gen_p = new_particle.p;
						update_velocity_derivative(new_particle,velocity);
						fit_particle([&](const vec3d &p) {
							return interpolate_fluid(p);
						},new_particle,interpolate_fluid_gradient(new_particle.p));
						new_particles_t[tn].push_back(new_particle);
						num_added ++;
					}
				}
			}
		};
		//
		if( m_fluid_filled || (m_narrowband_mask(i,j,k) && m_sizing_array(i,j,k))) {
			if( loose_interior && m_fluid(i,j,k) < -1.25*m_dx ) {
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
	m_particles.resize(0);
	reseeded = 0;
	for( size_t t=0; t<new_particles_t.size(); ++t ) {
		m_particles.insert(m_particles.end(),new_particles_t[t].begin(),new_particles_t[t].end());
		reseeded += new_particles_t[t].size();
	}
	removed = 0;
	for( size_t i=0; i<remove_particles.size(); i++) {
		if( ! remove_particles[i] ) m_particles.push_back(old_particles[i]);
		else ++ removed;
	}
	//
	console::write("number_particles",m_particles.size());
	console::write("number_removed",removed);
	console::write("number_reseeded",reseeded);
	//
	// Update hash table
	sort_particles();
}
//
void macnbflip3::update( const macarray3<double> &prev_velocity, const macarray3<double> &new_velocity, double dt, vec3d gravity, double PICFLIP ) {
	scoped_timer timer(this);
	//
	if(m_particles.size()) {
		//
		timer.tick(); console::dump( "Updating FLIP velocities...");
		m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
			Particle &particle = m_particles[n];
			if( particle.bullet ) {
				//
				// If the particle is a ballistic particle, just add gravity force
				particle.velocity += dt * gravity;
			} else {
				//
				if( m_param.use_apic ) {
					//
					// Fetch grid velocity
					particle.velocity = macarray_interpolator3::interpolate(new_velocity,vec3d(),m_dx,particle.p);
					//
					// Update particle velocity derivative (APIC)
					update_velocity_derivative(particle,new_velocity);
					//
				} else {
					//
					// Fetch grid velocity
					vec3d new_grid_velocity = macarray_interpolator3::interpolate(new_velocity,vec3d(),m_dx,particle.p);
					vec3d old_grid_velocity = macarray_interpolator3::interpolate(prev_velocity,vec3d(),m_dx,particle.p);
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
void macnbflip3::update( std::function<void(const vec3d &p, vec3d &velocity, double &mass, bool bullet )> func ) {
	m_parallel.for_each(m_particles.size(),[&]( size_t n ) {
		Particle &particle = m_particles[n];
		func(particle.p,particle.velocity,particle.mass,particle.bullet);
	});
}
//
void macnbflip3::get_levelset( array3<double> &fluid ) const {
	fluid.copy(m_fluid);
}
//
std::vector<macflip3_interface::particle3> macnbflip3::get_particles() const {
	std::vector<macflip3_interface::particle3> result;
	for( size_t n=0; n<m_particles.size(); ++n ) {
		macflip3_interface::particle3 particle;
		particle.p = m_particles[n].p;
		particle.r = m_particles[n].r;
		particle.bullet = m_particles[n].bullet;
		result.push_back(particle);
	}
	return result;
}
//
void macnbflip3::export_mesh_and_ballistic_particles( int frame, std::string dir_path ) const {
	//
	scoped_timer timer(this);
	//
	timer.tick(); console::dump( "Computing high-resolution levelset..." );
	//
	shared_array3<double> doubled_fluid(m_double_shape.cell(),1.0);
	shared_array3<double> doubled_sizing_array(m_double_shape.cell());
	shared_array3<double> doubled_solid(m_double_shape.nodal(),1.0);
	//
	array_upsampler3::upsample_to_double_cell<double>(m_fluid,m_dx,doubled_fluid());
	array_upsampler3::upsample_to_double_cell<double>(m_sizing_array,m_dx,doubled_sizing_array());
	array_upsampler3::upsample_to_double_nodal<double>(m_solid,m_dx,doubled_solid());
	//
	shared_bitarray3 mask(m_double_shape);
	std::vector<particlerasterizer3_interface::Particle3> points, ballistic_points;
	for( int n=0; n<m_particles.size(); ++n ) {
		particlerasterizer3_interface::Particle3 point;
		point.p = m_particles[n].p;
		point.r = m_particles[n].r;
		if( interpolate_fluid(m_particles[n].p) < 0.5*m_dx ) {
			points.push_back(point);
			mask().set(mask->shape().clamp(point.p/m_half_dx));
		} else {
			if( m_particles[n].bullet ) ballistic_points.push_back(point);
		}
	}
	//
	mask().dilate([&](int i, int j, int k, auto &it, int tn ) { it.set(); },4);
	doubled_fluid->activate_as(mask());
	//
	shared_array3<double> particle_levelset(m_double_shape,0.125*m_dx);
	m_highres_particlerasterizer->build_levelset(particle_levelset(),mask(),points);
	//
	doubled_fluid->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
		double rate = doubled_sizing_array()(i,j,k);
		double f = it(), p = particle_levelset()(i,j,k);
		it.set( rate * std::min(f,p) + (1.0-rate) * f );
	});
	//
	console::dump( "Done. Took %s\n", timer.stock("generate_highres_mesh").c_str());
	//
	macsurfacetracker3_driver &highres_macsurfacetracker = const_cast<macsurfacetracker3_driver &>(m_highres_macsurfacetracker);
	highres_macsurfacetracker->assign(doubled_solid(),doubled_fluid());
	//
	auto vertex_color_func = [&](const vec3d &p) { return p; };
	auto uv_coordinate_func = [&](const vec3d &p) { return vec2d(p[0],0.0); };
	//
	timer.tick(); console::dump( "Generating mesh..." );
	highres_macsurfacetracker->export_fluid_mesh(dir_path,frame,vertex_color_func,uv_coordinate_func);
	console::dump( "Done. Took %s\n", timer.stock("export_highres_mesh").c_str());
	//
	std::string particle_path = console::format_str("%s/%d_particles.dat",dir_path.c_str(),frame);
	timer.tick(); console::dump( "Writing ballistic particles..." );
	FILE *fp = fopen(particle_path.c_str(),"wb");
	unsigned size = ballistic_points.size();
	fwrite(&size,1,sizeof(unsigned),fp);
	for( size_t n=0; n<size; ++n ) {
		float position[3] = { (float)ballistic_points[n].p.v[0],
							  (float)ballistic_points[n].p.v[1],
							  (float)ballistic_points[n].p.v[2] };
		float radius = ballistic_points[n].r;
		fwrite(position,3,sizeof(float),fp);
		fwrite(&radius,1,sizeof(float),fp);
	}
	fclose(fp);
	console::dump( "Done. Size=%d. Took %s\n", size, timer.stock("write_ballistic").c_str());
}
//
void macnbflip3::sort_particles() {
	if( m_particles.size()) {
		std::vector<vec3d> points(m_particles.size());
		m_parallel.for_each(m_particles.size(),[&]( size_t n) {
			points[n] = m_particles[n].p;
		});
		m_pointgridhash->sort_points(points);
	} else {
		m_pointgridhash->clear();
	}
}
//
void macnbflip3::update_velocity_derivative( Particle& particle, const macarray3<double> &velocity ) {
	//
	if( m_param.use_apic ) {
		// Written by Takahiro Sato
		for (int dim : DIMS3) {
			vec3d &c = particle.c[dim];
			c = vec3d();
			//
			// particle position
			const vec3d p_pos = particle.p;
			///
			// cell index
			int i = std::floor((p_pos[0]-0.5*m_dx*(dim!=0))/m_dx);
			int j = std::floor((p_pos[1]-0.5*m_dx*(dim!=1))/m_dx);
			int k = std::floor((p_pos[2]-0.5*m_dx*(dim!=2))/m_dx);
			//
			// cell position
			// 0: (i,j,k), 1: (i+1,j,k), 2: (i,j+1,k), 3: (i+1,j+1,k),
			// 4: (i,j,k+1), 5: (i+1,j,k+1), 6: (i,j+1,k+1), 7: (i+1,j+1,k+1)
			vec3d cell_pos[8];
			cell_pos[0] = m_dx*vec3d(i+0.5*(dim!=0),   j+0.5*(dim!=1),   k+0.5*(dim!=2));
			cell_pos[1] = m_dx*vec3d(i+1+0.5*(dim!=0), j+0.5*(dim!=1),   k+0.5*(dim!=2));
			cell_pos[2] = m_dx*vec3d(i+0.5*(dim!=0),   j+1+0.5*(dim!=1), k+0.5*(dim!=2));
			cell_pos[3] = m_dx*vec3d(i+1+0.5*(dim!=0), j+1+0.5*(dim!=1), k+0.5*(dim!=2));
			cell_pos[4] = m_dx*vec3d(i+0.5*(dim!=0),   j+0.5*(dim!=1),   k+1+0.5*(dim!=2));
			cell_pos[5] = m_dx*vec3d(i+1+0.5*(dim!=0), j+0.5*(dim!=1),   k+1+0.5*(dim!=2));
			cell_pos[6] = m_dx*vec3d(i+0.5*(dim!=0),   j+1+0.5*(dim!=1), k+1+0.5*(dim!=2));
			cell_pos[7] = m_dx*vec3d(i+1+0.5*(dim!=0), j+1+0.5*(dim!=1), k+1+0.5*(dim!=2));
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
}
//
void macnbflip3::additionally_apply_velocity_derivative( macarray3<double> &momentum ) const {
	// Written by Takahiro Sato
	momentum.parallel_actives([&]( int dim, int i, int j, int k, auto &it ) {
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
		it.increment(mom);
	});
}
//
void macnbflip3::initialize_fluid() {
	//
	m_fluid.initialize(m_shape);
	m_fluid.set_as_levelset(m_dx);
}
//
void macnbflip3::initialize_solid() {
	//
	m_solid.initialize(m_shape.nodal());
	m_solid.set_as_levelset(m_dx);
}
//
void macnbflip3::seed_set_fluid( const array3<double> &fluid ) {
	//
	m_fluid.activate_as(fluid);
	m_fluid.parallel_actives([&]( int i, int j, int k, auto &it, int tn ) {
		it.set(std::min(fluid(i,j,k),it()));
	});
	m_fluid.flood_fill();
}
//
double macnbflip3::interpolate_fluid( const vec3d &p ) const {
	return m_fluid_filled ? -1.0 : array_interpolator3::interpolate(m_fluid,p/m_dx-vec3d(0.5,0.5,0.5));
}
//
double macnbflip3::interpolate_solid( const vec3d &p ) const {
	return array_interpolator3::interpolate(m_solid,p/m_dx);
}
//
vec3d macnbflip3::interpolate_fluid_gradient( const vec3d &p ) const {
	//
	double derivative[DIM3];
	array_derivative3::derivative(m_fluid,p/m_dx-vec3d(0.5,0.5,0.5),derivative);
	return vec3d(derivative).normal();
}
//
vec3d macnbflip3::interpolate_solid_gradient( const vec3d &p ) const {
	//
	double derivative[DIM3];
	array_derivative3::derivative(m_solid,p/m_dx,derivative);
	return vec3d(derivative).normal();
}
//
void macnbflip3::advect_levelset( const macarray3<double> &velocity, double dt, double erosion ) {
	//
	if( ! m_fluid_filled ) {
		//
		scoped_timer timer(this);
		timer.tick(); console::dump( ">>> Levelset advection\n");
		//
		unsigned dilate_width = 2+m_fluid.get_levelset_halfwidth()+std::ceil(m_macutility->compute_max_u(velocity)*dt/m_dx);
		for( int n=0; n<m_particles.size(); ++n ) {
			vec3i pi = m_shape.clamp(m_particles[n].p/m_dx);
			if( ! m_fluid.active(pi)) m_fluid.set(pi,m_fluid(pi));
		}
		m_fluid.dilate(dilate_width);
		shared_array3<double> fluid_save(m_fluid);
		m_macadvection->advect_scalar(m_fluid,velocity,fluid_save(),dt);
		//
		if( m_particles.size()) {
			m_redistancer->redistance(m_fluid,dilate_width);
			bool solid_exist = array_utility3::levelset_exist(m_solid);
			//
			// Erosion
			timer.tick(); console::dump( "Levelset erosion...");
			shared_array3<double> save_fluid (m_fluid);
			//
			m_fluid.parallel_actives([&]( int i, int j, int k, auto &it ) {
				if( solid_exist ) {
					if( this->interpolate_solid(m_dx*vec3i(i,j,k).cell()) > 0.5*m_dx ) {
						it.increment(erosion*m_dx);
					}
				} else {
					it.increment(erosion*m_dx);
				}
			});
			console::dump("Done. Took %s.\n", timer.stock("levelset_erosion").c_str());
			//
			// Compute new levelset surface
			timer.tick(); console::dump( "Building FLIP levelset...");
			//
			shared_bitarray3 mask(m_fluid.shape());
			std::vector<particlerasterizer3_interface::Particle3> points(m_particles.size());
			for( int n=0; n<m_particles.size(); ++n ) {
				vec3d p = m_particles[n].p;
				points[n].p = p;
				points[n].r = m_particles[n].r;
				mask().set(mask->shape().clamp(p/m_dx));
			}
			//
			mask().dilate(2);
			m_fluid.activate_as(mask());
			//
			shared_array3<double> particle_levelset(m_shape,0.125*m_dx);
			m_particlerasterizer->build_levelset(particle_levelset(),mask(),points);
			//
			console::dump("Done. Took %s.\n", timer.stock("particle_levelset_construction").c_str());
			//
			timer.tick(); console::dump( "Combining levelsets...");
			//
			m_fluid.dilate(3);
			m_fluid.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				double rate = m_sizing_array(i,j,k);
				double value = rate * std::min(m_fluid(i,j,k),particle_levelset()(i,j,k)) + (1.0-rate) * save_fluid()(i,j,k);
				it.set(value);
			});
			console::dump("Done. Took %s.\n", timer.stock("levelset_combine").c_str());
		}
		//
		// Re-initialize levelset
		timer.tick(); console::dump( "Extrapolate and redistancing levelset..." );
		m_redistancer->redistance(m_fluid,dilate_width);
		m_gridutility->extrapolate_levelset(m_solid,m_fluid);
		console::dump("Done. Took %s\n", timer.stock("extrapolate_redistance").c_str());
		console::dump("<<< Done. Took %s.\n", timer.stock("levelset_advection").c_str());
	}
}
//
void macnbflip3::sizing_func( array3<double> &sizing_array, const bitarray3 &mask, const macarray3<double> &velocity, double dt ) {
	sizing_array.clear(1.0);
}
//
void macnbflip3::collision_levelset( std::function<double(const vec3d& p)> levelset ) {
	//
	const double sqrt_DIM = sqrt(DIM3);
	//
	m_fluid.parallel_actives([&]( int i, int j, int k, auto &it, int tn ) {
		vec3d cell_p = m_dx*vec3i(i,j,k).cell();
		it.set(std::max(m_fluid(i,j,k),-levelset(cell_p)-sqrt_DIM*m_dx));
	});
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
			double alpha = m_fluid_filled ? 0.05 : 0.5 * array_interpolator3::interpolate(m_sizing_array,p/m_dx-vec3d(0.5,0.5,0.5));
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
