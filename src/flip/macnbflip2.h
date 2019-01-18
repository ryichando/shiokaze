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
public:
	//
	macnbflip2();
	LONG_NAME("MAC Narrowband FLIP 2D")
	//
	// Set solid
	virtual void assign_solid( const array2<double> &solid ) override;
	//
	// Seed FLIP particles
	virtual size_t seed( const array2<double> &fluid, const macarray2<double> &velocity ) override;
	//
	// Map FLIP momentum from particles to grid
	virtual void splat( macarray2<double> &momentum, macarray2<double> &mass ) const override;
	//
	// Advcect particles through the velocity field
	virtual void advect( const macarray2<double> &velocity, double time, double dt ) override;
	//
	// Update FLIP velocity
	virtual void update( const macarray2<double> &prev_velocity, const macarray2<double> &new_velocity,
						 double dt, vec2d gravity, double PICFLIP ) override;
	//
	// Update FLIP velocity
	virtual void update( std::function<void(const vec2d &p, vec2d &velocity, double &mass, bool bullet )> func ) override;
	//
	// Get levelset
	virtual void get_levelset( array2<double> &fluid ) const override;
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
protected:
	//
	virtual void initialize( const shape2 &shape, double dx ) override;
	virtual void post_initialize() override;
	virtual void configure( configuration &config ) override;
	//
	struct Parameters {
		//
		bool use_apic {true};
		double flip_convexhull_max_dist {3.0};
		double fit_particle_dist {3};
		unsigned narrowband {3};
		unsigned correct_depth {3};
		int RK_order {2};
		double erosion {0.5};
		unsigned min_particles_per_cell {6};
		unsigned max_particles_per_cell {6};
		unsigned minimal_live_count {5};
		double stiff {1.0};
		bool velocity_correction {true};
		double bullet_maximal_time {0.5};
		double sizing_eps {1e-2};
		bool loose_interior {true};
		bool draw_particles {true};
		bool draw_levelset {true};
	};
	//
	Parameters m_param;
	//
	// FLIP particles
	struct Particle {
		//
		vec2d p;
		vec2d c[DIM2];
		vec2d velocity;
		double mass;
		double r;
		char bullet;
		double bullet_time;
		double bullet_sizing_value;
		double sizing_value;
		unsigned live_count;
		//
		vec2d gen_p;
		char particle_id;
		char last_split_id;
	};
	//
	shape2 m_shape;						// Grid resolution
	double m_dx;						// Grid cell size
	std::vector<Particle> m_particles;	// FLIP particle array
	//
	gridutility2_driver m_gridutility{this,"gridutility2"};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	macutility2_driver m_macutility{this,"macutility2"};
	pointgridhash2_driver m_pointgridhash{this,"pointgridhash2"};
	macadvection2_driver m_macadvection{this,"macadvection2"};
	particlerasterizer2_driver m_particlerasterizer{this,"convexhullrasterizer2"};
	redistancer2_driver m_redistancer{this,"pderedistancer2"};
	parallel_driver m_parallel{this};
	//
	bool m_fluid_filled;
	bool m_solid_exit;
	//
	virtual void sort_particles();
	virtual void update_velocity_derivative( Particle& particle, const macarray2<double> &velocity );
	virtual void additionally_apply_velocity_derivative( macarray2<double> &momentum ) const;
	//
	static double grid_kernel( const vec2d &r, double dx );
	static vec2d grid_gradient_kernel( const vec2d &r, double dx );
	//
	array2<double>	m_fluid{this};					// Internal fluid levelset
	array2<double>	m_solid{this};					// Internal solid levelset
	array2<double>	m_sizing_array{this};			// Internal sizing array
	bitarray2		m_narrowband_mask{this};			// Internal narrowband mask
	//
	// A set of fluid levelset functions that are overridable
	virtual void seed_set_fluid( const array2<double> &fluid );
	virtual void initialize_fluid();
	virtual void initialize_solid();
	virtual void advect_levelset( const macarray2<double> &velocity, double dt, double erosion );
	virtual void collision_levelset( std::function<double(const vec2d& p)> levelset );
	virtual double interpolate_fluid( const vec2d &p ) const;
	virtual double interpolate_solid( const vec2d &p ) const;
	virtual vec2d interpolate_fluid_gradient( const vec2d &p ) const;
	virtual vec2d interpolate_solid_gradient( const vec2d &p ) const;
	virtual void drawCircle ( graphics_engine &g, const vec2d &p, double r, bool bullet, double sizing_value ) const;
	//
	virtual void fit_particle( std::function<double(const vec2d &p)> fluid, Particle &particle, const vec2d &gradient ) const;
	virtual void compute_narrowband();
	virtual void collision();
	virtual size_t correct( const macarray2<double> &velocity, const array2<double> *mask=nullptr );
	virtual size_t reseed( const macarray2<double> &velocity, bool loose_interior=false );
	virtual size_t mark_bullet( double time, const macarray2<double> &velocity );
	virtual size_t remove_bullet( double time );
	virtual void sizing_func( array2<double> &sizing_array, const bitarray2 &mask, const macarray2<double> &velocity, double dt );
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
