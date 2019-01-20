/*
**	polygon3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Februrary 6, 2018.
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
#ifndef SHKZ_POLYGON3_INTERFACE_H
#define SHKZ_POLYGON3_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <vector>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for loading mesh files. "polygon3" is provided as implementation.
/// \~japanese @brief メッシュファイルを読み込むためのインターフェース。"polygon3" が実装として提供される。
class polygon3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(polygon3_interface,"Polygon Loader","PolyLoader","Polygon loader module")
	/**
	 \~english @brief Load a mesh from a file.
	 @param[in] path Path to the mesh file.
	 @return \c true if loading was successful. \c false otherwise.
	 \~japanese @brief メッシュをファイルに読み込む。
	 @param[in] path メッシュファイルへのパス。
	 @return もし読み込みが成功すれば \c true を、失敗すれば \c false を返す。
	*/
	virtual bool load_mesh( std::string path ) = 0;
	/**
	 \~english @brief Get the loaded mesh.
	 @param[out] vertices Vertices.
	 @param[out] faces Mesh faces.
	 \~japanese @brief 読み込んだメッシュを取得する。
	 @param[out] vertices 頂点列。
	 @param[out] faces メッシュの face 群。
	*/
	virtual void get_mesh( std::vector<vec3d> &vertices, std::vector<std::vector<size_t> > &faces ) = 0;
};
//
using polygon3_ptr = std::unique_ptr<polygon3_interface>;
using polygon3_driver = recursive_configurable_driver<polygon3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif