/*
**	gridvisualizer3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 28, 2017. 
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
#ifndef SHKZ_GRIDVISUALIZER3_INTERFACE_H
#define SHKZ_GRIDVISUALIZER3_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/array3.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for visualizing grid attributes. "gridvisualizer3" is provided as implementation.
/// \~japanese @brief グリッドの属性を描画するためのインターフェース。"gridvisualizer3" が実装として提供される。
class gridvisualizer3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(gridvisualizer3_interface,"Grid Visualizer 3D","GridVisualizer","Grid visualizer module")
	/**
	 \~english @brief Draw active cells.
	 @param[in] g Graphics engine.
	 @param[in] q Target grid.
	 \~japanese @brief アクティブなセルを描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] q 目標となるグリッド。
	 */
	virtual void draw_active( graphics_engine &g, const array3<float> &q ) const = 0;
	/**
	 \~english @brief Draw filled cells.
	 @param[in] g Graphics engine.
	 @param[in] q Target grid.
	 \~japanese @brief 塗りつぶされたセルを描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] q 目標となるグリッド。
	 */
	virtual void draw_inside( graphics_engine &g, const array3<float> &q ) const = 0;
	/**
	 \~english @brief Draw grid.
	 @param[in] g Graphics engine.
	 \~japanese @brief グリッドを描画する。
	 @param[in] g グラフィックスエンジン。
	 */
	virtual void draw_grid( graphics_engine &g ) const = 0;
	/**
	 \~english @brief Draw a density field.
	 @param[in] g Graphics engine.
	 @param[in] density Density field.
	 \~japanese @brief 密度場を描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] density 密度場。
	 */
	virtual void draw_density( graphics_engine &g, const array3<float> &density ) const = 0;
	/**
	 \~english @brief Draw velocity.
	 @param[in] g Graphics engine.
	 @param[in] velocity Velocity field.
	 \~japanese @brief 速度場を描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] velocity 速度場。
	 */
	virtual void draw_velocity( graphics_engine &g, const array3<vec3f> &velocity ) const = 0;
	/**
	 \~english @brief Draw level set grid.
	 @param[in] g Graphics engine.
	 @param[in] levelset Level set grid.
	 \~japanese @brief レベルセットグリッドを描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] levelset レベルセットグリッド。
	 */
	virtual void draw_levelset( graphics_engine &g, const array3<float> &levelset ) const = 0;
	/**
	 \~english @brief Draw level set of solid.
	 @param[in] g Graphics engine.
	 @param[in] solid Level set of solid.
	 \~japanese @brief 壁のレベルセットグリッドを描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] solid 壁のレベルセットグリッド。
	 */
	virtual void draw_solid( graphics_engine &g, const array3<float> &solid ) const = 0;
	/**
	 \~english @brief Draw level set of fluid.
	 @param[in] g Graphics engine.
	 @param[in] fluid Level set of fluid.
	 \~japanese @brief 流体のレベルセットグリッドを描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] fluid 流体のレベルセットグリッド。
	 */
	virtual void draw_fluid( graphics_engine &g, const array3<float> &solid, const array3<float> &fluid ) const = 0;
	/**
	 \~english @brief Draw cell-based scalar.
	 @param[in] g Graphics engine.
	 @param[in] q Target grid.
	 \~japanese @brief セルベースのスカラー値を描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] q 目標となるグリッド。
	 */
	virtual void visualize_cell_scalar( graphics_engine &g, const array3<float> &q ) const = 0;
	/**
	 \~english @brief Draw nodal scalar.
	 @param[in] g Graphics engine.
	 @param[in] q Target grid.
	 \~japanese @brief 節点ベースのスカラー値を描画する。
	 @param[in] g グラフィックスエンジン。
	 @param[in] q 目標となるグリッド。
	 */
	virtual void visualize_nodal_scalar( graphics_engine &g, const array3<float> &q ) const = 0;
	//
private:
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
using gridvisualizer3_ptr = std::unique_ptr<gridvisualizer3_interface>;
using gridvisualizer3_driver = recursive_configurable_driver<gridvisualizer3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//