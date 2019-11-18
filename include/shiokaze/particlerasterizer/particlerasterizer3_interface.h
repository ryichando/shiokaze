/*
**	particlerasterizer3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 27, 2017. 
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
#ifndef SHKZ_PARTICLEMESHER3_H
#define SHKZ_PARTICLEMESHER3_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/array/bitarray3.h>
#include <shiokaze/array/array3.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for constructing level set from a set of particles. "convexhullrasterizer3" and "flatrasterizer3" are provided as implementaions.
/// \~japanese @brief 粒子群からレベルセットを生成するインターフェース。"convexhullrasterizer3" と "flatrasterizer3" が実装として提供される。
class particlerasterizer3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(particlerasterizer3_interface,"Particle Rasterizer 3D","Rasterizer","Particle rasterizer module")
	//
	/// \~english @brief Structure for a particle.
	/// \~japanese @brief 粒子の構造体。
	struct Particle3 {
		/**
		 \~english @brief Particle position.
		 \~japanese @brief 粒子の位置。
		 */
		vec3r p;
		/**
		 \~english @brief Particle radius.
		 \~japanese @brief 粒子の半径。
		 */
		Real r;
	};
	/**
	 \~english @brief Building a level set from the set of particles.
	 @param[out] fluid Output of level set grid.
	 @param[in] mask Stencil mask to calculate the level set. Only where mask is active will be calculated.
	 @param[in] particles Set of particles.
	 \~japanese @brief 粒子群からレベルセットを生成する。
	 @param[out] fluid レベルセット格子の出力。
	 @param[in] mask レベルセットの値を計算するステンシルマスク。ステンシルマスクが有効な部分だけ計算される。
	 @param[in] particles 粒子群。
	 */
	virtual void build_levelset( array3<Real> &fluid, const bitarray3 &mask, const std::vector<Particle3> &particles ) const = 0;
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
using particlerasterizer3_ptr = std::unique_ptr<particlerasterizer3_interface>;
using particlerasterizer3_driver = recursive_configurable_driver<particlerasterizer3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif