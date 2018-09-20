/*
**	global_timer.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 16, 2018.
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
#include <shiokaze/core/console.h>
#include <shiokaze/core/global_timer.h>
#include <cassert>
#include <sys/time.h>
//
SHKZ_USING_NAMESPACE
//
static double g_accumulated_time (0.0), g_pause_time (0.0);
static int g_pause_counter (0);
//
void global_timer::pause() {
	if( ! g_pause_counter ) {
		g_pause_time = get_milliseconds();
	}
	++ g_pause_counter;
}
//
void global_timer::resume() {
	assert( g_pause_counter );
	-- g_pause_counter;
	if( ! g_pause_counter ) {
		g_accumulated_time += get_milliseconds() - g_pause_time;
	}
}
//
double global_timer::get_milliseconds() {
	if( g_pause_counter ) {
		return g_pause_time;
	} else {
		struct timeval tv;
		gettimeofday(&tv,nullptr);
		double mtime = tv.tv_sec * 1000000 + tv.tv_usec;
		return mtime / 1000.0 - g_accumulated_time;
	}
}
//
