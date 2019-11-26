/*
**	WENO.h
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
#ifndef SHKZ_WENO_H
#define SHKZ_WENO_H
//
#include <limits>
//
// "Level Set Equations on Surfaces via the Closest Point Method"
// Colin B. Macdonald · Steven J. Ruuth
// https://link.springer.com/article/10.1007/s10915-008-9196-6
// Journal of Scientific Computing, June 2008, Volume 35, Issue 2–3, pp 219–240
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that implements WENO interpolation.
/// \~japanese @brief WENO 補間を実装するクラス。
class WENO {
public:
	/**
	 \~english @brief Interpolate using 6th order accurate WENO scheme.
	 @param[in] x Position between 0 and 1.
	 @param[in] v Values on positions of -2, -1, 0, 1, 2, 3.
	 @param[in] eps Very small number to avoid division by zero.
	 \~japanese @brief 6次精度 WENO スキームを用いて補間を行う。
	 @param[in] x 0 から 1 までの位置.
	 @param[in] v -2, -1, 0, 1, 2, 3 の位置での値。
	 @param[in] eps ゼロ割を防ぐための小さな値。
	 */
	static double interp6( double x, const double v[6], const double eps=std::numeric_limits<double>::epsilon() ) {
		//
		const double &f_m2 = v[0];	const double &f_m1 = v[1];	const double &f_p0 = v[2];
		const double &f_p1 = v[3];	const double &f_p2 = v[4];	const double &f_p3 = v[5];
		//
		const double x_m2 = -2.0;	const double x_m1 = -1.0;	const double x_p0 = 0.0;
		const double x_p1 = 1.0;	const double x_p2 = 2.0;	const double x_p3 = 3.0;
		//
		double C[3];
		C[0] = (x_p2-x) * (x_p3-x) / 20.0;
		C[1] = (x_p3-x) * (x-x_m2) / 10.0;
		C[2] = (x-x_m2) * (x-x_m1) / 20.0;
		//
		double S[3];
		S[0] = ((0814.*sqr(f_p1))+(4326.*sqr(f_p0))+(2976.*sqr(f_m1))+(0244.*sqr(f_m2))-(3579.*f_p0*f_p1)-(6927.*f_p0*f_m1)+(1854.*f_p0*f_m2)+(2634.*f_p1*f_m1)-(0683.*f_p1*f_m2)-(1659.*f_m1*f_m2)) / 180.0;
		S[1] = ((1986.*sqr(f_p1))+(1986.*sqr(f_p0))+(0244.*sqr(f_m1))+(0244.*sqr(f_p2))+(1074.*f_p0*f_p2)-(3777.*f_p0*f_p1)-(1269.*f_p0*f_m1)+(1074.*f_p1*f_m1)-(1269.*f_p2*f_p1)-(0293.*f_p2*f_m1)) / 180.0;
		S[2] = ((0814.*sqr(f_p0))+(4326.*sqr(f_p1))+(2976.*sqr(f_p2))+(0244.*sqr(f_p3))-(0683.*f_p0*f_p3)+(2634.*f_p0*f_p2)-(3579.*f_p0*f_p1)-(6927.*f_p1*f_p2)+(1854.*f_p1*f_p3)-(1659.*f_p2*f_p3)) / 180.0;
		//
		double P[3];
		P[0] = f_m2 + (f_m1-f_m2) * (x-x_m2) + (f_p0-2.*f_m1+f_m2) * (x-x_m2) * (x-x_m1) / 2.0 + (f_p1-3.*f_p0+3.*f_m1-f_m2) * (x-x_m2) * (x-x_m1) * (x-x_p0) / 6.0;
		P[1] = f_m1 + (f_p0-f_m1) * (x-x_m1) + (f_p1-2.*f_p0+f_m1) * (x-x_m1) * (x-x_p0) / 2.0 + (f_p2-3.*f_p1+3.*f_p0-f_m1) * (x-x_m1) * (x-x_p0) * (x-x_p1) / 6.0;
		P[2] = f_p0 + (f_p1-f_p0) * (x-x_p0) + (f_p2-2.*f_p1+f_p0) * (x-x_p0) * (x-x_p1) / 2.0 + (f_p3-3.*f_p2+3.*f_p1-f_p0) * (x-x_p0) * (x-x_p1) * (x-x_p2) / 6.0;
		//
		double a[3], sum(0.0);
		for( int i=0; i<3; ++i ) {
			a[i] = C[i] / (eps+sqr(S[i]));
			sum += a[i];
		}
		double w[3];
		for( int i=0; i<3; ++i ) {
			w[i] = a[i] / sum;
		}
		//
		return w[0]*P[0]+w[1]*P[1]+w[2]*P[2];
	};
	/**
	 \~english @brief Interpolate using 4th order accurate WENO scheme.
	 @param[in] x Position between 0 and 1.
	 @param[in] v Values on positions of -1, 0, 1, 2.
	 @param[in] eps Very small number to avoid division by zero.
	 \~japanese @brief 4次精度 WENO スキームを用いて補間を行う。
	 @param[in] x 0 から 1 までの位置.
	 @param[in] v -1, 0, 1, 2 の位置での値。
	 @param[in] eps ゼロ割を防ぐための小さな値。
	 */
	static double interp4( double x, const double v[4], const double eps=std::numeric_limits<double>::epsilon() ) {
		//
		const double &f_m1 = v[0];	const double &f_p0 = v[1];
		const double &f_p1 = v[2];	const double &f_p2 = v[3];
		//
		const double x_m1 = -1.0;	const double x_p0 = 0.0;
		const double x_p1 = 1.0;	const double x_p2 = 2.0;
#pragma unused(x_p1)
		//
		double C[2];
		C[0] = (x_p2-x) / 3.0;
		C[1] = (x-x_m1) / 3.0;
		//
		double S[2];
		S[0] = ((26.*f_p1*f_m1)-(52.*f_p0*f_m1)-(76.*f_p1*f_p0)+(25.*sqr(f_p1))+(64.*sqr(f_p0))+(13.*sqr(f_m1))) / 12.0;
		S[1] = ((26.*f_p2*f_p0)-(52.*f_p2*f_p1)-(76.*f_p1*f_p0)+(25.*sqr(f_p0))+(64.*sqr(f_p1))+(13.*sqr(f_p2))) / 12.0;
		//
		double P[2];
		P[0] = f_p0 + (f_p1-f_m1) * (x-x_p0) / 2.0 + (f_p1-2.*f_p0+f_m1) * sqr(x-x_p0) / 2.0;
		P[1] = f_p0 + (-f_p2+4.*f_p1-3.*f_p0) * (x-x_p0) / 2.0 + (f_p2-2.*f_p1+f_p0) * sqr(x-x_p0) / 2.0;
		//
		double a[2], sum(0.0);
		for( int i=0; i<2; ++i ) {
			a[i] = C[i] / (eps+sqr(S[i]));
			sum += a[i];
		}
		double w[2];
		for( int i=0; i<2; ++i ) {
			w[i] = a[i] / sum;
		}
		//
		return w[0]*P[0]+w[1]*P[1];
	}
private:
	static double sqr (const double &x) { return x*x; }
};
//
SHKZ_END_NAMESPACE
//
#endif
