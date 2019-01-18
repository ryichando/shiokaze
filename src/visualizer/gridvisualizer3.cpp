/*
**	gridvisualizer3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 28, 2017. 
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
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/visualizer/gridvisualizer3_interface.h>
#include <shiokaze/array/shared_array3.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
//
class gridvisualizer3 : public gridvisualizer3_interface {
private:
	//
	virtual void draw_active( graphics_engine &g, const array3<double> &q ) const override {
		if( m_param.draw_active ) {
			g.color4(1.0,0.0,0.0,0.25);
			g.point_size(3.0);
			g.begin(graphics_engine::MODE::POINTS);
			q.const_serial_actives([&](int i, int j, int k, const auto &it) {
				g.vertex3v((m_dx*vec3i(i,j,k).cell()).v);
			});
			g.end();
			g.point_size(1.0);
		}
	}
	virtual void draw_inside( graphics_engine &g, const array3<double> &q ) const override {
		if( m_param.draw_active ) {
			g.color4(1.0,0.0,0.0,0.25);
			g.point_size(3.0);
			g.begin(graphics_engine::MODE::POINTS);
			q.const_serial_inside([&](int i, int j, int k, const auto &it) {
				g.vertex3v((m_dx*vec3i(i,j,k).cell()).v);
			});
			g.end();
			g.point_size(1.0);
		}
	}
	virtual void draw_grid( graphics_engine &g ) const override {
		// TO BE IMPLEMENTED
	}
	virtual void draw_density( graphics_engine &g, const array3<double> &density ) const override {
		if( m_param.draw_density ) {
			g.point_size(3.0);
			g.begin(graphics_engine::MODE::POINTS);
			density.const_serial_actives([&](int i, int j, int k, const auto &it) {
				g.color4(1.0,1.0,1.0,it());
				g.vertex3v((m_dx*vec3i(i,j,k).cell()).v);
			});
			g.end();
			g.point_size(1.0);
		}
	}
	virtual void draw_velocity( graphics_engine &g, const array3<vec3d> &velocity ) const override {
		if( m_param.draw_velocity ) {
			//
			g.color4(1.0,1.0,1.0,0.5);
			velocity.const_serial_actives([&](int i, int j, int k, const auto &it) {
				vec3d p0 = m_dx*vec3i(i,j,k).cell();
				vec3d p1 = p0+m_dx*it();
				g.vertex3v(p0.v);
				g.vertex3v(p1.v);
			});
		}
	}
	virtual void draw_levelset( graphics_engine &g, const array3<double> &levelset ) const override {
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		mesher->generate_mesh(levelset,vertices,faces);
		//
		for( unsigned i=0; i<faces.size(); i++ ) {
			g.begin(graphics_engine::MODE::LINE_LOOP);
			for( unsigned j=0; j<faces[i].size(); j++ ) g.vertex3v(vertices[faces[i][j]].v);
			g.end();
		}
	}
	virtual void draw_solid( graphics_engine &g, const array3<double> &solid ) const override {
		if( m_param.draw_solid ) {
			g.color4(1.0,0.8,0.5,0.3);
			draw_levelset(g,solid);
		}
	}
	virtual void draw_fluid( graphics_engine &g, const array3<double> &solid, const array3<double> &fluid ) const override {
		if( m_param.draw_fluid ) {
			//
			shared_array3<double> combined(fluid.type());
			m_gridutility->combine_levelset(solid,fluid,*combined.get());
			//
			g.color4(1.0,1.0,1.0,0.3);
			draw_levelset(g,*combined.get());
		}
	}
	//
	virtual void visualize_cell_scalar( graphics_engine &g, const array3<double> &q ) const override {
		// TO BE IMPLEMENTED...
	}
	virtual void visualize_nodal_scalar( graphics_engine &g, const array3<double> &q ) const override {
		// TO BE IMPLEMENTED...
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	virtual void configure( configuration &config ) override {
		config.get_bool("DrawActive",m_param.draw_active,"Should draw active");
		config.get_bool("DrawInside",m_param.draw_inside,"Should draw inside");
		config.get_bool("DrawGrid",m_param.draw_grid,"Should draw grid");
		config.get_bool("DrawSolid",m_param.draw_solid,"Should draw solid");
		config.get_bool("DrawDensity",m_param.draw_density,"Should draw density");
		config.get_bool("DrawVelocity",m_param.draw_velocity,"Should draw velocity");
	}
	//
	shape3 m_shape;
	double m_dx;
	//
	struct Parameters {
		bool draw_active {true};
		bool draw_inside {true};
		bool draw_grid {true};
		bool draw_solid {true};
		bool draw_fluid {true};
		bool draw_density {true};
		bool draw_velocity {true};
	};
	//
	Parameters m_param;
	gridutility3_driver m_gridutility{this,"gridutility3"};
	cellmesher3_driver mesher{this,"marchingcubes"};
	//
};
//
extern "C" module * create_instance() {
	return new gridvisualizer3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//