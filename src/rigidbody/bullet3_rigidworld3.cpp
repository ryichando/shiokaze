/*
**	bullet3_rigidbody3.cpp
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
#include "btBulletDynamicsCommon.h"
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
using rg3 = rigidbody3_interface;
//
class bullet3_rigidbody3 : public rigidbody3_interface {
public:
	//
	bullet3_rigidbody3() = default;
	bullet3_rigidbody3 ( double scale ) : m_scale(scale) {}
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
		btTransform tranform;
		m_motionState->getWorldTransform(tranform);
		tranform.setOrigin(tranform.getOrigin() / m_scale);
		tranform.getOpenGLMatrix(m);
	}
	//
	std::vector<polyshape3> m_polyshapes;
	position3 m_position;
	attribution3 m_attribute;
	velocity3 m_velocity;
	//
	std::vector<btCollisionShape *> m_collision_shapes;
	std::vector<btTriangleIndexVertexArray *> m_triangles;
	btDefaultMotionState *m_motionState;
	btRigidBody* m_rigidbody;
	double m_scale;
	//
	struct mesh_data {
		std::vector<int> triangle_array;
		std::vector<btScalar> vertices_array;
	};
	std::vector<mesh_data *> m_mesh_data_array;
};
//
class bullet3_rigidworld3 : public rigidworld3_interface {
protected:
	//
	LONG_NAME("Bullet3 Rigidbody Engine")
	MODULE_NAME("bullet3_rigidworld3")
	AUTHOR_NAME("Erwin Coumans et al.")
	//
	virtual ~bullet3_rigidworld3() {
		//
		clear();
		//
		if( m_dynamics_world ) delete m_dynamics_world;
		if( m_solver ) delete m_solver;
		if( m_overlappingPairCache ) delete m_overlappingPairCache;
		if( m_dispatcher ) delete m_dispatcher;
		if( m_collisionConfiguration ) delete m_collisionConfiguration;
	}
	//
	virtual std::string engine_name() const override {
		return "Bullet3";
	}
	//
	virtual void clear() override {
		//
		if( m_dynamics_world ) {
			//
			for (int i = m_dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--) {
				btCollisionObject* obj = m_dynamics_world->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(obj);
				if (body && body->getMotionState()) {
					delete body->getMotionState();
				}
				m_dynamics_world->removeCollisionObject(obj);
				delete obj;
			}
			//
			for( auto &rigidbody : m_rigidbodies ) {
				for( auto &shape : rigidbody.m_collision_shapes ) delete shape;
				for( auto &tri : rigidbody.m_triangles ) delete tri;
				for( auto &mesh : rigidbody.m_mesh_data_array ) delete mesh;
			}
		}
		//
		m_rigidbodies.clear();
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_vec3d("Gravity",m_param.gravity.v,"Gravity vector");
		config.get_double("Margin",m_param.margin,"Collision margin");
		config.get_double("Scale",m_param.scale,"Scaling of domain");
		config.get_unsigned("MaxSubsteps",m_param.max_substeps,"Maximal substeps");
		config.get_unsigned("StepSubdivision",m_param.step_subdivision,"Step subdivision count");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		//
		clear();
		//
		if( ! m_dynamics_world ) {
			//
			m_collisionConfiguration = new btDefaultCollisionConfiguration();
			m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
			m_overlappingPairCache = new btDbvtBroadphase();
			m_solver = new btSequentialImpulseConstraintSolver;
			m_dynamics_world = new btDiscreteDynamicsWorld(m_dispatcher,m_overlappingPairCache,m_solver,m_collisionConfiguration);
			//
			const double &scale = m_param.scale;
			m_dynamics_world->setGravity(btVector3(m_param.gravity[0]*scale,m_param.gravity[1]*scale,m_param.gravity[2]*scale));
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
		assert(m_dynamics_world);
		const double &scale = m_param.scale;
		//
		bullet3_rigidbody3 rigidbody(scale);
		rigidbody.m_polyshapes = polyshapes;
		rigidbody.m_attribute = attribute;
		rigidbody.m_position = position;
		if( ! position.angle ) rigidbody.m_position.axis = vec3d(1.0,0.0,0.0);
		//
		btCompoundShape* compound_shape = new btCompoundShape;
		rigidbody.m_collision_shapes.push_back(compound_shape);
		btTransform local = btTransform::getIdentity();
		//
		for( unsigned i=0; i<polyshapes.size(); ++i ) {
			//
			if( polyshapes[i].type == rg3::CONVEX ) {
				//
				btConvexHullShape* shape = new btConvexHullShape();
				compound_shape->addChildShape(local,shape);
				rigidbody.m_collision_shapes.push_back(shape);
				//
				const auto &vertices = polyshapes[i].vertices;
				for( size_t n=0; n<vertices.size(); ++n ) {
					shape->addPoint(btVector3(vertices[n][0]*scale,vertices[n][1]*scale,vertices[n][2]*scale),false);
				}
				shape->recalcLocalAabb();
				shape->setMargin(m_param.margin);
				//
			} else if( polyshapes[i].type == rg3::MESH ) {
				//
				assert( attribute.density == 0.0 );
				//
				const auto &faces = polyshapes[i].faces;
				const auto &vertices = polyshapes[i].vertices;
				//
				bullet3_rigidbody3::mesh_data *mesh_data = new bullet3_rigidbody3::mesh_data();
				rigidbody.m_mesh_data_array.push_back(mesh_data);
				//
				std::vector<int> &triangle_array = mesh_data->triangle_array;
				std::vector<btScalar> &vertices_array = mesh_data->vertices_array;
				//
				triangle_array.resize(3*faces.size());
				vertices_array.resize(3*vertices.size());
				//
				for( size_t n=0; n<faces.size(); ++n ) {
					assert(faces[n].size()==3);
					for( int m : {0,1,2} ) triangle_array[3*n+m] = faces[n][m];
				}
				for( size_t n=0; n<vertices.size(); ++n ) {
					for( int dim : DIMS3 ) vertices_array[DIM3*n+dim] = vertices[n][dim]*scale;
				}
				//
				btTriangleIndexVertexArray* index_vertex_arrays = new btTriangleIndexVertexArray(
																				faces.size(),
																				&triangle_array[0],
																				3*sizeof(int),
																				vertices.size(),
																				&vertices_array[0],
																				3*sizeof(btScalar));
				//
				rigidbody.m_triangles.push_back(index_vertex_arrays);
				btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(index_vertex_arrays,false);
				//
				compound_shape->addChildShape(local,shape);
				rigidbody.m_collision_shapes.push_back(shape);
			}
		}
		//
		btVector3 local_inertia(0.0,0.0,0.0);
		if( attribute.density ) {
			compound_shape->calculateLocalInertia(attribute.density,local_inertia);
		}
		//
		btTransform rigidbody_transform;
		rigidbody_transform.setIdentity();
		rigidbody_transform.setOrigin(
			btVector3(
				rigidbody.m_position.center[0]*scale,
				rigidbody.m_position.center[1]*scale,
				rigidbody.m_position.center[2]*scale));
		btQuaternion rotation;
		rotation.setRotation(btVector3(
				rigidbody.m_position.axis[0],
				rigidbody.m_position.axis[1],
				rigidbody.m_position.axis[2]),rigidbody.m_position.angle);
		rigidbody_transform.setRotation(rotation);
		//
		rigidbody.m_motionState = new btDefaultMotionState(rigidbody_transform);
		btRigidBody::btRigidBodyConstructionInfo rigidbody_CI(
			attribute.density,rigidbody.m_motionState,compound_shape,local_inertia);
		rigidbody_CI.m_friction = attribute.friction;
		rigidbody_CI.m_restitution = attribute.restitution;
		//
		rigidbody.m_rigidbody = new btRigidBody(rigidbody_CI);
		m_dynamics_world->addRigidBody(rigidbody.m_rigidbody);
		m_rigidbodies.push_back(rigidbody);
		//
		return &m_rigidbodies.back();
	}
	//
	virtual void advance( double dt ) override {
		//
		// Instruct the world to perform a single step of simulation.
		m_dynamics_world->stepSimulation(dt,m_param.max_substeps,dt/(double)m_param.step_subdivision);
		//
		// Now update the internal position and angle of the body.
		for( auto &rigidbody : m_rigidbodies ) {
			//
			const btRigidBody &body = *rigidbody.m_rigidbody;
			//
			btQuaternion orientation = body.getOrientation();
			btScalar angle = orientation.getAngle();
			btVector3 axis = orientation.getAxis();
			rigidbody.m_position.angle = angle;
			rigidbody.m_position.axis = vec3d(axis[0],axis[1],axis[2]);
			//
			btVector3 velocity = body.getLinearVelocity();
			rigidbody.m_velocity.center_velocity = vec3d(velocity[0],velocity[1],velocity[2]);
			//
			btVector3 angularVelocity = body.getAngularVelocity();
			rigidbody.m_velocity.angular_velocity = vec3d(angularVelocity[0],angularVelocity[1],angularVelocity[2]);
			//
			btVector3 center = body.getCenterOfMassPosition();
			rigidbody.m_position.center = vec3d(center[0],center[1],center[2]);
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
		double margin {0.0};
		double scale {0.5};
		unsigned max_substeps {1};
		unsigned step_subdivision {1};
	};
	//
	Parameters m_param;
	//
	std::vector<bullet3_rigidbody3> m_rigidbodies;
	//
	btDefaultCollisionConfiguration* m_collisionConfiguration {nullptr};
	btCollisionDispatcher* m_dispatcher {nullptr};
	btBroadphaseInterface* m_overlappingPairCache {nullptr};
	btSequentialImpulseConstraintSolver* m_solver {nullptr};
	btDiscreteDynamicsWorld* m_dynamics_world {nullptr};
};
//
extern "C" module * create_instance() {
	return new bullet3_rigidworld3;
}
//
extern "C" const char *license() {
	return "zlib";
}
//