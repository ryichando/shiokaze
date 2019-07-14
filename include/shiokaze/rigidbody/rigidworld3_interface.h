/*
**	rigidworld3_interface.h
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
#ifndef SHKZ_RIGIDWORLD3_H
#define SHKZ_RIGIDWORLD3_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/math/vec.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
class rigidbody3_interface {
public:
	//
	enum shape_type { CONVEX, MESH };
	//
	struct polyshape3 {
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		shape_type type;
	};
	//
	struct position3 {
		vec3d center;
		vec3d axis;
		double angle;
	};
	//
	struct attribution3 {
		std::string name;
		double density;
		double friction;
		double restitution;
		bool drawable;
		void *user_pointer;
	};
	//
	struct velocity3 {
		vec3d center_velocity;
		vec3d angular_velocity;
	};
	//
	virtual std::vector<polyshape3> get_shapes() const = 0;
	virtual position3 get_position() const = 0;
	virtual attribution3 get_attribution() const = 0;
	virtual velocity3 get_velocity() const = 0;
	virtual void getOpenGLMatrix( float m[16] ) const = 0;
	//
	vec3d get_velocity( const vec3d &p ) const {
		vec3d r = p-get_position().center;
		velocity3 u = get_velocity();
		vec3d a = u.angular_velocity;
		return a ^ r;
	}
};
//
class rigidworld3_interface : public recursive_configurable_module {
public:
	//
	using rg3 = rigidbody3_interface;
	//
	DEFINE_MODULE(rigidworld3_interface,"Rigidbody World 3D","Rigidbody","Rigidbody module")
	//
	virtual rigidbody3_interface * add_rigidbody(
		const std::vector<rg3::polyshape3> &polyshapes, const rg3::attribution3 &attribute, const rg3::position3& position, const rg3::velocity3 &velocity
	) = 0;
	//
	virtual std::string engine_name() const = 0;
	virtual void clear() = 0;
	virtual void advance( double dt ) = 0;
	virtual void draw( graphics_engine &g ) const = 0;
	//
	virtual std::vector<rigidbody3_interface *> get_rigidbody_list() = 0;
	//
	std::vector<const rigidbody3_interface *> get_rigidbody_list() const {
		std::vector<rigidbody3_interface *> tmp_result = const_cast<rigidworld3_interface *>(this)->get_rigidbody_list();
		std::vector<const rigidbody3_interface *> result;
		for( auto it=tmp_result.begin(); it!=tmp_result.end(); ++it ) result.push_back(*it);
		return result;
	}
	//
};
//
using rigidworld3_ptr = std::unique_ptr<rigidworld3_interface>;
using rigidworld3_driver = recursive_configurable_driver<rigidworld3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
