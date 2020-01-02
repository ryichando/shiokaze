/*
**	macnbflip3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 28, 2017.
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
#ifndef SHKZ_MACNBFLIP3_H
#define SHKZ_MACNBFLIP3_H
//
#include <functional>
#include <vector>
#include <shiokaze/flip/macflip3_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/particlerasterizer/particlerasterizer3_interface.h>
#include <shiokaze/advection/macadvection3_interface.h>
#include <shiokaze/redistancer/redistancer3_interface.h>
#include <shiokaze/surfacetracker/maclevelsetsurfacetracker3_interface.h>
#include <shiokaze/pointgridhash/pointgridhash3_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class macnbflip3 : public macflip3_interface {
protected:
	//
	LONG_NAME("MAC Narrowband FLIP 3D")
	//
	// Seed FLIP particles
	virtual size_t resample( const array3<Real> &fluid,
						 std::function<double(const vec3d &p)> solid,
						 const macarray3<Real> &velocity,
						 std::function<bool(const vec3d &p)> mask ) override;
	//
	// Map FLIP momentum from particles to grid
	virtual void splat( macarray3<macflip3_interface::mass_momentum3> &mass_and_momentum ) const override;
	//
	// Advcect particles through the velocity field
	virtual void advect( std::function<double(const vec3d &p)> solid,
						 std::function<vec3d(const vec3d &p)> velocity,
						 double time, double dt ) override;
	//
	// Mark bullet particles
	virtual void mark_bullet( std::function<double(const vec3d &p)> fluid, std::function<vec3d(const vec3d &p)> velocity, double time ) override;
	//
	// Correct particle position
	virtual void correct( std::function<double(const vec3d &p)> fluid, const macarray3<Real> &velocity ) override;
	//
	// Update fluid level set
	virtual void update( std::function<double(const vec3d &p)> solid, array3<Real> &fluid ) override;
	//
	// Update FLIP velocity
	virtual void update( const macarray3<Real> &prev_velocity,
						 const macarray3<Real> &new_velocity,
						 double dt, vec3d gravity, double PICFLIP ) override;
	//
	// Update FLIP velocity
	virtual void update( std::function<void(const vec3r &p, vec3r &velocity, Real &mass, bool bullet )> func ) override;
	//
	// Delete particles
	virtual size_t remove(std::function<bool(const vec3r &p, bool bullet)> test_function ) override;
	//
	// Draw FLIP partciels
	virtual void draw( graphics_engine &g, double time=0.0 ) const override;
	//
	// Get the number of particles
	virtual size_t get_particle_count() const override { return m_particles.size(); }
	//
	// Get all the FLIP particles.
	virtual std::vector<macflip3_interface::particle3> get_particles() const override;
	//
	virtual void initialize( const shape3 &shape, double dx ) override;
	virtual void post_initialize() override;
	virtual void configure( configuration &config ) override;
	//
	virtual bool const_send_message( std::string message, void *ptr=nullptr ) const override {
		if( message == "narrowband") {
			*static_cast<unsigned *>(ptr) = m_param.narrowband;
			return true;
		}
		return false;
	}
	//
	struct Parameters {
		//
		bool use_apic {true};
		double flip_convexhull_max_dist {3.0};
		double fit_particle_dist {3};
		unsigned narrowband {3};
		int RK_order {2};
		double erosion {0.5};
		unsigned min_particles_per_cell {6};
		unsigned max_particles_per_cell {6};
		unsigned minimal_live_count {5};
		double stiff {1.0};
		bool velocity_correction {true};
		double bullet_maximal_time {0.5};
		bool draw_particles {true};
		double decay_rate {10.0};
	};
	//
	Parameters m_param;
	//
	// FLIP particles
	struct Particle {
		//
		vec3r p;
		vec3r c[DIM3];
		vec3r velocity;
		Real mass;
		Real r;
		char bullet;
		Real bullet_time;
		Real sizing_value;
		unsigned live_count;
	};
	//
	shape3 m_shape;							// Grid resolution
	double m_dx;							// Grid cell size
	//
	std::vector<Particle> m_particles;		// FLIP particle array
	//
	pointgridhash3_driver m_pointgridhash{this,"pointgridhash3"};
	particlerasterizer3_driver m_particlerasterizer{this,"convexhullrasterizer3"};
	parallel_driver m_parallel{this};
	//
	virtual void sort_particles();
	virtual void update_velocity_derivative( Particle& particle, const macarray3<Real> &velocity );
	virtual void additionally_apply_velocity_derivative( macarray3<macflip3_interface::mass_momentum3> &mass_and_momentum ) const;
	//
	static double grid_kernel( const vec3d &r, double dx );
	static vec3d grid_gradient_kernel( const vec3d &r, double dx );
	//
	// A set of fluid levelset functions that are overridable
	virtual double interpolate_fluid( const array3<Real> &fluid, const vec3d &p ) const;
	virtual vec3d interpolate_fluid_gradient( std::function<double(const vec3d &p)> fluid, const vec3d &p ) const;
	virtual vec3d interpolate_fluid_gradient( const array3<Real> &fluid, const vec3d &p ) const;
	virtual vec3d interpolate_solid_gradient( std::function<double(const vec3d &p)> solid, const vec3d &p ) const;
	//
	virtual void fit_particle( std::function<double(const vec3d &p)> fluid, Particle &particle, const vec3d &gradient ) const;
	virtual void collision( std::function<double(const vec3d &p)> solid );
	virtual size_t mark_bullet( double time, std::function<double(const vec3d &p)> fluid, std::function<vec3d(const vec3d &p)> velocity );
	virtual size_t remove_bullet( double time );
};
//
SHKZ_END_NAMESPACE
//
#endif