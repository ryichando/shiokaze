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
#include <shiokaze/graphics/graphics_engine.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for implementing drawable classes.
/// \~japanese @brief 描画可能なクラスを実装するためのインターフェース。
class drawable : public runnable {
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
	 \~english @brief Re-initialize instance.
	 \~japanese @brief インスタンスを再初期化する。
	 */
	virtual void reinitialize() { recursive_initialize(m_environment); }
	/**
	 \~english @brief Set up graphics environment.
	 @param[in] g Graphics engine.
	 \~japanese @brief グラフィックス環境をセットアップする。
	 @param[in] g グラフィックスエンジン。
	 */
	virtual void setup_graphics ( const graphics_engine &g ) const { g.setup_graphics(); };
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
	 \~english @brief Function that catches key down event.
	 @param[in] key Capitalized key.
	 \~japanese @brief キーが押されたら呼ばれる関数。
	 @param[in] key 大文字の英文字。
	 */
	virtual bool keyboard( char key ) {
		switch ( key ) {
			case '/':
				set_running(! is_running());
				return true;
			case 'R':
				reinitialize();
				return true;
			case '.':
				if( ! is_running()) idle ();
				return true;
		}
		return false;
	}
	/**
	 \~english @brief Function that catches passive cursor event.
	 @param[in] width Window width.
	 @param[in] height Window height.
	 @param[in] x Cursor position on x pixel coordinate.
	 @param[in] y Cursor position on y pixel coordinate.
	 \~japanese @brief カーソルが動いた時に呼び出される関数。
	 @param[in] width ウィンドウの幅。
	 @param[in] height ウィンドウの高さ。
	 @param[in] x カーソルの x 座標のピクセル座標。
	 @param[in] y カーソルの y 座標のピクセル座標。
	 */
	virtual void cursor( int width, int height, double x, double y ) {}
	/**
	 \~english @brief Function that catches mouse event.
	 @param[in] width Window width.
	 @param[in] height Window height.
	 @param[in] x Cursor position on x pixel coordinate.
	 @param[in] y Cursor position on y pixel coordinate.
	 @param[in] button Mouse button number.
	 @param[in] action Mouse action (Mouse down = 1 Mouse release = 0)
	 \~japanese @brief マウスイベントが発生した時に呼ばれる関数。
	 @param[in] width ウィンドウの幅。
	 @param[in] height ウィンドウの高さ。
	 @param[in] x カーソルの x 座標のピクセル座標。
	 @param[in] y カーソルの y 座標のピクセル座標。
	 @param[in] button マウスのボタンのナンバー。
	 @param[in] action アクションの種類。マウスダウン＝1、マウスアップ=0。
	 */
	virtual void mouse( int width, int height, double x, double y, int button, int action ) {}
	/**
	 \~english @brief Function that catches passive cursor event.
	 @param[in] width Window width.
	 @param[in] height Window height.
	 @param[in] x Cursor position on x pixel coordinate.
	 @param[in] y Cursor position on y pixel coordinate.
	 @param[in] u Cursor movement on x pixel coordinate.
	 @param[in] v Cursor movement on y pixel coordinate.
	 \~japanese @brief カーソルが動いた時に呼び出される関数。
	 @param[in] width ウィンドウの幅。
	 @param[in] height ウィンドウの高さ。
	 @param[in] x カーソルの x 座標のピクセル座標。
	 @param[in] y カーソルの y 座標のピクセル座標。
	 @param[in] u カーソルの x 成分に動いた長さ。
	 @param[in] v カーソルの y 成分に動いた長さ。
	 */
	virtual void drag( int width, int height, double x, double y, double u, double v ) {}
	/**
	 \~english @brief Function that catches window resizing events.
	 @param[in] g Graphics engine.
	 @param[in] width Width of the window.
	 @param[in] height Height of the window.
	 \~japanese @brief ウィンドウのリサイズイベントが発生する時に呼ばれる関数。
	 @param[in] g グラフィックスエンジン。
	 @param[in] width ウィンドウの横幅。
	 @param[in] height ウィンドウの高さ。
	 */
	virtual void resize( const graphics_engine &g, int width, int height ) { view_change(g,width,height); }
	/**
	 \~english @brief Function that catches view change evenets.
	 @param[in] g Graphics engine.
	 @param[in] width Width of the window.
	 @param[in] height Height of the window.
	 \~japanese @brief ビューの変更が発生する時に呼ばれる関数。
	 @param[in] g グラフィックスエンジン。
	 @param[in] width ウィンドウの横幅。
	 @param[in] height ウィンドウの高さ。
	 */
	virtual void view_change( const graphics_engine &g, int width, int height ) { g.configure_view(width,height,SPATIAL_DIM); }
	/**
	 \~english @brief Function that catches draw event.
	 @param[in] g Graphics engine.
	 @param[in] width Width of the window.
	 @param[in] height Height of the window.
	 \~japanese @brief 描画イベントが発生する時に呼ばれる関数。
	 @param[in] g グラフィックスエンジン。
	 @param[in] width ウィンドウの横幅。
	 @param[in] height ウィンドウの高さ。
	 */
	virtual void draw( const graphics_engine &g, int width, int height ) const {}
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
	//
private:
	environment_map m_environment;
};
//
SHKZ_END_NAMESPACE
//
#endif