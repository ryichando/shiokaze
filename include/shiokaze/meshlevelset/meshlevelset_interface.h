/*
**	meshlevelset_interface.h
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
#ifndef SHKZ_MESHLEVELSET_INTERFACE_H
#define SHKZ_MESHLEVELSET_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/array/array3.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that converts mesh to level set grid. "SDFGen" is provided as implementation.
/// \~japanese @brief メッシュをレベルセットに変換するインターフェース。"SDFGen" が実装として提供される。
class meshlevelset_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(meshlevelset_interface,"Signed Distance Field Converter","MeshLevelset","Mesh to levelset module")
	/**
	 \~english @brief Set a mesh.
	 @param[in] vertices Vertices.
	 @param[in] faces Faces.
	 \~japanese @brief メッシュをセットする。
	 @param[in] vertices 頂点列。
	 @param[in] faces 面。
	 */
	virtual void set_mesh( const std::vector<vec3d> &vertices, const std::vector<std::vector<size_t> > &faces ) = 0;
	/**
	 \~english @brief Generate a level set grid from the set mesh.
	 \~japanese @brief セットされたメッシュからレベルセットを生成する。
	 */
	virtual void generate_levelset() = 0;
	/**
	 \~english @brief Get level set at an arbitrary position.
	 @param[in] p Poisition.
	 \~japanese @brief 任意の位置でのレベルセットを生成する。
	 @param[in] p 位置。
	 */
	virtual double get_levelset( const vec3d &p ) const = 0;
	//
private:
	virtual void initialize( double dx ) = 0;
	virtual void initialize( const configurable::environment_map &environment ) override {
		assert(check_set(environment,{"dx"}));
		double dx = *reinterpret_cast<const double *>(environment.at("dx"));
		initialize(dx);
	}
};
//
using meshlevelset_ptr = std::unique_ptr<meshlevelset_interface>;
using meshlevelset_driver = recursive_configurable_driver<meshlevelset_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
