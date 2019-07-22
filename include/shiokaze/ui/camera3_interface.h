/*
**	camera3_interface.h
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
#ifndef SHKZ_CAMERA3_INTERFACE_H
#define SHKZ_CAMERA3_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/math/vec.h>
#include <cmath>
#include "UI_interface.h"
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for 3D camera manipulations.
/// \~japanese @brief 3次元のカメラの操作を行うインターフェース。
class camera3_interface : public recursive_configurable_module, public UI_interface {
public:
	//
	DEFINE_MODULE(camera3_interface,"Camera 3D","Camera","3D Camera module")
	/**
	 \~english @brief Set a bounding box.
	 @param[in] p0 Left bottom corner position.
	 @param[in] p1 Right top corner position.
	 @param[in] reset_view Whether to reset the view according to the bounding box info.
	 \~japanese @brief バウンディングボックスを設定する。
	 @param[in] p0 左下のコーナー位置。
	 @param[in] p1 右上のコーナー位置。
	 @param[in] reset_view バウンディングボックスの情報を元にカメラの設定をリセットするか。
	 */
	virtual void set_bounding_box( const double *p0, const double *p1, bool reset_view ) = 0;
	/**
	 \~english @brief Set up a camera with a target position, origin position, fov and others.
	 @param[in] target Target position.
	 @param[in] position Camera position.
	 @param[in] up Top unit vector.
	 @param[in] fov Field of view.
	 \~japanese @brief 視野角、カメラの位置、ターゲット位置を指定してカメラを向ける。
	 @param[in] target ターゲット位置。
	 @param[in] position カメラの位置。
	 @param[in] up 上方向のユニットベクトル。
	 @param[in] fov 視野角。
	 */
	virtual void look_at( const double *target, const double *position, const double *up, double fov ) = 0;
	/**
	 \~english @brief Get camera information about the target position, origin position, fov and others.
	 @param[out] target Target position.
	 @param[out] position Camera position.
	 @param[out] up Top unit vector.
	 @param[out] fov Field of view.
	 \~japanese @brief 視野角、カメラの位置、ターゲット位置などを取得する。
	 @param[out] target ターゲット位置。
	 @param[out] position カメラの位置。
	 @param[out] up 上方向のユニットベクトル。
	 @param[out] fov 視野角。
	 */
	virtual void get( double *target, double *position, double *up, double *fov ) const = 0;
	/**
	 \~english @brief Set up a camera with a target position.
	 @param[in] target Target position.
	 \~japanese @brief ターゲット位置を指定してカメラを向ける。
	 @param[in] target ターゲット位置。
	 */
	void look_at( const double *target ) {
		//
		double target_old[3], position[3], up[3], fov;
		get(target_old,position,up,&fov);
		look_at(target,position,up,fov);
	}
	/**
	 \~english @brief Set up a camera with a target position, origin position and fov.
	 @param[in] target Target position.
	 @param[in] direction Direction from the target position. (unit vector)
	 @param[in] distance Distance from the camera.
	 @param[in] up Top unit vector.
	 @param[in] fov Field of view.
	 @param[in] near Near clip view.
	 @param[in] far Far clip view.
	 \~japanese @brief 視野角、カメラの位置、ターゲット位置を指定してカメラを向ける。
	 @param[in] target ターゲット位置。
	 @param[in] direction ターゲット位置から見たカメラの方向。(単位ベクトル)
	 @param[in] distance カメラからの距離
	 @param[in] up 上方向のユニットベクトル。
	 @param[in] fov 視野角。
	 @param[in] near ニアクリッピング。
	 @param[in] far ファークリッピング。
	 */
	void look_from( const double *target, const double *direction, double distance, const double *up, double fov, double near, double far ) {
		//
		double len = sqrt(direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2]);
		double position[3] = { target[0]+distance*direction[0]/len, target[1]+distance*direction[1]/len, target[2]+distance*direction[2]/len };
		look_at(target,position,up,fov);
	}
	/**
	 \~english @brief Change the distance of camera from the target position.
	 @param[in] distance New distance.
	 \~japanese @brief ターゲット位置からカメラまでの距離を変更する。
	 @param[in] distance 新しい距離。
	 */
	void set_distance( double distance ) {
		//
		double target[3], position[3], up[3], fov;
		get(target,position,up,&fov);
		double direction[3] = { position[0]-target[0], position[1]-target[1], position[2]-target[2] };
		double len = sqrt(direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2]);
		position[0] = target[0]+distance*direction[0]/len;
		position[1] = target[1]+distance*direction[1]/len;
		position[2] = target[2]+distance*direction[2]/len;
		look_at(target,position,up,fov);
	}
	/**
	 \~english @brief Get the distance of camera from the target position.
	 @return Distance from the target to the camera.
	 \~japanese @brief ターゲット位置からカメラまでの距離を得る。
	 @return カメラからターゲット位置までの距離
	 */
	double get_distance() const {
		double target[3], position[3], up[3], fov;
		get(target,position,up,&fov);
		double direction[3] = { position[0]-target[0], position[1]-target[1], position[2]-target[2] };
		return sqrt(direction[0]*direction[0]+direction[1]*direction[1]+direction[2]*direction[2]);
	}
	/**
	 \~english @brief Change the field of view.
	 @param[in] fov New field of view.
	 \~japanese @brief 視野角を変更する。
	 @param[in] fov 新しい視野角。
	 */
	void set_fov( double fov ) {
		//
		double target[3], position[3], up[3], fov_old;
		get(target,position,up,&fov_old);
		look_at(target,position,up,fov);
	}
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
using camera3_ptr = std::unique_ptr<camera3_interface>;
using camera3_driver = recursive_configurable_driver<camera3_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
//