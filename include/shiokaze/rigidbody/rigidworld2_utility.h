/*
**	rigidworld2_utility.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on August 8, 2019.
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
#ifndef SHKZ_RIGIDWORLD2_UTILITY_H
#define SHKZ_RIGIDWORLD2_UTILITY_H
//
#include "rigidworld2_interface.h"
#include <cmath>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that provides various utility functions for rigidbody physics.
/// \~japanese @brief 剛体物理のための様々なユーティリティ関数群を提供するクラス。
class rigidworld2_utility {
public:
	//
	using rg2 = rigidbody2_interface;
	/**
	 \~english @brief Add walls to square walls that enclose the simulatio domain.
	 @param[in] world Pointer to an instance of rigidbody world.
	 @param[in] attribute Attribution of the wall.
	 @param[in] p0 Left bottom corner of the wall.
	 @param[in] p1 Right top corner of the wall.
	 \~japanese @brief 剛体ワールドに計算空間を覆うような四角の壁を追加する。
	 @param[in] world 剛体ワールドのインスタンスへのポインタ。
	 @param[in] attribute 壁の属性。
	 @param[in] p0 壁の左下の位置。
	 @param[in] p1 壁の右上の位置。
	 */
	static void add_container_wall( rigidworld2_interface *world, const rg2::attribution2 &attribute, const vec2d &p0, const vec2d& p1 ) {
		//
		vec2d center = 0.5 * (p0+p1);
		vec2d hw = p1-center;
		//
		rg2::velocity2 wall_velocity = { {0.0,0.0}, 0.0 };
		rg2::position2 wall_position = { center, 0.0 };
		//
		std::vector<vec2d> bottom = {{-hw[0],-hw[1]},{hw[0],-hw[1]}};
		std::vector<vec2d> left = {{-hw[0],-hw[1]},{-hw[0],hw[1]}};
		std::vector<vec2d> right = {{hw[0],-hw[1]},{hw[0],hw[1]}};
		std::vector<vec2d> top = {{-hw[0],hw[1]},{hw[0],hw[1]}};
		//
		std::vector<rg2::polyshape2> wall_polyshaps = { {left,rg2::EDGE}, {bottom,rg2::EDGE}, {right,rg2::EDGE}, {top,rg2::EDGE} };
		world->add_rigidbody(wall_polyshaps,attribute,wall_position,wall_velocity);
	}
	/**
	 \~english @brief Get the veloicty at a specific position.
	 @param[in] rigidbody Pointer to rigidbody instance.
	 @param[in] p Position to get the velocity.
	 @return Velocity.
	 \~japanese @brief 特定の位置での速度情報を得る。
	 @param[in] rigidbody 剛体インスタンスへのポインタ。
	 @param[in] p 速度を求めたい位置。
	 @return 速度。
	 */
	static vec2d get_velocity( const rigidbody2_interface *rigidbody, const vec2d &p ) {
		vec2d r = p-rigidbody->get_position().center;
		rigidbody2_interface::velocity2 u = rigidbody->get_velocity();
		double a = u.angular_velocity;
		return u.center_velocity+a*vec2d(-r[1],r[0]);
	}
	/**
	 \~english @brief Compute the inverse of OpenGL matrix of rigidbody transformation.
	 @param[in] rigidbody Pointer to rigidbody instance.
	 @param[out] inv Inverse matrix.
	 @return If the inversion succeeds.
	 \~japanese @brief ワールド位置を剛体のローカル座標に変換する。
	 @param[in] rigidbody 剛体インスタンスへのポインタ。
	 @param[in] inv 逆行列。
	 @return 逆行列の計算が成功したか。
	 */
	//https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
	static bool getInverseOpenGLMatrix( const rigidbody2_interface *rigidbody, double inv[9] ) {
		//
		float m[9];
		rigidbody->getOpenGLMatrix(m);
		//
		double det = m[0]*(m[1+3*1]*m[2+3*2]-m[1+3*2]*m[2+3*1])
		-m[1+3*0]*(m[0+3*1]*m[2+3*2]-m[2+3*1]*m[0+3*2])
		+m[2+3*0]*(m[0+3*1]*m[1+3*2]-m[1+3*1]*m[0+3*2]);
		//
		if( ! det ) return false;
		double invdet = 1.0 / det;
		double A[9];
		//
		inv[0+3*0] = (m[1+3*1] * m[2+3*2] - m[2+3*1] * m[1+3*2]) * invdet;
		inv[0+3*1] = (m[0+3*2] * m[2+3*1] - m[0+3*1] * m[2+3*2]) * invdet;
		inv[0+3*2] = (m[0+3*1] * m[1+3*2] - m[0+3*2] * m[1+3*1]) * invdet;
		inv[1+3*0] = (m[1+3*2] * m[2+3*0] - m[1+3*0] * m[2+3*2]) * invdet;
		inv[1+3*1] = (m[0+3*0] * m[2+3*2] - m[0+3*2] * m[2+3*0]) * invdet;
		inv[1+3*2] = (m[1+3*0] * m[0+3*2] - m[0+3*0] * m[1+3*2]) * invdet;
		inv[2+3*0] = (m[1+3*0] * m[2+3*1] - m[2+3*0] * m[1+3*1]) * invdet;
		inv[2+3*1] = (m[2+3*0] * m[0+3*1] - m[0+3*0] * m[2+3*1]) * invdet;
		inv[2+3*2] = (m[0+3*0] * m[1+3*1] - m[1+3*0] * m[0+3*1]) * invdet;
		return true;
	}
	/**
	 \~english @brief Convert world position to rigidbody local position.
	 @param[in] inv Inverse of rigidbody OpenGL transformation matrix.
	 @param[in] p World coordinate position.
	 @return Local coordinate position.
	 \~japanese @brief ワールド位置を剛体のローカル座標に変換する。
	 @param[in] inv OpenGL のトランフォーム行列の逆行列。
	 @param[in] p ワールド座標系の位置。
	 @return ローカル座標系の位置。
	 */
	static vec2d get_local_position( double inv[9], const vec2d &p ) {
		//
		vec2d result;
		double b[] = { p[0], p[1], 1.0 };
		for( int i=0; i<3; ++i ) for( int j=0; j<3; ++j ) {
			result[j] += inv[i+3*j] * b[j];
		}
		return result;
	}
};
//
SHKZ_END_NAMESPACE
//
#endif