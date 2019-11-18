/*
**	camera2_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 16, 2019.
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
#ifndef SHKZ_CAMERA2_INTERFACE_H
#define SHKZ_CAMERA2_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/math/vec.h>
#include "UI_interface.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for 2D camera manipulations.
/// \~japanese @brief 2次元のカメラの操作を行うインターフェース。
class camera2_interface : public recursive_configurable_module, public UI_interface {
public:
	//
	DEFINE_MODULE(camera2_interface,"Camera 2D","Camera","2D Camera module")
	/**
	 \~english @brief Set a bounding box.
	 @param[in] p0 Left bottom corner position.
	 @param[in] p1 Right top corner position.
	 \~japanese @brief バウンディングボックスを設定する。
	 @param[in] p0 左下のコーナー位置。
	 @param[in] p1 右上のコーナー位置。
	 */
	virtual void set_bounding_box( const double *p0, const double *p1 ) = 0;
	/**
	 \~english @brief Configure 2D coordinate view.
	 @param[in] origin Origin.
	 @param[in] scake Scale.
	 \~japanese @brief 2次元座標系を設定する。
	 @param[in] origin 原点。
	 @param[in] scale スケール。
	 */
	virtual void set_2D_coordinate( const double *origin, double scale ) = 0;
	/**
	 \~english @brief Convert event.
	 @param[in] event input raw event information.
	 @return information with physical coordinate position.
	 \~japanese @brief イベントを変換する。
	 @param[in] event ウィンドウ座標系のオリジナルのイベント情報。
	 @return 物理座標系の位置に変換されたイベント。
	 */
	virtual UI_interface::event_structure convert( const UI_interface::event_structure &event ) const = 0;
};
//
using camera2_ptr = std::unique_ptr<camera2_interface>;
using camera2_driver = recursive_configurable_driver<camera2_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//