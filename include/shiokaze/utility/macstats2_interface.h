/*
**	macstats2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 18, 2018.
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
#ifndef SHKZ_MACSTATS2_INTERFACE_H
#define SHKZ_MACSTATS2_INTERFACE_H
//
#include <shiokaze/array/macarray2.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/timestepper/timestepper_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for analyizing MAC velocity. "macstats2" is provided as implementation.
/// \~japanese @brief MAC 速度場を分析するインターフェース。"macstats2" が実装として提供される。
class macstats2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macstats2_interface,"MAC Stats 2D","MacStats","MAC Statistics Analyzer")
	/**
	 \~english @brief Print status of the mac velocity.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[in] velocity Velocity.
	 @param[in] tmstepper Pointer to an instance of timestepper_interface.
	 \~japanese @brief MAC 速度場の統計をコンソールに出力する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 流体のレベルセット。
	 @param[in] velocity 速度場。
	 @param[in] tmstepper timestepper_interface のインスタンスへのポインター。
	 */
	virtual void dump_stats( const array2<Real> &solid, const array2<Real> &fluid, const macarray2<Real> &velocity, const timestepper_interface *tmstepper ) const = 0;
private:
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
using macstats2_ptr = std::unique_ptr<macstats2_interface>;
using macstats2_driver = recursive_configurable_driver<macstats2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//