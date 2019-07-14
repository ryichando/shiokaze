/*
**	demo-example.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 2, 2017.
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
#include <shiokaze/ui/drawable.h>
#include <shiokaze/core/console.h>
#include <shiokaze/math/vec.h>
//
SHKZ_USING_NAMESPACE
//
class demo : public drawable {
private:
	//
	LONG_NAME("Demo")
	ARGUMENT_NAME("Demo")
	//
	virtual void load( configuration &config ) override {
		console::dump( "Loading demo...\n");
	}
	//
	virtual void configure( configuration &config ) override {
		console::dump( "Configuring demo...\n");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		console::dump( "Initializing demo...\n");
	};
	//
	virtual bool keyboard ( char key ) override {
		// Dump keyboard event
		console::dump( "Keyboard %c\n", key );
		return true;
	};
	//
	virtual void idle() override {
		// Do nothing for now
	};
	//
	virtual void cursor( int width, int height, double x, double y ) override {
		// Record the mouse location
		m_mouse_pos = vec2d(x,y);
	};
	//
	virtual void mouse( int width, int height, double x, double y, int button, int action ) override {
		// Dump mouse event
		console::dump( "Mouse (x,y) = (%.2f,%.2f), (%d), (%d)\n", x, y, button, action );
	};
	//
	virtual void draw( graphics_engine &g, int width, int height ) const override {
		//
		// Draw a message
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec2d(0.025,0.025).v, "This is a demo window");
		//
		// Draw a point at the mouse location
		g.point_size(2.0);
		g.begin(graphics_engine::MODE::POINTS);
		g.vertex3v(m_mouse_pos.v);
		g.end();
		g.point_size(1.0);
		//
	};
	//
	vec2d m_mouse_pos;
};
//
extern "C" module * create_instance() {
	return new demo;
}
//
extern "C" const char *license() {
	return "MIT";
}
