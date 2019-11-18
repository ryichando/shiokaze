/*
**	box2d_rigidbody2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017.
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
#include <Box2D/Box2D.h>
#include <shiokaze/rigidbody/rigidworld2_interface.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
using rg2 = rigidbody2_interface;
//
class box2d_rigidbody2 : public rigidbody2_interface {
public:
	//
	virtual std::vector<polyshape2> get_shapes() const override {
		return m_polyshapes;
	}
	virtual position2 get_position() const override {
		return m_position;
	}
	virtual attribution2 get_attribution() const override {
		return m_attribute;
	}
	virtual velocity2 get_velocity() const override {
		return m_velocity;
	}
	//
	b2Body *m_body;	// Box2D body pointer
	std::vector<polyshape2> m_polyshapes;
	position2 m_position;
	attribution2 m_attribute;
	velocity2 m_velocity;
};
//
class box2d_rigidworld2 : public rigidworld2_interface {
protected:
	//
	LONG_NAME("Box2D Rigidbody Engine")
	AUTHOR_NAME("Erin Catto")
	//
	virtual ~box2d_rigidworld2() {
		clear();
		if( m_world ) {
			delete m_world;
			m_world = nullptr;
		}
	}
	//
	virtual std::string engine_name() const override {
		return "Box2D";
	}
	//
	virtual void clear() override {
		if( m_world ) {
			for( auto &rigidbody : m_rigidbodies ) {
				m_world->DestroyBody(rigidbody.m_body);
			}
		}
		m_rigidbodies.clear();
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_vec2d("Gravity",m_param.gravity.v,"Gravity vector");
		config.get_unsigned("VelocityIterations",m_param.velocity_iterations,"Velocity iteration count");
		config.get_unsigned("PositionIterations",m_param.position_iterations,"Position iteration count");
		config.get_double("Scale",m_param.scale,"Scaling of domain");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		//
		clear();
		m_world = new b2World(b2Vec2(m_param.gravity[0]*m_param.scale,m_param.gravity[1]*m_param.scale));
	}
	//
	virtual rigidbody2_interface * add_rigidbody(
		const std::vector<rg2::polyshape2> &polyshapes,
		const rg2::attribution2 &attribute,
		const rg2::position2& position,
		const rg2::velocity2 &velocity
	) override {
		//
		assert(m_world);
		//
		box2d_rigidbody2 rigidbody;
		rigidbody.m_polyshapes = polyshapes;
		rigidbody.m_attribute = attribute;
		rigidbody.m_position = position;
		// ---------
		b2BodyDef body_def;
		if( attribute.density == 0.0 ) {
			body_def.type = b2_staticBody;
		} else {
			body_def.type = b2_dynamicBody;
		}
		body_def.angle = position.angle;
		body_def.position.Set(m_param.scale*position.center[0],m_param.scale*position.center[1]);
		// ---------
		b2Body* body = m_world->CreateBody(&body_def);
		rigidbody.m_body = body;
		// ---------
		for( unsigned i=0; i<polyshapes.size(); ++i ) {
			//
			b2FixtureDef fixture_def;
			fixture_def.density = attribute.density;
			fixture_def.friction = attribute.friction;
			fixture_def.restitution = attribute.restitution;
			//
			if( polyshapes[i].type == rg2::POLYGON ) {
				//
				std::vector<b2Vec2> points(polyshapes[i].polygon.size());
				for( int n=0; n<polyshapes[i].polygon.size(); ++n ) {
					points[n] = b2Vec2(m_param.scale*polyshapes[i].polygon[n][0],m_param.scale*polyshapes[i].polygon[n][1]);
				}
				//
				b2PolygonShape polygon;
				polygon.Set(&points[0],points.size());
				//
				fixture_def.shape = &polygon;
				body->CreateFixture(&fixture_def);
				//
			} else if( polyshapes[i].type == rg2::EDGE ) {
				//
				assert( polyshapes[i].polygon.size() == 2 );
				const auto &polygon = polyshapes[i].polygon;
				//
				b2EdgeShape edge;
				edge.Set(b2Vec2(m_param.scale*polygon[0][0],m_param.scale*polygon[0][1]),b2Vec2(m_param.scale*polygon[1][0],m_param.scale*polygon[1][1]));
				//
				fixture_def.shape = &edge;
				body->CreateFixture(&fixture_def);
			}
		}
		m_rigidbodies.push_back(rigidbody);
		return &m_rigidbodies.back();
	}
	//
	virtual void advance( double dt ) override {
		//
		// Instruct the world to perform a single step of simulation.
		m_world->Step(dt,m_param.velocity_iterations,m_param.position_iterations);
		//
		// Now update the internal position and angle of the body.
		for( auto &rigidbody : m_rigidbodies ) {
			// ----------
			b2Body &body = *rigidbody.m_body;
			// ----------
			b2Vec2 position = body.GetPosition();
			b2Vec2 velocity = body.GetLinearVelocity();
			Real angle = body.GetAngle();
			Real angularVelocity = body.GetAngularVelocity();
			// -----------
			rigidbody.m_position.center = vec2d(position.x,position.y) / m_param.scale;
			rigidbody.m_position.angle = angle;
			rigidbody.m_velocity.center_velocity = vec2d(velocity.x,velocity.y) / m_param.scale;
			rigidbody.m_velocity.angular_velocity = angularVelocity;
		}
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		for( const auto &rigidbody : m_rigidbodies ) {
			//
			if( rigidbody.m_attribute.drawable ) {
				//
				Real m[9];
				rigidbody.getOpenGLMatrix(m);
				//
				for( unsigned i=0; i<rigidbody.m_polyshapes.size(); ++i ) {
					//
					const auto &polyshape = rigidbody.m_polyshapes[i];
					const std::vector<vec2d> &polygon = polyshape.polygon;
					//
					auto plot_polylines = [&]() {
						for( unsigned n=0; n<polygon.size(); ++n ) {
							//
							const vec2d &p = polygon[n];
							double before_p [] = { p[0], p[1], 1.0 };
							double transformed_p [3];
							//
							for( int row=0; row<3; ++row ) {
								double value (0.0);
								for( int k=0; k<3; ++k ) {
									value += before_p[k] * m[row+3*k];
								}
								transformed_p[row] = value;
							}
							//
							double w = transformed_p[2];
							transformed_p[0] /= w;
							transformed_p[1] /= w;
							//
							g.vertex2v(transformed_p);
						}
					};
					if( polyshape.type == rg2::EDGE ) {
						g.color4(1.0,1.0,1.0,1.0);
						g.begin(graphics_engine::MODE::LINES);
						plot_polylines();
						g.end();
					} else if( polyshape.type == rg2::POLYGON ) {
						g.color4(0.5,0.3,0.2,0.6);
						g.begin(graphics_engine::MODE::TRIANGLE_FAN);
						plot_polylines();
						g.end();
						g.color4(1.0,1.0,1.0,1.0);
						g.begin(graphics_engine::MODE::LINE_LOOP);
						plot_polylines();
						g.end();
					}
				}
			}
		}
	}
	//
	virtual std::vector<rigidbody2_interface *> get_rigidbody_list() override {
		std::vector<rigidbody2_interface *> result;
		for( auto &r : m_rigidbodies ) result.push_back(&r);
		return result;
	}
	//
	struct Parameters {
		unsigned velocity_iterations {6};
		unsigned position_iterations {2};
		vec2d gravity {0.0,-9.8};
		double scale {100.0};
	};
	//
	Parameters m_param;
	//
	std::vector<box2d_rigidbody2> m_rigidbodies;
	b2World *m_world; // Box2D world pointer
	//
};
//
extern "C" module * create_instance() {
	return new box2d_rigidworld2;
}
//
extern "C" const char *license() {
	return "zlib";
}
//