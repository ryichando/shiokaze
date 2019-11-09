/*
**	signed_rigidbody2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on August 14, 2019.
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
#ifndef SHKZ_SIGNED_RIGIDBODY2_INTERFACE_H
#define SHKZ_SIGNED_RIGIDBODY2_INTERFACE_H
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief 2D Signed rigidbody interface.
/// \~japanese @brief 2D 符号付き距離関数の剛体のインターフェース。
class signed_rigidbody2_interface {
public:
	/**
	 \~english @brief Get the world-coordinate bounding box of the rigidbody.
	 @param[out] p0 Left bottom corner position.
	 @param[out] p1 Right top corner position.
	 \~japanese @brief 剛体のワールド座標系のバウンディングボックスを計算する。
	 @param[out] p0 左下のコーナーの位置。
	 @param[out] p1 右上のコーナーの位置。
	 */
	virtual void get_bounding_box( vec2d &p0, vec2d &p1 ) const = 0;
	/**
	 \~english @brief Get the signed distance at a world position.
	 @param[in] p World-coordinate position.
	 @return Signed distance.
	 \~japanese @brief ワールド座標系での符号付き距離を得る。
	 @param[in] p ワールド座標系での位置。
	 @return 符号付き距離。
	 */
	virtual double get_signed_distance( const vec2d &p ) const = 0;
	/**
	 \~english @brief Get moment of inertia
	 @return Moment of inertia
	 \~japanese @brief 慣性モーメントを得る。
	 @return 慣性モーメント。
	 */
	virtual double get_moment_of_inertia() const = 0;
	/**
	 \~english @brief Get mass.
	 @return Mass. Zero if infinite.
	 \~japanese @brief 質量を得る。
	 @return 質量。ゼロなら無限大。
	 */
	virtual double get_mass() const = 0;
	/**
	 \~english @brief Get center of gravity.
	 @return Center of gravity.
	 \~japanese @brief 重心を得る。
	 @return 重心。
	 */
	virtual vec2d get_center_of_gravity() const = 0;
	/**
	 \~english @brief Get translational velocity.
	 @return Translational velocity.
	 \~japanese @brief 速度を得る。
	 @return 速度。
	 */
	virtual vec2d get_velocity() const = 0;
	/**
	 \~english @brief Get rotation.
	 @return Rotation.
	 \~japanese @brief 回転角度を得る。
	 @return 回転角度。
	 */
	virtual double get_rotation() const = 0;
	/**
	 \~english @brief Get angular velocity.
	 @return Angular velocity.
	 \~japanese @brief 回転速度を得る。
	 @return 回転速度。
	 */
	virtual double get_angular_velocity() const = 0;
};
//
SHKZ_END_NAMESPACE
//
#endif