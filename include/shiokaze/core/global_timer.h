/*
**	global_timer.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 16, 2018.
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
#ifndef SHKZ_GLOBAL_TIMER_H
#define SHKZ_GLOBAL_TIMER_H
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that deals with global timings.
/// \~japanese @brief グローバルなタイマーを管理するクラス。
class global_timer {
public:
	/**
	 \~english @brief Pause global time measurement.
	 \~japanese @brief グローバル時間の計測を止める。
	 */
	static void pause();
	/**
	 \~english @brief Reume global time measurement.
	 \~japanese @brief グローバル時間の計測を再開する。
	 */
	static void resume();
	/**
	 \~english @brief Get the current global time.
	 @return Current globla time.
	 \~japanese @brief 現在のグローバル時間を得る。
	 @return 現在のグローバル時間。
	 */
	static double get_milliseconds();
	//
};
//
SHKZ_END_NAMESPACE
//
#endif