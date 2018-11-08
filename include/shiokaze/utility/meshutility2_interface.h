/*
**	meshutility2_interface.h
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
#ifndef SHKZ_MESHUTILITY2_INTERFACE_H
#define SHKZ_MESHUTILITY2_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that provides mesh utility functions. "meshutility2" is provided as implementation.
/// \~japanese @brief メッシュのユーティリティ関数群を提供するインターフェース。"meshutility2" が実装として提供される。
class meshutility2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(meshutility2_interface,"Mesh Utility 2D","MeshUtility","Mesh utility module")
	/**
	 \~english @brief Extract surface contours from the level set on a four points of square.
	 @param[in] v Level set on four vertices given by v[i][j].
	 @param[in] vertices Position of four vertices given by vertices[i][j].
	 @param[in] p Line segments.
	 @param[out] pnum Number of output segments.
	 @param[in] fill Whether to fill internal region.
	 \~japanese @brief 4つの点で構成される四角形のレベルセットからサーフェスの輪郭を抽出する。
	 @param[in] v 4点でのレベルセットの値。v[i][j] で与えられる。
	 @param[in] vertices 4点の頂点の位置。vertices[i][j] で与えられる。
	 @param[in] p 線のセグメント群。
	 @param[out] pnum 出力のセグメントの数。
	 @param[in] fill 内部を塗りつぶすか。
	 */
	virtual void march_points( double v[2][2], const vec2d vertices[2][2], vec2d p[8], int &pnum, bool fill ) const = 0;
	/**
	 \~english @brief Compute the distance from a point to a segment.
	 @param[in] p0 Segment first end point.
	 @param[in] p1 Segment last end point.
	 @param[in-out] p Position of a point to be projected on the nearest segment position.
	 \~japanese @brief 点からセグメントまでの距離を計算する。
	 @param[in] p0 セグメントの始点の位置。
	 @param[in] p1 セグメントの終点の位置。
	 @param[in-out] p ポイントの位置。セグメントの一番近い点が出力される。
	 */
	virtual void distance( const vec2d &p0, const vec2d &p1, vec2d &p ) const = 0;
};
//
using meshutility2_ptr = std::unique_ptr<meshutility2_interface>;
using meshutility2_driver = recursive_configurable_driver<meshutility2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif