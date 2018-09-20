/*
**	macflip2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 13, 2017. 
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
#ifndef SHKZ_MACFLIP2_INTERFACE_H
#define SHKZ_MACFLIP2_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/array/macarray2.h>
#include <functional>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for FLIP. "macnbflip2" and "macexnbflip2" are provided as actual implementaions.
/// \~japanese @brief FLIP のためのインターフェース。"macnbflip2" と "macexnbflip2" が実際の実装として提供される。
class macflip2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macflip2_interface,"MAC FLIP 2D","FLIP","FLIP engine module")
	/**
	 \~english @brief Assign solid level set.
	 @param[in] solid Level set grid of solid.
	 \~japanese @brief 壁のレベルセットを与える。
	 @param[in] solid 壁のレベルセットグリッド。
	 */
	virtual void assign_solid( const array2<double> &solid ) = 0;
	/**
	 \~english @brief Seed FLIP particles where the given input fluid level set region is inside.
	 @param[in] fluid Level set grid of fluid.
	 @param[in] velocity Grid of which initial velocity field is assigned.
	 \~japanese @brief 与えられた入力の流体レベルセットグリッドの内部に FLIP 粒子を散布する。
	 @param[in] fluid 流体のレベルセットグリッド。
	 @param[in] velocity 初期速度場を与えるための速度場。
	 */
	virtual size_t seed( const array2<double> &fluid, const macarray2<double> &velocity ) = 0;
	/**
	 \~english @brief Splat FLIP momentum and mass onto the grids.
	 @param[out] momentum Grid of momentum to be mapped.
	 @param[out] mass Grid of mass to be mapped.
	 \~japanese @brief FLIP 粒子の運動量と質量をグリッドに転写する。
	 @param[out] momentum 転写される運動量のグリッド。
	 @param[out] mass 転写される質量のグリッド。
	 */
	virtual void splat( macarray2<double> &momentum, macarray2<double> &mass ) const = 0;
	/**
	 \~english @brief Advect FLIP particles along the input velocity field.
	 @param[in] velocity Velocity field from which to advect.
	 @param[in] time Time current simulation time. Alternatively speaking, accumulated dt.
	 @param[in] dt Time step size.
	 \~japanese @brief 入力の速度場に沿って FLIP 粒子を移流する。
	 @param[in] velocity 移流に使われる速度場。
	 @param[in] time 現在のシミュレーション時間。あるいは、dt を蓄積したもの。
	 @param[in] dt タイムステップサイズ。
	 */
	virtual void advect( const macarray2<double> &velocity, double time, double dt ) = 0;
	/**
	 \~english @brief Update momentum of FLIP particles.
	 @param[in] prev_velocity Velocity field before the pressure projection.
	 @param[in] new_velocity New velocity after the pressure projection.
	 @param[in] dt Time step size.
	 @param[in] gravity Gravity coefficient.
	 @param[in] PICFLIP PIC and FLIP interpolation coefficient. 1.0 indicates FLIP and 0.0 indicates PIC. When APIC is used, this parameter will be simply ignored.
	 \~japanese @brief FLIP 粒子の運動量を更新する。
	 @param[in] prev_velocity 圧力投影前の速度場。
	 @param[in] new_velocity 圧力投影後の速度場。
	 @param[in] dt タイムステップサイズ。
	 @param[in] gravity 重力定数。
	 @param[in] PICFLIP PIC と FLIP を補間する定数。1.0 なら FLIP を、0.0 なら PIC となる。もし APIC が使用される時、このパラメータは無視される。
	 */
	virtual void update( const macarray2<double> &prev_velocity, const macarray2<double> &new_velocity,
						 double dt, vec2d gravity, double PICFLIP ) = 0;
	/**
	 \~english @brief Directly update momentum of FLIP particles.
	 @param[in] func Function to specify new momentum.
	 \~japanese @brief FLIP 粒子の運動量を直接更新する。
	 @param[in] func 新しい運動量を指定する関数。
	 */
	virtual void update( std::function<void(const vec2d &p, vec2d &velocity, double &mass, bool bullet )> func ) = 0;
	/**
	 \~english @brief Get the level set of FLIP particles.
	 @param[out] Level set of the current FLIP particles.
	 \~japanese @brief FLIP 粒子のレベルセットを取得する。
	 @param[out] 現在の FLIP 粒子のレベルセット。
	 */
	virtual void get_levelset( array2<double> &fluid ) const = 0;
	/**
	 \~english @brief Draw FLIP particles.
	 @param[in] g Graphics engine.
	 @param[in] time Current simulation time.
	 \~japanese @brief FLIP 粒子を描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] time 現在のシミュレーション時間。
	 */
	virtual void draw( const graphics_engine &g, double time=0.0 ) const = 0;
	/**
	 \~english @brief Get the number of FLIP particles.
	 @return Number of FLIP particles.
	 \~japanese @brief FLIP 粒子の数を取得する。
	 @return Number of FLIP particles.
	 */
	virtual size_t get_particle_count() const = 0;
	//
	/// \~english @brief Structure for FLIP particles
	/// \~japanese @brief FLIP 粒子の構造体
	struct particle2 {
		/**
		 \~english @brief Position.
		 \~japanese @brief 位置。
		 */
		vec2d p;
		/**
		 \~english @brief Radius.
		 \~japanese @brief 半径。
		 */
		double r;
		/**
		 \~english @brief Whether the particle is ballistic.
		 \~japanese @brief 弾丸粒子か。
		 */
		bool bullet;
		/**
		 \~english @brief Time when bullet is marked.
		 \~japanese @brief 弾丸粒子にマーキングされた時の時間
		 */
		double bullet_time;
	};
	/**
	 \~english @brief Get all the FLIP particles.
	 @return The full list of FLIP particles.
	 \~japanese @brief 全ての FLIP 粒子を得る。
	 @return FLIP 粒子の全リスト。
	 */
	virtual std::vector<particle2> get_particles() const = 0;
	//
private:
	//
	// Initialize FLIP instance
	virtual void initialize( const shape2 &shape, double dx ) = 0;
	virtual void initialize( const configurable::environment_map &environment ) override {
		//
		assert(check_set(environment,{"shape","dx"}));
		initialize(
			get_env<shape2>(environment,"shape"),
			get_env<double>(environment,"dx")
		);
	}
};
//
using macflip2_ptr = std::unique_ptr<macflip2_interface>;
using macflip2_driver = recursive_configurable_driver<macflip2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif