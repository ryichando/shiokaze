/*
**	camera2.cpp
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
#include <shiokaze/ui/camera2_interface.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/core/console.h>
//
SHKZ_USING_NAMESPACE
//
class camera2 : public camera2_interface {
protected:
	//
	virtual void configure ( configuration &config ) override {
		//
		config.get_double("ScrollSpeed",m_param.scroll_speed,"Scroll speed");
		config.get_double("MinScale",m_param.min_scale,"Minimal scale");
		config.get_bool("ResetView",m_param.reset_view,"Reset view");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		//
		if( ! m_bounding_box_set || m_param.reset_view ) {
			m_origin = vec2d();
			m_scale = 1.0;
		}
	}
	//
	virtual void resize( int width, int height ) override {
		//
		m_width = width;
		m_height = height;
	}
	//
	virtual void set_bounding_box( const double *p0, const double *p1 ) override {
		//
		if( ! m_bounding_box_set || m_param.reset_view ) {
			double scale (1.0);
			scale = std::max(p1[0]-p0[0],p1[1]-p0[1]);
			set_2D_coordinate(p0,scale);
		}
		m_bb0 = vec2d(p0);
		m_bb1 = vec2d(p1);
		m_bounding_box_set = true;
	}
	//
	virtual void set_2D_coordinate( const double *origin, double scale ) override {
		//
		m_origin = vec2d(origin);
		m_scale = scale;
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
		} else {
			return false;
		}
	}
	//
	virtual void cursor( double x, double y, double z ) override {
		//
		if( m_dragging ) {
			if( m_space_pressing ) {
				m_origin = m_save_origin + m_scale * (m_mouse_dragging-vec2d(x,-y)) / (double) m_width;
			}
		} else {
			m_mouse_dragging = vec2d(x,-y);
		}
		m_current_pos = convert(vec2d(x,y));
	}
	//
	virtual void scroll( double xoffset, double yoffset ) override {
		//
		double new_scale = std::max(m_param.min_scale,m_scale - m_param.scroll_speed * yoffset);
		double ratio = new_scale / m_scale;
		vec2d corner = convert(vec2d(0.0,m_height));
		m_origin += corner-m_current_pos;
		m_origin *= ratio;
		m_origin -= ratio*corner-m_current_pos;
		m_scale = new_scale;
	};
	//
	virtual void mouse( double x, double y, double z, int button, int action, int mods ) override {
		//
		if( button == UI_interface::RELEASE ) {
			m_dragging = false;
		}
		m_save_origin = m_origin;
	}
	//
	virtual void drag( double x, double y, double z, double u, double v, double w ) override {
		//
		double scale (10.0);
		m_p0 = convert(vec2d(x,y));
		m_p1 = convert(vec2d(x+scale*u,y+scale*v));
		m_dragging = true;
	};
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		double left = m_origin[0];
		double right = m_origin[0] + m_scale;
		double bottom = m_origin[1];
		double top = m_origin[1] + m_scale * m_height / (double)m_width;
		//
		g.set_viewport(0,0,m_width,m_height);
		g.set_2D_coordinate(left,right,bottom,top);
		//
		if( ! m_space_pressing && m_dragging ) {
			g.color4(1.0,1.0,1.0,1.0);
			graphics_utility::draw_arrow(g,m_p0.v,m_p1.v);
		} else if( m_space_pressing ) {
			//
			g.color4(1.0,1.0,1.0,0.5);
			vec2d corner0 = convert(vec2d(0.0,m_height));
			vec2d corner1 = convert(vec2d(m_width,0.0));
			//
			g.begin(graphics_engine::MODE::LINES);
			for( int i=corner0[0]; i<=corner1[0]; ++i ) {
				g.vertex2v(vec2d(i,corner0[1]).v);
				g.vertex2v(vec2d(i,corner1[1]).v);
			}
			for( int j=corner0[1]; j<=corner1[1]; ++j ) {
				g.vertex2v(vec2d(corner0[0],j).v);
				g.vertex2v(vec2d(corner1[0],j).v);
			}
			g.end();
			//
			if( m_scale < 20.0 ) {
				for( int i=corner0[0]; i<=corner1[0]; ++i ) {
					for( int j=corner0[1]; j<=corner1[1]; ++j ) {
						g.draw_string(vec2d(i,j).v,console::format_str("(%d,%d)",i,j));
					}
				}
			}
		}
		//
		if( m_bounding_box_set ) {
			g.color4(1.0,1.0,1.0,1.0);
			g.begin(graphics_engine::MODE::LINE_LOOP);
			g.vertex2(m_bb0[0],m_bb0[1]);
			g.vertex2(m_bb1[0],m_bb0[1]);
			g.vertex2(m_bb1[0],m_bb1[1]);
			g.vertex2(m_bb0[0],m_bb1[1]);
			g.end();
		}
	}
	//
	vec2d convert( const vec2d &input ) const {
		//
		vec2d p;
		p[0] = input[0] / (double)m_width;
		p[1] = (1.0-input[1]/m_height) * (m_height / (double)m_width);
		return m_scale * p + m_origin;
	}
	//
	virtual UI_interface::event_structure convert( const UI_interface::event_structure &event ) const override {
		//
		UI_interface::event_structure result(event);
		//
		vec2d out = convert(vec2d(event.x,event.y));
		result.x = out[0];
		result.y = out[1];
		//
		vec2d out1 = convert(vec2d(event.x+event.u,event.y+event.v));
		result.u = out1[0]-out[0];
		result.v = out1[1]-out[1];
		return result;
	}
	//
	virtual bool relay_event( const event_structure &event ) const override {
		bool handling =
			(event.type == event_structure::KEYBOARD ||
			 event.type == event_structure::CURSOR ||
			 event.type == event_structure::DRAG ||
			 event.type == event_structure::MOUSE ) &&
			 m_space_pressing == true;
		return ! handling;
	}
	//
	virtual UI_interface::CURSOR_TYPE get_current_cursor() const override {
		return m_space_pressing ? UI_interface::HAND_CURSOR : ARROW_CURSOR;
	}
	//
	struct Parameters {
		double scroll_speed {0.01};
		double min_scale {0.01};
		bool reset_view {false};
	};
	Parameters m_param;
	//
	vec2d m_origin, m_p0, m_p1;
	double m_scale {1.0};
	int m_width {1}, m_height {1};
	bool m_dragging {false};
	bool m_space_pressing {false};
	vec2d m_mouse_dragging, m_save_origin;
	vec2d m_current_pos;
	bool m_bounding_box_set {false};
	vec2d m_bb0, m_bb1;
};
//
extern "C" module * create_instance() {
	return new camera2();
}
//
extern "C" const char *license() {
	return "MIT";
}