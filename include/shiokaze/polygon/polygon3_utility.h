/*
**	polygon3_utility.h
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
#ifndef SHKZ_POLYGON3_UTILITY_H
#define SHKZ_POLYGON3_UTILITY_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <vector>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Utility class that normalizes and re-positions objects of three dimensional mesh.
/// \~japanese @brief 3次元空間のメッシュのオブジェクトを正規化し再配置するためのユーティリティクラス。
class polygon3_utility {
public:
	/**
	 \~english @brief Normalizes and re-positions a three dimensional mesh.
	 @param[in-out] vertices Verticse to be altered.
	 @param[in] origin Origin in physical space.
	 @param[in] scale Scaling in physical space.
	 @param[in] axis Rotation axis.
	 @param[in] rotation Rotation degree.
	 \~japanese @brief 3次元メッシュのオブジェクトを正規化し再配置する。
	 @param[in-out] vertices 変更される頂点列。
	 @param[in] origin 物理空間の原点。
	 @param[in] scale 物理空間のスケール。
	 @param[in] axis 回転軸。
	 @param[in] rotation 回転の度合い。
	 */
	static void transform( std::vector<vec3d> &vertices, vec3d origin=vec3d(), double scale=1.0, int axis=0, double rotation=0 ) {
		//
		// Normalize
		unsigned nvertices (vertices.size());
		vec3d min_v(1e18,1e18,1e18);
		vec3d max_v = -1.0 * min_v;
		//
		for( unsigned n=0; n<nvertices; n++ ) {
			for( int dim : DIMS3 ) {
				min_v[dim] = std::min(vertices[n][dim],min_v[dim]);
				max_v[dim] = std::max(vertices[n][dim],max_v[dim]);
			}
		}
		double norm_s = max_v[0]-min_v[0];
		if( norm_s ) {
			for( unsigned n=0; n<nvertices; n++ ) {
				vertices[n] = scale * (vertices[n]-min_v) / norm_s + origin - 0.5 * scale * vec3d(1.0,0.0,1.0);
			}
		}
		//
		// Rotation
		if( rotation ) {
			vec3d min_v(1e18,1e18,1e18);
			vec3d max_v = -1.0 * min_v;
			for( unsigned n=0; n<nvertices; n++ ) {
				for( uint dim=0; dim<DIM3; dim++ ) {
					min_v[dim] = std::min(vertices[n][dim],min_v[dim]);
					max_v[dim] = std::max(vertices[n][dim],max_v[dim]);
				}
			}
			vec3d center = 0.5 * (max_v+min_v);
			for( unsigned n=0; n<nvertices; n++ ) {
				//
				vec3d p = vertices[n];
				p = p-center;
				double theta = rotation / 180.0 * 3.1415;
				if( axis == 0 ) {
					p = vec3d(p[0],cos(theta)*p[1]-sin(theta)*p[2],sin(theta)*p[1]+cos(theta)*p[2]);
				} else if( axis == 1 ) {
					p = vec3d(cos(theta)*p[0]-sin(theta)*p[2],p[1],sin(theta)*p[0]+cos(theta)*p[2]);
				} else if( axis == 2 ) {
					p = vec3d(p[0],cos(theta)*p[1]-sin(theta)*p[2],sin(theta)*p[1]+cos(theta)*p[2]);
				}
				p = p+center;
				vertices[n] = p;
			}
		}
	}
};
//
SHKZ_END_NAMESPACE
//
#endif