/*
**	macbackwardflip3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Dec 8, 2017. 
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
#ifndef SHKZ_BACKWARDFLIP3_INTERFACE_H
#define SHKZ_BACKWARDFLIP3_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/array/array3.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for "A Long-Term Semi-Lagrangian Method for Accurate Velocity Advection". "macbackwardflip3" is provided as an actual implementation.
/// \~japanese @brief "A Long-Term Semi-Lagrangian Method for Accurate Velocity Advection" のインターフェース。"macbackwardflip3" が実装として提供される。
class macbackwardflip3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macbackwardflip3_interface,"MAC Backward FLIP 3D","BackwardFLIP","Backward FLIP Advection module");
	/**
	 \~english @brief Perform a long-term backtace.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 \~japanese @brief ロングターム・バックトレースを行う。
	 @param[in] solid 壁面のレベルセット。
	 @param[in] fluid 流体のレベルセット。
	 */
	virtual bool backtrace( const array3<float> &solid, const array3<float> &fluid ) = 0;
	/**
	 \~english @brief Get the reconstructed velocity field after the long-term backtrace.
	 @param[out] u_reconstructed Reconstructed velocity.
	 \~japanese @brief バックトレース後に再構築された速度場を得る。
	 @param[out] u_reconstructed 再構築された速度場。
	 */
	virtual bool fetch( macarray3<float> &u_reconstructed ) const = 0;
	/**
	 \~english @brief Get the reconstructed density field after the long-term backtrace.
	 @param[out] density_reconstructed Reconstructed density.
	 \~japanese @brief バックトレース後に再構築された密度場を得る。
	 @param[out] density_reconstructed 再構築された密度場。
	 */
	virtual bool fetch( array3<float> &density_reconstructed ) const = 0;
	/**
	 \~english @brief Add a layer of velocity field.
	 @param[in] u1 Velocity at the end of the step.
	 @param[in] u0 Velocity at the beggining of the step.
	 @param[in] u_reconstructed Reconstructed dirty velocity of the beggining of the step - can be nullptr.
	 @param[in] g Pressure gradient and the external forces (scaled by dt) - can be nullptr.
	 @param[in] d1 Density field of the end of the step - can be nullptr.
	 @param[in] d0 Density field of the beggining of the step - can be nullptr.
	 @param[in] d_added Density field added in the current step - can be nullptr.
	 @param[in] dt Time-step size of the current step.
	 \~japanese @brief 速度場のレイヤーを追加する。
	 @param[in] u1 タイムステップを計算した後の速度場。
	 @param[in] u0 タイムステップを計算する前の速度場。
	 @param[in] u_reconstructed タイムステップを計算する前の速度場を再構築したもの。nullptr でも可。
	 @param[in] g 圧力勾配と外力項 (タイムステップが掛けられたもの)。nullptr でも可。
	 @param[in] d1 タイムステップを計算した後の密度場。nullptr でも可。
	 @param[in] d0 タイムステップを計算する前の密度場。nullptr でも可。
	 @param[in] d_added タイムステップ計算中に追加された密度場。nullptr でも可。
	 @param[in] dt 現在のタイムステップ幅。
	 */
	virtual void register_buffer(
						const macarray3<float> &u1,				// Velocity at the end of the step
						const macarray3<float> &u0,				// Velocity at the beggining of the step
						const macarray3<float> *u_reconstructed,	// Reconstructed dirty velocity of the beggining of the step - can be nullptr
						const macarray3<float> *g,					// Pressure gradient and the external forces (scaled by dt) - can be nullptr
						const array3<float> *d1,					// Density field of the end of the step - can be nullptr
						const array3<float> *d0,					// Density field of the beggining of the step - can be nullptr
						const array3<float> *d_added,				// Density field source of the current step - can be nullptr
						double dt ) = 0;							// Time-step size of the current step
	/**
	 \~english @brief Draw simulation information for debugging.
	 @param[in] g Graphics engine.
	 \~japanese @brief デバッグのためにシミュレーション情報を描画する。
	 @param[in] g グラフィックスエンジン。
	 */
	virtual void draw( graphics_engine &g ) const = 0;
	//
private:
	//
	virtual void initialize( const shape3 &shape, double dx ) = 0;
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
using macbackwardflip3_ptr = std::unique_ptr<macbackwardflip3_interface>;
using macbackwardflip3_driver = recursive_configurable_driver<macbackwardflip3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//