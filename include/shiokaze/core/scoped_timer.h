/*
**	scoped_timer.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 15, 2018.
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
#ifndef SHKZ_SCOPE_TIMER_H
#define SHKZ_SCOPE_TIMER_H
//
#include <string>
#include <stack>
#include <shiokaze/core/credit.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Timer designed to calculate the timings within scope.
/// \~japanese @brief スコープの中の処理の時間を計測するクラス。
class scoped_timer {
public:
	/**
	 \~english @brief Cnstructor.
	 @param[in] master_name Master name.
	 @param[in] secondary_name Secondary name.
	 \~japanese @brief コンストラクタ。
	 @param[in] master_name マスター名。
	 @param[in] secondary_name サブネーム。
	 */
	scoped_timer( std::string master_name="", std::string secondary_name="" );
	/**
	 \~english @brief Constructor.
	 @param[in] cr Poiter to an instance of credit.
	 @param[in] secondary_name Secondary name.
	 \~japanese @brief コンストラクタ。
	 @param[in] cr クレジットのインスタンスへのポインター。
	 @param[in] secondary_name サブネーム。
	 */
	scoped_timer( const credit *cr, std::string secondary_name="" );
	/**
	 \~english @brief Default destructor for scoped_timer.
	 \~japanese @brief scoped_timer のデフォルトデストラクタ。
	 */
	virtual ~scoped_timer();
	/**
	 \~english @brief Start recording time.
	 \~japanese @brief 時間の計測を開始する。
	 */
	void tick();
	/**
	 \~english @brief Stop recording time.
	 @param[in] subname Subname.
	 @return Time in milliseconds.
	 \~japanese @brief 時間の計測を終了する。
	 @param[in] subname サブネーム。
	 @return ミリセカンド秒。
	 */
	double tock( std::string subname="" );
	/**
	 \~english @brief Stop recording time and return time string.
	 @param[in] subname Subname.
	 @return Human readable timing information.
	 \~japanese @brief 時間の計測を終了し、可読性のある時間の文字列を返す。
	 @param[in] subname サブネーム。
	 @return 可読性のある時間の文字。
	 */
	std::string stock( std::string subname="" );
	//
private:
	//
	std::stack<double> m_time_stack;
	std::string m_master_name;
	int m_pause_counter;
	//
};
//
SHKZ_END_NAMESPACE
//
#endif