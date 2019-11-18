/*
**	camera3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 11, 2017. 
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
#include <shiokaze/ui/camera3_interface.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/math/vec.h>
#define _USE_MATH_DEFINES
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
class camera3 : public camera3_interface {
protected:
	//
	virtual void configure ( configuration &config ) override {
		//
		config.get_double("ScrollSpeed",m_param.scroll_speed,"Scroll speed");
		config.get_double("MinScale",m_param.min_scale,"Minimal scale");
		config.get_double("RotateSpeed",m_param.rotate_speed,"Rotation speed");
		config.get_bool("ResetView",m_param.reset_view,"Reset view");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		//
		if( ! m_bounding_box_set || m_param.reset_view ) {
			m_target = vec3d(0.5,0.2,0.5);
			m_position = vec3d(-0.4,1.6,-3.0);
			m_up = vec3d(0.0,1.0,0.0);
			m_fov = 35.0;
			m_near = 0.1;
			m_far = 10.0;
		}
	}
	//
	virtual void resize( int width, int height ) override {
		//
		m_width = width;
		m_height = height;
	}
	//
	void update_clipping() {
		//
		m_near = std::numeric_limits<double>::max();
		m_far = std::numeric_limits<double>::min();
		vec3d dir = (m_target-m_position).normal();
		double w = m_bb1[0]-m_bb0[0];
		double h = m_bb1[1]-m_bb0[1];
		double d = m_bb1[2]-m_bb0[2];
		//
		for( int i=0; i<2; ++i ) for( int j=0; j<2; ++j ) for( int k=0; k<2; ++k ) {
			vec3d r = vec3d(i*w,j*h,k*d) - m_position;
			double dot = r*dir;
			m_near = std::min(dot,m_near);
			m_far = std::max(dot,m_far);
		}
		//
		const double eps (1e-3);
		const double margin (0.25);
		if( m_near < eps ) m_near = eps;
		m_near = (1.0-margin)*m_near;
		m_far = (1.0+margin)*m_far;
	}
	//
	virtual void set_bounding_box( const double *p0, const double *p1 ) override {
		//
		m_bb0 = vec3d(p0);
		m_bb1 = vec3d(p1);
		//
		if( ! m_bounding_box_set || m_param.reset_view ) {
			//
			double w = m_bb1[0]-m_bb0[0];
			double h = m_bb1[1]-m_bb0[1];
			double d = m_bb1[2]-m_bb0[2];
			double m = std::max(w,d);
			vec3d target = 0.5*vec3d(w,0.75*h,d);
			vec3d position = vec3d(-0.4,target[1]+1.0,-3.0);
			vec3d up = vec3d(0.0,1.0,0.0);
			double fov (35.0), near(0.1), far(10.0);
			look_at(target.v,position.v,up.v,fov);
			set_distance(2.75*m);
		} else {
			update_clipping();
		}
		//
		m_bounding_box_set = true;
	}
	//
	virtual void look_at( const double *target, const double *position, const double *up, double fov ) override {
		//
		m_fov = fov;
		for( int dim : DIMS3 ) {
			m_target[dim] = target[dim];
			m_position[dim] = position[dim];
			m_up[dim] = up[dim];
		}
		//
		if( m_bounding_box_set ) {
			update_clipping();
		}
	}
	//
	virtual void get( double *target, double *position, double *up, double *fov ) const override {
		//
		*fov = m_fov;
		for( int dim : DIMS3 ) {
			target[dim] = m_target[dim];
			position[dim] = m_position[dim];
			up[dim] = m_up[dim];
		}
	}
	//
	virtual bool keyboard( int key, int action, int mods ) override {
		//
		if( key == UI_interface::KEY_SPACE ) {
			if( action == UI_interface::PRESS ) {
				m_space_pressing = true;
			} else if( action == UI_interface::RELEASE ) {
				m_space_pressing = false;
			}
			return true;
		}
		//
		if( action == UI_interface::PRESS ) {
			if( mods & MOD_SHIFT ) {
				m_shift_pressing = true;
				return true;
			}
		} else if( action == UI_interface::RELEASE ) {
			if( ! (mods & MOD_SHIFT)) {
				if( m_shift_pressing ) {
					m_shift_pressing = false;
					return true;
				}
			}
		}
		return false;
	}
	//
	virtual void cursor( double x, double y, double z ) override {
		//
		if( ! m_dragging ) {
			m_drag_start = m_dragging_pos = vec2d(x,y);
			m_drag_start_position = m_position;
			m_drag_start_target = m_target;
			m_dragging_xvec = ((m_position-m_target) ^ m_up).normal();
		} else {
			m_dragging_pos = vec2d(x,y);
		}
	}
	//
	virtual void scroll( double xoffset, double yoffset ) override {
		//
		double distance = get_distance();
		distance = std::max(m_param.min_scale,distance-m_param.scroll_speed*yoffset);
		set_distance(distance);
	}
	//
	virtual void mouse( double x, double y, double z, int button, int action, int mods ) override {
		//
		if( button == UI_interface::RELEASE ) {
			m_dragging = false;
		}
	}
	//
	virtual void drag( double x, double y, double z, double u, double v, double w ) override {
		m_dragging = true;
		//
		if( m_space_pressing ) {
			//
			double x = m_dragging_pos[0]-m_drag_start[0];
			double y = m_dragging_pos[1]-m_drag_start[1];
			m_target = m_drag_start_target + m_param.rotate_speed * (x * m_dragging_xvec + y * m_up);
			update_clipping();
			//
		} else if( m_shift_pressing ) {
			//
			vec3d eye_r = m_drag_start_position - m_target;
			double dot = eye_r * m_up;
			eye_r -= dot * m_up;
			double d = eye_r.len();
			eye_r = eye_r / d;
			//
			double factor = (m_drag_start[0]-m_dragging_pos[0]) * m_param.rotate_speed;
			vec3d aside = m_up ^ eye_r;
			vec3d rotated = (factor * aside + (1.0-factor) * eye_r).normal();
			rotated *= d;
			rotated += dot * m_up;
			d = rotated.len();
			//
			m_position = m_target + rotated - (m_drag_start[1]-m_dragging_pos[1]) * m_param.rotate_speed * m_up;
			update_clipping();
		}
	}
	//
	vec3d convert( const vec2d &input ) const {
		//
		vec3d r = m_target-m_position;
		vec3d ex = r ^ m_up;
		vec3d ey = ex ^ r;
		ex.normalize();
		ey.normalize();
		double scale = r.len() * tan(0.5*m_fov/180.0*M_PI);
		//
		vec2d p_2D;
		p_2D[0] = 2.0 * (input[0] / (double)m_width - 0.5) * m_width / (double)m_height;
		p_2D[1] = 2.0 * ((1.0-input[1]/m_height) - 0.5);
		//
		return m_target + scale * (ex * p_2D[0] + ey * p_2D[1]);
	}
	//
	virtual UI_interface::event_structure convert( const UI_interface::event_structure &event ) const override {
		//
		UI_interface::event_structure result(event);
		//
		vec3d out = convert(vec2d(event.x,event.y));
		result.x = out[0];
		result.y = out[1];
		result.z = out[2];
		//
		vec3d out1 = convert(vec2d(event.x+event.u,event.y+event.v));
		result.u = out1[0]-out[0];
		result.v = out1[1]-out[1];
		result.w = out1[2]-out[2];
		return result;
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		g.set_viewport(0,0,m_width,m_height);
		g.look_at(m_target.v,m_position.v,m_up.v,m_fov,m_near,m_far);
		//
		if( m_bounding_box_set ) {
			g.color4(1.0,1.0,1.0,0.5);
			graphics_utility::draw_wired_box(g,m_bb0.v,m_bb1.v);
		}
	}
	//
	virtual bool relay_event( const event_structure &event ) const override {
		//
		bool handling =
			(event.type == event_structure::KEYBOARD ||
			 event.type == event_structure::CURSOR ||
			 event.type == event_structure::DRAG ||
			 event.type == event_structure::MOUSE ) &&
			 (m_space_pressing == true || m_shift_pressing == true);
		return ! handling;
	}
	//
	virtual UI_interface::CURSOR_TYPE get_current_cursor() const override {
		return (m_space_pressing || m_shift_pressing) ? UI_interface::HAND_CURSOR : ARROW_CURSOR;
	}
	//
	struct Parameters {
		double scroll_speed {0.01};
		double rotate_speed {0.001};
		double min_scale {0.01};
		bool reset_view {false};
	};
	Parameters m_param;
	//
	int m_width {1}, m_height {1};
	bool m_dragging {false};
	bool m_space_pressing {false}, m_shift_pressing {false};
	vec3d m_target, m_position, m_up;
	vec2d m_drag_start, m_dragging_pos;
	vec3d m_drag_start_position, m_drag_start_target, m_dragging_xvec;
	double m_fov, m_near, m_far;
	bool m_bounding_box_set {false};
	vec3d m_bb0, m_bb1;
};
//
extern "C" module * create_instance() {
	return new camera3();
}
//
extern "C" const char *license() {
	return "MIT";
}