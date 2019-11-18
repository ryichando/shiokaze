/*
**	meshutility2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 4, 2017.
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
#include <shiokaze/utility/meshutility2_interface.h>
//
SHKZ_USING_NAMESPACE
//
class meshutility2 : public meshutility2_interface {
private:
	//
	virtual void march_points( double v[2][2], const vec2d vertices[2][2], vec2d p[8], int &pnum, bool fill ) const override {
		//
		pnum = 0;
		int quads[][2] = { {0,0}, {1,0}, {1,1}, {0,1} };
		for( int n=0; n<4; n++ ) {
			// Inside Liquid
			if( fill ) {
				if( v[quads[n][0]][quads[n][1]] <= 0.0 ) {
					p[pnum] = vertices[quads[n][0]][quads[n][1]];
					pnum ++;
				}
			}
			// If Line Crossed
			if( std::copysign(1.0,v[quads[n][0]][quads[n][1]]) * std::copysign(1.0,v[quads[(n+1)%4][0]][quads[(n+1)%4][1]]) < 0.0 ) {
				// Calculate Cross Position
				double y0 = v[quads[n][0]][quads[n][1]];
				double y1 = v[quads[(n+1)%4][0]][quads[(n+1)%4][1]];
				if( y0-y1 ) {
					double a = y0/(y0-y1);
					vec2d p0 = vertices[quads[n][0]][quads[n][1]];
					vec2d p1 = vertices[quads[(n+1)%4][0]][quads[(n+1)%4][1]];
					p[pnum] = (1.0-a)*p0+a*p1;
					pnum ++;
				}
			}
		}
	}
	virtual void distance( const vec2d &p0, const vec2d &p1, vec2d &p ) const override {
		//
		vec2d a(p1[0]-p0[0],p1[1]-p0[1]);
		vec2d b(p[0]-p0[0],p[1]-p0[1]);
		double alen = a.len();
		if( alen ) {
			double dot = (a*b)/alen;
			dot = std::min(alen,std::max(0.0,dot));
			p = p0 + (a/alen) * dot;
		} else {
			p = p0;
		}
	}
};
//
extern "C" module * create_instance() {
	return new meshutility2();
}
//
extern "C" const char *license() {
	return "MIT";
}