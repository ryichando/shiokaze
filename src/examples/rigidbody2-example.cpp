/*
**	rigidbody2-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 25, 2019.
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
#include <shiokaze/rigidbody/rigidworld2_interface.h>
#include <shiokaze/rigidbody/rigidworld2_utility.h>
//
SHKZ_USING_NAMESPACE
//
using rg2 = rigidbody2_interface;
//
class rigidbody2_example : public drawable {
private:
	//
	LONG_NAME("Rigidbody 2D Example")
	ARGUMENT_NAME("RigidbodyExample")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_double("BoxWidth",m_param.box_width,"Box width");
		config.get_unsigned("Substeps",m_param.substeps,"Substeps");
		config.get_double("TimeStep",m_param.timestep,"Timestep size");
		config.get_double("ViewScale",m_view_scale,"View scale");
	}
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		height = width;
	}
	//
	virtual void post_initialize() override {
		//
		m_world->clear();
		//
		const double vs (m_view_scale);
		const double gap (0.05);
		m_camera->set_bounding_box(vec2d().v,vec2d(vs,vs).v,true);
		//
		rg2::attribution2 wall_attribute = { "wall", 0.0, 1.0, 0.5, true, nullptr };
		rigidworld2_utility::add_container_wall(m_world.get(),wall_attribute,vec2d(gap,gap),vec2d(vs-gap,vs-gap));
		//
		rg2::attribution2 square_attribute = { "square", 1.0, 1.0, 0.5, true, nullptr };
		rg2::velocity2 square_velocity = { {0.0,0.0}, 0.0 };
		//
		const double width (m_param.box_width);
		//
		std::vector<vec2d> square_shape = {{-width/2.0,-width/2.0},{width/2.0,-width/2.0},{width/2.0,width/2.0},{-width/2.0,width/2.0}};
		std::vector<rg2::polyshape2> square_polyshaps = { {square_shape,rg2::POLYGON} };
		//
		m_world->add_rigidbody(square_polyshaps,square_attribute,{{vs*0.5,vs*0.75},0.2},square_velocity);
		m_world->add_rigidbody(square_polyshaps,square_attribute,{{vs*0.6,vs*0.4},0.2},square_velocity);
		//
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
	virtual void draw( graphics_engine &g ) const override {
		//
		// Draw a message
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec2d(0.025,0.025).v, console::format_str("Engine name = %s", m_world->engine_name().c_str()));
		m_world->draw(g);
	};
	//
	struct Parameters {
		double box_width {0.1};
		unsigned substeps {1};
		double timestep {0.01};
	};
	//
	Parameters m_param;
	//
	rigidworld2_driver m_world{this,"box2d_rigidworld2"};
	double m_view_scale {1.0};
};
//
extern "C" module * create_instance() {
	return new rigidbody2_example;
}
//
extern "C" const char *license() {
	return "MIT";
}
