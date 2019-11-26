/*
**	matinv.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 7, 2017.
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
// A[i][j] should correspond to A_ij
//
#ifndef SHKZ_MATINV
#define SHKZ_MATINV
//
#include <shiokaze/core/common.h>
//
SHKZ_BEGIN_NAMESPACE
//
template<class T> class matinv {
public:
	static bool invert2x2( const T A[2][2], T result[2][2] ) {
		T det = A[0][0]*A[1][1]-A[0][1]*A[1][0];
		if( det ) {
			result[0][0] = A[1][1]/det;
			result[1][0] = -A[1][0]/det;
			result[1][1] = A[0][0]/det;
			result[0][1] = -A[0][1]/det;
		}
		return det;
	}
	//
	static bool invert3x3( const T A[3][3], T result[3][3] ) {
		T determinant = determinant3x3(A);
		if ( ! determinant ) return false;
		T invdet = 1.0/determinant;
		result[0][0] =  (A[1][1]*A[2][2]-A[1][2]*A[2][1])*invdet;
		result[1][0] = -(A[1][0]*A[2][2]-A[2][0]*A[1][2])*invdet;
		result[2][0] =  (A[1][0]*A[2][1]-A[2][0]*A[1][1])*invdet;
		result[0][1] = -(A[0][1]*A[2][2]-A[2][1]*A[0][2])*invdet;
		result[1][1] =  (A[0][0]*A[2][2]-A[2][0]*A[0][2])*invdet;
		result[2][1] = -(A[0][0]*A[2][1]-A[0][1]*A[2][0])*invdet;
		result[0][2] =  (A[0][1]*A[1][2]-A[0][2]*A[1][1])*invdet;
		result[1][2] = -(A[0][0]*A[1][2]-A[0][2]*A[1][0])*invdet;
		result[2][2] =  (A[0][0]*A[1][1]-A[0][1]*A[1][0])*invdet;
		return true;
	}
	//
	static bool invert4x4( const T A[4][4], T result[4][4] ) {
		T M[16];
		T Minv[16];
		for( uint i=0; i<4; i++ ) {
			for( uint j=0; j<4; j++ ) {
				M[i+4*j] = A[i][j];
			}
		}
		if( ! myInvertMatrix(M,Minv)) return false;
		for( uint i=0; i<4; i++ ) {
			for( uint j=0; j<4; j++ ) {
				result[i][j] = Minv[i+4*j];
			}
		}
		return true;
	}
	//
	static T determinant3x3( const T A[3][3] ) {
		return A[0][0]*(A[1][1]*A[2][2]-A[1][2]*A[2][1])
		-A[1][0]*(A[0][1]*A[2][2]-A[2][1]*A[0][2])
		+A[2][0]*(A[0][1]*A[1][2]-A[1][1]*A[0][2]);
	}
	//
private:
	//
	// Original: gluInvertMatrix() copied from
	// http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
	static bool myInvertMatrix(const T m[16], T invOut[16]) {
		T inv[16], det; int i;
		inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
		+ m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
		inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
		- m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
		inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
		+ m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
		inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
		- m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
		inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
		- m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
		inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
		+ m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
		inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
		- m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
		inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
		+ m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
		inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
		+ m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
		inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
		- m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
		inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
		+ m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
		inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
		- m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
		inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
		- m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
		inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
		+ m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
		inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
		- m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
		inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
		+ m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
		det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
		if (det == 0) return false;
		det = 1.0 / det;
		for (i = 0; i < 16; i++) invOut[i] = inv[i] * det;
		return true;
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
