/*
**	scoped_timer.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 15, 2018.
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
#include <shiokaze/core/scoped_timer.h>
#include <shiokaze/core/global_timer.h>
#include <sys/time.h>
#include <cstring>
#include <cassert>
//
SHKZ_USING_NAMESPACE
//
static double g_accumulated_time (0.0), g_pause_time (0.0);
static int g_pause_counter (0);
//
scoped_timer::scoped_timer( std::string master_name, std::string secondary_name ) : m_master_name(master_name) {
	if( ! secondary_name.empty()) m_master_name += "_" + secondary_name;
	m_pause_counter = g_pause_counter;
}
//
scoped_timer::scoped_timer( const credit *cr, std::string secondary_name ) : scoped_timer(cr->get_argument_name(),secondary_name) {}
//
scoped_timer::~scoped_timer() {
	assert( m_pause_counter == g_pause_counter );
}
//
void scoped_timer::tick() {
	m_time_stack.push(global_timer::get_milliseconds());
}
//
double scoped_timer::tock( std::string subname ) {
	assert( ! m_time_stack.empty());
	double t = m_time_stack.top();
	double duration = global_timer::get_milliseconds()-t;
	if( ! subname.empty()) {
		if( m_master_name.empty() ) console::write(subname,t);
		else console::write(m_master_name+"_"+subname,duration);
	}
	m_time_stack.pop();
	return duration;
}
//
std::string scoped_timer::stock( std::string subname ) {
	return console::tstr(tock(subname));
}
//
