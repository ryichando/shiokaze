/*
**	macbackwardflip2_interface.h
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
#ifndef SHKZ_BACKWARDFLIP2_INTERFACE_H
#define SHKZ_BACKWARDFLIP2_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/array/array2.h>
#include <shiokaze/array/macarray2.h>
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for "A Long-Term Semi-Lagrangian Method for Accurate Velocity Advection". "macbackwardflip2" is provided as an actual implementation.
/// \~japanese @brief "A Long-Term Semi-Lagrangian Method for Accurate Velocity Advection" のインターフェース。"macbackwardflip2" が実装として提供される。
class macbackwardflip2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macbackwardflip2_interface,"MAC Backward FLIP 2D","BackwardFLIP","Backward FLIP Advection module")
	/**
	 \~english @brief Perform a long-term backtace.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 \~japanese @brief ロングターム・バックトレースを行う。
	 @param[in] solid 壁面のレベルセット。
	 @param[in] fluid 流体のレベルセット。
	 */
	virtual bool backtrace( const array2<double> &solid, const array2<double> &fluid ) = 0;
	/**
	 \~english @brief Get the reconstructed velocity field after the long-term backtrace.
	 @param[out] u_reconstructed Reconstructed velocity.
	 \~japanese @brief バックトレース後に再構築された速度場を得る。
	 @param[out] u_reconstructed 再構築された速度場。
	 */
	virtual bool fetch( macarray2<double> &u_reconstructed ) const = 0;
	/**
	 \~english @brief Get the reconstructed density field after the long-term backtrace.
	 @param[out] density_reconstructed Reconstructed density.
	 \~japanese @brief バックトレース後に再構築された密度場を得る。
	 @param[out] density_reconstructed 再構築された密度場。
	 */
	virtual bool fetch( array2<double> &density_reconstructed ) const = 0;
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
	virtual void registerBuffer(
						const macarray2<double> &u1,
						const macarray2<double> &u0,
						const macarray2<double> *u_reconstructed,
						const macarray2<double> *g,
						const array2<double> *d1,
						const array2<double> *d0,
						const array2<double> *d_added,
						double dt ) = 0;
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
	virtual void initialize( const shape2 &shape, double dx ) = 0;
	virtual void initialize( const configurable::environment_map &environment ) override {
		//
		assert(check_set(environment,{"shape","dx"}));
		//
		const shape2 &shape = *reinterpret_cast<const shape2 *>(environment.at("shape"));
		double dx = *reinterpret_cast<const double *>(environment.at("dx"));
		initialize(shape,dx);
	}
};
//
using macbackwardflip2_ptr = std::unique_ptr<macbackwardflip2_interface>;
using macbackwardflip2_driver = recursive_configurable_driver<macbackwardflip2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//