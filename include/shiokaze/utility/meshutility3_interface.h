/*
**	meshutility3_interface.h
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
#ifndef SHKZ_MESHUTILITY3_INTERFACE_H
#define SHKZ_MESHUTILITY3_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that provides mesh utility functions. "meshutility3" is provided as implementation.
/// \~japanese @brief メッシュのユーティリティ関数群を提供するインターフェース。"meshutility3" が実装として提供される。
class meshutility3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(meshutility3_interface,"Mesh Utility 3D","MeshUtility","Mesh utility module")
	/**
	 \~english @brief Compute the distance from a point to a segment.
	 @param[in] x0 Query point.
	 @param[in] x1 Segment first end point.
	 @param[in] x2 Segment last end point.
	 @param[out] out Closest point on the segment.
	 @return Distance from the segment to the point.
	 \~japanese @brief 点からセグメントまでの距離を計算する。
	 @param[in] x0 調べる点。
	 @param[in] x1 セグメントの始点。
	 @param[in] x2 セグメントの終点。
	 @param[out] out セグメント上の一番近い点。
	 @return セグメントと点の距離。
	 */
	virtual double point_segment_distance(const vec3d &x0, const vec3d &x1, const vec3d &x2, vec3d &out ) const = 0;
	/**
	 \~english @brief Compute the distance from a point to a segment.
	 @param[in] x0 Query point.
	 @param[in] x1 First vertex position of the triangle.
	 @param[in] x2 Second vertex position of the triangle.
	 @param[in] x3 Third vertex position of the triangle.
	 @param[out] out Closest point on the triangle.
	 @return Distance from the triangle to the point.
	 \~japanese @brief 三角形と点の距離を計算する。
	 @param[in] x0 調べる点。
	 @param[in] x1 三角形の最初の頂点の位置。
	 @param[in] x2 三角形の2番目の頂点の位置。
	 @param[in] x3 三角形の3番目の頂点の位置。
	 @param[out] out 三角形上の一番近い点。
	 @return 三角形と点との距離。
	 */
	virtual double point_triangle_distance(const vec3d &x0, const vec3d &x1, const vec3d &x2, const vec3d &x3, vec3d &out ) const = 0;
};
//
using meshutility3_ptr = std::unique_ptr<meshutility3_interface>;
using meshutility3_driver = recursive_configurable_driver<meshutility3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif