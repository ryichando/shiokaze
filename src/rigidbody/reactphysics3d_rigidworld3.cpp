/*
**	reactphysics3d_rigidbody3.cpp
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
#include <shiokaze/rigidbody/rigidworld3_interface.h>
#include "reactphysics3d.h"
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
using rg3 = rigidbody3_interface;
//
static rp3d::Quaternion get_quaternion( const vec3d &axis, double angle ) {
	double d = axis.len();
	if( d ) {
		double s = sin(0.5*angle)/d;
		return rp3d::Quaternion(s*axis[0],s*axis[1],s*axis[2],cos(0.5*angle));
	} else {
		return rp3d::Quaternion();
	}
}
//
class reactphysics3d_rigidbody3 : public rigidbody3_interface {
public:
	//
	reactphysics3d_rigidbody3() = default;
	reactphysics3d_rigidbody3( double scale ) : m_scale(scale) {}
	//
	virtual std::vector<polyshape3> get_shapes() const override {
		return m_polyshapes;
	}
	virtual position3 get_position() const override {
		return m_position;
	}
	virtual attribution3 get_attribution() const override {
		return m_attribute;
	}
	virtual velocity3 get_velocity() const override {
		return m_velocity;
	}
	virtual void getOpenGLMatrix( float m[16] ) const override {
		rp3d::Transform transform = m_body->getTransform();
		transform.setPosition( transform.getPosition() / m_scale );
		transform.getOpenGLMatrix(m);
	}
	//
	std::vector<polyshape3> m_polyshapes;
	position3 m_position;
	attribution3 m_attribute;
	velocity3 m_velocity;
	//
	rp3d::RigidBody* m_body {nullptr};
	double m_scale;
	//
	struct mesh_data {
		std::vector<int> triangle_array;
		std::vector<float> vertices_array;
		std::vector<rp3d::PolygonVertexArray::PolygonFace> polyfaces;
		rp3d::PolygonVertexArray *polygon_vertex_array {nullptr};
		rp3d::PolyhedronMesh *polyhedron_mesh {nullptr};
		rp3d::TriangleMesh *triangle_mesh {nullptr};
		rp3d::CollisionShape *collision_shape {nullptr};
	};
	std::vector<mesh_data *> m_mesh_data_array;
};
//
class reactphysics3d_rigidworld3 : public rigidworld3_interface {
protected:
	//
	LONG_NAME("Reactphysics 3D Rigidbody Engine")
	MODULE_NAME("reactphysics3d_rigidworld3")
	AUTHOR_NAME("Daniel Chappuis")
	//
	virtual ~reactphysics3d_rigidworld3() {
		//
		clear();
		//
		if( m_world ) delete m_world;
	}
	//
	virtual std::string engine_name() const override {
		return "ReactPhysics3D";
	}
	//
	virtual void clear() override {
		//
		for( auto &rigidbody : m_rigidbodies ) {
			//
			if( rigidbody.m_body ) m_world->destroyRigidBody(rigidbody.m_body);
			//
			for( auto &mesh : rigidbody.m_mesh_data_array ) {
				if( mesh->polygon_vertex_array ) delete mesh->polygon_vertex_array;
				if( mesh->polyhedron_mesh ) delete mesh->polyhedron_mesh;
				if( mesh->triangle_mesh ) delete mesh->triangle_mesh;
				if( mesh->collision_shape ) delete mesh->collision_shape;
				delete mesh;
			}
		}
		m_rigidbodies.clear();
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_vec3d("Gravity",m_param.gravity.v,"Gravity vector");
		config.get_double("Scale",m_param.scale,"Scaling of domain");
		config.get_unsigned("VelocityIterations",m_param.velocity_iterations,"Velocity iteration count");
		config.get_unsigned("PositionIterations",m_param.position_iterations,"Position iteration count");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		//
		clear();
		if( ! m_world ) {
			//
			const double scale (m_param.scale);
			rp3d::Vector3 gravity(m_param.gravity[0]*scale,m_param.gravity[1]*scale,m_param.gravity[2]*scale);
			m_world = new rp3d::DynamicsWorld(gravity,m_settings);
			m_world->setNbIterationsVelocitySolver(m_param.velocity_iterations);
			m_world->setNbIterationsPositionSolver(m_param.position_iterations);
		}
	}
	//
	virtual rigidbody3_interface * add_rigidbody(
		const std::vector<rg3::polyshape3> &polyshapes,
		const rg3::attribution3 &attribute,
		const rg3::position3& position,
		const rg3::velocity3 &velocity
	) override {
		//
		const double &scale = m_param.scale;
		//
		reactphysics3d_rigidbody3 rigidbody(scale);
		rigidbody.m_polyshapes = polyshapes;
		rigidbody.m_attribute = attribute;
		rigidbody.m_position = position;
		if( ! position.angle ) rigidbody.m_position.axis = vec3d(1.0,0.0,0.0);
		//
		rp3d::Vector3 center(position.center[0]*scale,position.center[1]*scale,position.center[2]*scale);
		rp3d::Quaternion orientation = get_quaternion(rigidbody.m_position.axis,rigidbody.m_position.angle);
		//
		rp3d::Transform transform(center,orientation);
		rigidbody.m_body = m_world->createRigidBody(transform);
		rigidbody.m_body->setLinearVelocity(rp3d::Vector3(
			velocity.center_velocity[0],
			velocity.center_velocity[1],
			velocity.center_velocity[2]));
		rigidbody.m_body->setAngularVelocity(rp3d::Vector3(
			velocity.angular_velocity[0],
			velocity.angular_velocity[1],
			velocity.angular_velocity[2]));
		//
		rp3d::Material material(m_settings);
		material.setBounciness(attribute.restitution);
		material.setFrictionCoefficient(attribute.friction);
		rigidbody.m_body->setMaterial(material);
		//
		if( attribute.density == 0.0 ) {
			rigidbody.m_body->setType(rp3d::BodyType::KINEMATIC);
		} else {
			rigidbody.m_body->setType(rp3d::BodyType::DYNAMIC);
		}
		//
		for( unsigned i=0; i<rigidbody.m_polyshapes.size(); ++i ) {
			//
			const auto &faces = rigidbody.m_polyshapes[i].faces;
			const auto &vertices = rigidbody.m_polyshapes[i].vertices;
			//
			reactphysics3d_rigidbody3::mesh_data *mesh_data = new reactphysics3d_rigidbody3::mesh_data();
			rigidbody.m_mesh_data_array.push_back(mesh_data);
			//
			std::vector<int> &triangle_array = mesh_data->triangle_array;
			std::vector<float> &vertices_array = mesh_data->vertices_array;
			//
			vec3d approx_center;
			double sum (0.0);
			for( const auto &v : vertices ) {
				approx_center += v;
				sum += 1.0;
			}
			approx_center /= sum;
			//
			vertices_array.resize(3*vertices.size());
			for( size_t n=0; n<vertices.size(); ++n ) {
				for( int dim : DIMS3 ) vertices_array[DIM3*n+dim] = (vertices[n]-approx_center)[dim]*scale;
			}
			for( size_t n=0; n<faces.size(); ++n ) {
				for( auto &v : faces[n] ) triangle_array.push_back(v);
			}
			//
			if( polyshapes[i].type == rg3::CONVEX ) {
				//
				auto &polyfaces = mesh_data->polyfaces;
				polyfaces.resize(faces.size());
				rp3d::PolygonVertexArray::PolygonFace* face = polyfaces.data();
				size_t pos_idx (0);
				for( size_t n=0; n<faces.size(); ++n ) {
					face->indexBase = pos_idx;
					face->nbVertices = faces[n].size();
					pos_idx += faces[n].size();
					face ++;
				}
				//
				mesh_data->polygon_vertex_array = new rp3d::PolygonVertexArray(
					vertices.size(), vertices_array.data() , 3*sizeof(float), triangle_array.data() , sizeof(int), faces.size(), polyfaces.data(),
					rp3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
					rp3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE
				);
				//
				mesh_data->polyhedron_mesh = new rp3d::PolyhedronMesh(mesh_data->polygon_vertex_array);
				mesh_data->collision_shape  = new rp3d::ConvexMeshShape(mesh_data->polyhedron_mesh);
				//
			} else if( polyshapes[i].type == rg3::MESH ) {
				//
				assert( attribute.density == 0.0 );
				//
				rp3d::TriangleVertexArray* triangle_soup = new rp3d::TriangleVertexArray(
					vertices.size(), vertices_array.data(), 3*sizeof(float), faces.size(), triangle_array.data() , 3*sizeof(int),
					rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
					rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
				);
				//
				mesh_data->triangle_mesh = new rp3d::TriangleMesh;
				mesh_data->triangle_mesh->addSubpart(triangle_soup);
				mesh_data->collision_shape  = new rp3d::ConcaveMeshShape(mesh_data->triangle_mesh);
			}
			//
			rp3d::Transform transform = rp3d::Transform::identity();
			transform.setPosition(scale*rp3d::Vector3(approx_center[0],approx_center[1],approx_center[2]));
			//
			rp3d::decimal mass = rp3d::decimal(attribute.density ? attribute.density : 1.0);
			rigidbody.m_body->addCollisionShape(mesh_data->collision_shape,transform,mass);
		}
		//
		m_rigidbodies.push_back(rigidbody);
		return &m_rigidbodies.back();
	}
	//
	virtual void advance( double dt ) override {
		//
		// Advance a timestep
		m_world->update(dt);
		//
		// Now update the internal position and angle of the body.
		for( auto &rigidbody : m_rigidbodies ) {
			//
			const rp3d::RigidBody *body = rigidbody.m_body;
			rp3d::Transform transform = body->getTransform();
			//
			rp3d::Quaternion orientation = transform.getOrientation();
			rp3d::Vector3 center = transform.getPosition();
			//
			rp3d::Vector3 axis;
			rp3d::decimal angle;
			orientation.getRotationAngleAxis(angle,axis);
			//
			rigidbody.m_position.angle = angle;
			rigidbody.m_position.axis = vec3d(axis[0],axis[1],axis[2]);
			rigidbody.m_position.center = vec3d(center[0],center[1],center[2]);
			//
			rp3d::Vector3 omega = body->getAngularVelocity();
			rp3d::Vector3 velocity = body->getLinearVelocity();
			//
			rigidbody.m_velocity.center_velocity = vec3d(velocity[0],velocity[1],velocity[2]);
			rigidbody.m_velocity.angular_velocity = vec3d(omega[0],omega[1],omega[2]);
		}
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		using ge = graphics_engine;
		const double &scale = m_param.scale;
		//
		for( const auto &rigidbody : m_rigidbodies ) {
			//
			if( rigidbody.m_attribute.drawable ) {
				//
				float m[16];
				rigidbody.getOpenGLMatrix(m);
				//
				if( rigidbody.m_attribute.density == 0.0 ) {
					g.color4(0.2,0.3,0.5,0.75);
				} else {
					g.color4(0.5,0.3,0.2,0.75);
				}
				//
				for( const auto &shape : rigidbody.m_polyshapes ) {
					for( unsigned i=0; i<shape.faces.size(); i++ ) {
						g.begin( ge::MODE::LINE_LOOP);
						for( unsigned j=0; j<shape.faces[i].size(); j++ ) {
							//
							const vec3d &p = shape.vertices[shape.faces[i][j]];
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
		}
	}
	//
	virtual std::vector<rigidbody3_interface *> get_rigidbody_list() override {
		std::vector<rigidbody3_interface *> result;
		for( auto &r : m_rigidbodies ) result.push_back(&r);
		return result;
	}
	//
	struct Parameters {
		vec3d gravity {0.0,-9.8,0.0};
		double scale {1.0};
		unsigned velocity_iterations {15};
		unsigned position_iterations {8};
	};
	//
	Parameters m_param;
	//
	rp3d::WorldSettings m_settings;
	rp3d::DynamicsWorld *m_world {nullptr};
	std::vector<reactphysics3d_rigidbody3> m_rigidbodies;
};
//
extern "C" module * create_instance() {
	return new reactphysics3d_rigidworld3;
}
//
extern "C" const char *license() {
	return "zlib";
}
//