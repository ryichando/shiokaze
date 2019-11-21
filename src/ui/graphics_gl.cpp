/*
**	graphics_gl.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 13, 2018.
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
#include "graphics_gl.h"
#include <cstdio>
//
SHKZ_USING_NAMESPACE
//
graphics_gl::graphics_gl () {
	//
	set_HiDPI_scaling_factor(1.0);
	m_ratio = 1.0;
	m_x = m_y = m_width = m_height = 0;
}
//
void graphics_gl::set_HiDPI_scaling_factor( double factor ) {
	//
	m_HiDPI_factor = factor;
}
//
void graphics_gl::setup_graphics ( std::map<std::string,const void *> params ) {
	//
	glClearColor(0.0,0.0,0.0,0.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	point_size(1.0);
	line_width(1.0);
}
//
bool graphics_gl::get_supported ( FEATURE feature ) const {
	//
	if( feature == FEATURE::OPACITY ) {
		return true;
	} else if( feature == FEATURE::_3D ) {
		return true;
	}
	return false;
}
//
void graphics_gl::set_2D_coordinate( double left, double right, double bottom, double top ) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(left,right,bottom,top,-1.0,1.0);
}
//
void graphics_gl::set_viewport( unsigned x, unsigned y, unsigned width, unsigned height ) {
	glViewport(x,y,width,height);
	m_x = x;
	m_y = y;
	m_width = width;
	m_height = height;
	m_ratio = width / (double) height;
}
//
void graphics_gl::get_viewport( unsigned &x, unsigned &y, unsigned &width, unsigned &height ) const {
	x = m_x;
	y = m_y;
	width = m_width;
	height = m_height;
}
//
void graphics_gl::look_at( const double target[3], const double position[3], const double up[3], double fov, double near, double far ) {
	//
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov,m_ratio,near,far);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt( position[0], position[1], position[2], target[0], target[1], target[2], up[0], up[1], up[2] );
}
//
void graphics_gl::clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}
//
void graphics_gl::get_background_color( double color[3] ) const {
	color[0] = color[1] = color[2] = 0.0;
}
//
void graphics_gl::get_foreground_color( double color[3] ) const {
	color[0] = color[1] = color[2] = 1.0;
}
//
void graphics_gl::color4v( const double *v ) {
	glColor4dv(v);
}
//
void graphics_gl::begin( MODE mode ) {
	auto convert_mode = []( MODE mode ) {
		switch(mode) {
			case MODE::POINTS:
				return GL_POINTS;
			case MODE::LINES:
				return GL_LINES;
			case MODE::LINE_STRIP:
				return GL_LINE_STRIP;
			case MODE::LINE_LOOP:
				return GL_LINE_LOOP;
			case MODE::TRIANGLES:
				return GL_TRIANGLES;
			case MODE::TRIANGLE_STRIP:
				return GL_TRIANGLE_STRIP;
			case MODE::TRIANGLE_FAN:
				return GL_TRIANGLE_FAN;
			default:
				throw;
		}
	};
	glBegin(convert_mode(mode));
}
//
void graphics_gl::end() {
	glEnd();
}
//
void graphics_gl::point_size( double size ) {
	glPointSize(m_HiDPI_factor*size);
}
//
void graphics_gl::line_width( double width ) {
	glLineWidth(m_HiDPI_factor*width);
}
//
void graphics_gl::vertex3v( const double *v ) {
	glVertex3dv(v);
}
//
void graphics_gl::draw_string( const double *v, std::string str ) {
	const char *str_ptr = str.c_str();
	glRasterPos3dv(v);
	auto font = GLUT_BITMAP_HELVETICA_10;
	if( m_HiDPI_factor > 1.2 ) font = GLUT_BITMAP_HELVETICA_12;
	if( m_HiDPI_factor > 1.8 ) font = GLUT_BITMAP_HELVETICA_18;
	while (*str_ptr) glutBitmapCharacter(font, *str_ptr++);
}
//
double graphics_gl::get_HiDPI_scaling_factor() const {
	return m_HiDPI_factor;
}
//