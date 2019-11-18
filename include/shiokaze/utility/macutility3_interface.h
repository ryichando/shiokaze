/*
**	macutility3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 19, 2017. 
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
#ifndef SHKZ_MACUTILITY3_INTERFACE_H
#define SHKZ_MACUTILITY3_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/core/dylibloader.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that provides various utility functions for MAC grids. "macutility3" is provided as implementation.
/// \~japanese @brief MAC グリッドのための様々なユーティリティ関数群を提供するインターフェース。"macutility3" が実装として提供される。
class macutility3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macutility3_interface,"MAC Utility 3D","MacUtility","MAC Utility Tools")
	/**
	 \~english @brief Compute the maximal velocity.
	 @param[in] velocity Velocity field.
	 @return Magnitude of the maximal velocity.
	 \~japanese @brief 最大速度を計算する。
	 @param[in] velocity 速度場。
	 @return 最大速度の絶対値。
	 */
	virtual double compute_max_u ( const macarray3<Real> &velocity ) const = 0;
	/**
	 \~english @brief Remove the solid normal component of velocity inside the solid.
	 @param[in] solid Solid level set.
	 @param[in] velocity Velocity field.
	 \~japanese @brief 壁の中の速度場の壁の法線成分の速度場を取り除く。
	 @param[in] solid 壁のレベルセット。
	 @param[in] velocity 速度場。
	 */
	virtual void constrain_velocity( const array3<Real> &solid, macarray3<Real> &velocity ) const = 0;
	/**
	 \~english @brief Extrapolate velocity field toward solid and call constrain_velocity routine.
	 @param[in] solid Solid level set.
	 @param[in] velocity Velocity field.
	 @param[in] extrapolate_width Extrapolation length.
	 \~japanese @brief 速度場を壁のレベルセットに外挿して、constrain_velocity ルーチンを呼ぶ。
	 @param[in] solid 壁のレベルセット。
	 @param[in] velocity 速度場。
	 @param[in] extrapolate_width 外挿の幅。
	 */
	virtual void extrapolate_and_constrain_velocity( const array3<Real> &solid, macarray3<Real> &velocity, int extrapolate_width ) const = 0;
	/**
	 \~english @brief Compute area fraction of solid level set.
	 @param[in] solid Solid level set.
	 @param[out] areas Area fractions of solid.
	 \~japanese @brief 壁のレベルセットのセルの面に占める割合を計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[out] areas 面の壁の占める割合。
	 */
	virtual void compute_area_fraction( const array3<Real> &solid, macarray3<Real> &areas ) const = 0;
	/**
	 \~english @brief Compute fraction between cells of fluid level set.
	 @param[in] fluid Fluid level set.
	 @param[out] rhos Density fractions of fluid.
	 \~japanese @brief 水のレベルセットのセルの中心間に占める割合を計算する。
	 @param[in] fluid 水のレベルセット。
	 @param[out] rhos セルの中心間に占める水の割合。
	 */
	virtual void compute_fluid_fraction( const array3<Real> &fluid, macarray3<Real> &rhos ) const = 0;
	/**
	 \~english @brief Compute fraction between cells of fluid level set, considering interference by solid level set.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[out] density Density fractions of fluid level set.
	 \~japanese @brief 壁のレベルセットによる障壁を考慮して、水のレベルセットのセルの中心間に占める割合を計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 水のレベルセット。
	 @param[out] density セルの中心間に占める水の割合。
	 */
	virtual void compute_face_density( const array3<Real> &solid, const array3<Real> &fluid, macarray3<Real> &density ) const = 0;
	/**
	 \~english @brief Get the kinetic energy.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[in] velocity Velocity field.
	 @return Total kinetic energy.
	 \~japanese @brief 運動エネルギーを計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 水のレベルセット。
	 @param[in] velocity 速度場。
	 @return 運動エネルギー。
	 */
	virtual double get_kinetic_energy( const array3<Real> &solid, const array3<Real> &fluid, const macarray3<Real> &velocity ) const = 0;
	/**
	 \~english @brief Get the gravitational potential energy.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[in] velocity Velocity field.
	 @param[in] gravity Gravity vector.
	 @return Gravitational potential energy.
	 \~japanese @brief 重力ポテンシャルエネルギーを計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 水のレベルセット。
	 @param[in] velocity 速度場。
	 @param[in] gravity 重力加速度ベクトル。
	 @return 重力ポテンシャルエネルギー。
	 */
	virtual double get_gravitational_potential_energy( const array3<Real> &solid, const array3<Real> &fluid, vec3d gravity ) const = 0;
	/**
	 \~english @brief Get the surface tension potential energy.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[in] velocity Velocity field.
	 @param[in] tension_coeff Surface tension coefficient.
	 @return Surface tension potential energy.
	 \~japanese @brief 表面張力ポテンシャルエネルギーを計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 水のレベルセット。
	 @param[in] tension_coeff 表面張力の係数。
	 @param[in] velocity 速度場。
	 @return 表面張力ポテンシャルエネルギー。
	 */
	virtual double get_surfacetension_potential_energy( const array3<Real> &solid, const array3<Real> &fluid, double tension_coeff ) const = 0;
	/**
	 \~english @brief Get the total energy.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[in] velocity Velocity field.
	 @param[in] gravity Gravity vector.
	 @param[in] tension_coeff Surface tension coefficient.
	 @return Total energy.
	 \~japanese @brief 全エネルギーを計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 水のレベルセット。
	 @param[in] velocity 速度場。
	 @param[in] gravity 重力加速度のベクトル。
	 @param[in] tension_coeff 表面張力の係数。
	 @return 全エネルギー。
	 */
	virtual double get_total_energy( const array3<Real> &solid, const array3<Real> &fluid, const macarray3<Real> &velocity, vec3d gravity, double tension_coeff ) const = 0;
	/**
	 \~english @brief Get all kinds of energy.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[in] velocity Velocity field.
	 @param[in] gravity Gravity vector.
	 @param[in] tension_coeff Surface tension coefficient.
	 @return List of respective energy {gravitaional energy,kinetic energy, surface area energy}
	 \~japanese @brief 全エネルギーの種類を計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 水のレベルセット。
	 @param[in] velocity 速度場。
	 @param[in] gravity 重力加速度のベクトル。
	 @param[in] tension_coeff 表面張力の係数。
	 @return それぞれのエネルギーの種類。{位置エネルギー、運動エネルギー、表面積エネルギー}
	 */
	virtual std::tuple<double,double,double> get_all_kinds_of_energy( const array3<Real> &solid, const array3<Real> &fluid, const macarray3<Real> &velocity, vec3d gravity, double tension_coeff ) const = 0;
	/**
	 \~english @brief Get the jacobian of a velocity.
	 @param[in] p Position in physical space.
	 @param[in] velocity Velocity field.
	 @param[out] jacobian Jacobian output.
	 \~japanese @brief 速度のヤコビアンを取得する。
	 @param[in] p 物理空間での位置。
	 @param[in] velocity 速度場。
	 @param[out] jacobian 出力のヤコビアン。
	 */
	virtual void get_velocity_jacobian( const vec3d &p, const macarray3<Real> &velocity, vec3r jacobian[DIM3] ) const = 0;
	/**
	 \~english @brief Assign initial velocity field, solid level set, fluid level set, density field from the dynamic library.
	 @param[in] dylib Reference to an instance of dynamic library.
	 @param[out] velocity Initial velocity field.
	 @param[out] solid Initial solid level set.
	 @param[out] fluid Initial fluid level set.
	 @param[out] density Initial density field.
	 \~japanese @brief 動的ライブラリから速度場、壁のレベルセット、流体のレベルセットと密度場をセットする。
	 @param[out] velocity 初期の速度場。
	 @param[out] solid 初期の壁のレベルセット。
	 @param[out] fluid 初期の流体のレベルセット。
	 @param[out] density 初期の密度場。
	 */
	virtual void assign_initial_variables( const dylibloader &dylib, macarray3<Real> &velocity,
									array3<Real> *solid=nullptr, array3<Real> *fluid=nullptr,array3<Real> *density=nullptr ) const = 0;
	/**
	 \~english @brief Add force to the velocity field.
	 @param[in] p Position in physical space.
	 @param[in] f External force.
	 @param[in-out] external_force Vector field in which the external force is added.
	 \~japanese @brief 速度場に外力を加える。
	 @param[in] p 物理空間の位置。
	 @param[in] f 外力。
	 @param[in-out] external_force 外力が加えられるベクトル場。
	 */
	virtual void add_force( vec3d p, vec3d f, macarray3<Real> &external_force ) const = 0;
	//
private:
	virtual void initialize( const shape3 &gn, double dx ) = 0;
	virtual void initialize( const configurable::environment_map &environment ) override {
		//
		assert(check_set(environment,{"shape","dx"}));
		initialize(
			get_env<shape3>(environment,"shape"),
			get_env<double>(environment,"dx")
		);
	}
};
//
using macutility3_ptr = std::unique_ptr<macutility3_interface>;
using macutility3_driver = recursive_configurable_driver<macutility3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif