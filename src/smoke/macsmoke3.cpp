/*
**	macsmoke3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 17, 2017. 
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
#include "macsmoke3.h"
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/filesystem.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_derivative3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/macarray_interpolator3.h>
#include <shiokaze/utility/utility.h>
#include <cmath>
#include <random>
//
SHKZ_USING_NAMESPACE
//
macsmoke3::macsmoke3 () {
	//
	m_shape = shape3(64,64,64);
	m_dx = m_shape.dx();
}
//
void macsmoke3::setup_window( std::string &name, int &width, int &height ) const {
	height = width;
}
//
void macsmoke3::load( configuration &config ) {
	//
	std::string name("plume3"); config.get_string("Name",name,"Scene file name");
	m_dylib.open_library(filesystem::resolve_libname(name));
	m_dylib.load(config);
	m_dylib.overwrite(config);
	//
	m_param.render_density = console::system("mitsuba > /dev/null 2>&1") == 0;
	config.get_bool("RenderDensity",m_param.render_density,"Whether to render density");
}
//
void macsmoke3::configure( configuration &config ) {
	//
	// Configure the set of tools
	m_dylib.configure(config);
	//
	config.get_bool("UseDustParticles",m_param.use_dust,"Whether to use dust particles instead of density field");
	if( m_param.use_dust ) {
		config.get_unsigned("DustSampleNum",m_param.r_sample,"Subsampling number for dust particles per dimension divided by 2");
	} else {
		config.get_double("MinimalActiveDensity",m_param.minimal_density,"Minimal density to trim active cells");
	}
	config.get_bool("MouseInteration",m_param.mouse_interaction, "Enable mouse interaction");
	config.get_bool("ShowGraph",m_param.show_graph,"Show graph");
	config.get_double("BuoyancyFactor",m_param.buoyancy_factor,"Buoyancy force rate");
	config.get_unsigned("SolidExtrapolationDepth",m_param.extrapolated_width,"Solid extrapolation depth");
	config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
	config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
	config.get_unsigned("ResolutionZ",m_shape[2],"Resolution towards Z axis");
	config.get_unsigned("RenderSampleCount",m_param.render_sample_count,"Sample count for rendering");
	config.get_double("VolumeScale",m_param.volume_scale,"Volume scaling for rendering");
	//
	double view_scale (1.0);
	config.get_double("ViewScale",view_scale,"View scale");
	//
	double resolution_scale (1.0);
	config.get_double("ResolutionScale",resolution_scale,"Resolution doubling scale");
	//
	m_shape *= resolution_scale;
	m_dx = view_scale * m_shape.dx();
}
//
void macsmoke3::post_initialize () {
	//
	scoped_timer timer(this);
	//
	timer.tick(); console::dump( ">>> Started initialization (%dx%dx%d)\n", m_shape[0], m_shape[1], m_shape[2] );
	//
	auto initialize_func = reinterpret_cast<void(*)(const shape3 &shape, double dx)>(m_dylib.load_symbol("initialize"));
	if( initialize_func ) initialize_func(m_shape,m_dx);
	//
	// Initialize arrays
	m_force_exist = false;
	m_velocity.initialize(m_shape);
	m_external_force.initialize(m_shape);
	//
	m_solid.initialize(m_shape.nodal());
	m_fluid.initialize(m_shape.cell(),-1.0);
	m_density.initialize(m_shape.cell(),0.0);
	//
	if( m_param.use_dust ) {
		m_accumulation.initialize(m_shape.cell(),0.0);
	}
	m_dust_particles.clear();
	//
	// Assign initial variables from script
	m_velocity.activate_all();
	m_macutility->assign_initial_variables(m_dylib,m_velocity,&m_solid,nullptr,&m_density);
	//
	// Ensure divergence free
	double max_u = m_macutility->compute_max_u(m_velocity);
	if( max_u ) {
		double CFL = m_timestepper->get_target_CFL();
		m_macproject->project(CFL*m_dx/max_u,m_velocity,m_solid,m_fluid);
	}
	//
	// Seed dust particles if requested
	if( m_param.use_dust ) {
		timer.tick(); console::dump( "Seeding dust particles..." );
		//
		shared_array3<Real> density_copy(m_density);
		density_copy->dilate();
		//
		double space = 1.0 / m_param.r_sample;
		density_copy->const_serial_actives([&]( int i, int j, int k, const auto &it ) {
			for( int ii=0; ii<m_param.r_sample; ++ii ) for( int jj=0; jj<m_param.r_sample; ++jj ) for( int kk=0; kk<m_param.r_sample; ++kk ) {
				vec3d unit_pos = 0.5*vec3d(space,space,space)+vec3d(ii*space,jj*space,kk*space);
				vec3d pos = m_dx*(unit_pos+vec3d(i,j,k));
				if( array_interpolator3::interpolate<Real>(m_solid,pos/m_dx) > 0.0 &&
					array_interpolator3::interpolate<Real>(m_density,pos/m_dx-vec3d(0.5,0.5,0.5))) {
					m_dust_particles.push_back(pos);
				}
			}
		});
		rasterize_dust_particles(m_density);
		console::dump( "Done. Seeded=%d. Took %s.\n", m_dust_particles.size(), timer.stock("seed_m_dust_particles").c_str());
	}
	//
	m_camera->set_bounding_box(vec3d().v,m_shape.box(m_dx).v);
	console::dump( "<<< Initialization finished. Took %s\n", timer.stock("initialization").c_str());
	//
	if( m_param.show_graph ) {
		m_graphplotter->clear();
		m_graph_id = m_graphplotter->create_entry("Kinetic Energy");
	}
}
//
void macsmoke3::drag( double x, double y, double z, double u, double v, double w ) {
	//
	if( m_param.mouse_interaction ) {
		double scale (1e3);
		m_macutility->add_force(vec3d(x,y,z),scale*vec3d(u,v,w),m_external_force);
		m_force_exist = true;
	}
}
//
void macsmoke3::inject_external_force( macarray3<Real> &velocity ) {
	//
	if( m_force_exist ) {
		velocity += m_external_force;
		m_external_force.clear();
		m_force_exist = false;
	}
}
//
void macsmoke3::add_source ( macarray3<Real> &velocity, array3<Real> &density, double time, double dt ) {
	//
	scoped_timer timer(this);
	//
	auto add_func = reinterpret_cast<void(*)(const vec3d &, vec3d &, double &, double, double)>(m_dylib.load_symbol("add"));
	if( add_func ) {
		timer.tick(); console::dump( "Adding sources..." );
		//
		// Velocity
		velocity.parallel_all([&](int dim, int i, int j, int k, auto &it) {
			vec3d p = m_dx*vec3i(i,j,k).face(dim);
			double dummy; vec3d u;
			add_func (p,u,dummy,time,dt);
			if( u[dim] ) it.increment(u[dim]);
		});
		//
		// Density
		auto add_density = [&]( array3<Real> &density ) {
			density.parallel_all([&](int i, int j, int k, auto &it) {
				vec3d p = m_dx*vec3i(i,j,k).cell();
				double d(0.0); vec3d dummy;
				add_func (p,dummy,d,time,dt);
				density.increment(i,j,k,d);
			});
		};
		//
		// Density
		unsigned seeded (0);
		if( m_param.use_dust ) {
			//
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<> dis(-1.0,1.0);
			//
			add_density(m_accumulation);
			//
			double scale = 1.0 / pow(m_param.r_sample,DIM3);
			bool should_re_rasterize (false);
			m_accumulation.serial_op([&]( int i, int j, int k, auto &it) {
				double d = it();
				while( d > scale ) {
					vec3d p = m_dx*vec3i(i,j,k).cell()+0.5*m_dx*vec3d(dis(gen),dis(gen),dis(gen));
					m_dust_particles.push_back(p); ++ seeded;
					should_re_rasterize = true;
					d -= scale;
				}
				it.set(d);
			});
			//
			if( should_re_rasterize ) {
				rasterize_dust_particles(density);
			}
			//
		} else {
			add_density(density);
		}
		//
		if( m_param.use_dust ) console::dump( "Done. Seeded=%d. Took %s.\n", seeded, timer.stock("add_func").c_str());
		else console::dump( "Done. Took %s.\n", timer.stock("add_func").c_str());
	}
}
//
void macsmoke3::rasterize_dust_particles( array3<Real> &rasterized_density ) {
	//
	rasterized_density.clear();
	double scale = 1.0 / pow(m_param.r_sample,DIM3);
	for( const vec3d &p : m_dust_particles ) {
		vec3i pi = p/m_dx;
		if( ! m_shape.out_of_bounds(pi)) {
			rasterized_density.increment(pi[0],pi[1],pi[2],scale);
		}
	}
}
//
void macsmoke3::add_buoyancy_force( macarray3<Real> &velocity, const array3<Real> &density, double dt ) {
	//
	velocity[1].parallel_all([&]( int i, int j, int k, auto &it, int tn ) {
		vec3d pi = vec3i(i,j,k).face(1);
		Real d = array_interpolator3::interpolate<Real>(density,(pi-vec3d(0.5,0.5,0.5)));
		it.increment(m_param.buoyancy_factor*dt*d);
	});
}
//
void macsmoke3::idle() {
	//
	scoped_timer timer(this);
	//
	// Add to graph
	add_to_graph();
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity),m_dx);
	double CFL = m_timestepper->get_current_CFL();
	unsigned step = m_timestepper->get_step_count();
	timer.tick(); console::dump( ">>> %s step started (dt=%.2e,CFL=%.2f)...\n", dt, CFL, console::nth(step).c_str());
	//
	// Advection
	if( m_param.use_dust ) advect_dust_particles(m_velocity,dt);
	else {
		m_density.dilate(std::ceil(m_timestepper->get_current_CFL()));
		m_macadvection->advect_scalar(m_density,m_velocity,m_fluid,dt,"density");
		m_density.parallel_actives([&](auto &it) {
			if( std::abs(it()) <= m_param.minimal_density ) it.set_off();
		});
	}
	//
	shared_macarray3<Real> velocity_save(m_velocity);
	m_macadvection->advect_vector(m_velocity,velocity_save(),m_fluid,dt,"velocity");
	//
	// Add buoyancy force
	add_buoyancy_force(m_velocity,m_density,dt);
	//
	// Add source
	add_source(m_velocity,m_density,m_timestepper->get_current_time(),dt);
	//
	// Add external force
	inject_external_force(m_velocity);
	//
	// Projection
	m_macproject->project(dt,m_velocity,m_solid,m_fluid);
	m_macutility->extrapolate_and_constrain_velocity(m_solid,m_velocity,m_param.extrapolated_width);
	//
	console::dump( "<<< %s step done. Took %s\n", console::nth(step).c_str(), timer.stock("simstep").c_str());
	//
	// Export density
	export_density();
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void macsmoke3::advect_dust_particles( const macarray3<Real> &velocity, double dt ) {
	//
	m_parallel.for_each( m_dust_particles.size(), [&]( size_t n, int tn ) {
		vec3d &p = m_dust_particles[n];
		vec3d u0 = macarray_interpolator3::interpolate<Real>(velocity,p/m_dx);
		vec3d u1 =  macarray_interpolator3::interpolate<Real>(velocity,(p+dt*u0)/m_dx);
		p += 0.5 * dt * (u0+u1);
	});
	//
	m_parallel.for_each( m_dust_particles.size(), [&]( size_t n, int tn ) {
		vec3d &p = m_dust_particles[n];
		double phi = array_interpolator3::interpolate<Real>(m_solid,p/m_dx);
		if( phi < 0.0 ) {
			Real derivative[DIM3];
			array_derivative3::derivative(m_solid,p/m_dx,derivative);
			p = p - phi*vec3d(derivative).normal();
		}
		for( unsigned dim : DIMS3 ) {
			if( p[dim] < 0.0 ) p[dim] = 0.0;
			if( p[dim] > m_dx*m_shape[dim] ) p[dim] = m_dx*m_shape[dim];
		}
	});
	//
	rasterize_dust_particles(m_density);
}
//
void macsmoke3::add_to_graph() {
	//
	if( m_param.show_graph ) {
		//
		// Compute total energy
		const double time = m_timestepper->get_current_time();
		const double total_energy = m_macutility->get_kinetic_energy(m_solid,m_fluid,m_velocity);
		//
		// Add to graph
		m_graphplotter->add_point(m_graph_id,time,total_energy);
	}
}
//
void macsmoke3::draw_dust_particles( graphics_engine &g ) const {
	using ge = graphics_engine;
	g.color4(1.0,1.0,1.0,1.0);
	g.begin(ge::MODE::POINTS);
	for( const vec3d &p : m_dust_particles ) {
		g.vertex3v(p.v);
	}
	g.end();
}
//
void macsmoke3::draw( graphics_engine &g ) const {
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
	//
	// Draw projection component
	m_macproject->draw(g);
	//
	// Draw concentration
	if( m_param.use_dust ) draw_dust_particles(g);
	else m_gridvisualizer->draw_density(g,m_density);
	//
	// Draw graph
	m_graphplotter->draw(g);
}
//
void macsmoke3::export_density () const {
	//
	scoped_timer timer(this);
	if( console::get_root_path().size()) {
		int frame = m_timestepper->should_export_frame();
		if( frame ) {
			timer.tick(); console::dump( "Exporting %s density...", console::nth(frame).c_str());
			do_export_density(frame);
			console::dump( "Done. Took %s\n", timer.stock("export_mesh").c_str());
			if( m_param.render_density ) {
				render_density(frame);
			}
		}
	}
}
//
void macsmoke3::do_export_density( int frame ) const {
	//
	std::string dir_path = console::get_root_path()+"/density";
	if( ! filesystem::is_exist(dir_path)) filesystem::create_directory(dir_path);
	//
	std::string path = console::format_str("%s/%d_density.vol",dir_path.c_str(),frame);
	FILE *fp = fopen(path.c_str(),"wb");
	assert(fp);
	//
	const char *vol_str = "VOL";
	const char version (3);
	const int value (1), xn (m_shape[0]), yn (m_shape[1]), zn (m_shape[2]);
	//
	const Real minX (-0.5*xn*m_dx);
	const Real minY (-0.5*yn*m_dx);
	const Real minZ (-0.5*zn*m_dx);
	const Real maxX (0.5*xn*m_dx);
	const Real maxY (0.5*yn*m_dx);
	const Real maxZ (0.5*zn*m_dx);
	//
	fwrite(vol_str,3,1,fp);
	fwrite(&version,sizeof(version),1,fp);
	fwrite(&value,sizeof(value),1,fp);
	fwrite(&xn,sizeof(xn),1,fp);
	fwrite(&yn,sizeof(yn),1,fp);
	fwrite(&zn,sizeof(zn),1,fp);
	fwrite(&value,sizeof(value),1,fp);
	fwrite(&minX,sizeof(minX),1,fp);
	fwrite(&minY,sizeof(minY),1,fp);
	fwrite(&minZ,sizeof(minZ),1,fp);
	fwrite(&maxX,sizeof(maxX),1,fp);
	fwrite(&maxY,sizeof(maxY),1,fp);
	fwrite(&maxZ,sizeof(maxZ),1,fp);
	//
	std::vector<Real> density_linearized(xn*yn*zn);
	m_density.const_parallel_all([&](int i, int j, int k, auto &it) {
		density_linearized[i+j*(xn)+k*(xn*yn)] = it();
	});
	for (size_t n=0; n<xn*yn*zn; ++n) {
		fwrite(&density_linearized[n],sizeof(value),1,fp);
	}
	//
	fclose(fp);
}
//
void macsmoke3::render_density( int frame ) const {
	//
	scoped_timer timer(this);
	global_timer::pause();
	//
	assert(console::get_root_path().size());
	//
	std::string mitsuba_path = console::get_root_path() + "/smoke_mitsuba";
	std::string copy_from_path = filesystem::find_resource_path("smoke","mitsuba");
	if( ! filesystem::is_exist(mitsuba_path)) {
		if( filesystem::is_exist(copy_from_path)) {
			console::run( "cp -r %s %s", copy_from_path.c_str(), mitsuba_path.c_str());
		} else {
			console::dump( "Could not lcoate mitsuba files (%s).\n", copy_from_path.c_str());
			exit(0);
		}
	}
	//
	std::string render_command = console::format_str("cd %s; python render.py %d %d %g %s",
				mitsuba_path.c_str(),frame,m_param.render_sample_count,m_param.volume_scale,"img");
	//
	console::dump("Running command: %s\n", render_command.c_str());
	console::system(render_command.c_str());
	//
	global_timer::resume();
}
//
extern "C" module * create_instance() {
	return new macsmoke3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//
