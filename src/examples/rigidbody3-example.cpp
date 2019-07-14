/*
**	rigidbody3-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 26, 2019.
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
#include <shiokaze/ui/drawable.h>
#include <shiokaze/core/console.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/rigidbody/rigidworld3_interface.h>
#include <shiokaze/polygon/polygon3_interface.h>
#include <shiokaze/polygon/polygon3_utility.h>
#include <shiokaze/core/filesystem.h>
#include <shiokaze/rigidbody/hacd_io.h>
//
SHKZ_USING_NAMESPACE
//
using rg3 = rigidbody3_interface;
//
class rigidbody3_example : public drawable {
private:
	//
	LONG_NAME("Rigidbody 3D Example")
	ARGUMENT_NAME("RigidbodyExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_double("BoxWidth",m_param.box_width,"Box width");
		config.get_bool("UseTriangulatedBox",m_param.use_triangulated_box,"Use triangulated box");
		config.get_bool("UseMeshFile",m_param.use_mesh_file,"Use mesh file");
		config.get_string("MeshFilePath",m_param.mesh_file_path,"Mesh file path");
		config.get_unsigned("Substeps",m_param.substeps,"Substeps");
		config.get_double("TimeStep",m_param.timestep,"Timestep size");
		//
		double view_scale (1.0);
		config.get_double("ViewScale",view_scale,"View scale");
		set_view_scale(view_scale);
		//
		if( m_param.use_mesh_file ) {
			if( ! filesystem::is_exist(m_param.mesh_file_path.c_str()) ) {
				m_param.mesh_file_path = filesystem::find_resource_path("objects",m_param.mesh_file_path.c_str());
				if( ! filesystem::is_exist(m_param.mesh_file_path.c_str())) {
					console::dump( "Error: MeshFilePath variable is not valid.\n" );
					exit(0);
				}
				if( ! filesystem::is_exist((m_param.mesh_file_path+".hacd").c_str())) {
					console::dump( "Error: HACD file is not available.\n" );
					exit(0);
				}
			}
		}
	}
	//
	virtual void post_initialize() override {
		//
		polygon_storage.clear();
		m_world->clear();
		//
		const double vs = get_view_scale();
		const double hw = vs * 0.5;
		//
		rg3::attribution3 wall_attribute = { "wall", 0.0, 0.5, 0.75, false, nullptr };
		//
		rg3::velocity3 wall_velocity = { vec3d(), vec3d() };
		rg3::position3 wall_position = { {hw,hw,hw}, vec3d(), 0.0 };
		//
		rg3::polyshape3 bottom;
		bottom.vertices = {vec3d(-hw,-hw,-hw),vec3d(hw,-hw,-hw),vec3d(hw,-hw,hw),vec3d(-hw,-hw,hw)};
		bottom.faces = {{0,1,2},{0,2,3}};
		bottom.type = rg3::MESH;
		//
		rg3::polyshape3 top;
		top.vertices = {vec3d(-hw,hw,-hw),vec3d(hw,hw,-hw),vec3d(hw,hw,hw),vec3d(-hw,hw,hw)};
		top.faces = {{0,1,2},{0,2,3}};
		top.type = rg3::MESH;
		//
		rg3::polyshape3 right;
		right.vertices = {vec3d(hw,-hw,-hw),vec3d(hw,-hw,hw),vec3d(hw,hw,hw),vec3d(hw,hw,-hw)};
		right.faces = {{0,1,2},{0,2,3}};
		right.type = rg3::MESH;
		//
		rg3::polyshape3 left;
		left.vertices = {vec3d(-hw,-hw,-hw),vec3d(-hw,-hw,hw),vec3d(-hw,hw,hw),vec3d(-hw,hw,-hw)};
		left.faces = {{0,1,2},{0,2,3}};
		left.type = rg3::MESH;
		//
		rg3::polyshape3 front;
		front.vertices = {vec3d(-hw,-hw,-hw),vec3d(hw,-hw,-hw),vec3d(hw,hw,-hw),vec3d(-hw,hw,-hw)};
		front.faces = {{0,1,2},{0,2,3}};
		front.type = rg3::MESH;
		//
		rg3::polyshape3 back;
		back.vertices = {vec3d(-hw,-hw,hw),vec3d(hw,-hw,hw),vec3d(hw,hw,hw),vec3d(-hw,hw,hw)};
		back.faces = {{0,1,2},{0,2,3}};
		back.type = rg3::MESH;
		//
		m_world->add_rigidbody({bottom,top,right,left,front,back},wall_attribute,wall_position,wall_velocity);
		//
		rg3::attribution3 dynamic_attribute = { "convex", 1.0, 0.5, 0.25, true, nullptr };
		rg3::velocity3 dynamic_velocity = { vec3d(), vec3d() };
		//
		if( m_param.use_mesh_file ) {
			//
			const double w (3.0 * m_param.box_width);
			//
			polygon_storage.push_back(polygon_info());
			dynamic_attribute.user_pointer = &polygon_storage.back();
			dynamic_attribute.drawable = false;
			//
			std::vector<vec3d> &full_vertices = polygon_storage.back().vertices;
			std::vector<std::vector<size_t> > &full_faces = polygon_storage.back().faces;
			//
			polygon_loader->load_mesh(m_param.mesh_file_path);
			polygon_loader->get_mesh(full_vertices,full_faces);
			//
			vec3d center = polygon3_utility::get_center_of_gravity(full_vertices,full_faces);
			vec3d corner0, corner1;
			polygon3_utility::compute_AABB(full_vertices,corner0,corner1);
			//
			double scale (1.0);
			for( int dim : DIMS3 ) {
				scale = std::min(scale,1.0/(corner1-corner0)[dim]);
			}
			//
			for( auto &v : full_vertices ) {
				v = w * scale * (v - center);
			}
			//
			std::vector<convex_object> objects = read_hacd(m_param.mesh_file_path+".hacd");
			std::vector<rg3::polyshape3> polyshapes;
			//
			for( auto &obj : objects ) {
				//
				rg3::polyshape3 mesh_poly;
				//
				for( auto &v : obj.vertices ) {
					v = w * scale * (v - center);
				}
				//
				mesh_poly.vertices = obj.vertices;
				mesh_poly.faces = obj.faces;
				mesh_poly.type = rg3::CONVEX;
				//
				polyshapes.push_back(mesh_poly);
			}
			//
			m_world->add_rigidbody(polyshapes,dynamic_attribute,{vs*vec3d(0.5,0.25,0.5),vec3d(),0.0},dynamic_velocity);
			m_world->add_rigidbody(polyshapes,dynamic_attribute,{vs*vec3d(0.5,0.75,0.5),vec3d(),0.0},dynamic_velocity);
			//
		} else {
			//
			rg3::polyshape3 box;
			box.type = rg3::CONVEX;
			const double w (m_param.box_width);
			//
			if( m_param.use_triangulated_box ) {
				box.vertices = { vec3d(-w,-w,-w),vec3d(w,-w,-w),vec3d(w,-w,w),vec3d(-w,-w,w),
								 vec3d(-w,w,-w),vec3d(w,w,-w),vec3d(w,w,w),vec3d(-w,w,w) };
				//
				box.faces = {{0,1,2},{0,2,3},
							 {6,5,4},{7,6,4},
							 {3,7,4},{3,4,0},
							 {1,5,6},{6,2,1},
							 {0,4,5},{5,1,0},
							 {2,6,7},{7,3,2}};
			} else {
				box.vertices = {vec3d(-w,-w,w),vec3d(w,-w,w),vec3d(w,-w,-w),vec3d(-w,-w,-w),vec3d(-w,w,w),vec3d(w,w,w),vec3d(w,w,-w),vec3d(-w,w,-w)};
				box.faces = {{0,3,2,1},{4,5,6,7},{0,1,5,4},{1,2,6,5},{2,3,7,6},{0,4,7,3}};
			}
			//
			m_world->add_rigidbody({box},dynamic_attribute,{vs*vec3d(0.5,0.75,0.5),vec3d(),0.0},dynamic_velocity);
			m_world->add_rigidbody({box},dynamic_attribute,{vs*vec3d(0.6,0.4,0.5),vec3d(1.0,0.0,0.0),0.5},dynamic_velocity);
		}
	};
	//
	virtual void idle() override {
		//
		for( unsigned i=0; i<m_param.substeps; ++i ) {
			const double dt = m_param.timestep / m_param.substeps;
			m_world->advance(dt);
		}
	};
	//
	virtual void draw( graphics_engine &g, int width, int height ) const override {
		//
		using ge = graphics_engine;
		//
		g.color4(1.0,1.0,1.0,0.5);
		graphics_utility::draw_wired_box(g,get_view_scale());
		//
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec3d().v, console::format_str("Engine name = %s", m_world->engine_name().c_str()));
		//
		m_world->draw(g);
		//
		std::vector<const rigidbody3_interface *> rigibody_list = m_world->get_rigidbody_list();
		for( const auto &rigidody : rigibody_list ) {
			//
			const polygon_info *polygon = static_cast<const polygon_info *>(rigidody->get_attribution().user_pointer);
			//
			if( polygon ) {
				//
				const std::vector<vec3d> &vertices = polygon->vertices;
				const std::vector<std::vector<size_t> > &faces = polygon->faces;
				//
				float m[16];
				rigidody->getOpenGLMatrix(m);
				//
				g.color4(0.5,0.3,0.2,0.75);
				for( unsigned i=0; i<faces.size(); i++ ) {
					g.begin( ge::MODE::LINE_LOOP);
					for( unsigned j=0; j<faces[i].size(); j++ ) {
						//
						const vec3d &p = vertices[faces[i][j]];
						double before_p [] = { p[0], p[1], p[2], 1.0 };
						double transformed_p [4];
						//
						for( int row=0; row<4; ++row ) {
							double value (0.0);
							for( int k=0; k<4; ++k ) {
								value += before_p[k] * m[row+4*k];
							}
							transformed_p[row] = value;
						}
						//
						g.vertex3v(transformed_p);
					}
					g.end();
				}
			}
		}
	};
	//
	rigidworld3_driver m_world{this,"bullet3_rigidworld3"};
	polygon3_driver polygon_loader{this,"polygon3"};
	//
	struct polygon_info {
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
	};
	std::vector<polygon_info> polygon_storage;
	//
	struct Parameters {
		double box_width {0.1};
		bool use_triangulated_box {false};
		bool use_mesh_file {false};
		unsigned substeps {1};
		double timestep {0.01};
		std::string mesh_file_path {"bunny_watertight_low.ply"};
	};
	//
	Parameters m_param;
};
//
extern "C" module * create_instance() {
	return new rigidbody3_example;
}
//
extern "C" const char *license() {
	return "MIT";
}
