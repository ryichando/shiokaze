/*
**	graphics_svg.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 19, 2019.
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
#include <shiokaze/graphics/graphics_interface.h>
#include <shiokaze/math/vec.h>
#include <vector>
#include <cstring>
//
SHKZ_USING_NAMESPACE
//
class graphics_svg : public graphics_interface {
private:
	//
	LONG_NAME("SVG Graphics Engine")
	ARGUMENT_NAME("svg_graphics")
	//
	struct Color {
		double color[4];
	};
	//
	struct Primitive : public Color {
		MODE mode;
		double point_size;
		double line_width;
		std::vector<vec2d> points;
	};
	//
	struct String : public Color {
		std::string string;
		unsigned size;
		vec2d p;
	};
	//
	struct ViewBox {
		vec2d p0;
		vec2d p1;
	};
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("CanvasX",m_canvas_width,"Canvas width");
		config.get_unsigned("CanvasY",m_canvas_height,"Canvas height");
		config.get_double("PointScale",m_point_scale,"Point size scaling factor");
		config.get_double("LineScale",m_line_scale,"Line width scaling factor");
		config.get_double("FontSize",m_font_size,"Text font size");
		config.get_bool("EnableOpacity",m_enable_opacity,"Enable opacity support");
	}
	//
	virtual void setup_graphics ( std::map<std::string,const void *> params=std::map<std::string,const void *>() ) override {
		//
		m_viewport.p0 = m_coordsys.p0 = vec2d(0.0,0.0);
		m_viewport.p1 = vec2d(m_canvas_width,m_canvas_height);
		m_coordsys.p1 = vec2d(1.0,m_canvas_height/(double)m_canvas_width);
		m_scale = (m_viewport.p1-m_viewport.p0).norm_inf();
	}
	//
	virtual std::string get_graphics_engine_name () const override {
		return "SVG";
	}
	//
	virtual bool get_supported ( FEATURE feature ) const override {
		if( feature == FEATURE::OPACITY ) {
			return m_enable_opacity;
		} else if( feature == FEATURE::_3D ) {
			return false;
		}
		return false;
	}
	//
	virtual void set_viewport( unsigned x, unsigned y, unsigned width, unsigned height ) override {
		m_viewport.p0 = vec2d(x,y);
		m_viewport.p1 = vec2d(x+width,y+height);
	}
	//
	virtual void get_viewport( unsigned &x, unsigned &y, unsigned &width, unsigned &height ) const override {
		x = m_viewport.p0[0];
		y = m_viewport.p0[1];
		width = m_viewport.p1[0]-m_viewport.p1[0];
		height = m_viewport.p1[1]-m_viewport.p1[1];
	}
	//
	virtual void set_2D_coordinate( double left, double right, double bottom, double top ) override {
		m_coordsys.p0 = vec2d(left,bottom);
		m_coordsys.p1 = vec2d(right,top);
	}
	//
	virtual void look_at( const double target[3], const double position[3], const double up[3], double fov, double near, double far ) override {
		// Not supported yet
	}
	//
	virtual void clear() override {
		m_primitives.clear();
	}
	//
	virtual void get_background_color( double color[3] ) const override {
		color[0] = color[1] = color[2] = 1.0;
	}
	//
	virtual void get_foreground_color( double color[3] ) const override {
		color[0] = color[1] = color[2] = 0.0;
	}
	//
	virtual void color4v( const double *v ) override {
		std::memcpy(m_color,v,4*sizeof(double));
	}
	//
	vec2d convert_position( const double *v ) const {
		//
		double x = (v[0]-m_coordsys.p0[0]) * m_scale + m_coordsys.p0[0];
		double y = (v[1]-m_coordsys.p0[1]) * m_scale + m_coordsys.p0[1];
		double h = (m_coordsys.p1[1]-m_coordsys.p0[1]) * m_scale;
		return vec2d(x,h-y);
	}
	//
	virtual void vertex3v( const double *v ) override {
		//
		m_current_primitive.points.push_back(convert_position(v));
	}
	//
	virtual void begin( MODE mode ) override {
		//
		m_current_primitive.mode = mode;
		m_current_primitive.point_size = m_point_size;
		m_current_primitive.line_width = m_line_width;
		std::memcpy(m_current_primitive.color,m_color,4*sizeof(double));
	}
	//
	virtual void end() override {
		//
		m_primitives.push_back(m_current_primitive);
		m_current_primitive.points.clear();
		m_current_primitive.points.shrink_to_fit();
	}
	//
	virtual void point_size( double size ) override {
		m_point_size = m_point_scale * size;
	}
	//
	virtual void line_width( double width ) override {
		m_line_width = m_line_scale * width;
	}
	//
	virtual void draw_string( const double *v, std::string str, unsigned size=0 ) override {
		//
		String string_primitive;
		string_primitive.string = str;
		string_primitive.p = convert_position(v);
		string_primitive.size = size;
		std::memcpy(string_primitive.color,m_color,4*sizeof(double));
		m_strings.push_back(string_primitive);
	}
	//
	virtual bool send_message( std::string message, void *ptr ) override {
		//
		if( message == "example" ) {
			write_example(ptr);
			return true;
		}
		return false;
	}
	//
	virtual bool const_send_message( std::string message, void *ptr ) const override {
		if( message == "write" ) {
			//
			FILE *fp = std::fopen((const char *)ptr,"wb");
			assert(fp);
			write_header(fp);
			//
			for( const auto &primitive : m_primitives ) {
				if( primitive.mode == MODE::POINTS ) {
					write_point(fp,primitive);
				} else if( primitive.mode == MODE::LINES ) {
					write_lines(fp,primitive);
				} else if( primitive.mode == MODE::LINE_STRIP ) {
					write_lines(fp,primitive);
				} else if( primitive.mode == MODE::LINE_LOOP ) {
					write_lines(fp,primitive);
				} else if( primitive.mode == MODE::TRIANGLES ) {
					write_triangles(fp,primitive);
				} else if( primitive.mode == MODE::TRIANGLE_STRIP ) {
					write_triangles(fp,primitive);
				} else if( primitive.mode == MODE::TRIANGLE_FAN ) {
					write_triangles(fp,primitive);
				}
			}
			//
			for( const auto &primitive : m_strings ) {
				write_string(fp,primitive);
			}
			//
			write_footer(fp);
			std::fclose(fp);
			return true;
		}
		return false;
	}
	//
	void write_header( std::FILE *fp ) const {
		//
		std::fprintf( fp, "<?xml version=\"1.0\" standalone=\"no\"?>\n" );
		std::fprintf( fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n" );
		std::fprintf( fp, "<svg width=\"%g\" height=\"%g\" viewBox=\"%g %g %g %g\" ",
			m_viewport.p1[0]-m_viewport.p0[0], m_viewport.p1[1]-m_viewport.p0[1],
			m_coordsys.p0[0],m_coordsys.p0[1],
			(m_coordsys.p1[0]-m_coordsys.p0[0]) * m_scale,
			(m_coordsys.p1[1]-m_coordsys.p0[1]) * m_scale );
		//
		std::fprintf( fp, "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n" );
	}
	//
	void write_footer( std::FILE *fp ) const {
		std::fprintf( fp, "</svg>\n" );
	}
	//
	std::string opacity_fill_string( const Color &color ) const {
		//
		if( m_enable_opacity ) {
			return std::string("fill-opacity=\"") + std::to_string(color.color[3]) + "\"";
		} else {
			return std::string();
		}
	}
	//
	std::string opacity_stroke_string( const Color &color ) const {
		//
		if( m_enable_opacity ) {
			return std::string("stroke-opacity=\"") + std::to_string(color.color[3]) + "\"";
		} else {
			return std::string();
		}
	}
	//
	void write_point( FILE *fp, const Primitive &primitive ) const {
		//
		int rgb[3];
		convert_integer_rgb(primitive.color,rgb);
		//
		const double r = primitive.point_size;
		for( const auto &p : primitive.points ) {
			std::fprintf(fp,"<circle cx=\"%g\" cy=\"%g\" r=\"%g\" fill=\"rgb(%d,%d,%d)\" %s/>\n",
						p[0], p[1], r, rgb[0], rgb[1], rgb[2], opacity_fill_string(primitive).c_str() );
		}
	}
	//
	void write_lines( FILE *fp, const Primitive &primitive ) const {
		//
		int rgb[3];
		convert_integer_rgb(primitive.color,rgb);
		const double w = primitive.line_width;
		//
		if( primitive.mode == MODE::LINES ) {
			if( primitive.points.size() > 1 ) {
				for( unsigned n=0; n<primitive.points.size()-1; n+=2 ) {
					const vec2d &p0 = primitive.points[n];
					const vec2d &p1 = primitive.points[n+1];
					std::fprintf(fp,"<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"stroke:rgb(%d,%d,%d);stroke-width:%g\" %s/>\n",
							p0[0], p0[1], p1[0], p1[1], rgb[0], rgb[1], rgb[2], w, opacity_stroke_string(primitive).c_str() );
				}
			}
		} if( primitive.mode == MODE::LINE_STRIP || primitive.mode == MODE::LINE_LOOP ) {
			if( primitive.points.size() > 1 ) {
				std::fprintf(fp,"<path d=\"M %g %g ", primitive.points[0][0], primitive.points[0][1] );
				for( unsigned n=0; n<primitive.points.size(); ++n ) {
					const vec2d &p = primitive.points[n];
					std::fprintf(fp,"L %g %g ", p[0], p[1] );
				}
				if( primitive.mode == MODE::LINE_LOOP ) {
					std::fprintf(fp,"Z");
				}
				std::fprintf(fp,"\" style=\"stroke:rgb(%d,%d,%d);stroke-width:%g\" fill=\"none\" %s/>\n",
					rgb[0], rgb[1], rgb[2], w, opacity_stroke_string(primitive).c_str());
			}
		}
	}
	//
	void write_triangles( FILE *fp, const Primitive &primitive ) const {
		//
		int rgb[3];
		convert_integer_rgb(primitive.color,rgb);
		//
		if( primitive.mode == MODE::TRIANGLES ) {
			if( primitive.points.size() >= 3 ) {
				for( unsigned n=0; n<primitive.points.size()-2; n+=3 ) {
					const vec2d &p0 = primitive.points[n];
					const vec2d &p1 = primitive.points[n+1];
					const vec2d &p2 = primitive.points[n+2];
					std::fprintf(fp,"<polygon points=\"%g,%g %g,%g %g,%g\" style=\"fill:rgb(%d,%d,%d)\" %s/>\n",
							p0[0], p0[1], p1[0], p1[1], p2[0], p2[1], rgb[0], rgb[1], rgb[2], opacity_fill_string(primitive).c_str() );
				}
			}
		} else if( primitive.mode == MODE::TRIANGLE_STRIP ) {
			if( primitive.points.size() >= 3 ) {
				for( unsigned n=0; n<primitive.points.size()-2; ++n ) {
					const vec2d &p0 = primitive.points[n];
					const vec2d &p1 = primitive.points[n+1];
					const vec2d &p2 = primitive.points[n+2];
					std::fprintf(fp,"<polygon points=\"%g,%g %g,%g %g,%g\" style=\"fill:rgb(%d,%d,%d)\" %s/>\n",
							p0[0], p0[1], p1[0], p1[1], p2[0], p2[1], rgb[0], rgb[1], rgb[2], opacity_fill_string(primitive).c_str() );
				}
			}
		} else if( primitive.mode == MODE::TRIANGLE_FAN ) {
			if( primitive.points.size() >= 3 ) {
				for( unsigned n=1; n<primitive.points.size()-1; ++n ) {
					const vec2d &p0 = primitive.points[0];
					const vec2d &p1 = primitive.points[n];
					const vec2d &p2 = primitive.points[n+1];
					std::fprintf(fp,"<polygon points=\"%g,%g %g,%g %g,%g\" style=\"fill:rgb(%d,%d,%d)\" %s/>\n",
							p0[0], p0[1], p1[0], p1[1], p2[0], p2[1], rgb[0], rgb[1], rgb[2], opacity_fill_string(primitive).c_str() );
				}
			}
		}
	}
	//
	void write_string( FILE *fp, const String &primitive ) const {
		//
		int rgb[3];
		convert_integer_rgb(primitive.color,rgb);
		double font_size = primitive.size ? primitive.size : m_font_size;
		//
		std::fprintf(fp,"<text x=\"%g\" y=\"%g\" fill=\"rgb(%d,%d,%d)\" font-size=\"%g\" %s>%s</text>\n",
							primitive.p[0], primitive.p[1], rgb[0], rgb[1], rgb[2], font_size, primitive.string.c_str(), opacity_fill_string(primitive).c_str() );
	}
	//
	void write_example( void *ptr ) {
		//
		clear();
		setup_graphics();
		//
		color3(0.0,0.0,0.0);
		begin(graphics_engine::MODE::POINTS);
		vertex2(0.35,0.6);
		end();
		//
		point_size(2.0);
		color3(0.0,0.0,0.0);
		begin(graphics_engine::MODE::POINTS);
		vertex2(0.5,0.7);
		end();
		//
		point_size(4.0);
		color3(0.0,0.0,0.0);
		begin(graphics_engine::MODE::POINTS);
		vertex2(0.5,0.8);
		end();
		point_size(1.0);
		//
		line_width(1.0);
		color3(1.0,0.5,0.5);
		begin(graphics_engine::MODE::LINES);
		vertex2(0.0,0.5);
		vertex2(1.0,0.5);
		end();
		//
		line_width(4.0);
		color3(1.0,0.5,0.5);
		begin(graphics_engine::MODE::LINES);
		vertex2(0.0,0.4);
		vertex2(1.0,0.4);
		end();
		line_width(1.0);
		//
		color3(0.5,1.0,0.5);
		begin(graphics_engine::MODE::LINE_LOOP);
		vertex2(0.0,0.1);
		vertex2(0.25,0.25);
		vertex2(0.5,0.1);
		end();
		//
		color3(0.0,0.5,1.0);
		begin(graphics_engine::MODE::TRIANGLES);
		vertex2(0.1,0.1);
		vertex2(0.2,0.1);
		vertex2(0.15,0.2);
		end();
		//
		color3(0.5,0.5,0.5);
		begin(graphics_engine::MODE::TRIANGLE_STRIP);
		vertex2(0.3,0.1);
		vertex2(0.5,0.1);
		vertex2(0.45,0.2);
		vertex2(0.65,0.3);
		end();
		//
		color3(0.75,0.0,0.75);
		begin(graphics_engine::MODE::TRIANGLE_FAN);
		vertex2(0.3,0.4);
		vertex2(0.5,0.4);
		vertex2(0.45,0.5);
		vertex2(0.65,0.8);
		end();
		//
		color3(1.0,0.5,0.0);
		draw_string(vec3d(0.2,0.8,0.0).v,"Hello");
		//
		const_send_message("write",ptr);
	}
	//
	void convert_integer_rgb( const double *color, int *rgb ) const {
		for( unsigned n=0; n<3; ++n ) rgb[n] = 255.0*std::min(1.0,color[n]);
	}
	//
	ViewBox m_viewport;
	ViewBox m_coordsys;
	//
	std::vector<Primitive> m_primitives;
	std::vector<String> m_strings;
	//
	Primitive m_current_primitive;
	//
	unsigned m_canvas_width {1280}, m_canvas_height {1280};
	double m_scale {1.0};
	double m_point_size {1.0};
	double m_line_width {1.0};
	double m_point_scale {1.0};
	double m_line_scale {1.0};
	double m_font_size {30};
	bool m_enable_opacity {false};
	double m_color[4];
};
//
extern "C" module * create_instance() {
	return new graphics_svg;
}
//
extern "C" const char *license() {
	return "MIT";
}
//