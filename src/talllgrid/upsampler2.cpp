/*
**	upsampler2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Dec 27, 2018.
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
**/
//
#include "upsampler2.h"
#include <algorithm>
//
SHKZ_USING_NAMESPACE
//
void upsampler2::build_upsampler( const array2<double> &fluid, double dx, int narrowband ) {
	//
	m_tallcells.resize(fluid.shape().w);
	for( auto &e : m_tallcells ) {
		e.start = -1;
		e.end = -1;
		e.index = 0;
		e.active = false;
	}
	m_dx = dx;
	//
	fluid.const_serial_inside([&]( int i, int j, const auto &it ) {
		if( m_tallcells[i].active ) {
			m_tallcells[i].start = std::min(m_tallcells[i].start,j);
		} else {
			m_tallcells[i].active = true;
			m_tallcells[i].start = j;
		}
	});
	//
	for( int i=0; i<m_tallcells.size(); ++i ) {
		if( m_tallcells[i].active ) {
			int j = m_tallcells[i].start;
			while( j < fluid.shape().h && fluid(i,j) < 0.0 ) ++ j;
			//
			m_tallcells[i].start += 3;
			m_tallcells[i].end = j-narrowband-1;
			//
			if( m_tallcells[i].end <= m_tallcells[i].start ) {
				m_tallcells[i].start = m_tallcells[i].end = -1;
				m_tallcells[i].active = false;
			}
		}
	}
	//
	m_index = 0;
	for( int i=0; i<m_tallcells.size(); ++i ) {
		if( m_tallcells[i].active ) {
			m_tallcells[i].index = m_index;
			m_index += 2;
		}
	}
	m_index_map.initialize(fluid.shape());
	fluid.const_serial_inside([&]( int i, int j, const auto &it ) {
		if( ! m_tallcells[i].active || j < m_tallcells[i].start || j > m_tallcells[i].end ) {
			m_index_map.set(i,j,m_index++);
		}
	});
}
//
std::function<bool( const vec2i &pi, std::vector<size_t> &indices, std::vector<double> &coefficients, std::vector<vec2d> &positions )> upsampler2::get_upsampler () const {
	return [&]( const vec2i &pi, std::vector<size_t> &indices, std::vector<double> &coefficients, std::vector<vec2d> &positions ) {
		int i = pi[0];
		int j = pi[1];
		//
		if( m_tallcells[i].active && (j >= m_tallcells[i].start && j <= m_tallcells[i].end )) {
			auto& tallcell = m_tallcells[i];
			double theta = (j - tallcell.start) / (double) (tallcell.end - tallcell.start);
			if( 1.0-theta ) {
				indices.push_back(tallcell.index);
				coefficients.push_back(1.0-theta);
				positions.push_back(m_dx*vec2i(i,tallcell.start).cell());
			}
			if( theta ) {
				indices.push_back(tallcell.index+1);
				coefficients.push_back(theta);
				positions.push_back(m_dx*vec2i(i,tallcell.end).cell());
			}
			return true;
		} else {
			if( m_index_map.active(i,j)) {
				indices.push_back(m_index_map(i,j));
				coefficients.push_back(1.0);
				positions.push_back(m_dx*vec2i(i,j).cell());
				return true;
			}
		}
		return false;
	};
}
//
void upsampler2::draw( graphics_engine &g ) const {
	//
	for( int i=0; i<m_tallcells.size(); ++i ) {
		const auto &e = m_tallcells[i];
		if( e.start < e.end ) {
			//
			g.color4(1.0,1.0,1.0,1.0);
			g.begin(graphics_engine::MODE::LINE_LOOP);
			g.vertex2v((m_dx*vec2d(i,e.start)).v);
			g.vertex2v((m_dx*vec2d(i+1,e.start)).v);
			g.vertex2v((m_dx*vec2d(i+1,e.end+1)).v);
			g.vertex2v((m_dx*vec2d(i,e.end+1)).v);
			g.end();
		}
	}
}
//