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
#include <shiokaze/surfacetracker/macsurfacetracker3_interface.h>
#include <shiokaze/pointgridhash/pointgridhash3_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class macnbflip3 : public macflip3_interface {
public:
	//
	macnbflip3();
	LONG_NAME("MAC Narrowband FLIP 3D")
	//
	// Set solid
	virtual void assign_solid( const array3<double> &solid ) override;
	//
	// Seed FLIP particles
	virtual size_t seed( const array3<double> &fluid, const macarray3<double> &velocity ) override;
	//
	// Map FLIP velocity from particles to grid
	virtual void splat( macarray3<double> &momentum, macarray3<double> &mass ) const override;
	//
	// Advcect particles through the velocity field
	virtual void advect( const macarray3<double> &velocity, double time, double dt ) override;
	//
	// Update FLIP velocity
	virtual void update( const macarray3<double> &prev_velocity, const macarray3<double> &new_velocity,
						 double dt, vec3d gravity, double PICFLIP ) override;
	//
	// Update FLIP velocity
	virtual void update( std::function<void(const vec3d &p, vec3d &velocity, double &mass, bool bullet )> func ) override;
	//
	// Get levelset
	virtual void get_levelset( array3<double> &fluid ) const override;
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
	// Export mesh and particles
	virtual void export_mesh_and_ballistic_particles( int frame, std::string dir_path ) const override;
	//
protected:
	//
	virtual void initialize( const shape3 &shape, double dx ) override;
	virtual void post_initialize() override;
	virtual void configure( configuration &config ) override;
	//
	struct Parameters {
		//
		bool use_apic {true};
		double fit_particle_dist {3.0};
		unsigned levelset_half_bandwidth {2};
		unsigned narrowband {3};
		unsigned correct_depth {3};
		int RK_order {2};
		double erosion {0.5};
		unsigned min_particles_per_cell {6};
		unsigned max_particles_per_cell {6};
		unsigned minimal_live_count {5};
		double stiff {1.0};
		bool velocity_correction {true};
		double surface_margin {0.125};
		double bullet_maximal_time {0.5};
		double sizing_eps {1e-2};
		bool loose_interior {true};
		bool draw_particles {true};
		//
		double decay_rate {10.0};
		unsigned diffuse_count {4};
		double diffuse_rate {0.75};
	};
	//
	Parameters m_param;
	//
	// FLIP particles
	struct Particle {
		vec3d p;
		vec3d c[DIM3];
		vec3d velocity;
		double mass;
		double r;
		char bullet;
		double bullet_time;
		double bullet_sizing_value;
		double sizing_value;
		unsigned live_count;
		//
		vec3d gen_p;
		char particle_id;
		char last_split_id;
	};
	//
	shape3 m_shape;							// Grid resolution
	double m_dx;							// Grid cell size
	//
	shape3 m_double_shape;					// Grid resolution
	double m_half_dx;						// Grid cell size
	//
	std::vector<Particle> m_particles;		// FLIP particle array
	//
	gridutility3_driver m_gridutility{this,"gridutility3"};
	macutility3_driver m_macutility{this,"macutility3"};
	pointgridhash3_driver m_pointgridhash{this,"pointgridhash3"};
	macadvection3_driver m_macadvection{this,"macadvection3"};
	particlerasterizer3_driver m_particlerasterizer{this,"convexhullrasterizer3"};
	particlerasterizer3_driver m_highres_particlerasterizer{this,"flatrasterizer3"};
	redistancer3_driver m_redistancer{this,"pderedistancer3"};
	macsurfacetracker3_driver m_highres_macsurfacetracker{this,"maclevelsetsurfacetracker3"};

	parallel_driver m_parallel{this};
	//
	bool m_fluid_filled;
	bool m_solid_exit;
	//
	virtual void sort_particles();
	virtual void update_velocity_derivative( Particle& particle, const macarray3<double> &velocity );
	virtual void additionally_apply_velocity_derivative( macarray3<double> &momentum ) const;
	//
	static double grid_kernel( const vec3d &r, double dx );
	static vec3d grid_gradient_kernel( const vec3d &r, double dx );
	//
	array3<double>	m_fluid{this};				// Internal fluid levelset
	array3<double>	m_solid{this};				// Internal solid levelset
	array3<double>	m_sizing_array{this};		// Internal sizing array
	bitarray3		m_narrowband_mask{this};		// Internal narrowband mask
	//
	// A set of fluid levelset functions that are overridable
	virtual void seed_set_fluid( const array3<double> &fluid );
	virtual void initialize_fluid();
	virtual void initialize_solid();
	virtual void advect_levelset( const macarray3<double> &velocity, double dt, double erosion );
	virtual void collision_levelset( std::function<double(const vec3d& p)> levelset );
	virtual double interpolate_fluid( const vec3d &p ) const;
	virtual double interpolate_solid( const vec3d &p ) const;
	virtual vec3d interpolate_fluid_gradient( const vec3d &p ) const;
	virtual vec3d interpolate_solid_gradient( const vec3d &p ) const;
	//
	virtual void fit_particle( std::function<double(const vec3d &p)> fluid, Particle &particle, const vec3d &gradient ) const;
	virtual size_t mark_bullet( double time, const macarray3<double> &velocity );
	virtual size_t remove_bullet( double time );
	virtual void collision();
	virtual size_t compute_narrowband();
	virtual size_t correct( const macarray3<double> &velocity, const array3<double> *mask=nullptr );
	virtual void reseed( const macarray3<double> &velocity, size_t &reseeded, size_t &removed, bool loose_interior=false );
	virtual void sizing_func( array3<double> &sizing_array, const bitarray3 &mask, const macarray3<double> &velocity, double dt );
};
//
SHKZ_END_NAMESPACE
//
#endif