/*
**	macvisualizer3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 21, 2017. 
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
#ifndef SHKZ_MACVISUALIZER3_INTERFACE_H
#define SHKZ_MACVISUALIZER3_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/macarray3.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for visualizing MAC grids. "macvisualizer3" is provided as implementation.
/// \~japanese @brief MAC グリッドの属性を描画するためのインターフェース。"macvisualizer3" が実装として提供される。
class macvisualizer3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macvisualizer3_interface,"MAC Visualizer 3D","MacVisualizer","MAC visualizer module")
	/**
	 \~english @brief Draw velocity field.
	 @param[in] g Graphics engine.
	 @param[in] velocity Velocity field.
	 \~japanese @brief 速度場を描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] velocity 速度場。
	 */
	virtual void draw_velocity( graphics_engine &g, const macarray3<float> &velocity ) const = 0;
	/**
	 \~english @brief Draw scalar field.
	 @param[in] g Graphics engine.
	 @param[in] array Target grid.
	 \~japanese @brief スカラー値を描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] array Target grid.
	 */
	virtual void visualize_scalar( graphics_engine &g, const macarray3<float> &array ) const = 0;
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
using macvisualizer3_ptr = std::unique_ptr<macvisualizer3_interface>;
using macvisualizer3_driver = recursive_configurable_driver<macvisualizer3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//