/*
**	graphics_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on February 1, 2018.
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
#ifndef SHKZ_GRAPHICS_INTERFACE_H
#define SHKZ_GRAPHICS_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include "graphics_engine.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Graphics engine moduled interface.
/// \~japanese @brief 画像処理を行うモジュールインターフェース。
class graphics_interface : public recursive_configurable_module, public graphics_engine {
public:
	//
	DEFINE_MODULE(graphics_interface,"Graphics Engine","Graphics","Graphics engine module")
	//
};
//
using graphics_interface_ptr = std::unique_ptr<graphics_interface>;
using graphics_interface_driver = recursive_configurable_driver<graphics_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
