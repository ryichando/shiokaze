/*
**	cellmesher3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 4, 2017. 
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
#ifndef SHKZ_CELLMESHER3_H
#define SHKZ_CELLMESHER3_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/array3.h>
#include <shiokaze/math/vec.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that generates surrface meshes from level set grid. "dualmc" and "marchingcubes" are provided as actual implementations.
/// \~japanese @brief レベルセットグリッドからサーフェスメッシュを生成するインターフェース。"dualmc" と "marchingcubes" が具体的な実装として提供される。
class cellmesher3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(cellmesher3_interface,"Cell Mesher 3D","CellMesher","Cell mesher module")
	/**
	 \~english @brief Generate mesh from a level set grid.
	 @param[in] levelset Level set grid.
	 @param[out] vertices Vertices of generarted surface meshes.
	 @param[out] faces Triangles or quads of generated surface meshes.
	 \~japanese @brief レベルセットグリッドからサーフェスメッシュを生成する。
	 @param[in] levelset レベルセットグリッド。
	 @param[out] vertices 生成されたメッシュの頂点列。
	 @param[out] faces 生成されたメッシュの三角形あるいは四角形配列。
	 */
	virtual void generate_mesh( const array3<double> &levelset, std::vector<vec3d> &vertices, std::vector<std::vector<size_t> > &faces ) const = 0;
	//
private:
	//
	virtual void initialize ( const shape3 &shape, double dx ) = 0;
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
using cellmesher3_ptr = std::unique_ptr<cellmesher3_interface>;
using cellmesher3_driver = recursive_configurable_driver<cellmesher3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif