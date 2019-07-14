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
		size_t nvertices (vertices.size());
		vec3d min_v;
		vec3d max_v;
		compute_AABB(vertices,min_v,max_v);
		//
		for( size_t n=0; n<nvertices; n++ ) {
			for( int dim : DIMS3 ) {
				min_v[dim] = std::min(vertices[n][dim],min_v[dim]);
				max_v[dim] = std::max(vertices[n][dim],max_v[dim]);
			}
		}
		double norm_s = max_v[0]-min_v[0];
		if( norm_s ) {
			for( size_t n=0; n<nvertices; n++ ) {
				vertices[n] = scale * (vertices[n]-min_v) / norm_s + origin - 0.5 * scale * vec3d(1.0,0.0,1.0);
			}
		}
		//
		// Rotation
		if( rotation ) {
			vec3d min_v(1e18,1e18,1e18);
			vec3d max_v = -1.0 * min_v;
			for( size_t n=0; n<nvertices; n++ ) {
				for( uint dim=0; dim<DIM3; dim++ ) {
					min_v[dim] = std::min(vertices[n][dim],min_v[dim]);
					max_v[dim] = std::max(vertices[n][dim],max_v[dim]);
				}
			}
			vec3d center = 0.5 * (max_v+min_v);
			for( size_t n=0; n<nvertices; n++ ) {
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
	/**
	 \~english @brief Compute the center of gravity of a mesh object.
	 @param[in] vertices Vertices.
	 @param[in] faces Facs.
	 @return Center of gravity.
	 \~japanese @brief 3次元メッシュの重心を求める。
	 @param[in] vertices 頂点列。
	 @param[in] faces 面の列。
	 @return 重心。
	 */
	template <class N> static vec3d get_center_of_gravity( std::vector<vec3d> &vertices, std::vector<std::vector<N> > &faces ) {
		//
		//http://stackoverflow.com/questions/2083771/a-method-to-calculate-the-centre-of-mass-from-a-stl-stereo-lithography-file
		//
		vec3d center_of_gravity;
		double totalVolume(0.0), currentVolume(0.0);
		for( const auto &triangle : faces ) {
			//
			assert(triangle.size() == 3);
			vec3d v[3] = { vertices[triangle[0]], vertices[triangle[1]], vertices[triangle[2]] };
			totalVolume += currentVolume = (
									+ v[0][0]*v[1][1]*v[2][2]
									- v[0][0]*v[2][1]*v[1][2]
									- v[1][0]*v[0][1]*v[2][2]
									+ v[1][0]*v[2][1]*v[0][2]
									+ v[2][0]*v[0][1]*v[1][2]
									- v[2][0]*v[1][1]*v[0][2]) / 6.0;
			center_of_gravity[0] += ((v[0][0]+v[1][0]+v[2][0])/4.0) * currentVolume;
			center_of_gravity[1] += ((v[0][1]+v[1][1]+v[2][1])/4.0) * currentVolume;
			center_of_gravity[2] += ((v[0][2]+v[1][2]+v[2][2])/4.0) * currentVolume;
		}
		center_of_gravity = center_of_gravity / totalVolume;
		return center_of_gravity;
	}
	/**
	 \~english @brief Compute the AABB of a mesh object.
	 @param[in] vertices Vertices.
	 @param[out] corner0 Corner of min location.
	 @param[out] corner1 Corner of max location.
	 \~japanese @brief 3次元メッシュの重心を求める。
	 @param[in] vertices 頂点列。
	 @param[out] corner0 最小地点の位置。
	 @param[out] corner1 最大地点の位置。
	 */
	static void compute_AABB( std::vector<vec3d> &vertices, vec3d &corner0, vec3d &corner1 ) {
		//
		corner0 = vec3d(1e18,1e18,1e18);
		corner1 = -1.0 * corner0;
		//
		for( size_t n=0; n<vertices.size(); n++ ) {
			for( int dim : DIMS3 ) {
				corner0[dim] = std::min(vertices[n][dim],corner0[dim]);
				corner1[dim] = std::max(vertices[n][dim],corner1[dim]);
			}
		}
	}
	//
};
//
SHKZ_END_NAMESPACE
//
#endif