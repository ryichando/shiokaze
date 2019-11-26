/*
**	utility.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 15, 2017.
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
#ifndef SHKZ_UTILITY_H
#define SHKZ_UTILITY_H
//
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <vector>
#include <functional>
#include <limits>
//
#include <shiokaze/math/vec.h>
#include <shiokaze/math/vec.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that provides various utility functions.
/// \~japanese @brief 様々なユーティリティ関数を提供するクラス。
class utility {
public:
	/**
	 \~english @brief Check of the number of power of two.
	 @param[in] n Number to examine.
	 @return \c true if the number is power of two. \c false otherwise.
	 \~japanese @brief 数字が2の冪乗が調べる。
	 @param[in] n 調べる数字。
	 @return もし2の冪乗なら \c true を、そうでなければ \c false を返す。
	 */
	static bool is_power_of_two(int n) {
		return !(n & (n - 1)); //this checks if the integer n is a power of two or not
	}
	/**
	 \~english @brief Get of the number of active bit in power of two number.
	 @return Index of active bit.
	 \~japanese @brief 2の冪乗の数字のアクティブビットの番号を得る。
	 @return アクティブビットのインデックス。
	 */
	static unsigned char log2( size_t n ) {
		size_t tmp (n);
		unsigned char result (0);
		while( tmp >>= 1 ) ++ result;
		return result;
	}
	/**
	 \~english @brief Get microseconds.
	 @return Microseconds.
	 \~japanese @brief マイクロ秒を取得する。
	 @return マイクロ秒。
	 */
	static unsigned long get_microseconds() {
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		return tv.tv_sec*1000000 + tv.tv_usec;
	}
	/**
	 \~english @brief Get milliseconds.
	 @return Milliseconds.
	 \~japanese @brief ミリ秒を取得する。
	 @return ミリ秒。
	 */
	static double get_milliseconds() {
		return get_microseconds()/1000.0;
	}
	/**
	 \~english @brief Get seconds.
	 @return Seconds.
	 \~japanese @brief 秒を取得する。
	 @return 秒。
	 */
	static double get_seconds() {
		return get_milliseconds()/1000.0;
	}	
	/**
	 \~english @brief Get the three dimensional box level set.
	 @param[in] p Query position.
	 @param[in] p0 Lower left corner position of a box.
	 @param[in] p1 Uppper right corner position of a box.
	 @return Box level set value.
	 \~japanese @brief 3次元のボックスのレベルセットを取得する。
	 @param[in] p 調べる位置。
	 @param[in] p0 ボックスの左最下部の頂点の位置。
	 @param[in] p1 ボックスの右最上部の頂点の位置。
	 @return ボックスのレベルセットの値。
	 */
	static double box( vec3d p, vec3d p0, vec3d p1 ) {
		double sd = -9999.0;
		for( unsigned dim : DIMS3 ) {
			sd = std::max(sd,p0[dim]-p[dim]);
			sd = std::max(sd,p[dim]-p1[dim]);
		}
		return sd;
	}
	/**
	 \~english @brief Get the two dimensional box level set.
	 @param[in] p Query position.
	 @param[in] p0 Lower left corner position of a box.
	 @param[in] p1 Uppper right corner position of a box.
	 @return Box level set value.
	 \~japanese @brief 2次元のボックスのレベルセットを取得する。
	 @param[in] p 調べる位置。
	 @param[in] p0 ボックスの左最下部の頂点の位置。
	 @param[in] p1 ボックスの右最上部の頂点の位置。
	 @return ボックスのレベルセットの値。
	 */
	static double box( vec2d p, vec2d p0, vec2d p1 ) {
		double sd = -9999.0;
		for( unsigned dim : DIMS2 ) {
			sd = std::max(sd,p0[dim]-p[dim]);
			sd = std::max(sd,p[dim]-p1[dim]);
		}
		return sd;
	}
	/**
	 \~english @brief Get of the value is NaN.
	 @param[in] v Value to examine.
	 @return \c true if the v is NaN. \c false otherwise.
	 \~japanese @brief 値が NaN か調べる。
	 @param[in] v 調べる値。
	 @return もし v が NaN なら \c true を、そうでないなら \c false を返す。
	 */
	static bool is_nan(const double v) {
		return (v != v);
	}
	/**
	 \~english @brief Get the fraction of the level set.
	 @param[in] phi0 One side of level set value.
	 @param[in] phi1 Another side of level set value.
	 @return Fraction occupied by the level set.
	 @param[in] phi0 片方のレベルセットの値。
	 @param[in] phi1 もう片方のレベルセットの値。
	 \~japanese @brief レベルセットの占める割合を取得する。
	 @return レベルセットの占める割合を返す。
	 */
	static double fraction( double phi0, double phi1 ) {
		if( phi0*phi1 >= 0.0 ) {
			if( phi0 < 0.0 || phi1 < 0.0 ) return 1.0;
			return 0.0;
		} else {
			static constexpr double eps = std::numeric_limits<double>::min();
			return -std::min(phi0,phi1)/std::max(std::abs(phi1-phi0),eps);
		}
	};
	/**
	 \~english @brief Get the area of level set defined on four vertices of a unit square.
	 @param[in] isosurf Four level set values accessed by isosurf[i][j].
	 @return Area.
	 \~japanese @brief 4つの頂点からなる単位四角形の4つの頂点で定義されるレベルセットの面積を取得する。
	 @param[in] isosurf isosurf[i][j] でアクセスされる4つのレベルセットの値。
	 @return 面積。
	 */
	static double get_area( const double isosurf[2][2] ) {
		//
		static const int quads[][2] = { {0,0}, {1,0}, {1,1}, {0,1} };
		auto calcMarchingPoints = [&]( const double values[4], double p[8][2], int &pnum ) {
			pnum = 0;
			for( int n=0; n<4; n++ ) {
				if( values[n] < 0.0 ) {
					p[pnum][0] = quads[n][0];
					p[pnum][1] = quads[n][1];
					pnum ++;
				}
				if( values[n] * values[(n+1)%4] < 0 ) {
					double y0 = values[n];
					double y1 = values[(n+1)%4];
					if( y0-y1 ) {
						double a = y0/(y0-y1);
						int p0[2] = { quads[n][0], quads[n][1] };
						int p1[2] = { quads[(n+1)%4][0], quads[(n+1)%4][1] };
						p[pnum][0] = (1.0-a)*p0[0]+a*p1[0];
						p[pnum][1] = (1.0-a)*p0[1]+a*p1[1];
						pnum ++;
					}
				}
			}
		};
		//
		double p[8][2], v[4];
		for( int n=0; n<4; ++n ) v[n] = isosurf[quads[n][0]][quads[n][1]];
		int pnum;
		calcMarchingPoints(v,p,pnum);
		double sum (0.0);
		for( int m=0; m<pnum; m++ ) {
			sum += p[m][0]*p[(m+1)%pnum][1]-p[m][1]*p[(m+1)%pnum][0];
		}
		return 0.5 * sum;
	}
	/**
	 \~english @brief Compute the area of a mesh.
	 @param[in] vertices Vertices.
	 @param[in] faces Faces.
	 @param[in] test_func function.
	 \~japanese @brief 3次元メッシュの表面積を計算する。
	 @param[in] vertices 頂点列。
	 @param[in] faces 面列。
	 @param[in] test_func テスト関数。
	 */
	template <class N> static double compute_area( const std::vector<vec3d> &vertices, const std::vector<std::vector<N> > &faces, std::function<bool(const vec3d &p)> test_func ) {
		//
		double area (0.0);
		for( const auto &f : faces ) {
			vec3d avg;
			for( const auto &i : f ) { avg += vertices[i]; }
			avg /= f.size();
			if( test_func(avg)) {
				const vec3d &v0 = vertices[f[0]];
				for( int n=1; n<f.size()-1; ++n ) {
					const vec3d &v1 = vertices[f[n]];
					const vec3d &v2 = vertices[f[n+1]];
					area += 0.5 * ((v1-v0)^(v2-v0)).len();
				}
			}
		}
		return area;
	}
	/**
	 \~english @brief Compute the length of a contour.
	 @param[in] vertices Vertices.
	 @param[in] faces Faces.
	 @param[in] test_func function.
	 \~japanese @brief 2次元メッシュの長さを計算する。
	 @param[in] vertices 頂点列。
	 @param[in] faces 面列。
	 @param[in] test_func テスト関数。
	 */
	template <class N> static double compute_length( const std::vector<vec2d> &vertices, const std::vector<std::vector<N> > &faces, std::function<bool(const vec2d &p)> test_func ) {
		//
		double length (0.0);
		for( const auto &f : faces ) {
			vec2d avg;
			for( const auto &i : f ) { avg += vertices[i]; }
			avg /= f.size();
			if( test_func(avg)) {
				for( int n=1; n<f.size(); ++n ) {
					const vec2d &v0 = vertices[f[n-1]];
					const vec2d &v1 = vertices[f[n]];
					length += (v1-v0).len();
				}
			}
		}
		return length;
	}
//
};
//
SHKZ_END_NAMESPACE
//
#endif