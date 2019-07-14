/*
**	rigidworld2_interface.h
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
#ifndef SHKZ_RIGIDWORLD2_H
#define SHKZ_RIGIDWORLD2_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/math/vec.h>
#include <cmath>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
class rigidbody2_interface {
public:
	//
	enum shape_type { EDGE, POLYGON };
	//
	struct polyshape2 {
		std::vector<vec2d> polygon;
		shape_type type;
	};
	//
	struct position2 {
		vec2d center;
		double angle;
	};
	//
	struct attribution2 {
		std::string name;
		double density;
		double friction;
		double restitution;
		bool drawable;
		void *user_pointer;
	};
	//
	struct velocity2 {
		vec2d center_velocity;
		double angular_velocity;
	};
	//
	virtual std::vector<polyshape2> get_shapes() const = 0;
	virtual position2 get_position() const = 0;
	virtual attribution2 get_attribution() const = 0;
	virtual velocity2 get_velocity() const = 0;
	//
	void getOpenGLMatrix( float m[9] ) const {
		position2 position = get_position();
		m[0] = cos(position.angle); m[3] = -sin(position.angle); m[6] = position.center[0];
		m[1] = sin(position.angle); m[4] = cos(position.angle); m[7] = position.center[1];
		m[2] = 0.0; m[5] = 0.0; m[8] = 1.0;
	}
	//
	vec2d get_velocity( const vec2d &p ) const {
		vec2d r = p-get_position().center;
		velocity2 u = get_velocity();
		double a = u.angular_velocity;
		return u.center_velocity+a*vec2d(-r[1],r[0]);
	}
};
//
class rigidworld2_interface : public recursive_configurable_module {
public:
	//
	using rg2 = rigidbody2_interface;
	//
	DEFINE_MODULE(rigidworld2_interface,"Rigidbody World 2D","Rigidbody","Rigidbody module")
	//
	virtual rigidbody2_interface * add_rigidbody(
		const std::vector<rg2::polyshape2> &polyshapes, const rg2::attribution2 &attribute, const rg2::position2& position, const rg2::velocity2 &velocity
	) = 0;
	//
	virtual std::string engine_name() const = 0;
	virtual void clear() = 0;
	virtual void advance( double dt ) = 0;
	virtual void draw( graphics_engine &g ) const = 0;
	//
	virtual std::vector<rigidbody2_interface *> get_rigidbody_list() = 0;
	//
	std::vector<const rigidbody2_interface *> get_rigidbody_list() const {
		std::vector<rigidbody2_interface *> tmp_result = const_cast<rigidworld2_interface *>(this)->get_rigidbody_list();
		std::vector<const rigidbody2_interface *> result;
		for( auto it=tmp_result.begin(); it!=tmp_result.end(); ++it ) result.push_back(*it);
		return result;
	}
	//
};
//
using rigidworld2_ptr = std::unique_ptr<rigidworld2_interface>;
using rigidworld2_driver = recursive_configurable_driver<rigidworld2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
