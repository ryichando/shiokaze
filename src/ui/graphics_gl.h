/*
**	graphics_gl.h
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
#ifndef SHKZ_GRAPHICS_GL_H
#define SHKZ_GRAPHICS_GL_H
//
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include <GLFW/glfw3.h>
//
#include <shiokaze/graphics/graphics_engine.h>
//
SHKZ_BEGIN_NAMESPACE
//
class graphics_gl : public graphics_engine {
public:
	//
	graphics_gl ();
	//
	virtual void setup_graphics ( double r=0.0, double g=0.0, double b=0.0, double a=1.0 ) const override;
	//
	virtual void viewport(unsigned x, unsigned y, unsigned width, unsigned height) const override;
	virtual void ortho(double left, double right, double bottom, double top, double nearZ, double farZ) const override;
	//
	virtual void clear ( BUFFER_TYPE type=BUFFER_TYPE::COLOR_BUFFER_BIT ) const override;
	virtual void clear_stencil( int s ) const override;
	//
	void set_camera( const double target[3], const double position[3] );
	virtual void configure_view( unsigned width, unsigned height, unsigned dim ) const override;
	virtual void push_screen_coord ( unsigned width, unsigned height ) const override;
	virtual void pop_screen_coord () const override;
	//
	virtual void color3( double r, double g, double b ) const override;
	virtual void color4( double r, double g, double b, double a ) const override;
	//
	virtual void color3v( const double *v ) const override;
	virtual void color4v( const double *v ) const override;
	//
	virtual void raster3( double x, double y, double z ) const override;
	virtual void raster2( double x, double y ) const override;
	//
	virtual void raster3v( const double *v ) const override;
	virtual void raster2v( const double *v ) const override;
	//
	virtual void enable( CAPABILITY cap ) const override;
	virtual void disable( CAPABILITY cap ) const override;
	//
	virtual void begin( MODE mode ) const override;
	virtual void end() const override;
	virtual void point_size( double size ) const override;
	virtual void line_width( double width ) const override;
	//
	virtual void stencil_op( ACTION fail, ACTION pass ) const override;
	virtual void stencil_func( FUNC func, int ref, unsigned mask ) const override;
	//
	virtual void logic_op( OPERATION op ) const override;
	//
	virtual void blend_func( FACTOR sfactor, FACTOR dfactor ) const override;
	virtual void blend_color( double r, double g, double b, double a ) const override;
	//
	virtual void matrix_mode( MAT_MODE mode ) const override;
	virtual void load_identity() const override;
	virtual void push_matrix() const override;
	virtual void pop_matrix() const override;
	//
	virtual void translate( double x, double y, double z ) const override;
	virtual void scale( double x, double y, double z ) const override;
	virtual void multiply( const double *m ) const override;
	//
	virtual void vertex3( double x, double y, double z ) const override;
	virtual void vertex2( double x, double y ) const override;
	//
	virtual void vertex3v( const double *v ) const override;
	virtual void vertex2v( const double *v ) const override;
	//
	virtual void draw_string( const double *p, std::string str ) const override;
	//
	virtual void setHiDPIScalingFactor( double factor );
	virtual double get_HiDPI_scaling_factor() const override;
	//
private:
	//
	double m_HiDPI_factor;
	double m_position[3], m_target[3];
};
//
SHKZ_END_NAMESPACE
//
#endif
