/*
**	macnbflip2.h
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
#ifndef SHKZ_MACNBFLIP2_H
#define SHKZ_MACNBFLIP2_H
//
#include <functional>
#include <vector>
#include <shiokaze/math/vec.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/flip/macflip2_interface.h>
#include <shiokaze/utility/gridutility2_interface.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/particlerasterizer/particlerasterizer2_interface.h>
#include <shiokaze/redistancer/redistancer2_interface.h>
#include <shiokaze/advection/macadvection2_interface.h>
#include <shiokaze/pointgridhash/pointgridhash2_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class macnbflip2 : public macflip2_interface {
protected:
	//
	LONG_NAME("MAC Narrowband FLIP 2D")
	//
	// Seed FLIP particles
	virtual size_t resample( const array2<Real> &fluid,
						 std::function<double(const vec2d &p)> solid,
						 const macarray2<Real> &velocity,
						 std::function<bool(const vec2d &p)> mask ) override;
	//
	// Map FLIP momentum from particles to grid
	virtual void splat( macarray2<macflip2_interface::mass_momentum2> &mass_and_momentum ) const override;
	//
	// Advcect particles through the velocity field
	virtual void advect( std::function<double(const vec2d &p)> solid,
						 std::function<vec2d(const vec2d &p)> velocity,
						 double time, double dt ) override;
	//
	// Mark bullet particles
	virtual void mark_bullet( std::function<double(const vec2d &p)> fluid, std::function<vec2d(const vec2d &p)> velocity, double time ) override;
	//
	// Correct particle position
	virtual void correct( std::function<double(const vec2d &p)> fluid, const macarray2<Real> &velocity ) override;
	//
	// Update fluid level set
	virtual void update( std::function<double(const vec2d &p)> solid, array2<Real> &fluid ) override;
	//
	// Update FLIP velocity
	virtual void update( const macarray2<Real> &prev_velocity,
						 const macarray2<Real> &new_velocity,
						 double dt, vec2d gravity, double PICFLIP ) override;
	//
	// Update FLIP velocity
	virtual void update( std::function<void(const vec2r &p, vec2r &velocity, Real &mass, bool bullet )> func ) override;
	//
	// Delete particles
	virtual size_t remove(std::function<bool(const vec2r &p, bool bullet)> test_function ) override;
	//
	// Draw FLIP partciels
	virtual void draw( graphics_engine &g, double time=0.0 ) const override;
	//
	// Get the number of particles
	virtual size_t get_particle_count() const override { return m_particles.size(); }
	//
	// Get all the FLIP particles.
	virtual std::vector<macflip2_interface::particle2> get_particles() const override;
	//
	virtual void initialize( const shape2 &shape, double dx ) override;
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
		vec2r p;
		vec2r c[DIM2];
		vec2r velocity;
		Real mass;
		Real r;
		char bullet;
		Real bullet_time;
		Real sizing_value;
		unsigned live_count;
	};
	//
	shape2 m_shape;						// Grid resolution
	double m_dx;						// Grid cell size
	std::vector<Particle> m_particles;	// FLIP particle array
	//
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	pointgridhash2_driver m_pointgridhash{this,"pointgridhash2"};
	particlerasterizer2_driver m_particlerasterizer{this,"convexhullrasterizer2"};
	parallel_driver m_parallel{this};
	//
	virtual void sort_particles();
	virtual void update_velocity_derivative( Particle& particle, const macarray2<Real> &velocity );
	virtual void additionally_apply_velocity_derivative( macarray2<macflip2_interface::mass_momentum2> &mass_and_momentum ) const;
	//
	static double grid_kernel( const vec2d &r, double dx );
	static vec2d grid_gradient_kernel( const vec2d &r, double dx );
	//
	// A set of fluid levelset functions that are overridable
	virtual double interpolate_fluid( const array2<Real> &fluid, const vec2d &p ) const;
	virtual vec2d interpolate_fluid_gradient( std::function<double(const vec2d &p)> fluid, const vec2d &p ) const;
	virtual vec2d interpolate_fluid_gradient( const array2<Real> &fluid, const vec2d &p ) const;
	virtual vec2d interpolate_solid_gradient( std::function<double(const vec2d &p)> solid, const vec2d &p ) const;
	virtual void draw_flip_circle ( graphics_engine &g, const vec2d &p, double r, bool bullet ) const;
	//
	virtual void fit_particle( std::function<double(const vec2d &p)> fluid, Particle &particle, const vec2d &gradient ) const;
	virtual void collision( std::function<double(const vec2d &p)> solid );
	virtual size_t mark_bullet( double time, std::function<double(const vec2d &p)> fluid, std::function<vec2d(const vec2d &p)> velocity );
	virtual size_t remove_bullet( double time );
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
