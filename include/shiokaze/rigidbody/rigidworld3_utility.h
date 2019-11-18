/*
**	rigidworld3_utility.h
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
#ifndef SHKZ_RIGIDWORLD3_UTILITY_H
#define SHKZ_RIGIDWORLD3_UTILITY_H
//
#include "rigidworld3_interface.h"
#include <cmath>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that provides various utility functions for rigidbody physics.
/// \~japanese @brief 剛体物理のための様々なユーティリティ関数群を提供するクラス。
class rigidworld3_utility {
public:
	//
	using rg3 = rigidbody3_interface;
	//
	/**
	 \~english @brief Add walls to square walls that enclose the simulatio domain.
	 @param[in] world Pointer to an instance of rigidbody world.
	 @param[in] attribute Attribution of the wall.
	 @param[in] p0 Left bottom front corner of the wall.
	 @param[in] p1 Right top back corner of the wall.
	 \~japanese @brief 剛体ワールドに計算空間を覆うような四角の壁を追加する。
	 @param[in] world 剛体ワールドのインスタンスへのポインタ。
	 @param[in] attribute 壁の属性。
	 @param[in] p0 壁の左下前の位置。
	 @param[in] p1 壁の右上後ろの位置。
	 */
	static void add_container_wall( rigidworld3_interface *world, const rg3::attribution3 &attribute, const vec3d &p0, const vec3d& p1 ) {
		//
		vec3d center = 0.5 * (p0+p1);
		vec3d hw = p1-center;
		//
		rg3::velocity3 wall_velocity = { vec3d(), vec3d() };
		rg3::position3 wall_position = { center, vec3d(), 0.0 };
		//
		rg3::polyshape3 bottom;
		bottom.vertices = {vec3d(-hw[0],-hw[1],-hw[2]),vec3d(hw[0],-hw[1],-hw[2]),vec3d(hw[0],-hw[1],hw[2]),vec3d(-hw[0],-hw[1],hw[2])};
		bottom.faces = {{0,1,2},{0,2,3}};
		bottom.type = rg3::MESH;
		//
		rg3::polyshape3 top;
		top.vertices = {vec3d(-hw[0],hw[1],-hw[2]),vec3d(hw[0],hw[1],-hw[2]),vec3d(hw[0],hw[1],hw[2]),vec3d(-hw[0],hw[1],hw[2])};
		top.faces = {{0,1,2},{0,2,3}};
		top.type = rg3::MESH;
		//
		rg3::polyshape3 right;
		right.vertices = {vec3d(hw[0],-hw[1],-hw[2]),vec3d(hw[0],-hw[1],hw[2]),vec3d(hw[0],hw[1],hw[2]),vec3d(hw[0],hw[1],-hw[2])};
		right.faces = {{0,1,2},{0,2,3}};
		right.type = rg3::MESH;
		//
		rg3::polyshape3 left;
		left.vertices = {vec3d(-hw[0],-hw[1],-hw[2]),vec3d(-hw[0],-hw[1],hw[2]),vec3d(-hw[0],hw[1],hw[2]),vec3d(-hw[0],hw[1],-hw[2])};
		left.faces = {{0,1,2},{0,2,3}};
		left.type = rg3::MESH;
		//
		rg3::polyshape3 front;
		front.vertices = {vec3d(-hw[0],-hw[1],-hw[2]),vec3d(hw[0],-hw[1],-hw[2]),vec3d(hw[0],hw[1],-hw[2]),vec3d(-hw[0],hw[1],-hw[2])};
		front.faces = {{0,1,2},{0,2,3}};
		front.type = rg3::MESH;
		//
		rg3::polyshape3 back;
		back.vertices = {vec3d(-hw[0],-hw[1],hw[2]),vec3d(hw[0],-hw[1],hw[2]),vec3d(hw[0],hw[1],hw[2]),vec3d(-hw[0],hw[1],hw[2])};
		back.faces = {{0,1,2},{0,2,3}};
		back.type = rg3::MESH;
		//
		world->add_rigidbody({bottom,top,right,left,front,back},attribute,wall_position,wall_velocity);
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
	vec3d get_velocity( const rigidbody3_interface *rigidbody, const vec3d &p ) const {
		vec3d r = p-rigidbody->get_position().center;
		rigidbody3_interface::velocity3 u = rigidbody->get_velocity();
		vec3d a = u.angular_velocity;
		return a ^ r;
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
	//https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
	bool getInverseOpenGLMatrix( const rigidbody3_interface *rigidbody, double invOut[16] ) const {
		//
		Real m[16];
		rigidbody->getOpenGLMatrix(m);
		//
		double inv[16], det; int i;
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
	vec3d get_local_position( double inv[16], const vec3d &p ) const {
		//
		vec3d result;
		double b[] = { p[0], p[1], p[2], 1.0 };
		for( int i=0; i<4; ++i ) for( int j=0; j<4; ++j ) {
			result[j] += inv[i+4*j] * b[j];
		}
		return result;
	}
};
//
SHKZ_END_NAMESPACE
//
#endif