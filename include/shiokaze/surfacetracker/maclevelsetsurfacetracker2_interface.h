/*
**	maclevelsetsurfacetracker2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 28, 2017. 
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
#ifndef SHKZ_MACLEVELSETSURFACETRACKER2_INTERFACE_H
#define SHKZ_MACLEVELSETSURFACETRACKER2_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/array2.h>
#include <shiokaze/array/macarray2.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for advecting level set surfaces. "maclevelsetsurfacetracker2" is provided as implementation.
/// \~japanese @brief レベルセット界面を移流するインターフェース。"maclevelsetsurfacetracker2" が実装として提供される。
class maclevelsetsurfacetracker2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(maclevelsetsurfacetracker2_interface,"MAC Levelset Surface Tracker 2D","LevelsetSurfaceTracker","Moving level set tracking module")
	/**
	 \~english @brief Advect level set.
	 @param[in] fluid Cell centered fluid level set.
	 @param[in] solid Nodal solid level set.
	 @param[in] u Velocity with which to advect.
	 @param[in] dt Time step size.
	 \~japanese @brief レベルセットを移流する。
	 @param[in] fluid セルセンターのレベルセット。
	 @param[in] solid 節点ベースの壁のレベルセット。
	 @param[in] u 移流に使用する速度場。
	 @param[in] dt タイムステップサイズ。
	 */
	virtual void advect( array2<float> &fluid, const array2<float> &solid, const macarray2<float> &u, double dt ) = 0;
	//
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
using macsurfacetracker2_ptr = std::unique_ptr<maclevelsetsurfacetracker2_interface>;
using macsurfacetracker2_driver = recursive_configurable_driver<maclevelsetsurfacetracker2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif