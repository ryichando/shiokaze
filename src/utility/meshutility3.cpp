/*
**	meshutility3.cpp
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
#include <shiokaze/utility/meshutility3_interface.h>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
class meshutility3 : public meshutility3_interface {
private:
	//
	AUTHOR_NAME("Christopher Batty")
	//
	virtual double point_segment_distance(const vec3d &x0, const vec3d &x1, const vec3d &x2, vec3d &out ) const override {
		//
		vec3d dx(x2-x1);
		double m2=dx.norm2();
		if( ! m2 ) {
			out = x1;
			return (x0-out).len();
		}
		// find parameter value of closest point on segment
		double s12=(x2-x0)*dx/m2;
		if(s12<0){
			s12=0;
		}else if(s12>1){
			s12=1;
		}
		// and find the distance
		out = s12*x1+(1-s12)*x2;
		return (x0-out).len();
	}
	//
	virtual double point_triangle_distance(const vec3d &x0, const vec3d &x1, const vec3d &x2, const vec3d &x3, vec3d &out ) const override {
		//
		// first find barycentric coordinates of closest point on infinite plane
		vec3d x13(x1-x3), x23(x2-x3), x03(x0-x3);
		double m13=x13.norm2(), m23=x23.norm2(), d=x13*x23;
		double invdet=1.0/fmax(m13*m23-d*d,1e-30);
		double a=x13*x03, b=x23*x03;
		// the barycentric coordinates themselves
		double w23=invdet*(m23*a-d*b);
		double w31=invdet*(m13*b-d*a);
		double w12=1-w23-w31;
		if(w23>=0 && w31>=0 && w12>=0){ // if we're inside the triangle
			out = w23*x1+w31*x2+w12*x3;
			return (x0-out).len();
		}else{ // we have to clamp to one of the edges
			if(w23>0) // this rules out edge 2-3 for us
				return fmin(point_segment_distance(x0,x1,x2,out), point_segment_distance(x0,x1,x3,out));
			else if(w31>0) // this rules out edge 1-3
				return fmin(point_segment_distance(x0,x1,x2,out), point_segment_distance(x0,x2,x3,out));
			else // w12 must be >0, ruling out edge 1-2
				return fmin(point_segment_distance(x0,x1,x3,out), point_segment_distance(x0,x2,x3,out));
		}
	}
	//
};
//
extern "C" module * create_instance() {
	return new meshutility3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//