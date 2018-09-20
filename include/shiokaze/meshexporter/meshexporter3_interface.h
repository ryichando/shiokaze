/*
**	meshexporter3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 12, 2018. 
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
#ifndef SHKZ_MESHEXPORTER3_INTERFACE_H
#define SHKZ_MESHEXPORTER3_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that provides mesh loading and exporting. "meshexporter3" is provided as implementation.
/// \~japanese @brief メッシュの読み込みと書き出しを提供するインターフェース。"meshexporter3" が実装として提供される。
class meshexporter3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(meshexporter3_interface,"Mesh Exporter 3D","MeshExporter","Mesh exporter module")
	/**
	 \~english @brief Set a mesh.
	 @param[in] vertices Verticse.
	 @param[in] faces Faces.
	 \~japanese @brief メッシュをセットする。
	 @param[in] vertices 頂点列。
	 @param[in] faces 面の配列。
	 */
	virtual void set_mesh( const std::vector<vec3d> &vertices, const std::vector<std::vector<size_t> > &faces ) = 0;
	/**
	 \~english @brief Set vertex colors.
	 @param[in] vertex_colors Vertex color in RGB.
	 \~japanese @brief 頂点の色を設定する。
	 @param[in] vertex_colors 頂点の色。
	 */
	virtual void set_vertex_colors( const std::vector<vec3d> &vertex_colors ) = 0;
	/**
	 \~english @brief Set texture coordinate.
	 @param[in] uv_coordinates UV coordinates on vertices.
	 \~japanese @brief テクスチャ座標をセットする。
	 @param[in] uv_coordinates 頂点毎のテクスチャ座標。
	 */
	virtual void set_texture_coordinates( const std::vector<vec2d> &uv_coordinates ) = 0;
	/**
	 \~english @brief Export as PLY file.
	 @param[in] path File path.
	 \~japanese @brief PLY として出力する。
	 @param[in] path ファイルのパス。
	 */
	virtual bool export_ply( std::string path ) = 0;
	/**
	 \~english @brief Export as Mistuba file.
	 @param[in] path File path.
	 \~japanese @brief Mistuba として出力する。
	 @param[in] path ファイルのパス。
	 */
	virtual bool export_mitsuba( std::string path ) = 0;
};
//
using meshexporter3_ptr = std::unique_ptr<meshexporter3_interface>;
using meshexporter3_driver = recursive_configurable_driver<meshexporter3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
