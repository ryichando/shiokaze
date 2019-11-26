/*
**	timestepper_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 11, 2017.
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
#ifndef SHKZ_TIMESTEPPER_INTERFACE_H
#define SHKZ_TIMESTEPPER_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for handling adaptive time stepping.
/// \~japanese @brief アダプティブなタイムステップを扱うインターフェース。
class timestepper_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(timestepper_interface,"Adaptive Time Stepper","TimeStepper","Time stepper module")
	/**
	 \~english @brief Advance time by the maximal velocity.
	 @param[in] max_velocity The magnitude of the maximal velocity in the domain.
	 @param[in] dx Grid cell size.
	 @return Time step size.
	 @param[in] max_velocity 計算領域内の最大速度。
	 @param[in] dx グリッドセルの大きさ。
	 \~japanese @brief 時間を最大速度場で進める。
	 @return タイムステップサイズ。
	 */
	virtual double advance( double max_velocity, double dx ) = 0;
	/**
	 \~english @brief Get if video frame should be exported.
	 @return Frame number if a video frame should be exported. Zero otherwise.
	 \~japanese @brief ビデオフレームが出力されるべきか取得する。
	 @return もし出力されるべきなら、そのフレーム番号を返す。もし出力されるべきでないなら、ゼロを返す。
	 */
	virtual int should_export_frame() const = 0;
	/**
	 \~english @brief Get simulation time spent for computing one video frame.
	 @return Millisecond time.
	 \~japanese @brief 一つのビデオフレームを計算するのにかかった時間を取得する。
	 @return ミリセカンド秒。
	 */
	virtual double get_simulation_time_per_video_frame() const = 0;
	/**
	 \~english @brief Get simulation time spent for computing one time step.
	 @return Millisecond time.
	 \~japanese @brief 一つのタイムステップを計算するのにかかった時間を取得する。
	 @return ミリセカンド秒。
	 */
	virtual double get_simulation_time_per_step() const = 0;
	/**
	 \~english @brief Get current time (accumulated time step sizes).
	 @return Millisecond time.
	 \~japanese @brief 現在の時間を取得する。(蓄積されたタイムステップサイズ)
	 @return ミリセカンド秒。
	 */
	virtual double get_current_time() const = 0;
	/**
	 \~english @brief Get total calculation time.
	 @return Millisecond time.
	 \~japanese @brief 現在の計算にかかった総時間を取得する。
	 @return ミリセカンド秒。
	 */
	virtual double get_total_calculation_time() const = 0;
	/**
	 \~english @brief Get the current CFL number.
	 @return CFL number.
	 \~japanese @brief 現在のCFL数を取得する。
	 @return CFL数。
	 */
	virtual double get_current_CFL() const = 0;
	/**
	 \~english @brief Get the target CFL number.
	 @return CFL number.
	 \~japanese @brief 目標とするCFL数を取得する。
	 @return CFL数。
	 */
	virtual double get_target_CFL() const = 0;
	/**
	 \~english @brief Get the current step count.
	 @return Step count.
	 \~japanese @brief 現在のステップカウントを取得する。
	 @return ステップ数。
	 */
	virtual unsigned get_step_count() const = 0;
	/**
	 \~english @brief Get if we should quit the simulation because the simulation ran long enough.
	 @return \c true if the simulation should quit. \false otherwise.
	 \~japanese @brief シミュレーションの実行時間が十分になったので、シミュレーションを終えるべきかを取得する。
	 @return もしシミュレーションを終了すべきなら \c true を、そうでないなら \c false を返す。
	 */
	virtual bool should_quit() const = 0;
};
//
using timestepper_ptr = std::unique_ptr<timestepper_interface>;
using timestepper_driver = recursive_configurable_driver<timestepper_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//