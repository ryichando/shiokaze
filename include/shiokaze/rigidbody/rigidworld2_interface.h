/*
**	rigidworld2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 25, 2019.
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
#ifndef SHKZ_RIGIDWORLD2_H
#define SHKZ_RIGIDWORLD2_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/math/vec.h>
#include <cmath>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief 2D Rigidbody interface.
/// \~japanese @brief 2D 剛体のインターフェース。
class rigidbody2_interface {
public:
	//
	/// \~english @brief Type of shape.
	/// \~japanese @brief 形状の種類。
	enum shape_type {
		/// \~english @brief Edge shape.
		/// \~japanese @brief エッジの形状。
		EDGE,
		/// \~english @brief Convex hull shape.
		/// \~japanese @brief コンベックスハルの形状。
		POLYGON
	};
	//
	/// \~english @brief Shape of a polygon.
	/// \~japanese @brief ポリゴンの形状。
	struct polyshape2 {
		//
		/// \~english @brief Vertices of the polygon.
		/// \~japanese @brief ポリゴンの頂点列。
		std::vector<vec2d> polygon;
		//
		/// \~english @brief Polygon type.
		/// \~japanese @brief ポリゴンのタイプ。
		shape_type type;
	};
	//
	/// \~english @brief Position and the angle information of a polygon.
	/// \~japanese @brief ポリゴンの位置とアングルの情報。
	struct position2 {
		//
		/// \~english @brief Center of gravity.
		/// \~japanese @brief 重心。
		vec2d center;
		//
		/// \~english @brief Angle of rotation.
		/// \~japanese @brief 回転角。
		double angle;
	};
	//
	/// \~english @brief Attribution of a polygon.
	/// \~japanese @brief ポリゴンの属性。
	struct attribution2 {
		//
		/// \~english @brief Name of the polygon.
		/// \~japanese @brief ポリゴンの名前。
		std::string name;
		//
		/// \~english @brief Density of a polygon.
		/// \~japanese @brief ポリゴンの密度。
		double density;
		//
		/// \~english @brief Friction coefficient of the polygon.
		/// \~japanese @brief ポリゴンの摩擦係数。
		double friction;
		//
		/// \~english @brief Restitution coefficient of the polygon.
		/// \~japanese @brief ポリゴンの反発係数。
		double restitution;
		//
		/// \~english @brief Is drawable.
		/// \~japanese @brief 描画可能か。
		bool drawable;
		//
		/// \~english @brief User pointer.
		/// \~japanese @brief ユーザー指定のポインタ。
		void *user_pointer;
	};
	//
	/// \~english @brief Velocity information of a polygon.
	/// \~japanese @brief ポリゴンの速度の情報。
	struct velocity2 {
		//
		/// \~english @brief Linear veloicity.
		/// \~japanese @brief 重心の速度。
		vec2d center_velocity;
		//
		/// \~english @brief Angular velocity.
		/// \~japanese @brief 回転速度。
		double angular_velocity;
	};
	/**
	 \~english @brief Get the list of convex shapes of the polygon.
	 @return List of convex polygon shapes.
	 \~japanese @brief ポリゴンのコンベックスハルの形状のリストを得る。
	 @return ポリゴンのコンベックスハルのリスト。
	 */
	virtual std::vector<polyshape2> get_shapes() const = 0;
	/**
	 \~english @brief Get the position information of the polygon.
	 @return Position information.
	 \~japanese @brief ポリゴンの位置情報を得る。
	 @return 位置情報。
	 */
	virtual position2 get_position() const = 0;
	/**
	 \~english @brief Get the attribution information of the polygon.
	 @return Attribution information.
	 \~japanese @brief ポリゴンの属性情報を得る。
	 @return 属性情報。
	 */
	virtual attribution2 get_attribution() const = 0;
	/**
	 \~english @brief Get the velocity information of the polygon.
	 @return Velocity information.
	 \~japanese @brief ポリゴンの速度情報を得る。
	 @return 速度情報。
	 */
	virtual velocity2 get_velocity() const = 0;
	/**
	 \~english @brief Get the OpenGL transformation matrix.
	 @param[out] m OpenGL format matrix.
	 \~japanese @brief OpenGL の行列情報を得る。
	 @param[out] m OpenGL フォーマットの行列。
	 */
	void getOpenGLMatrix( float m[9] ) const {
		position2 position = get_position();
		m[0] = cos(position.angle); m[3] = -sin(position.angle); m[6] = position.center[0];
		m[1] = sin(position.angle); m[4] = cos(position.angle); m[7] = position.center[1];
		m[2] = 0.0; m[5] = 0.0; m[8] = 1.0;
	}
	/**
	 \~english @brief Get the veloicty at a specific position.
	 @param[in] p Position to get the velocity.
	 @return Velocity.
	 \~japanese @brief 特定の位置での速度情報を得る。
	 @param[in] p 速度を求めたい位置。
	 @return 速度。
	 */
	vec2d get_velocity( const vec2d &p ) const {
		vec2d r = p-get_position().center;
		velocity2 u = get_velocity();
		double a = u.angular_velocity;
		return u.center_velocity+a*vec2d(-r[1],r[0]);
	}
};
//
/// \~english @brief 2D Rigidody world interface.
/// \~japanese @brief 2D 剛体のワールドのインターフェース。
class rigidworld2_interface : public recursive_configurable_module {
public:
	//
	using rg2 = rigidbody2_interface;
	//
	DEFINE_MODULE(rigidworld2_interface,"Rigidbody World 2D","Rigidbody","Rigidbody module")
	//
	/**
	 \~english @brief Add a rigidbody to the world.
	 @param[in] polyshapes List of convex shapes that represents an object.
	 @param[in] attribute Attribution information.
	 @param[in] position Position information.
	 @param[in] velocity Velocity information.
	 @return Pointer to the added rigidbody. Deallocated automatically by world.
	 \~japanese @brief 剛体を世界に追加する。
	 @param[in] polyshapes オブジェクトを表現するコンベックスハルの形状のリスト。
	 @param[in] attribute 属性の情報。
	 @param[in] position 位置の情報。
	 @param[in] velocity 速度の情報。
	 @return 追加された剛体へのポインタ。世界によって自動的にデアロケーションされる。
	 */
	virtual rigidbody2_interface * add_rigidbody(
		const std::vector<rg2::polyshape2> &polyshapes, const rg2::attribution2 &attribute, const rg2::position2& position, const rg2::velocity2 &velocity
	) = 0;
	/**
	 \~english @brief Get the name of internal rigidbody physics.
	 @return Name of the engine.
	 \~japanese @brief 内部で使用される剛体物理エンジンの名前を得る。
	 @return エンジンの名前。
	 */
	virtual std::string engine_name() const = 0;
	/**
	 \~english @brief Clear out the world.
	 \~japanese @brief 世界をクリアする。
	 */
	virtual void clear() = 0;
	/**
	 \~english @brief Advance physics calculation by a timestep.
	 @param[in] dt Time step size.
	 \~japanese @brief タイムステップだけ物理計算を進める。
	 @param[in] dt タイムステップサイズ。
	 */
	virtual void advance( double dt ) = 0;
	/**
	 \~english @brief Draw physics world.
	 @param[in] g Graphics engine.
	 \~japanese @brief 物理世界を描く。
	 @param[in] g グラフィックスエンジン。
	 */
	virtual void draw( graphics_engine &g ) const = 0;
	/**
	 \~english @brief Get the list of rigidbodies in the world.
	 @return The list of all the rigidbodies.
	 \~japanese @brief 世界に登録されている剛体のリストを得る。
	 @return 全ての剛体のリスト。
	 */
	virtual std::vector<rigidbody2_interface *> get_rigidbody_list() = 0;
	/**
	 \~english @brief Get the list of rigidbodies in the world.
	 @return The list of all the rigidbodies.
	 \~japanese @brief 世界に登録されている剛体のリストを得る。
	 @return 全ての剛体のリスト。
	 */
	std::vector<const rigidbody2_interface *> get_rigidbody_list() const {
		std::vector<rigidbody2_interface *> tmp_result = const_cast<rigidworld2_interface *>(this)->get_rigidbody_list();
		std::vector<const rigidbody2_interface *> result;
		for( auto it=tmp_result.begin(); it!=tmp_result.end(); ++it ) result.push_back(*it);
		return result;
	}
	//
};
//
using rigidworld2_ptr = std::unique_ptr<rigidworld2_interface>;
using rigidworld2_driver = recursive_configurable_driver<rigidworld2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
