/*
**	DenseMatrix.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 14, 2017. 
**
**	License should follow both:
**	https://stackexchange.com/legal
**	https://creativecommons.org/licenses/by-nc-nd/2.5/in/deed.en_US
*/
//
#ifndef SHKZ_DENSEMATRIX_H
#define SHKZ_DENSEMATRIX_H
//
#include <array>
#include <vector>
#include <cassert>
#include <shiokaze/math/vec.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/core/common.h>
//
SHKZ_BEGIN_NAMESPACE
//
template<class T, int N> class SquareDenseMatrix {
public:
	SquareDenseMatrix () {
		clear();
	}
	static SquareDenseMatrix identity() {
		SquareDenseMatrix result;
		for( int i=0; i<N; ++i ) result(i,i) = T(1.0);
		return result;
	}
	void clear() {
		for( int i=0; i<N; ++i ) for( int j=0; j<N; ++j ) m[i][j] = T();
	}
	bool empty() const {
		for( int i=0; i<N; ++i ) for( int j=0; j<N; ++j ) if(m[i][j]) return false;
		return true;
	}
	const T& operator()(unsigned i, unsigned j) const {
		return m[i][j];
	}
	T& operator()(unsigned i, unsigned j) {
		return m[i][j];
	}
	void apply( T rhs[N] ) const {
		T save[N];
		for( int i=0; i<N; ++i ) save[i] = rhs[i];
		for( int i=0; i<N; ++i ) {
			rhs[i] = 0.0;
			for( int j=0; j<N; ++j ) {
				rhs[i] += m[i][j]*save[j];
			}
		}
	}
	void apply( std::vector<T> &rhs ) const {
		assert( rhs.size() == N );
		apply(rhs.data());
	}
	void apply( vec2<T> &rhs ) const {
		assert( N == 2 );
		apply(rhs.v);
	}
	void apply( vec3<T> &rhs ) const {
		assert( N == 3 );
		apply(rhs.v);
	}
	SquareDenseMatrix operator*( const T &scalar ) const {
		SquareDenseMatrix result;
		for( int i=0; i<N; ++i ) for( int j=0; j<N; ++j ) result = scalar * m[i][j];
		return result;
	}
	std::vector<T> operator*( const std::vector<T> &rhs ) const {
		std::vector<T> result(rhs);
		apply(result);
		return result;
	}
	vec2<T> operator*( const vec2<T> &rhs ) const {
		vec2<T> result(rhs);
		apply(result);
		return result;
	}
	vec3<T> operator*( const vec3<T> &rhs ) const {
		vec3<T> result(rhs);
		apply(result);
		return result;
	}
	SquareDenseMatrix transpose() const {
		SquareDenseMatrix result;
		for( int i=0; i<N; ++i ) for( int j=0; j<N; ++j ) result(i,j) = m[j][i];
			return result;
	}
	SquareDenseMatrix operator*( const SquareDenseMatrix &matrix ) const {
		SquareDenseMatrix result;
		for( int i=0; i<N; ++i ) {
			for( int j=0; j<N; ++j ) {
				T value = T();
				for( int k=0; k<N; ++k ) {
					value += m[i][k]*matrix.m[k][j];
				}
				result(i,j) = value;
			}
		}
		return result;
	}
	SquareDenseMatrix invert() const {
		if( N == 2 ) {
			SquareDenseMatrix result;
			T det = m[0][0]*m[1][1]-m[0][1]*m[1][0];
			if( det ) {
				result(0,0) = m[1][1]/det;
				result(1,0) = -m[1][0]/det;
				result(1,1) = m[0][0]/det;
				result(0,1) = -m[0][1]/det;
			}
			return result;
		} else if( N == 3 ) {
			SquareDenseMatrix result;
			T det = +m[0][0]*(m[1][1]*m[2][2]-m[1][2]*m[2][1])
					-m[1][0]*(m[0][1]*m[2][2]-m[2][1]*m[0][2])
					+m[2][0]*(m[0][1]*m[1][2]-m[1][1]*m[0][2]);
			if ( ! det ) return result;
			T invdet = 1.0/det;
			result(0,0) =  (m[1][1]*m[2][2]-m[1][2]*m[2][1])*invdet;
			result(1,0) = -(m[1][0]*m[2][2]-m[2][0]*m[1][2])*invdet;
			result(2,0) =  (m[1][0]*m[2][1]-m[2][0]*m[1][1])*invdet;
			result(0,1) = -(m[0][1]*m[2][2]-m[2][1]*m[0][2])*invdet;
			result(1,1) =  (m[0][0]*m[2][2]-m[2][0]*m[0][2])*invdet;
			result(2,1) = -(m[0][0]*m[2][1]-m[0][1]*m[2][0])*invdet;
			result(0,2) =  (m[0][1]*m[1][2]-m[0][2]*m[1][1])*invdet;
			result(1,2) = -(m[0][0]*m[1][2]-m[0][2]*m[1][0])*invdet;
			result(2,2) =  (m[0][0]*m[1][1]-m[0][1]*m[1][0])*invdet;
			return result;
		} else if( N == 4 ) {
			SquareDenseMatrix result;
			T M[16], Minv[16];
			for( int i=0; i<4; i++ ) for( int j=0; j<4; j++ ) {
				M[i+4*j] = m[i][j];
			}
			if( ! _myInvertMatrix4x4(M,Minv)) return result;
			for( uint i=0; i<4; i++ ) for( uint j=0; j<4; j++ ) {
				result(i,j) = Minv[i+4*j];
			}
			return result;
		} else {
			SquareDenseMatrix result;
			_inverse(*this,result);
			return result;
		}
	}
	// m[i][j] should correspond to m_ij
	std::array<std::array<T,N>,N> m;
	//
private:
	//
	// http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
	static bool _myInvertMatrix4x4(const T m[16], T invOut[16]) {
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
	//
	// Original: http://www.geeksforgeeks.org/adjoint-inverse-matrix/
	// Function to get cofactor of A[p][q] in temp[][]. n is current
	// dimension of A[][]
	static void _getCofactor(const SquareDenseMatrix &A, SquareDenseMatrix &temp, int p, int q, int n) {
		int i = 0, j = 0;
		// Looping for each element of the matrix
		for (int row = 0; row < n; row++) {
			for (int col = 0; col < n; col++) {
				//  Copying into temporary matrix only those element
				//  which are not in given row and column
				if (row != p && col != q) {
					temp(i,j++) = A(row,col);
					// Row is filled, so increase row index and
					// reset col index
					if (j == n - 1) {
						j = 0;
						i++;
					}
				}
			}
		}
	}
	//
	// Recursive function for finding determinant of matrix. n is current dimension of A[][].
	static double _determinant(const SquareDenseMatrix &A, int n) {
		double D (0.0); // Initialize result
		//  Base case : if matrix contains single element
		if (n == 1)	return A(0,0);
		SquareDenseMatrix temp; // To store cofactors
		int sign = 1; // To store sign multiplier
		// Iterate for each element of first row
		for (int f = 0; f < n; f++)	{
			// Getting Cofactor of A[0][f]
			_getCofactor(A, temp, 0, f, n);
			D += sign * A(0,f) * _determinant(temp, n-1);
			// terms are to be added with alternate sign
			sign = -sign;
		}
		return D;
	}
	//
	// Function to get adjoint of A[N][N] in adj[N][N].
	static void _adjoint(const SquareDenseMatrix &A, SquareDenseMatrix &adj) {
		if (N == 1)	{
			adj(0,0) = 1;
			return;
		}
		// temp is used to store cofactors of A[][]
		int sign = 1;
		SquareDenseMatrix temp;
		for (int i=0; i<N; i++) {
			for (int j=0; j<N; j++)	{
				// Get cofactor of A[i][j]
				_getCofactor(A, temp, i, j, N);
				// sign of adj[j][i] positive if sum of row
				// and column indexes is even.
				sign = ((i+j)%2==0)? 1: -1;
				// Interchanging rows and columns to get the
				// transpose of the cofactor matrix
				adj(j,i) = (sign)*(_determinant(temp, N-1));
			}
		}
	}
	// Function to calculate and store inverse, returns false if
	// matrix is singular
	static bool _inverse(const SquareDenseMatrix &A, SquareDenseMatrix &result) {
		// Find determinant of A[][]
		double det = _determinant(A,N);
		if (det == 0.0) {
			return false;
		}
		// Find adjoint
		SquareDenseMatrix adj;
		_adjoint(A,adj);
		// Find Inverse using formula "inverse(A) = adj(A)/det(A)"
		for (int i=0; i<N; i++) for (int j=0; j<N; j++) result(i,j) = adj(i,j)/det;
		return true;
	}
};
//
template <class T, int N> static inline SquareDenseMatrix<T,N> operator*(T scalar, const SquareDenseMatrix<T,N> &matrix) {
	return matrix*scalar;
}
//
template<class T> using Matrix2x2 = SquareDenseMatrix<T,2>;
template<class T> using Matrix3x3 = SquareDenseMatrix<T,3>;
template<class T> using Matrix4x4 = SquareDenseMatrix<T,4>;
//
SHKZ_END_NAMESPACE
//
#endif
//