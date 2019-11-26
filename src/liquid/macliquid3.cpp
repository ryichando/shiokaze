/*
**	macliquid3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 18, 2017.
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
#include "macliquid3.h"
#include <shiokaze/core/filesystem.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/shared_bitarray3.h>
#include <shiokaze/array/array_upsampler3.h>
#include <shiokaze/array/macarray_extrapolator3.h>
#include <shiokaze/array/macarray_interpolator3.h>
#include <shiokaze/array/array_gradient3.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/core/filesystem.h>
#include <cmath>
#include <numeric>
//
SHKZ_USING_NAMESPACE
//
macliquid3::macliquid3() {
	//
	m_shape = shape3(64,32,64);
}
//
void macliquid3::load( configuration &config ) {
	//
	std::string name("waterdrop3"); config.get_string("Name",name,"Scene file name");
	m_dylib.open_library(filesystem::resolve_libname(name));
	m_dylib.load(config);
	m_dylib.overwrite(config);
}
//
void macliquid3::configure( configuration &config ) {
	//
	if( console::get_root_path().size()) {
		m_export_path = console::get_root_path() + "/mesh";
		if( ! filesystem::is_exist(m_export_path)) filesystem::create_directory(m_export_path);
	}
	//
	m_dylib.configure(config);
	//
	m_param.render_mesh = console::system("mitsuba > /dev/null 2>&1") == 0;
	//
	config.get_vec3d("Gravity",m_param.gravity.v,"Gravity vector");
	config.get_bool("MouseInteration",m_param.mouse_interaction, "Enable mouse interaction");
	config.get_bool("VolumeCorrection",m_param.volume_correction,"Should perform volume correction");
	config.get_double("VolumeChangeTolRatio",m_param.volume_change_tol_ratio,"Volume change tolerance ratio");
	config.get_double("SurfaceTension",m_param.surftens_k,"Surface tenstion coefficient");
	config.get_bool("ShowGraph",m_param.show_graph,"Show graph");
	config.get_string("MeshPath",m_export_path, "Path to the directory to export meshes");
	config.get_bool("RenderMesh",m_param.render_mesh,"Whether to render mesh files");
	config.get_bool("RenderTransparent",m_param.render_transparent,"Whether to render transparent view");
	config.get_unsigned("RenderSampleCount",m_param.render_sample_count,"Sample count for rendering");
	config.get_unsigned("RenderTransparentSampleCount",m_param.render_transparent_sample_count,"Sample count for transparent rendering");
	config.get_vec3d("TargetPos",m_param.target.v,"Camera target position");
	config.get_vec3d("OriginPos",m_param.origin.v,"Camera origin position");
	config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
	config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
	config.get_unsigned("ResolutionZ",m_shape[2],"Resolution towards Z axis");
	//
	double view_scale (1.0);
	config.get_double("ViewScale",view_scale,"View scale");
	//
	double resolution_scale (1.0);
	config.get_double("ResolutionScale",resolution_scale,"Resolution doubling scale");
	//
	m_shape *= resolution_scale;
	m_dx = view_scale * m_shape.dx();
	//
	m_doubled_shape = 2 * m_shape;
	m_half_dx = 0.5 * m_dx;
	//
	m_highres_mesher->set_environment("shape",&m_doubled_shape);
	m_highres_mesher->set_environment("dx",&m_half_dx);
}
//
void macliquid3::post_initialize() {
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( ">>> Started initialization (%dx%dx%d)\n", m_shape[0], m_shape[1], m_shape[2] );
	//
	auto initialize_func = reinterpret_cast<void(*)(const shape3 &m_shape, double m_dx)>(m_dylib.load_symbol("initialize"));
	if( initialize_func ) {
		initialize_func(m_shape,m_dx);
	}
	//
	// Initialize arrays
	m_prev_frame = 1;
	m_force_exist = false;
	m_velocity.initialize(m_shape);
	m_external_force.initialize(m_shape);
	m_solid.initialize(m_shape.nodal());
	m_fluid.initialize(m_shape.cell());
	//
	// Assign initial variables from script
	m_macutility->assign_initial_variables(m_dylib,m_velocity,&m_solid,&m_fluid);
	//
	// Compute the initial volume
	timer.tick(); console::dump( "Computing the initial volume..." );
	m_initial_volume = m_gridutility->get_volume(m_solid,m_fluid);
	//
	// Get Injection function
	m_check_inject_func = reinterpret_cast<bool(*)(double, double, double, unsigned)>(m_dylib.load_symbol("check_inject"));
	m_inject_func = reinterpret_cast<bool(*)(const vec3d &, double, double, double, unsigned, double &, vec3d &)>(m_dylib.load_symbol("inject"));
	m_post_inject_func = reinterpret_cast<double(*)(double, double, double, unsigned, double&)>(m_dylib.load_symbol("post_inject"));
	//
	console::dump( "Done. Volume = %.3f. Took %s.\n", m_initial_volume, timer.stock("initialize_compute_volume").c_str());
	//
	double max_u = m_macutility->compute_max_u(m_velocity);
	if( max_u ) {
		//
		// Projection
		double CFL = m_timestepper->get_target_CFL();
		m_macproject->project(CFL*m_dx/max_u,m_velocity,m_solid,m_fluid);
	}
	//
	m_camera->set_bounding_box(vec3d().v,m_shape.box(m_dx).v);
	//
	if( m_param.show_graph ) {
		m_graphplotter->clear();
		if( m_param.gravity.norm2() ) m_graph_lists[0] = m_graphplotter->create_entry("Gravitational Energy");
		m_graph_lists[1] = m_graphplotter->create_entry("Kinetic Energy");
		if( m_param.surftens_k ) m_graph_lists[2] = m_graphplotter->create_entry("Surface Area Energy");
		m_graph_lists[3] = m_graphplotter->create_entry("Total Energy");
	}
	//
	console::dump( "<<< Initialization finished. Took %s\n", timer.stock("initialization").c_str());
}
//
void macliquid3::setup_window( std::string &name, int &width, int &height ) const {
	height = width;
}
//
void macliquid3::drag( double x, double y, double z, double u, double v, double w ) {
	//
	if( m_param.mouse_interaction ) {
		double scale (1e3);
		m_macutility->add_force(vec3d(x,y,z),scale*vec3d(u,v,w),m_external_force);
		m_force_exist = true;
	}
}
//
void macliquid3::inject_external_force( macarray3<Real> &velocity, double dt ) {
	//
	scoped_timer timer(this);
	timer.tick(); console::dump( "Adding external forces..." );
	//
	if( m_force_exist ) {
		velocity += m_external_force;
		m_external_force.clear();
		m_force_exist = false;
	}
	// Add gravity force
	m_velocity += dt*m_param.gravity;
	console::dump( "Done. Took %s\n", timer.stock("add_force").c_str());
}
//
void macliquid3::inject_external_fluid( array3<Real> &fluid, macarray3<Real> &velocity, double dt ) {
	//
	scoped_timer timer(this);
	unsigned step = m_timestepper->get_step_count();
	double time = m_timestepper->get_current_time();
	//
	if( m_check_inject_func && m_check_inject_func(m_dx,dt,time,step)) {
		timer.tick(); console::dump( ">>> Liquid injection started...\n" );
		size_t total_injected (0);
		if( m_inject_func ) {
			total_injected = do_inject_external_fluid(fluid,velocity,dt,time,step);
		}
		if( m_post_inject_func ) {
			timer.tick(); console::dump( "Computing volume change..." );
			double volume_change = (m_dx*m_dx*m_dx) * total_injected;
			m_post_inject_func(m_dx,dt,time,step,volume_change);
			if( volume_change ) {
				m_initial_volume += volume_change;
			}
			console::dump( "Done. Change=%e. Took %s\n", volume_change, timer.stock("compute_volume_change").c_str());
			console::write("injection_volume_change",volume_change);
		}
		console::dump( "<<< Done. Took %s\n", timer.stock("total_liquid_injection").c_str());
	}
}
//
size_t macliquid3::do_inject_external_fluid( array3<Real> &fluid, macarray3<Real> &velocity, double dt, double time, unsigned step ) {
	//
	scoped_timer timer(this);
	auto interp_vel = [&]( const vec3d &p ) {
		return macarray_interpolator3::interpolate(velocity,vec3d(),m_dx,p);
	};
	//
	timer.tick(); console::dump( "Injecting liquid..." );
	std::vector<size_t> inject_count(fluid.get_thread_num(),0);
	std::vector<std::vector<vec3i> > injected_positions(fluid.get_thread_num());
	double current_CFL = m_timestepper->get_current_CFL();
	//
	size_t total_injected (0);
	fluid.parallel_all([&]( int i, int j, int k, auto &it, int tid ) {
		//
		vec3d p = m_dx*vec3i(i,j,k).cell();
		double value (it()); vec3d u (interp_vel(p));
		double old_value (value);
		if( m_inject_func(p,m_dx,dt,time,step,value,u)) {
			if( value < 0.0 ) {
				injected_positions[tid].push_back(vec3i(i,j,k));
				if( old_value >= 0.0 ) inject_count[tid] ++;
			}
			if( std::abs(value) < fluid.get_background_value() ||
				(value < fluid.get_background_value() && it.active())) {
				it.set(std::min(value,old_value));
				if( old_value >= 0.0 && value < 0.0 ) {
					inject_count[tid] ++;
				}
			}
		}
	});
	fluid.flood_fill();
	total_injected = std::accumulate(inject_count.begin(),inject_count.end(),0);
	//
	shared_bitarray3 eval_cells(fluid.shape());
	for( const auto &e : injected_positions ) for( const auto &pi : e ) {
		eval_cells->set(pi);
	}
	console::write("injected_count",total_injected);
	console::dump( "Done. Count=%u. Took %s\n", total_injected, timer.stock("inject_fluid").c_str());
	//
	timer.tick(); console::dump( "Assigning velocity of injected liquid..." );
	eval_cells->dilate(1);
	eval_cells->const_serial_actives([&]( int i, int j, int k ) {
		double original_fluid (fluid(i,j,k));
		double value (original_fluid); vec3d u;
		if( m_inject_func(m_dx*vec3i(i,j,k).cell(),m_dx,dt,time,step,value,u)) {
			for( int dim : DIMS3 ) {
				vec3d p0 = m_dx*vec3i(i,j,k).face(dim);
				vec3d p1 = m_dx*vec3i(i+dim==0,j+dim==1,k+dim==2).face(dim);
				value = original_fluid; u = interp_vel(p0);
				m_inject_func(p0,m_dx,dt,time,step,value,u);
				velocity[dim].set(i,j,k,u[dim]);
				value = original_fluid; u = interp_vel(p1);
				m_inject_func(p1,m_dx,dt,time,step,value,u);
				velocity[dim].set(i+dim==0,j+dim==1,k+dim==2,u[dim]);
			}
		}
	});
	console::dump( "Done. Took %s\n", timer.stock("assign_injected_velocity").c_str());
	//
	return total_injected;
}
//
void macliquid3::set_volume_correction( macproject3_interface *m_macproject ) {
	//
	// Compute volume
	scoped_timer timer(this);
	timer.tick(); console::dump( "Computing volume..." );
	double volume = m_gridutility->get_volume(m_solid,m_fluid);
	console::dump( "Done. Volume = %.3f (Volume change: %.2f%%). Took %s\n", volume, 100.0*volume/m_initial_volume, timer.stock("compute_volume").c_str());
	console::write("volume", volume);
	console::write("volume_change", volume/m_initial_volume);
	if( volume/m_initial_volume < 0.01 ) {
		console::dump( "Volume is nearly zero. Quitting...\n");
		exit(0);
	}
	//
	// Set volume correction if requested
	if( m_param.volume_correction ) {
		double change_ratio = std::abs(1.0-volume/m_initial_volume);
		if( change_ratio > m_param.volume_change_tol_ratio ) {
			double target_volume;
			if( volume > m_initial_volume ) {
				target_volume = (1.0+m_param.volume_change_tol_ratio) * m_initial_volume;
			} else {
				target_volume = (1.0-m_param.volume_change_tol_ratio) * m_initial_volume;
			}
			console::dump( "Report: volume correction is turned on. (target=%.3f, original=%.3f)\n", target_volume, m_initial_volume );
			m_macproject->set_target_volume(volume,target_volume);
		} else {
			console::dump( "Report: volume correction is not turned on (change ratio does not exceed %g but is only %.4f).\n", m_param.volume_change_tol_ratio, change_ratio);
		}
	}
}
//
void macliquid3::extend_both( int w ) {
	//
	scoped_timer timer(this);
	//
	timer.tick(); console::dump( "Extending velocity field...");
	unsigned width = w+m_timestepper->get_current_CFL();
	macarray_extrapolator3::extrapolate<Real>(m_velocity,width);
	m_macutility->constrain_velocity(m_solid,m_velocity);
	m_fluid.dilate(width);
	console::dump( "Done. Count=%d. Took %s\n", width, timer.stock("extend_velocity").c_str());
}
//
void macliquid3::idle() {
	//
	scoped_timer timer(this);
	//
	unsigned step = m_timestepper->get_step_count()+1;
	timer.tick(); console::dump( ">>> %s step started...\n", console::nth(step).c_str());
	//
	// Add to graph
	add_to_graph();
	//
	// Compute the timestep size
	timer.tick(); console::dump( "Computing time step...");
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity),m_dx);
	double CFL = m_timestepper->get_current_CFL();
	console::dump( "Done. dt=%.2e,CFL=%.2f. Took %s\n", dt, CFL, timer.stock("compute_timestep").c_str());
	//
	// Extend both the velocity field and the level set
	extend_both();
	//
	// Advect surface
	m_macsurfacetracker->advect(m_fluid,m_solid,m_velocity,dt);
	//
	// Velocity advection
	shared_macarray3<Real> velocity_save(m_velocity);
	m_macadvection->advect_vector(m_velocity,velocity_save(),m_fluid,dt,"velocity");
	//
	// Add external force
	inject_external_force(m_velocity,dt);
	//
	// Inject liquid
	inject_external_fluid(m_fluid,m_velocity,dt);
	//
	// Set volume correction
	set_volume_correction(m_macproject.get());
	//
	// Projection
	m_macproject->project(dt,m_velocity,m_solid,m_fluid,m_param.surftens_k);
	//
	console::dump( "<<< %s step done. Took %s\n", console::nth(step).c_str(), timer.stock("simstep").c_str());
	//
	// Export mesh
	export_mesh();
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void macliquid3::export_mesh() const {
	//
	scoped_timer timer(this);
	if( m_export_path.size()) {
		int frame = m_timestepper->should_export_frame();
		if( frame ) {
			for( unsigned n=m_prev_frame; n<=frame; ++n ) {
				timer.tick(); console::dump( ">>> Exporting %s mesh (time=%g secs)\n", console::nth(n).c_str(),m_timestepper->get_current_time());
				do_export_mesh(n);
				console::dump( "<<< Done. Took %s\n", timer.stock("export_mesh").c_str());
				if( m_param.render_mesh ) {
					render_mesh(n);
				}
			}
			const_cast<unsigned&>(m_prev_frame) = frame;
		}
	}
}
//
void macliquid3::do_export_mesh( unsigned frame ) const {
	//
	scoped_timer timer(this);
	//
	assert(m_export_path.size());
	//
	auto vertex_color_func = [&](const vec3d &p) { return p; };
	auto uv_coordinate_func = [&](const vec3d &p) { return vec2d(p[0],0.0); };
	//
	timer.tick(); console::dump( "Generating mesh..." );
	m_macsurfacetracker->export_fluid_mesh(m_export_path,frame,m_solid,m_fluid,vertex_color_func,uv_coordinate_func);
	console::dump( "Done. Took %s\n", timer.stock("generate_mesh").c_str());
	do_export_solid_mesh();
}
//
void macliquid3::render_mesh( unsigned frame ) const {
	//
	scoped_timer timer(this);
	global_timer::pause();
	//
	assert(console::get_root_path().size());
	//
	std::string mitsuba_path = console::get_root_path() + "/liquid_mitsuba";
	std::string copy_from_path = filesystem::find_resource_path("liquid","mitsuba");
	if( ! filesystem::is_exist(mitsuba_path)) {
		if( filesystem::is_exist(copy_from_path)) {
			console::run( "cp -r %s %s", copy_from_path.c_str(), mitsuba_path.c_str());
		} else {
			console::dump( "Could not lcoate mitsuba files (%s).\n", copy_from_path.c_str());
			exit(0);
		}
	}
	//
	std::string render_command = console::format_str("cd %s; /usr/bin/python render.py %d %d %g %g %g %g %g %g %s",
				mitsuba_path.c_str(),
				frame,
				m_param.render_sample_count,
				m_param.target[0], m_param.target[1], m_param.target[2],
				m_param.origin[0], m_param.origin[1], m_param.origin[2],"mesh");
	//
	console::dump("Running command: %s\n", render_command.c_str());
	console::system(render_command.c_str());
	//
	if( m_param.render_transparent ) {
		//
		std::string render_command = console::format_str("cd %s; /usr/bin/python render.py %d %d %g %g %g %g %g %g %s",
				mitsuba_path.c_str(),
				frame,
				m_param.render_transparent_sample_count,
				m_param.target[0], m_param.target[1], m_param.target[2],
				m_param.origin[0], m_param.origin[1], m_param.origin[2],"transparent");
		//
		console::dump("Running command: %s\n", render_command.c_str());
		console::system(render_command.c_str());
	}
	//
	global_timer::resume();
}
//
void macliquid3::do_export_solid_mesh() const {
	//
	scoped_timer timer(this);
	auto uv_coordinate_func = [&](const vec3d &p) {
		return vec2d(p[0],p[2]);
	};
	//
	std::string static_solids_directory_path = console::format_str("%s/static_solids",m_export_path.c_str());
	std::string path_wo_suffix = console::format_str("%s/levelset_solid",static_solids_directory_path.c_str());
	//
	if( ! filesystem::is_exist(static_solids_directory_path)) {
		//
		filesystem::create_directory(static_solids_directory_path);
		meshexporter3_driver &m_mesh_exporter = const_cast<meshexporter3_driver &>(this->m_mesh_exporter);
		//
		if( array_utility3::levelset_exist(m_solid)) {
			//
			timer.tick(); console::dump( "Generating solid mesh..." );
			shared_array3<Real> solid_to_visualize(m_doubled_shape.nodal());
			if( ! m_gridutility->assign_visualizable_solid(m_dylib,m_half_dx,solid_to_visualize())) {
				array_upsampler3::upsample_to_double_nodal<Real>(m_solid,m_dx,solid_to_visualize());
			}
			//
			std::vector<vec3d> vertices;
			std::vector<std::vector<size_t> > faces;
			m_highres_mesher->generate_mesh(solid_to_visualize(),vertices,faces);
			//
			m_mesh_exporter->set_mesh(vertices,faces);
			std::vector<vec2d> uv_coordinates (vertices.size());
			for( unsigned n=0; n<vertices.size(); ++n ) {
				uv_coordinates[n] = uv_coordinate_func(vertices[n]);
			}
			//
			m_mesh_exporter->set_texture_coordinates(uv_coordinates);
			m_mesh_exporter->export_ply(console::format_str("%s.ply",path_wo_suffix.c_str()));
			m_mesh_exporter->export_mitsuba(console::format_str("%s.serialized",path_wo_suffix.c_str()));
			//
			console::dump("Done. Took %s.\n", timer.stock("export_solid_mesh").c_str());
			//
		} else {
			//
			// Export dummy solid file
			std::vector<vec3d> vertices;
			std::vector<std::vector<size_t> > faces(1);
			vertices.push_back(vec3d(1e3,1e3,1e3));
			vertices.push_back(vec3d(1e3+1.0,1e3,1e3));
			vertices.push_back(vec3d(1e3,1e3,1e3+1.0));
			faces[0].push_back(0);
			faces[0].push_back(1);
			faces[0].push_back(2);
			m_mesh_exporter->set_mesh(vertices,faces);
			m_mesh_exporter->export_ply(console::format_str("%s.ply",path_wo_suffix.c_str()));
			m_mesh_exporter->export_mitsuba(console::format_str("%s.serialized",path_wo_suffix.c_str()));
		}
	}
}
//
void macliquid3::add_to_graph() {
	//
	if( m_param.show_graph ) {
		//
		// Compute total energy
		const double time = m_timestepper->get_current_time();
		const auto energy_list = m_macutility->get_all_kinds_of_energy(m_solid,m_fluid,m_velocity,m_param.gravity,m_param.surftens_k);
		const double total_energy = std::get<0>(energy_list)+std::get<1>(energy_list)+std::get<2>(energy_list);
		//
		// Add to graph
		if( m_param.gravity.norm2() ) m_graphplotter->add_point(m_graph_lists[0],time,std::get<0>(energy_list));
		m_graphplotter->add_point(m_graph_lists[1],time,std::get<1>(energy_list));
		if( m_param.surftens_k ) m_graphplotter->add_point(m_graph_lists[2],time,std::get<2>(energy_list));
		m_graphplotter->add_point(m_graph_lists[3],time,total_energy);
	}
}
//
void macliquid3::draw( graphics_engine &g ) const {
	//
	// Draw projection component
	m_macproject->draw(g);
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
	//
	// Visualize solid
	shared_array3<Real> solid_to_visualize(m_solid.shape());
	if( ! m_gridutility->assign_visualizable_solid(m_dylib,m_dx,solid_to_visualize())) solid_to_visualize->copy(m_solid);
	if( array_utility3::levelset_exist(solid_to_visualize())) m_gridvisualizer->draw_solid(g,solid_to_visualize());
	//
	// Visualize levelset
	m_gridvisualizer->draw_fluid(g,m_solid,m_fluid);
	//
	// Draw graph
	m_graphplotter->draw(g);
}
//
extern "C" module * create_instance() {
	return new macliquid3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//