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
	virtual bool keyboard( int key, int action, int mods ) override {
		// Dump keyboard event
		if( key < 255 ) {
			console::dump( "Keyboard %c action = %d, mods = %d\n", key, action, mods );
		} else {
			console::dump( "Keyboard = special, action = %d, mods = %d\n", action, mods );
		}
		return false;
	};
	//
	virtual void idle() override {
		// Do nothing for now
	};
	//
	virtual void cursor( double x, double y, double z ) override {
		m_mouse_pos = vec2d(x,y);
	};
	//
	virtual void mouse( double x, double y, double z, int button, int action, int mods ) override {
		//
		// Dump mouse event
		console::dump( "button = %d, action = %d, mods = %d, mouse = (%.2f,%.2f)\n", button, action, mods, m_mouse_pos[0], m_mouse_pos[1] );
	};
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		// Draw a message
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec2d(0.025,0.025).v, "This is a demo window");
		//
		// Draw a point at the mouse location
		g.point_size(2.0);
		g.begin(graphics_engine::MODE::POINTS);
		g.vertex2v(m_mouse_pos.v);
		g.end();
		g.point_size(1.0);
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
