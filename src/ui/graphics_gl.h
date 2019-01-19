
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
	virtual void setup_graphics ( double r=0.0, double g=0.0, double b=0.0, double a=1.0 ) override;
	virtual void configure_view( unsigned width, unsigned height, unsigned dim ) override;
	virtual void clear() override;
	//
	virtual void color4v( const double *v ) override;
	virtual void vertex3v( const double *v ) override;
	//
	virtual void begin( MODE mode ) override;
	virtual void end() override;
	//
	virtual void point_size( double size ) override;
	virtual void line_width( double width ) override;
	//
	virtual void draw_string( const double *v, std::string str ) const override;
	//
	void setHiDPIScalingFactor( double factor );
	double get_HiDPI_scaling_factor() const;
	//
	void set_camera( const double target[3], const double position[3] );
	//
private:
	//
	double m_HiDPI_factor;
	double m_position[3], m_target[3];
	int m_dimension;
};
//
SHKZ_END_NAMESPACE
//
#endif
