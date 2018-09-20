/*
**	meshutility2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 4, 2017. 
**
**	meshutility2::intersection is originally written from:
**	http://flassari.is/2008/11/line-line-intersection-in-cplusplus/
*/
//
#include <shiokaze/utility/meshutility2_interface.h>
//
SHKZ_USING_NAMESPACE
//
class meshutility2 : public meshutility2_interface {
private:
	//
	AUTHOR_NAME("flassari.is")
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
					assert( a >= 0.0 && a <= 1.0 );
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
	// http://flassari.is/2008/11/line-line-intersection-in-cplusplus/
	virtual bool intersection ( const vec2d &p1, const vec2d &p2, const vec2d &p3, const vec2d &p4, vec2d &out ) const override {
		//
		// Store the values for fast access and easy
		// equations-to-code conversion
		double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
		double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];
		double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		//
		// If d is zero, there is no intersection
		if ( ! d ) return false;
		//
		// Get the x and y
		double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
		double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
		double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
		//
		// Check if the x and y coordinates are within both lines
		if ( x < std::min(x1, x2) || x > std::max(x1, x2) || x < std::min(x3, x4) || x > std::max(x3, x4) ) return false;
		if ( y < std::min(y1, y2) || y > std::max(y1, y2) || y < std::min(y3, y4) || y > std::max(y3, y4) ) return false;
		//
		// Return the point of intersection
		out = vec2d(x,y);
		return true;
	}
	//
};
//
extern "C" module * create_instance() {
	return new meshutility2();
}
//
extern "C" const char *license() {
	return "Unknown";
}