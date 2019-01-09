/*
**	pointgridhash2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 11, 2018. 
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
#ifndef SHKZ_POINTGRIDHASH2_INTERFACE_H
#define SHKZ_POINTGRIDHASH2_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/shape.h>
#include <shiokaze/math/vec.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for sorting points into hashing grids. "pointgridhash2" is provided as implementation.
/// \~japanese @brief ポイント群をグリッドのハッシュに振り分けるインターフェース。"pointgridhash2" が実装として提供される。
class pointgridhash2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(pointgridhash2_interface,"Point Grid Hash 2D","GridHash","Grid hashing module")
	/**
	 \~english @brief Type of the hash grid.
	 \~japanese @brief ハッシュグリッドの種類。
	 */
	enum { CELL_MODE=0x01, NODAL_MODE=0x02, FACE_MODE=0x04 };
	/**
	 \~english @brief Clear the hash.
	 \~japanese @brief ハッシュをクリアする。
	 */
	virtual void clear() = 0;
	/**
	 \~english @brief Sort points into the hashing grids.
	 @param[in] points Points to be sorted.
	 \~japanese @brief ポイント群をハッシュグリッドに振り分ける。
	 @param[in] ponits 振り分けるポイント群。
	 */
	virtual void sort_points( const std::vector<vec2d> &points ) = 0;
	/**
	 \~english @brief Get points in a cell.
	 @param[in] pi Cell index position.
	 \~japanese @brief セルの中のポイント群を取得する。
	 @param[in] pi セルのインデックス位置。
	 */
	virtual const std::vector<size_t> & get_points_in_cell( const vec2i &pi ) const = 0;
	/**
	 \~english @brief Get points of a node.
	 @param[in] pi Nodal index position.
	 \~japanese @brief ノードに割り振られたポイント群を取得する。
	 @param[in] pi ノードのインデックス位置。
	 */
	virtual const std::vector<size_t> & get_points_on_node( const vec2i &pi ) const = 0;
	/**
	 \~english @brief Type of the hash grid to query.
	 \~japanese @brief 求めるハッシュグリッドの種類。
	 */
	enum hash_type { USE_NODAL, USE_CELL, USE_FACE };
	/**
	 \~english @brief Get if the hash cell contains at least one point.
	 @param[in] pi Index position.
	 @param[in] type Hash cell query type.
	 \~japanese @brief ハッシュセルが少なくとも一つのポイントを含んでいるか取得する。
	 @param[in] pi インデックス位置。
	 @param[in] type ハッシュセルの種類。
	 */
	virtual bool exist( const vec2i &pi, hash_type type ) const = 0;
	/**
	 \~english @brief Get points in a cell and its all adjacent neighbors.
	 @param[in] pi Index position.
	 @param[in] type Hash cell query type.
	 \~japanese @brief セルとそのセルに隣接する全てのセルのポイント群を取得する。
	 @param[in] pi インデックス位置。
	 @param[in] type ハッシュセルの種類。
	 */
	virtual std::vector<size_t> get_cell_neighbors( const vec2i &pi, hash_type type=USE_CELL, unsigned half_witdh=1 ) const = 0;
	/**
	 \~english @brief Get points in a node and its all adjacent neighbors.
	 @param[in] pi Index position.
	 @param[in] type Hash cell query type.
	 @return Points.
	 \~japanese @brief ノードとそのノードに隣接する全てのセルのポイント群を取得する。
	 @param[in] pi インデックス位置。
	 @param[in] type ハッシュセルの種類。
	 @return ポイント群。
	 */
	virtual std::vector<size_t> get_nodal_neighbors( const vec2i &pi, hash_type type=USE_NODAL, unsigned half_witdh=1 ) const = 0;
	/**
	 \~english @brief Get points in a face and its all adjacent neighbors.
	 @param[in] pi Index position.
	 @param[in] type Hash cell query type.
	 @return Points.
	 \~japanese @brief 面とその面に隣接する全てのセルのポイント群を取得する。
	 @param[in] pi インデックス位置。
	 @param[in] type ハッシュセルの種類。
	 @return ポイント群。
	 */
	virtual std::vector<size_t> get_face_neighbors( const vec2i &pi, unsigned dim, hash_type type=USE_FACE ) const = 0;
	//
private:
	//
	virtual void initialize( const shape2 &shape, double dx, int mode=CELL_MODE | NODAL_MODE | FACE_MODE) = 0;
	virtual void initialize( const configurable::environment_map &environment ) override {
		//
		assert(check_set(environment,{"shape","dx"}));
		//
		const shape2 &shape = get_env<shape2>(environment,"shape");
		double dx = get_env<double>(environment,"dx");
		//
		if(check_set(environment,{"hashmode"})) {
			int mode = get_env<int>(environment,"hashmode");
			initialize(shape,dx,mode);
		} else {
			initialize(shape,dx);
		}
	}
	//
};
//
using pointgridhash2_ptr = std::unique_ptr<pointgridhash2_interface>;
using pointgridhash2_driver = recursive_configurable_driver<pointgridhash2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif