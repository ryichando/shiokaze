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
	setHiDPIScalingFactor(1.0);
	m_position[0] = -1.0;
	m_position[1] = 1.3;
	m_position[2] = -1.5;
	m_target[0] = 0.5;
	m_target[1] = 0.25;
	m_target[2] = 0.5;
}
//
void graphics_gl::setHiDPIScalingFactor( double factor ) {
	m_HiDPI_factor = factor;
}
//
void graphics_gl::setup_graphics ( double r, double g, double b, double a ) const {
	glClearColor(r,g,b,a);
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
void graphics_gl::viewport(unsigned x, unsigned y, unsigned width, unsigned height) const {
	glViewport(x,y,width,height);
}
//
void graphics_gl::ortho(double left, double right, double bottom, double top, double nearZ, double farZ) const {
	glOrtho(left,right,bottom,top,nearZ,farZ);
}
//
void graphics_gl::clear ( BUFFER_TYPE type ) const {
	GLbitfield mask;
	switch(type) {
		case BUFFER_TYPE::COLOR_BUFFER_BIT:
			mask = GL_COLOR_BUFFER_BIT;
			break;
		case BUFFER_TYPE::DEPTH_BUFFER_BIT:
			mask = GL_DEPTH_BUFFER_BIT;
			break;
		case BUFFER_TYPE::STENCIL_BUFFER_BIT:
			mask = GL_STENCIL_BUFFER_BIT;
			break;
	}
	glClear(mask);
}
//
void graphics_gl::clear_stencil( int s ) const {
	glClearStencil(s);
}
//
void graphics_gl::set_camera( const double target[3], const double position[3] ) {
	for( int i=0; i<3; ++i ) m_target[i] = target[i];
	for( int i=0; i<3; ++i ) m_position[i] = position[i];
}
//
void graphics_gl::configure_view( unsigned width, unsigned height, unsigned dim ) const {
	if( dim == 2 ) {
		glViewport(0,0,width,height);
		glLoadIdentity();
		glOrtho(0.0,1.0,0.0,height / (double)width,-1.0,1.0);
	} else if ( dim == 3 ) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective( 35, width / (double) height, 1, 1000 );
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glViewport( 0, 0, width, height );
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glScaled(-1.0, 1.0, 1.0);
		gluLookAt( m_position[0], m_position[1], m_position[2], m_target[0], m_target[1], m_target[2], 0, 1, 0 );
	}
}
//
void graphics_gl::push_screen_coord ( unsigned width, unsigned height ) const {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);	
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0,width,height,0.0,-1.0,1.0);
}
//
void graphics_gl::pop_screen_coord () const {
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}
//
void graphics_gl::color3( double r, double g, double b ) const {
	glColor3d(r,g,b);
}
//
void graphics_gl::color4( double r, double g, double b, double a ) const {
	glColor4d(r,g,b,a);
}
//
void graphics_gl::color3v( const double *v ) const {
	glColor3dv(v);
}
//
void graphics_gl::color4v( const double *v ) const {
	glColor4dv(v);
}
//
void graphics_gl::raster3( double x, double y, double z ) const {
	glRasterPos3d(x,y,z);
}
//
void graphics_gl::raster2( double x, double y ) const {
	glRasterPos2d(x,y);
}
//
void graphics_gl::raster3v( const double *v ) const {
	glRasterPos3dv(v);
}
//
void graphics_gl::raster2v( const double *v ) const {
	glRasterPos2dv(v);
}
//
void graphics_gl::enable( CAPABILITY cap ) const {
	switch(cap) {
		case CAPABILITY::BLEND:
			glEnable(GL_BLEND);
			break;
		case CAPABILITY::DEPTH_TEST:
			glEnable(GL_DEPTH_TEST);
			break;
		case CAPABILITY::STENCIL_TEST:
			glEnable(GL_STENCIL_TEST);
			break;
		case CAPABILITY::COLOR_LOGIC_OP:
			glEnable(GL_COLOR_LOGIC_OP);
			break;
	}
}
//
void graphics_gl::disable( CAPABILITY cap ) const {
	switch(cap) {
		case CAPABILITY::BLEND:
			glDisable(GL_BLEND);
			break;
		case CAPABILITY::DEPTH_TEST:
			glDisable(GL_DEPTH_TEST);
			break;
		case CAPABILITY::STENCIL_TEST:
			glDisable(GL_STENCIL_TEST);
			break;
		case CAPABILITY::COLOR_LOGIC_OP:
			glDisable(GL_COLOR_LOGIC_OP);
			break;
	}
}
//
void graphics_gl::begin( MODE mode ) const {
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
			case MODE::QUADS:
				return GL_QUADS;
			case MODE::QUAD_STRIP:
				return GL_QUAD_STRIP;
			case MODE::POLYGON:
				return GL_POLYGON;
			default:
				throw;
		}
	};
	glBegin(convert_mode(mode));
}
//
void graphics_gl::end() const {
	glEnd();
}
//
void graphics_gl::point_size( double size ) const {
	glPointSize(m_HiDPI_factor*size);
}
//
void graphics_gl::line_width( double width ) const {
	glLineWidth(m_HiDPI_factor*width);
}
//
void graphics_gl::stencil_op( ACTION fail, ACTION pass ) const {
	auto convert_action = []( ACTION action ) {
		switch(action) {
			case ACTION::KEEP:
				return GL_KEEP;
			case ACTION::ZERO:
				return GL_ZERO;
			case ACTION::REPLACE:
				return GL_REPLACE;
			case ACTION::INCR:
				return GL_INCR;
			case ACTION::INCR_WRAP:
				return GL_INCR_WRAP;
			case ACTION::DECR:
				return GL_DECR;
			case ACTION::DECR_WRAP:
				return GL_DECR_WRAP;
			case ACTION::INVERT:
				return GL_INVERT;
		}
	};
	glStencilOp(convert_action(fail),convert_action(pass),convert_action(pass));
}
//
void graphics_gl::stencil_func( FUNC func, int ref, unsigned mask ) const {
	//
	switch(func) {
		case FUNC::NEVER:
			glStencilFunc(GL_NEVER,ref,mask);
			break;
		case FUNC::LESS:
			glStencilFunc(GL_LESS,ref,mask);
			break;
		case FUNC::LEQUAL:
			glStencilFunc(GL_LEQUAL,ref,mask);
			break;
		case FUNC::GREATER:
			glStencilFunc(GL_GREATER,ref,mask);
			break;
		case FUNC::GEQUAL:
			glStencilFunc(GL_GEQUAL,ref,mask);
			break;
		case FUNC::EQUAL:
			glStencilFunc(GL_EQUAL,ref,mask);
			break;
		case FUNC::NOTEQUAL:
			glStencilFunc(GL_NOTEQUAL,ref,mask);
			break;
		case FUNC::ALWAYS:
			glStencilFunc(GL_ALWAYS,ref,mask);
			break;
	}
}
//
void graphics_gl::logic_op( OPERATION op ) const {
	//
	auto convert_operation = []( OPERATION op ) {
		switch(op) {
			case OPERATION::CLEAR:
				return GL_CLEAR;
			case OPERATION::SET:
				return GL_SET;
			case OPERATION::COPY:
				return GL_COPY;
			case OPERATION::COPY_INVERTED:
				return GL_COPY_INVERTED;
			case OPERATION::NOOP:
				return GL_NOOP;
			case OPERATION::INVERT:
				return GL_INVERT;
			case OPERATION::AND:
				return GL_AND;
			case OPERATION::NAND:
				return GL_NAND;
			case OPERATION::OR:
				return GL_OR;
			case OPERATION::NOR:
				return GL_NOR;
			case OPERATION::XOR:
				return GL_XOR;
			case OPERATION::EQUIV:
				return GL_EQUIV;
			case OPERATION::AND_REVERSE:
				return GL_AND_REVERSE;
			case OPERATION::AND_INVERTED:
				return GL_AND_INVERTED;
			case OPERATION::OR_REVERSE:
				return GL_OR_REVERSE;
			case OPERATION::OR_INVERTED:
				return GL_OR_INVERTED;
		}
	};
	glLogicOp(convert_operation(op));
}
//
void graphics_gl::blend_func( FACTOR sfactor, FACTOR dfactor ) const {
	auto convert_factor = []( FACTOR factor ) {
		switch(factor) {
			case FACTOR::ZERO:
				return GL_ZERO;
			case FACTOR::ONE:
				return GL_ONE;
			case FACTOR::SRC_COLOR:
				return GL_SRC_COLOR;
			case FACTOR::ONE_MINUS_SRC_COLOR:
				return GL_ONE_MINUS_SRC_COLOR;
			case FACTOR::DST_COLOR:
				return GL_DST_COLOR;
			case FACTOR::ONE_MINUS_DST_COLOR:
				return GL_ONE_MINUS_DST_COLOR;
			case FACTOR::SRC_ALPHA:
				return GL_SRC_ALPHA;
			case FACTOR::ONE_MINUS_SRC_ALPHA:
				return GL_ONE_MINUS_SRC_ALPHA;
			case FACTOR::DST_ALPHA:
				return GL_DST_ALPHA;
			case FACTOR::ONE_MINUS_DST_ALPHA:
				return GL_ONE_MINUS_DST_ALPHA;
		}
	};
	glBlendFunc(convert_factor(sfactor),convert_factor(dfactor));
}
//
void graphics_gl::blend_color( double r, double g, double b, double a ) const {
	glBlendColor(r,g,b,a);
}
//
void graphics_gl::matrix_mode( MAT_MODE mode ) const {
	switch(mode) {
		case MAT_MODE::MODELVIEW:
			glMatrixMode(GL_MODELVIEW);
			break;
		case MAT_MODE::PROJECTION:
			glMatrixMode(GL_PROJECTION);
			break;
	}
}
//
void graphics_gl::load_identity() const {
	glLoadIdentity();
}
//
void graphics_gl::push_matrix() const {
	glPushMatrix();
}
//
void graphics_gl::pop_matrix() const {
	glPopMatrix();
}
//
void graphics_gl::translate( double x, double y, double z ) const {
	glTranslated(x,y,z);
}
//
void graphics_gl::scale( double x, double y, double z ) const {
	glScaled(x,y,z);
}
//
void graphics_gl::multiply( const double *m ) const {
	glMultMatrixd(m);
}
//
void graphics_gl::vertex3( double x, double y, double z ) const {
	glVertex3d(x,y,z);
}
//
void graphics_gl::vertex2( double x, double y ) const {
	glVertex2d(x,y);
}
//
void graphics_gl::vertex3v( const double *v ) const {
	glVertex3dv(v);
}
//
void graphics_gl::vertex2v( const double *v ) const {
	glVertex2dv(v);
}
//
void graphics_gl::draw_string( const double *p, std::string str ) const {
	const char *str_ptr = str.c_str();
	glRasterPos2dv(p);
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