/*
**	deadline.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 31, 2017.
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
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <shiokaze/core/deadline.h>
//
SHKZ_USING_NAMESPACE
//
void deadline::add_deadline( std::string name, int month, int day ) {
	//
	_deadline d;
	d.name = name;
	d.month = month;
	d.day = day;
	m_deadlines.push_back(d);
	//
}
//
void deadline::get_next_deadline( std::string &name, int &_days ) const {
	//
	using namespace boost::gregorian;
	date today = day_clock::universal_day();
	//
	unsigned min_days (365);
	for( const auto &d : m_deadlines ) {
		//
		partial_date deadline_day(d.day,d.month);
		unsigned year = today.year();
		//
		int dday = (deadline_day.get_date(year) - today).days();
		if( dday < 0 ) {
			dday = (deadline_day.get_date(year+1) - today).days();
		}
		//
		if( dday < min_days ) {
			min_days = _days = dday;
			name = d.name;
		}
	}
}