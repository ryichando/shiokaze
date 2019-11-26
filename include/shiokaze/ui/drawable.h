/*
**	drawable.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on May 1, 2017.
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
#ifndef SHKZ_DRAWABLE_H
#define SHKZ_DRAWABLE_H
//
#include <shiokaze/core/runnable.h>
#include "UI_interface.h"
//
#if SPATIAL_DIM == 3
#include <shiokaze/ui/camera3_interface.h>
#elif SPATIAL_DIM == 2
#include <shiokaze/ui/camera2_interface.h>
#else
#error DSPATIAL_DIM must be defined either 2 or 3.
#endif
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for implementing drawable classes.
/// \~japanese @brief 描画可能なクラスを実装するためのインターフェース。
class drawable : public runnable, public UI_interface {
public:
	//
	LONG_NAME("Drawable")
	/**
	 \~english @brief Recursively call configure.
	 @param[in] config Configuration setting.
	 \~japanese @brief 再帰的に configure を呼ぶ。
	 @param[in] config 設定。
	 */
	virtual void recursive_initialize( const environment_map &environment ) override { 
		m_environment = environment;
		runnable::recursive_initialize (environment);
	}
	/**
	 \~english Handle an UI input event.
	 @param[in] Event information.
	 \~japanese @brief UI の入力イベントを処理する。
	 @param[in] イベントの情報。
	 */
	virtual bool handle_event( const event_structure &event ) override {
#ifdef SPATIAL_DIM
		bool handled = m_camera->handle_event(event);
		if( ! handled && m_camera->relay_event(event)) {
			return UI_interface::handle_event(m_camera->convert(event));
		}
		return handled;
#else
		return UI_interface::handle_event(event);
#endif
	}
	/**
	 \~english @brief Re-initialize instance.
	 \~japanese @brief インスタンスを再初期化する。
	 */
	virtual void reinitialize() { recursive_initialize(m_environment); }
	/**
	 \~english @brief Set up an initial new window environment.
	 @param[out] name Name of the window.
	 @param[out] width Width of the window.
	 @param[out] height Height of the window.
	 \~japanese @brief ウィンドウの最初の初期環境をセットアップする関数。
	 @param[out] name ウィンドウの名前。
	 @param[out] width ウィンドウの横幅。
	 @param[out] height ウィンドウの高さ。
	 */
	virtual void setup_window( std::string &name, int &width, int &height ) const {}
	/**
	 \~english @brief Function to tell the host program that program should quit.
	 @return If return \c true, program will exit. \c false otherwise.
	 \~japanese @brief ホストプログラムにプログラムが終了すべきか伝える関数。
	 @return もし終了するなら \c true を、そうでなければ \c false を返す。
	 */
	virtual bool should_quit() const override { return false; }
	/**
	 \~english @brief Function to tell the host program that screenshot should be taken.
	 @return If return \c true, the host program will take a screenshot and save to a file. \c false if not.
	 \~japanese @brief ホストプログラムにスクリーンショットを取るか伝える関数。
	 @return もし \c true を返せば、ホストプログラムはスクリーンショットを撮ってファイルに保存する。もし \c false なら、何もしない。
	 */
	virtual bool should_screenshot() const { return true; }
	/**
	 \~english @brief Tell the host program if a SHKZ logo should be hidden.
	 @return If return \c true, SHKZ logo will not be displayed. If return \c false logo will stay appeared.
	 \~japanese @brief ホストプログラムに SHKZ のロゴを隠すかどうか伝える関数。
	 @return もし \c true を返せば、SHKZ ロゴは表示されない。もし \c false を返せば、ロゴは表示される。
	 */
	virtual bool hide_logo() const { return false; }
	/**
	 \~english @brief Get current cursor icon.
	 @return Current icon.
	 \~japanese @brief 現在のカーソルのアイコンを得る。
	 @return 現在のアイコン。
	 */
	virtual UI_interface::CURSOR_TYPE get_current_cursor() const override {
#ifdef SPATIAL_DIM
		return m_camera->get_current_cursor();
#else
		return UI_interface::ARROW_CURSOR;
#endif
	}
	//
protected:
	//
	environment_map m_environment;
	//
#if SPATIAL_DIM == 3
	camera3_driver m_camera{this,"camera3"};
#elif SPATIAL_DIM == 2
	camera2_driver m_camera{this,"camera2"};
#endif
};
//
SHKZ_END_NAMESPACE
//
#endif