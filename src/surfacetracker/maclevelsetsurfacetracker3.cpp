/*
**	maclevelsetsurfacetracker3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 28, 2017. 
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
#include <shiokaze/surfacetracker/maclevelsetsurfacetracker3_interface.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/advection/macadvection3_interface.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <shiokaze/meshexporter/meshexporter3_interface.h>
#include <shiokaze/redistancer/redistancer3_interface.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/scoped_timer.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <cstdio>
//
SHKZ_USING_NAMESPACE
//
class maclevelsetsurfacetracker3 : public maclevelsetsurfacetracker3_interface {
protected:
	//
	LONG_NAME("MAC Levelset Surface Tracker 3D")
	MODULE_NAME("maclevelsetsurfacetracker3")
	//
	virtual void advect( array3<float> &fluid, const array3<float> &solid, const macarray3<float> &u, double dt ) override {
		//
		scoped_timer timer(this);
		if( dt ) {
			shared_array3<float> fluid_save(fluid);
			m_macadvection->advect_scalar(fluid,u,fluid_save(),dt,"levelset");
		}
		//
		// Re-initialize
		timer.tick(); console::dump( "Re-distancing fluid levelsets..." );
		m_redistancer->redistance(fluid,m_param.levelset_half_bandwidth_count);
		console::dump( "Done. Took %s\n", timer.stock("redistance_levelset").c_str());
		//
		// Extrapolation towards solid
		timer.tick(); console::dump( "Extrapolating fluid levelsets towards solid walls...");
		m_gridutility->extrapolate_levelset(solid,fluid);
		console::dump( "Done. Took %s\n", timer.stock("exprapolate_levelset").c_str());
	}
	//
	virtual void export_fluid_mesh( std::string path_to_directory, unsigned frame,
							  const array3<float> &solid, const array3<float> &fluid,
							  std::function<vec3d(const vec3d &)> vertex_color_func=nullptr,
							  std::function<vec2d(const vec3d &)> uv_coordinate_func=nullptr ) const override {
		//
		std::string path_wo_suffix = console::format_str("%s/%u_mesh", path_to_directory.c_str(), frame );
		export_fluid_mesh(path_wo_suffix,solid,fluid,true,vertex_color_func,uv_coordinate_func);
		//
		shared_array3<float> fluid_closed(m_shape);
		if( m_param.enclose_solid ) {
			m_gridutility->combine_levelset(solid,fluid,fluid_closed());
		} else {
			fluid_closed->copy(fluid);
		}
		const double eps (0.01*m_dx);
		//
		// Front and back
		double value;
		for( int i=0; i<m_shape[0]; ++i ) for( int j=0; j<m_shape[1]; ++j ) {
			value = fluid_closed()(i,j,0);
			if( value < 0.0 ) {
				fluid_closed().set(i,j,0,eps);
				fluid_closed().set(i,j,1,fluid_closed()(i,j,1));
			}
			value = fluid_closed()(i,j,m_shape[2]-1);
			if( value < 0.0 ) {
				fluid_closed().set(i,j,m_shape[2]-1,eps);
				fluid_closed().set(i,j,m_shape[2]-2,fluid_closed()(i,j,m_shape[2]-2));
			}
		}
		//
		// Left and right
		for( int j=0; j<m_shape[1]; ++j ) for( int k=0; k<m_shape[2]; ++k ) {
			value = fluid_closed()(0,j,k);
			if( value < 0.0 ) {
				fluid_closed().set(0,j,k,eps);
				fluid_closed().set(1,j,k,fluid_closed()(1,j,k));
			}
			value = fluid_closed()(m_shape[0]-1,j,k);
			if( value < 0.0 ) {
				fluid_closed().set(m_shape[0]-1,j,k,eps);
				fluid_closed().set(m_shape[0]-2,j,k,fluid_closed()(m_shape[0]-2,j,k));
			}
		}
		//
		// Floor and ceiling
		for( int k=0; k<m_shape[2]; ++k ) for( int i=0; i<m_shape[0]; ++i ) {
			value = fluid_closed()(i,0,k);
			if( value < 0.0 ) {
				fluid_closed().set(i,0,k,eps);
				fluid_closed().set(i,1,k,fluid_closed()(i,1,k));
			}
			value = fluid_closed()(i,m_shape[1]-1,k);
			if( value < 0.0 ) {
				fluid_closed().set(i,m_shape[1]-1,k,eps);
				fluid_closed().set(i,m_shape[1]-1,k,fluid_closed()(i,m_shape[1]-2,k));
			}
		}
		//
		export_fluid_mesh(path_wo_suffix+"_enclosed",solid,fluid_closed(), ! m_param.enclose_solid, vertex_color_func,uv_coordinate_func);
	}
	//
	void export_fluid_mesh( std::string path_wo_suffix, 
							const array3<float> &solid, const array3<float> &fluid,
							bool delete_solid_embedded,
							std::function<vec3d(const vec3d &)> vertex_color_func=nullptr,
							std::function<vec2d(const vec3d &)> uv_coordinate_func=nullptr ) const {
		//
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > original_faces, faces;
		//
		m_mesher->generate_mesh(fluid,vertices,original_faces);
		if( delete_solid_embedded ) {
			for( size_t n=0; n<original_faces.size(); ++n ) {
				bool has_outside (false);
				for( unsigned i=0; i<original_faces[n].size(); ++i ) {
					if( array_interpolator3::interpolate(solid,vertices[original_faces[n][i]]/m_dx) > 0.0 ) {
						has_outside = true;
						break;
					}
				}
				if( has_outside ) {
					faces.push_back(original_faces[n]);
				}
			}
		} else {
			faces = original_faces;
		}
		//
		meshexporter3_driver &mesh_exporter = const_cast<meshexporter3_driver &>(m_mesh_exporter);
		mesh_exporter->set_mesh(vertices,faces);
		if( vertex_color_func ) {
			std::vector<vec3d> vertex_colors (vertices.size());
			m_parallel.for_each(vertices.size(),[&](size_t n) {
				vertex_colors[n] = vertex_color_func(vertices[n]);
			});
			mesh_exporter->set_vertex_colors(vertex_colors);
		}
		if( uv_coordinate_func ) {
			std::vector<vec2d> uv_coordinates (vertices.size());
			m_parallel.for_each(vertices.size(),[&](size_t n) {
				uv_coordinates[n] = uv_coordinate_func(vertices[n]);
			});
			mesh_exporter->set_texture_coordinates(uv_coordinates);
		}
		//
		mesh_exporter->export_mitsuba(console::format_str("%s.serialized",path_wo_suffix.c_str()));
		mesh_exporter->export_ply(console::format_str("%s.ply",path_wo_suffix.c_str()));
	}
	//
	virtual void load( configuration &config ) override {
		m_macadvection.set_name("Levelset Advection 3D","LevelsetAdvection");
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("EncloseSolid",m_param.enclose_solid,"Should remove faces in solid on mesh export");
		config.get_unsigned("LevelsetHalfWidth",m_param.levelset_half_bandwidth_count,"Level set half bandwidth");
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	macadvection3_driver m_macadvection{this,"macadvection3"};
	redistancer3_driver m_redistancer{this,"pderedistancer3"};
	cellmesher3_driver m_mesher{this,"marchingcubes"};
	meshexporter3_driver m_mesh_exporter{this,"meshexporter3"};
	gridutility3_driver m_gridutility{this,"gridutility3"};
	macutility3_driver m_macutility{this,"macutility3"};
	parallel_driver m_parallel{this};
	//
	shape3 m_shape;
	double m_dx;
	//
	struct Parameters {
		bool enclose_solid {false};
		unsigned levelset_half_bandwidth_count {3};
	};
	Parameters m_param;
};
//
extern "C" module * create_instance() {
	return new maclevelsetsurfacetracker3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//