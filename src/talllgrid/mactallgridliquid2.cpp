/*
**	mactallgridliquid2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Dec 27, 2018.
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
#include "mactallgridliquid2.h"
#include <shiokaze/core/filesystem.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/macarray_extrapolator2.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/core/dylibloader.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
mactallgridliquid2::mactallgridliquid2 () {
	//
	m_param.gravity = vec2d(0.0,-9.8);
	m_param.volume_correction = true;
	m_param.volume_change_tol_ratio = 0.03;
	//
	m_shape = shape2{64,32};
	m_dx = m_shape.dx();
}
//
void mactallgridliquid2::load( configuration &config ) {
	//
	std::string name("waterdrop2"); config.get_string("Name",name,"Scene file name");
	m_dylib.open_library(filesystem::find_libpath(name));
	m_dylib.load(config);
	m_dylib.overwrite(config);
}
//
void mactallgridliquid2::configure( configuration &config ) {
	//
	m_dylib.configure(config);
	//
	config.get_vec2d("Gravity",m_param.gravity.v,"Gravity vector");
	config.get_bool("VolumeCorrection", m_param.volume_correction,"Should perform volume correction");
	config.get_double("VolumeChangeTolRatio",m_param.volume_change_tol_ratio,"Volume change tolerance ratio");
	//
	config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
	config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
	//
	double scale (1.0);
	config.get_double("ResolutionScale",scale,"Resolution doubling scale");
	//
	m_shape *= scale;
	m_dx = m_shape.dx();
}
//
void mactallgridliquid2::setup_window( std::string &name, int &width, int &height ) const {
	double ratio = m_shape[1] / (double)m_shape[0];
	height = width * ratio;
}
//
void mactallgridliquid2::post_initialize () {
	//
	auto initialize_func = reinterpret_cast<void(*)(const shape2 &shape, double dx)>(m_dylib.load_symbol("initialize"));
	if( initialize_func ) {
		initialize_func(m_shape,m_dx);
	}
	//
	// Initialize arrays
	m_force_exist = false;
	m_velocity.initialize(m_shape);
	m_external_force.initialize(m_shape);
	m_solid.initialize(m_shape.nodal());
	m_fluid.initialize(m_shape.cell());
	m_pressure.initialize(m_shape);
	//
	// Assign initial variables from script
	m_macutility->assign_initial_variables(m_dylib,m_velocity,&m_solid,&m_fluid);
	m_velocity.set_touch_only_actives(true);
	//
	// Assign to surface tracker
	m_macsurfacetracker->assign(m_solid,m_fluid);
	m_initial_volume = m_gridutility->get_area(m_solid,m_fluid);
	//
	// Build upsampler
	upsampler.build_upsampler(m_fluid,m_dx);
	//
	shared_macarray2<double> velocity_actives(m_velocity.type());
	for( int dim : DIMS2 ) {
		velocity_actives()[dim].activate_inside_as(m_fluid);
		velocity_actives()[dim].activate_inside_as(m_fluid,vec2i(dim==0,dim==1));
	}
	m_velocity.copy_active_as(velocity_actives());
	//
	double max_u = m_macutility->compute_max_u(m_velocity);
	if( max_u ) {
		//
		// Project to make sure that the velocity field is divergence free at the beggining
		double CFL = m_timestepper->get_target_CFL();
		project(CFL*m_dx/max_u);
	}
	//
}
//
void mactallgridliquid2::project(double dt) {
	//
	shared_macarray2<double> areas(m_velocity.shape());
	shared_macarray2<double> rhos(m_velocity.shape());
	//
	// Compute fractions
	m_macutility->compute_area_fraction(m_solid,areas());
	m_macutility->compute_fluid_fraction(m_fluid,rhos());
	//
	// Label cell indices
	size_t index (0);
	shared_array2<size_t> index_map(m_shape);
	m_fluid.const_serial_inside([&]( int i, int j, const auto &it) {
		index_map().set(i,j,index++);
	});
	//
	// Assemble the linear system for the full resoutions
	auto Lhs = m_factory->allocate_matrix(index,index);
	auto rhs = m_factory->allocate_vector(index);
	//
	index_map->const_parallel_actives([&]( int i, int j, const auto &it, int tn ) {
		//
		size_t n_index = it();
		rhs->set(n_index,0.0);
		//
		vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
		vec2i face[] = {vec2i(i+1,j),vec2i(i,j),vec2i(i,j+1),vec2i(i,j)};
		int direction[] = {0,0,1,1};
		int sgn[] = {1,-1,1,-1};
		//
		double diagonal (0.0);
		for( int nq=0; nq<4; nq++ ) {
			int dim = direction[nq];
			if( ! m_shape.out_of_bounds(query[nq]) ) {
				double area = areas()[dim](face[nq]);
				if( area ) {
					double rho = rhos()[dim](face[nq]);
					if( rho ) {
						double value = dt*area/(m_dx*m_dx*rho);
						if( m_fluid(query[nq]) < 0.0 ) {
							size_t m_index = index_map()(query[nq]);
							Lhs->add_to_element(n_index,m_index,-value);
						}
						diagonal += value;
					}
				}
				rhs->add(n_index,-sgn[nq]*area*m_velocity[dim](face[nq])/m_dx);
			}
		}
		Lhs->add_to_element(n_index,n_index,diagonal);
	});
	//
	// Build upsamling matrix
	auto U = m_factory->allocate_matrix(index,upsampler.get_index_size());
	auto func = upsampler.get_upsampler();
	index_map->const_parallel_actives([&]( int i, int j, const auto &it, int tn ) {
		//
		size_t row = it();
		std::vector<size_t> indices;
		std::vector<double> coefficients;
		std::vector<vec2d> positions;
		if( func(vec2i(i,j),indices,coefficients,positions)) {
			for( unsigned n=0; n<indices.size(); ++n ) {
				U->add_to_element(row,indices[n],coefficients[n]);
			}
		}
	});
	auto Ut = U->transpose();
	auto LhsU = Lhs->multiply(U.get());
	m_UtLhsU = Ut->multiply(LhsU.get());
	auto Utrhs = Ut->multiply(rhs.get());
	//
	// Solve the linear system
	auto result = m_factory->allocate_vector(U->columns());
	m_solver->solve(m_UtLhsU.get(),Utrhs.get(),result.get());
	auto result_upsampled = U->multiply(result.get());
	//
	// Re-arrange to the array
	m_pressure.clear();
	index_map->const_serial_actives([&](int i, int j, const auto& it) {
		m_pressure.set(i,j,result_upsampled->at(it()));
	});
	//
	// Update the full velocity
	m_velocity.parallel_actives([&](int dim, int i, int j, auto &it, int tn ) {
		double rho = rhos()[dim](i,j);
		vec2i pi(i,j);
		if( areas()[dim](i,j) && rho ) {
			if( pi[dim] == 0 || pi[dim] == m_velocity.shape()[dim] ) it.set(0.0);
			else {
				it.subtract(dt * (
					+ m_pressure(i,j)
					- m_pressure(i-(dim==0),j-(dim==1))
				) / (rho*m_dx));
			}
		} else {
			if( pi[dim] == 0 && m_fluid(pi) < 0.0 && it() < 0.0 ) it.set(0.0);
			else if( pi[dim] == m_velocity.shape()[dim] && m_fluid(pi-vec2i(dim==0,dim==1)) < 0.0 && it() > 0.0 ) it.set(0.0);
			else it.set_off();
		}
	});
}
//
void mactallgridliquid2::drag( int width, int height, double x, double y, double u, double v ) {
	//
	m_macutility->add_force(vec2d(x,y),vec2d(u,v),m_external_force);
	m_force_exist = true;
}
//
void mactallgridliquid2::inject_external_force( macarray2<double> &m_velocity, double dt ) {
	//
	if( m_force_exist ) {
		m_velocity.set_touch_only_actives(true);
		m_velocity += m_external_force;
		m_external_force.clear();
		m_force_exist = false;
	}
	// Add gravity force
	m_velocity += dt*m_param.gravity;
}
//
void mactallgridliquid2::extend_both() {
	//
	char bandwidth_half = m_fluid.get_levelset_halfwidth();
	auto current_CFL = std::ceil(m_timestepper->get_current_CFL());
	macarray_extrapolator2::extrapolate<double>(m_velocity,bandwidth_half+current_CFL);
	m_macutility->constrain_velocity(m_solid,m_velocity);
	m_fluid.dilate(bandwidth_half+current_CFL);
}
//
void mactallgridliquid2::idle() {
	//
	// Compute the timestep size
	double dt = m_timestepper->advance(m_macutility->compute_max_u(m_velocity)/m_dx);
	//
	// Extend both the velocity field and the level set
	extend_both();
	//
	// Advect surface
	m_macsurfacetracker->assign(m_solid,m_fluid);
	m_macsurfacetracker->advect(m_velocity,dt);
	m_macsurfacetracker->get(m_fluid);
	//
	// Build upsampler
	upsampler.build_upsampler(m_fluid,m_dx);
	//
	// Advect velocity
	shared_macarray2<double> velocity_save(m_velocity);
	m_macadvection->advect_vector(m_velocity,velocity_save(),dt);
	//
	// Add external force
	inject_external_force(m_velocity,dt);
	//
	// Project
	project(dt);
	//
	// Report stats
	m_macstats->dump_stats(m_solid,m_fluid,m_velocity,m_timestepper.get());
}
//
void mactallgridliquid2::cursor( int width, int height, double x, double y ) {
	m_cursor = vec2d(x,y);
}
//
void mactallgridliquid2::draw( graphics_engine &g, int width, int height ) const {
	//
	// Draw grid lines
	m_gridvisualizer->draw_grid(g);
	//
	// Draw surface tracker
	m_macsurfacetracker->draw(g);
	//
	// Draw levelset
	m_gridvisualizer->draw_solid(g,m_solid);
	//
	// Draw velocity
	m_macvisualizer->draw_velocity(g,m_velocity);
	//
	// Visualize pressure
	m_gridvisualizer->visualize_cell_scalar(g,m_pressure);
	//
	// Draw upsampler
	upsampler.draw(g);
	//
	// Test upsample
	auto func = upsampler.get_upsampler();
	std::vector<size_t> indices;
	std::vector<double> coefficients;
	std::vector<vec2d> positions;
	if( func(m_shape.find_cell(m_cursor/m_dx),indices,coefficients,positions)) {
		g.color4(1.0,1.0,1.0,1.0);
		for( unsigned n=0; n<indices.size(); ++n ) {
			g.draw_string(positions[n].v,console::format_str("Index = %d, Value = %.2f",indices[n],coefficients[n]).c_str());
		}
	}
}
//
extern "C" module * create_instance() {
	return new mactallgridliquid2;
}
//
extern "C" const char *license() {
	return "MIT";
}