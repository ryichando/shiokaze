/*
**	macadvection2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Dec 7, 2017. 
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
#ifndef SHKZ_MACADVECTION2_INTERFACE_H
#define SHKZ_MACADVECTION2_INTERFACE_H
//
#include <shiokaze/array/macarray2.h>
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for advection on MAC grids. "macadvection2" implementation is provided as default.
/// \~japanese @brief MAC格子で移流を行うためのインターフェース。"macadvection2" がデフォルトとして提供される。
class macadvection2_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(macadvection2_interface,"MAC Advection 2D","Advection","Advection module")
	/**
	 \~english @brief Advect scalar field.
	 @param[in] scalar Scalar to advect.
	 @param[in] velocity Velocity field.
	 @paran[in] fluid Fluid level set.
	 @param[in] dt Time step.
	 \~japanese @brief スカラー値を移流する。
	 @param[in] scalar 移流するスカラー値。
	 @param[in] velocity 速度場。
	 @paran[in] fluid 液体のレベルセット。
	 @param[in] dt 時間幅。
	 */
	virtual void advect_scalar(	array2<double> &scalar,				// Cell-centered
								const macarray2<double> &velocity,	// Face-located
								const array2<double> &fluid,		// Fluid level set
								double dt ) = 0;
	/**
	 \~english @brief Advect vector field.
	 @param[in] u Vector field to advect.
	 @param[in] velocity Velocity field.
	 @paran[in] fluid Fluid level set.
	 @param[in] dt Time step.
	 \~japanese @brief ベクトル場を移流する。
	 @param[in] u 移流するベクトル場。
	 @param[in] velocity 速度場。
	 @paran[in] fluid 液体のレベルセット。
	 @param[in] dt 時間幅.
	 */
	virtual void advect_vector(	macarray2<double> &u,				// Face-located
								const macarray2<double> &velocity,	// Face-located
								const array2<double> &fluid,		// Fluid level set
								double dt ) = 0;
	//
private:
	//
	virtual void initialize( const shape2 &shape, double dx ) = 0;
	virtual void initialize( const configurable::environment_map &environment ) override {
		//
		assert(check_set(environment,{"shape","dx"}));
		initialize(
			get_env<shape2>(environment,"shape"),
			get_env<double>(environment,"dx")
		);
	}
};
//
using macadvection2_ptr = std::unique_ptr<macadvection2_interface>;
using macadvection2_driver = recursive_configurable_driver<macadvection2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif