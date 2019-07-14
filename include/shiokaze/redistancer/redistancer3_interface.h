/*
**	redistancer3_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 5, 2017. 
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
#ifndef SHKZ_REDISTANCER2_H
#define SHKZ_REDISTANCER2_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/array/array3.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that re-distances level set grid. "pderedistancer3" and "fastmarch3" are provided as implementations.
/// \~japanese @brief レベルセットグリッドの再初期化を行うインターフェース。"pderedistancer3" と "fastmarch3" が実装として提供される。
class redistancer3_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(redistancer3_interface,"Redistancer 3D","Redistancer","Levelset redistancing module")
	/**
	 \~english @brief Redistance a level set grid.
	 @param[in-out] phi_array Level set grid.
	 @param[in] width Redistancing half bandwidth cell count.
	 \~japanese @brief レベルセットグリッドの再初期化を行う。
	 @param[in-out] phi_array レベルセットグリッド。
	 @param[in] width 再初期化を行う半分のバンド幅 (セル幅)。
	 */
	virtual void redistance( array3<float> &phi_array, unsigned width ) = 0;
	//
private:
	virtual void initialize( const shape3 &shape, double dx ) {};
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
using redistancer3_ptr = std::unique_ptr<redistancer3_interface>;
using redistancer3_driver = recursive_configurable_driver<redistancer3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif