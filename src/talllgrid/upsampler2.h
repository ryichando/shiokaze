/*
**	upsampler2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 17, 2017. 
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
#ifndef SHKZ_UPSAMPLER2_H
#define SHKZ_UPSAMPLER2_H
//
#include <shiokaze/array/array2.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <functional>
//
SHKZ_BEGIN_NAMESPACE
//
class upsampler2 {
public:
	void build_upsampler( const array2<double> &fluid, double dx, int narrowband=6 );
	std::function<bool( const vec2i &pi, std::vector<size_t> &indices, std::vector<double> &coefficients, std::vector<vec2d> &positions )> get_upsampler () const;
	size_t get_index_size() const { return m_index; }
	void draw( graphics_engine &g ) const;
	//
private:
	typedef struct {
		int start, end;
		size_t index;
		bool active;
	} tallcell;
	//
	std::vector<tallcell> m_tallcells;
	array2<size_t> m_index_map;
	size_t m_index {0};
	double m_dx;
};
//
SHKZ_END_NAMESPACE
//
#endif