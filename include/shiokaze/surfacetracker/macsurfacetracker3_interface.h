/*
**	macsurfacetracker3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on May 1, 2017. 
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
#ifndef SHKZ_MACSURFACETRACKER3_INTERFACE_H
#define SHKZ_MACSURFACETRACKER3_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/array3.h>
#include <shiokaze/array/macarray3.h>
#include <string>
#include <functional>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for advecting level set surfaces. "maclevelsetsurfacetracker3" is provided as implementation.
/// \~japanese @brief レベルセット界面を移流するインターフェース。"maclevelsetsurfacetracker3" が実装として提供される。
class macsurfacetracker3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macsurfacetracker3_interface,"MAC Surface Tracker 3D","SurfaceTracker","Moving level set tracking module")
	/**
	 \~english @brief Assign fluid and solid level set.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 \~japanese @brief 流体と壁のレベルセットをセットする。
	 */
	virtual void assign( const array3<float> &solid, const array3<float> &fluid ) = 0;
	/**
	 \~english @brief Advect level set.
	 @param[in] u Velocity with which to advect.
	 @param[in] dt Time step size.
	 \~japanese @brief レベルセットを移流する。
	 @param[in] u 移流に使用する速度場。
	 @param[in] dt タイムステップサイズ。
	 */
	virtual void advect( const macarray3<float> &u, double dt ) = 0;
	/**
	 \~english @brief Get the level set.
	 @param[out] fluid Output level set.
	 \~japanese @brief レベルセットを取得する。
	 @param[out] fluid 出力のレベルセット。
	 */
	virtual void get( array3<float> &fluid ) = 0;
	/**
	 \~english @brief Draw level set surface.
	 @param[in] g Graphics engine.
	 \~japanese @brief レベルセットサーフェスを描画する。
	 @param[int] g グラフィックスエンジン。
	 */
	virtual void draw( graphics_engine &g ) const = 0;
	/**
	 \~english @brief Export level set surface as a mesh file.
	 @param[in] path_to_directory Path to the directory to export.
	 @param[in] frame Frame number.
	 @param[in] vertex_color_func Vertex color function. Can be nullptr.
	 @param[in] uv_coordinate_func UV coordinate function. Can be nullptr.
	 \~japanese @brief レベルセット界面をメッシュファイルとしてファイルに書き出す。
	 @param[in] path_to_directory ディレクトリへのパス。
	 @param[in] frame フレーム番号。
	 @param[in] vertex_color_func 頂点カラー関数。nullptr も可。
	 @param[in] uv_coordinate_func UV座標関数。nullptr も可。
	 */
	virtual void export_fluid_mesh(std::string path_to_directory, unsigned frame,
							 std::function<vec3d(const vec3d &)> vertex_color_func=nullptr,
							 std::function<vec2d(const vec3d &)> uv_coordinate_func=nullptr ) const = 0;
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
using macsurfacetracker3_ptr = std::unique_ptr<macsurfacetracker3_interface>;
using macsurfacetracker3_driver = recursive_configurable_driver<macsurfacetracker3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif