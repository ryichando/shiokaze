/*
**	gridutility3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 15, 2017. 
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
#ifndef SHKZ_GRIDUTILITY3_INTERFACE_H
#define SHKZ_GRIDUTILITY3_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/array/array3.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for handling grid related operations. "gridutility3" is provided as implementation.
/// \~japanese @brief グリッドの関係する操作を処理するインターフェース。"gridutility3" が実装として提供される。
class gridutility3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(gridutility3_interface,"Grid Utility 3D","GridUtility","Grid utility module")
	/**
	 \~english @brief Convert a nodal grid to a cell-centered grid.
	 @param[in] nodal_array Nodal grid.
	 @param[out] result Resulting cell-based array.
	 \~japanese @brief ノードベースのグリッドをセルベースのグリッドに変換する。
	 @param[in] nodal_array ノードベースのグリッド。
	 @param[out] result 結果のセルベースのグリッド。
	 */
	virtual void convert_to_cell( const array3<double> &nodal_array, array3<double> &result ) const = 0;
	/**
	 \~english @brief Enclose a fluid level set by solid.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @param[out] combined Encapsulated level set.
	 @param[in] solid_offset Offset of the solid level set.
	 \~japanese @brief 流体のレベルセットを壁のレベルセットで包む。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 流体のレベルセット。
	 @param[out] combined 包まれた結果のレベルセット。
	 @param[in] solid_offset 壁のレベルセットのオフセット。
	 */
	virtual void combine_levelset( const array3<double> &solid, const array3<double> &fluid, array3<double> &combined, double solid_offset=0.0 ) const = 0;
	/**
	 \~english @brief Extrapolate fluid level set towards solid.
	 @param[in] solid Solid level set.
	 @param[in-out] fluid level set to extrapolate.
	 @param[in] threshold Solid level set isocontour.
	 \~japanese @brief 流体のレベルセットを壁の方向へ外挿する。
	 @param[in] solid 壁のレベルセット。
	 @param[in-out] fluid 外挿する流体のレベルセット。
	 @param[in] threshold 壁のレベルセットの表面のレベルセットの値。
	 */
	virtual void extrapolate_levelset( const array3<double> &solid, array3<double> &fluid, double threshold=0.0 ) const = 0;
	/**
	 \~english @brief Compute the gradient of a level set.
	 @param[in] levelset Level set.
	 @param[out] gradient Output gradient.
	 \~japanese @brief レベルセットの勾配を計算する。
	 @param[in] levelset レベルセット。
	 @param[out] gradient 出力の勾配。
	 */
	virtual void compute_gradient( const array3<double> &levelset, array3<vec3d> &gradient ) const = 0;
	/**
	 \~english @brief Enumerate topology.
	 @param[in] flag Binary array to enumerate.
	 @param[out] topology_array Output grid filled with topologocal number.
	 \~japanese @brief トポロジーを数え上げる。
	 @param[in] flag 数えるトポロジーのバイナリフラグを記録したグリッド。
	 @param[out] topology_array トポロジーの番号が記録されたグリッドの出力。
	 */
	virtual unsigned mark_topology( const array3<char> &flag, array3<unsigned> &topology_array ) const = 0;
	/**
	 \~english @brief Trim narrow band of a level set within one cell away from the interface.
	 @param[in] levelset Fluid level set.
	 \~japanese @brief レベルセットを境界から1セルだけ離れたセルにトリミングする。
	 @param[in] levelset 流体のレベルセット。
	 */
	virtual void trim_narrowband( array3<double> &levelset ) const = 0;
	/**
	 \~english @brief Get the volume of fluid level set.
	 @param[in] fluid Fluid level set of 2x2x2.
	 @return Volume.
	 \~japanese @brief レベルセットの体積を計算する。
	 @param[in] fluid 2x2x2 の流体のレベルセット。
	 @return 体積。
	 */
	virtual double get_cell_volume( const double fluid[2][2][2] ) const = 0;
	/**
	 \~english @brief Get the volume of fluid level set.
	 @param[in] solid Solid level set.
	 @param[in] fluid Fluid level set.
	 @return Area.
	 \~japanese @brief レベルセットの体積を計算する。
	 @param[in] solid 壁のレベルセット。
	 @param[in] fluid 流体のレベルセット。
	 @return 体積。
	 */
	virtual double get_volume( const array3<double> &solid, const array3<double> &fluid ) const = 0;
	/**
	 \~english @brief Assign solid level set for visualization.
	 @param[in] dylib Reference to an instance of dylibloader.
	 @param[in] dx Grid cell size.
	 @param[out] solid Output solid level set.
	 @return \c true if visualizable solid level set is assigned.
	 \~japanese @brief 可視化のためのレベルセットを設定する。
	 @param[in] dylib dylibloader のインスタンスへのリファレンス。
	 @param[in] dx グリッドセルの大きさ。
	 @param[out] solid 出力の可視化のためのレベルセット。
	 @return もし可視化のためのレベルセットがセットされたら、\c true を、そうでなければ \c false を返す。
	 */
	virtual bool assign_visualizable_solid( const dylibloader &dylib, double dx, array3<double> &solid ) const = 0;
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
using gridutility3_ptr = std::unique_ptr<gridutility3_interface>;
using gridutility3_driver = recursive_configurable_driver<gridutility3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//