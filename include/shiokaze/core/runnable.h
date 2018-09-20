/*
**	runnable.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 11, 2017. 
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
#ifndef SHKZ_RUNNABLE_H
#define SHKZ_RUNNABLE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that performs a task.
/// \~japanese @brief タスクを実行するクラス。
class runnable : public recursive_configurable_module {
public:
	//
	LONG_NAME("runnable")
	/**
	 \~english @brief Constructor for runnable.
	 \~japanese @brief runnable のコンストラクタ。
	 */
	runnable () : m_running(true) {}
	/**
	 \~english @brief Destructor for runnable.
	 \~japanese @brief runnable のデストラクタ。
	 */
	virtual ~runnable() {}
	/**
	 \~english @brief Function that is called only one time on start.
	 \~japanese @brief 最初に一度だけ呼ばれる関数。
	 */
	virtual void run_onetime() {}
	/**
	 \~english @brief idling function that is called while is_running() returns true.
	 \~japanese @brief is_running() が \c true を返す間に呼ばれるアイドリング関数。
	 */
	virtual void idle() {}
	/**
	 \~english @brief Function that if return true, the program will exit.
	 \~japanese @brief もし true を返したら、プログラムが終了する関数。
	 */
	virtual bool should_quit() const { return true; }
	/**
	 \~english @brief Get if the simulation is running.
	 \~japanese @brief シミュレーションが実行中がどうか取得する関数。
	 */
	virtual bool is_running() const { return m_running; }
	/**
	 \~english @brief Set if the simulation is running.
	 \~japanese @brief シミュレーションが実行中か設定する関数。
	 */
	virtual void set_running( bool running ) { m_running = running; }
	//
private:
	bool m_running;
protected:
	virtual void initialize( const environment_map &environment ) override { run_onetime(); }
};
//
SHKZ_END_NAMESPACE
//
#endif