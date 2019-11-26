/*
**	UI_interface.h
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
#ifndef SHKZ_UI_INTERFACE_H
#define SHKZ_UI_INTERFACE_H
//
#include <shiokaze/graphics/graphics_engine.h>
#include <dlfcn.h>
//
SHKZ_BEGIN_NAMESPACE
//
static bool *g_shkz_has_graphical_interface {nullptr};
//
/** @file */
/// \~english @brief Interface for input APIs.
/// \~japanese @brief 入力の API を提供するインターフェース。
class UI_interface {
public:
	//
	/// \~english @brief Key and mose action type.
	/// \~japanese @brief キーボードとマウスのアクションの種類。
	enum ACTION {
		/// \~english @brief Release event.
		/// \~japanese @brief リリースイベント。
		RELEASE = 0,
		/// \~english @brief Press event.
		/// \~japanese @brief プレスイベント。
		PRESS = 1,
		/// \~english @brief Repeat event.
		/// \~japanese @brief リピートイベント。
		REPEAT = 2,
	};
	//
	/// \~english @brief Modifier bits.
	/// \~japanese @brief 修飾キーのビット。
	enum MODIFIER {
		/// \~english @brief Shift modifier bit.
		/// \~japanese @brief シフトキーのビット。
		MOD_SHIFT = 0x0001,
		/// \~english @brief Control modifier bit.
		/// \~japanese @brief コントロールキーのビット。
		MOD_CONTROL = 0x0002,
		/// \~english @brief ALT modifier bit.
		/// \~japanese @brief ALT キーのビット。
		MOD_ALT = 0x0004,
		/// \~english @brief SUPER modifier bit.
		/// \~japanese @brief SUPER キーのビット。
		MOD_SUPER = 0x0008,
		/// \~english @brief Capslock modifier bit.
		/// \~japanese @brief キャップスロックキーのビット。
		MOD_CAPS_LOCK = 0x0010,
		/// \~english @brief NUM lock modifier bit.
		/// \~japanese @brief NUM ロックキーのビット。
		MOD_NUM_LOCK = 0x0020,
	};
	//
	/// \~english @brief Mouse button type.
	/// \~japanese @brief マウスボタンの種類。
	enum MOUSE_BUTTON {
		/// \~english @brief Left button.
		/// \~japanese @brief 左ボタン。
		LEFT = 1,
		/// \~english @brief Right button.
		/// \~japanese @brief 右ボタン。
		RIGHT = 2,
		/// \~english @brief Middle button.
		/// \~japanese @brief 中央ボタン。
		MIDDLE = 3,
	};
	//
	#include "keymap.h"
	//
	/// \~english @brief Event information structure.
	/// \~japanese @brief イベント情報の構造体。
	struct event_structure {
		//
		/// \~english @brief Event type.
		/// \~japanese @brief イベントの種類。
		enum EVENT_TYPE {
			/// \~english @brief Keyboard event.
			/// \~japanese @brief キーボードイベント。
			KEYBOARD,
			/// \~english @brief Cursor event.
			/// \~japanese @brief カーソルイベント。
			CURSOR,
			/// \~english @brief Mouse event.
			/// \~japanese @brief マウスイベント。
			MOUSE,
			/// \~english @brief Scroll event.
			/// \~japanese @brief スクロールイベント。
			SCROLL,
			/// \~english @brief Drag event.
			/// \~japanese @brief ドラッグイベント。
			DRAG,
			/// \~english @brief Resize event.
			/// \~japanese @brief リサイズイベント。
			RESIZE,
			/// \~english @brief Draw event.
			/// \~japanese @brief 描画イベント。
			DRAW,
		};
		/// \~english @brief Event type.
		/// \~japanese @brief イベントの種類。
		EVENT_TYPE type;
		/// \~english @brief Button type.
		/// \~japanese @brief ボタンの種類。
		int button {0};
		/// \~english @brief Key type.
		/// \~japanese @brief キーの種類。
		int key {0};
		/// \~english @brief Action type.
		/// \~japanese @brief アクションの種類。
		int action {0};
		/// \~english @brief Modifier information.
		/// \~japanese @brief 修飾キーの情報。
		int mods {0};
		/// \~english @brief Width.
		/// \~japanese @brief 横幅。
		int width {0};
		/// \~english @brief Height.
		/// \~japanese @brief 高さ。
		int height {0};
		/// \~english @brief X coordinate value.
		/// \~japanese @brief X 軸の値。
		double x {0.0};
		/// \~english @brief Y coordinate value.
		/// \~japanese @brief Y 軸の値。
		double y {0.0};
		/// \~english @brief Z coordinate value.
		/// \~japanese @brief Z 軸の値。
		double z {0.0};
		/// \~english @brief X coordinate displacement value.
		/// \~japanese @brief X 軸の変位の値。
		double u {0.0};
		/// \~english @brief Y coordinate displacement value.
		/// \~japanese @brief Y 軸の変位の値。
		double v {0.0};
		/// \~english @brief Z coordinate displacement value.
		/// \~japanese @brief Z 軸の変位の値。
		double w {0.0};
		/// \~english @brief Pointer to the graphics engine.
		/// \~japanese @brief グラフィックエンジンへのポインタ。
		graphics_engine *g {nullptr};
	};
	/**
	 \~english Handle an UI input event.
	 @param[in] Event information.
	 \~japanese @brief UI の入力イベントを処理する。
	 @param[in] イベントの情報。
	 */
	virtual bool handle_event( const event_structure &event ) {
		switch( event.type ) {
			case event_structure::KEYBOARD:
				return keyboard(event.key,event.action,event.mods);
				break;
			case event_structure::CURSOR:
				cursor(event.x,event.y,event.z);
				break;
			case event_structure::MOUSE:
				mouse(event.x,event.y,event.z,event.button,event.action,event.mods);
				break;
			case event_structure::SCROLL:
				scroll(event.x,event.y);
				break;
			case event_structure::DRAG:
				drag(event.x,event.y,event.z,event.u,event.v,event.w);
				break;
			case event_structure::RESIZE:
				resize(event.width,event.height);
				break;
			case event_structure::DRAW:
				assert(event.g);
				draw(*event.g);
				break;
		}
		return false;
	}
	/**
	 \~english @brief Get if the event shoule be relayed to other instances after handle_event of this instance is called.
	 @param[in] event Corresponding event.
	 @return \c t/rue if the event should be relayed to others.
	 \~japanese @brief このインスタンスの handle_event を呼んだ後、イベントを他のメンバーにリレーするか。
	 @param[in] event 該当イベント。
	 @return もし他のメンバーにイベントをリレーするなら \c true を返す。
	 */
	virtual bool relay_event( const event_structure &event ) const { return true; }
	//
	/// \~english @brief Cursor icon type.
	/// \~japanese @brief カーソルのアイコンの種類。
	enum CURSOR_TYPE {
		/// \~english @brief Regular arrow cursor.
		/// \~japanese @brief 通常の矢印カーソル。
		ARROW_CURSOR = 0,
		/// \~english @brief Hand cursor.
		/// \~japanese @brief ハンドカーソル。
		HAND_CURSOR = 1,
		/// \~english @brief Text input I beam cursor.
		/// \~japanese @brief テキスト入力の I 字のカーソル。
		IBEAM_CURSOR = 2,
		/// \~english @brief Cross hair cursor.
		/// \~japanese @brief クロスヘアカーソル。
		CROSSHAIR_CURSOR = 3,
		/// \~english @brief Horizontal resizing cursor.
		/// \~japanese @brief 水平方向のリサイズカーソル。
		HRESIZE_CURSOR = 4,
		/// \~english @brief Vertical resizing cursor.
		/// \~japanese @brief 垂直方向のリサイズカーソル。
		VRESIZE_CURSOR = 5,
	};
	/**
	 \~english @brief Get current cursor icon.
	 @return Current icon.
	 \~japanese @brief 現在のカーソルのアイコンを得る。
	 @return 現在のアイコン。
	 */
	virtual CURSOR_TYPE get_current_cursor() const {
		return ARROW_CURSOR;
	}
	/**
	 \~english @brief Get if a graphical interface is available.
	 @param[in] value Boolean value to set force single thread.
	 \~japanese @brief 強制的にシングルスレッドにするか設定する。
	 @param[in] value シングルスレッドにするか指定する値。
	 */
	static bool has_graphical_interface() {
		if( ! g_shkz_has_graphical_interface ) {
			g_shkz_has_graphical_interface = static_cast<bool *>(::dlsym(RTLD_DEFAULT,"g_shkz_has_graphical_interface"));
			assert(g_shkz_has_graphical_interface);
		}
		return *g_shkz_has_graphical_interface;
	}
	//
protected:
	//
	/**
	 \~english @brief Function that catches window resizing events.
	 @param[in] width Width of the window.
	 @param[in] height Height of the window.
	 \~japanese @brief ウィンドウのリサイズイベントが発生する時に呼ばれる関数。
	 @param[in] width ウィンドウの横幅。
	 @param[in] height ウィンドウの高さ。
	 */
	virtual void resize( int width, int height ) {}
	/**
	 \~english @brief Function that catches draw event.
	 @param[in] g Graphics engine.
	 \~japanese @brief 描画イベントが発生する時に呼ばれる関数。
	 @param[in] g グラフィックスエンジン。
	 */
	virtual void draw( graphics_engine &g ) const {}
	/**
	 \~english @brief Function that catches key down event.
	 @param[in] key Capitalized key.
	 @param[in] action Action code.
	 @param[in] mods Modifier bits.
	 \~japanese @brief キーが押されたら呼ばれる関数。
	 @param[in] key 大文字の英文字。
	 @param[in] action イベントの種類。
	 @param[in] mods 修飾キーのビット。
	 */
	virtual bool keyboard( int key, int action, int mods ) { return false; }
	/**
	 \~english @brief Function that catches passive cursor event.
	 @param[in] x Cursor position on x pixel coordinate.
	 @param[in] y Cursor position on y pixel coordinate.
	 @param[in] z Cursor position on y pixel coordinate.
	 \~japanese @brief カーソルが動いた時に呼び出される関数。
	 @param[in] x カーソルの x 座標のピクセル座標。
	 @param[in] y カーソルの y 座標のピクセル座標。
	 @param[in] z カーソルの z 座標のピクセル座標。
	 */
	virtual void cursor( double x, double y, double z ) {};
	/**
	 \~english @brief Function that catches mouse event.
	 @param[in] button Mouse button number.
	 @param[in] action Mouse action (Mouse down = 1 Mouse release = 0)
	 @param[in] mods Modifier bit.
	 \~japanese @brief マウスイベントが発生した時に呼ばれる関数。
	 @param[in] button マウスのボタンのナンバー。
	 @param[in] action アクションの種類。マウスダウン＝1、マウスアップ=0。
	 @param[in] mods 就職キーのビット。
	 */
	virtual void mouse( double x, double y, double z, int button, int action, int mods ) {};
	/**
	 \~english @brief Function that catches scroll event.
	 @param[in] xoffset X coordinate offset.
	 @param[in] yoffset Y coordinate offset.
	 \~japanese @brief スクロールイベントが発生した時に呼ばれる関数。
	 @param[in] xoffset X 軸のオフセット。
	 @param[in] yoffset Y 軸のオフセット。
	 */
	virtual void scroll( double xoffset, double yoffset ) {};
	/**
	 \~english @brief Function that catches drag event.
	 @param[in] x Cursor position on x pixel coordinate.
	 @param[in] y Cursor position on y pixel coordinate.
	 @param[in] u Drag vector on x pixel coordinate.
	 @param[in] v Drag vector on y pixel coordinate.
	 @param[in] w Drag vector on z pixel coordinate.
	 \~japanese @brief ドラッグイベントが発生した時に呼ばれる関数。
	 @param[in] x カーソルの x 座標のピクセル座標。
	 @param[in] y カーソルの y 座標のピクセル座標。
	 @param[in] u ドラッグ方向の x 座標のピクセル座標。
	 @param[in] v ドラッグ方向の y 座標のピクセル座標。
	 @param[in] w ドラッグ方向の z 座標のピクセル座標。
	 */
	virtual void drag( double x, double y, double z, double u, double v, double w ) {};
};
//
SHKZ_END_NAMESPACE
//
#endif
//