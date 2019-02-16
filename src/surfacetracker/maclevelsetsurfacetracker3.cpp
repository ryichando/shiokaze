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
#include <shiokaze/surfacetracker/macsurfacetracker3_interface.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/advection/macadvection3_interface.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/visualizer/gridvisualizer3_interface.h>
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <shiokaze/meshexporter/meshexporter3_interface.h>
#include <shiokaze/redistancer/redistancer3_interface.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/scoped_timer.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <cstdio>
//
SHKZ_USING_NAMESPACE
//
class maclevelsetsurfacetracker3 : public macsurfacetracker3_interface {
private:
	//
	LONG_NAME("MAC Levelset Surface Tracker 3D")
	//
	virtual void assign( const array3<double> &solid, const array3<double> &fluid ) override {
		m_solid.copy(solid);
		m_fluid.copy(fluid);
	}
	//
	virtual void advect( const macarray3<double> &u, double dt ) override {
		//
		scoped_timer timer(this);
		shared_array3<double> fluid_save(m_fluid);
		m_macadvection->advect_scalar(m_fluid,u,fluid_save(),dt,"levelset");
		//
		// Re-initialize
		timer.tick(); console::dump( "Re-distancing fluid levelsets..." );
		m_redistancer->redistance(m_fluid,m_param.levelset_half_bandwidth);
		console::dump( "Done. Took %s\n", timer.stock("redistance_levelset").c_str());
		//
		// Extrapolation towards solid
		timer.tick(); console::dump( "Extrapolating fluid levelsets towards solid walls...");
		m_gridutility->extrapolate_levelset(m_solid,m_fluid);
		console::dump( "Done. Took %s\n", timer.stock("exprapolate_levelset").c_str());
	}
	//
	virtual void get( array3<double> &fluid ) override { fluid.copy(m_fluid); }
	virtual void draw( graphics_engine &g ) const override {
		m_gridvisualizer->draw_fluid(g,m_solid,m_fluid);
	}
	//
	virtual void export_fluid_mesh( std::string path_to_directory, unsigned frame,
							  std::function<vec3d(const vec3d &)> vertex_color_func=nullptr,
							  std::function<vec2d(const vec3d &)> uv_coordinate_func=nullptr ) const override {
		//
		shared_array3<double> fluid_dilated(m_fluid);
		fluid_dilated->dilate();
		std::string path_wo_suffix = console::format_str("%s/%u_mesh", path_to_directory.c_str(), frame );
		export_fluid_mesh(path_wo_suffix,fluid_dilated(),true,vertex_color_func,uv_coordinate_func);
		//
		shared_array3<double> fluid_closed(m_shape);
		if( m_param.enclose_solid ) {
			m_gridutility->combine_levelset(m_solid,m_fluid,fluid_closed());
			fluid_closed->dilate();
		} else {
			fluid_closed->copy(fluid_dilated());
		}
		const double eps (0.01*m_dx);
		//
		// Front and back
		double value;
		for( int i=0; i<m_shape[0]; ++i ) for( int j=0; j<m_shape[1]; ++j ) {
			value = fluid_closed()(i,j,0);
			if( value < 0.0 ) fluid_closed().set(i,j,0,eps);
			value = fluid_closed()(i,j,m_shape[2]-1);
			if( value < 0.0 ) fluid_closed().set(i,j,m_shape[2]-1,eps);
		}
		//
		// Left and right
		for( int j=0; j<m_shape[1]; ++j ) for( int k=0; k<m_shape[2]; ++k ) {
			value = fluid_closed()(0,j,k);
			if( value < 0.0 ) fluid_closed().set(0,j,k,eps);
			value = fluid_closed()(m_shape[0]-1,j,k);
			if( value < 0.0 ) fluid_closed().set(m_shape[0]-1,j,k,eps);
		}
		//
		// Floor and ceiling
		for( int k=0; k<m_shape[2]; ++k ) for( int i=0; i<m_shape[0]; ++i ) {
			value = fluid_closed()(i,0,k);
			if( value < 0.0 ) fluid_closed().set(i,0,k,eps);
			value = fluid_closed()(i,m_shape[1]-1,k);
			if( value < 0.0 ) fluid_closed().set(i,m_shape[1]-1,k,eps);
		}
		//
		export_fluid_mesh(path_wo_suffix+"_enclosed",fluid_closed(), ! m_param.enclose_solid, vertex_color_func,uv_coordinate_func);
	}
	//
	virtual void export_fluid_mesh( std::string path_wo_suffix, const array3<double> &fluid, bool delete_solid_embedded,
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
					if( array_interpolator3::interpolate(m_solid,vertices[original_faces[n][i]]/m_dx) > 0.0 ) {
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
		mesh_exporter->export_ply(console::format_str("%s.ply",path_wo_suffix.c_str()));
	}
	//
	virtual void load( configuration &config ) override {
		m_macadvection.set_name("Levelset Advection 3D","LevelsetAdvection");
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("EncloseSolid",m_param.enclose_solid,"Should remove faces in solid on mesh export");
		config.get_unsigned("LevelsetHalfwidth",m_param.levelset_half_bandwidth,"Level set half bandwidth");
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
		//
		m_fluid.initialize(shape.cell());
		m_solid.initialize(shape.nodal());
	}
	//
	array3<double> m_solid{this}, m_fluid{this};
	macadvection3_driver m_macadvection{this,"macadvection3"};
	redistancer3_driver m_redistancer{this,"pderedistancer3"};
	cellmesher3_driver m_mesher{this,"marchingcubes"};
	meshexporter3_driver m_mesh_exporter{this,"meshexporter3"};
	gridutility3_driver m_gridutility{this,"gridutility3"};
	macutility3_driver m_macutility{this,"macutility3"};
	gridvisualizer3_driver m_gridvisualizer{this,"gridvisualizer3"};
	parallel_driver m_parallel{this};
	//
	shape3 m_shape;
	double m_dx;
	//
	struct Parameters {
		bool enclose_solid {false};
		unsigned levelset_half_bandwidth {2};
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