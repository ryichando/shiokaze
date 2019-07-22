/*
**	ui.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 9, 2017. 
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
//	TODO: This class should be cleaned up!
//
#ifndef SHKZ_UI_H
#define SHKZ_UI_H
//
#include <shiokaze/image/image_io_interface.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/configuration.h>
#include <shiokaze/system/sysstats_interface.h>
#include <shiokaze/math/vec.h>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
class drawable;
class ui {
public:
	//
	// Run the whole thing (designed to be called directly from main() func)
	static int run ( int argc, const char* argv[] );
	//
	// Get a pointer to the drawable instance
	drawable *get_instance() const { return m_instance; }
	//
	// Get a pointer to the graphics enegine
	graphics_engine *get_graphics_engine() const { return m_graphics_instance; }
	//
private:
	//
	ui ( drawable *instance );
	virtual ~ui();
	//
	void load( configuration &config );
	void configure( configuration &config );
	void run ();
	//
	drawable *m_instance;
	graphics_engine *m_graphics_instance;
	image_io_ptr m_image_io;
	//
	std::string m_screenshot_path;
	int m_until, m_frame, m_step;
	double m_window_scale;
	bool m_show_logo;
	bool m_paused {false};
	//
public:
	//
	int m_accumulation {0};
	vec2d m_pos0;
	vec2d m_mouse_pos;
};
//
SHKZ_END_NAMESPACE
//
#endif