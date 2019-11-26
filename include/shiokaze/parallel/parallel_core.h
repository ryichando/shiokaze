/*
**	parallel_core.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 1, 2017.
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
#ifndef SHKZ_PARALLEL_CORE_H
#define SHKZ_PARALLEL_CORE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <functional>
#include <vector>
#include <dlfcn.h>
//
SHKZ_BEGIN_NAMESPACE
//
static bool *g_shkz_force_single_thread {nullptr};
//
/** @file */
/// \~english @brief Abstract class that handles parallel operations. Used with loop_splitter. "stdthread" and "tbbthread" are provided as implementations.
/// \~japanese @brief 並列処理を行う抽象クラス。loop_splitter と共に使われる。"stdthread" と "tbbthread" が実装として提供される。
class parallel_core : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(parallel_core,"Parallel Core","Parallel","Paralell operation dispatcher module")
	/**
	 \~english @brief Perform a parallel loop operation.
	 @param[in] func Function that processes the actual loop.
	 @param[in] iterator_start Function that provides how to start the loop.
	 @param[in] iterator_advance Function that provides how to advance the loop.
	 \~japanese @brief 並列処理を行う。
	 @param[in] func 実際のループ処理を行う関数。
	 @param[in] iterator_start ループの最初の位置を取得する関数。
	 @param[in] iterator_advance ループの進行を行う関数。
	 */
	virtual void for_each(
		std::function<void(size_t n, int thread_index)> func,
		std::function<size_t(int thread_index)> iterator_start,
		std::function<bool(size_t &n, int thread_index)> iterator_advance,
		int num_threads ) const = 0;
	/**
	 \~english @brief Run operations in parallel.
	 @param[in] functions List of operation functions that are ran in parallel.
	 \~japanese @brief 処理を並列に行う。
	 @param[in] functions 並列に行う処理関数のリスト。
	 */
	virtual void run( const std::vector<std::function<void()> > &functions ) const = 0;
	/**
	 \~english @brief Set if force single thread.
	 @param[in] value Boolean value to set force single thread.
	 \~japanese @brief 強制的にシングルスレッドにするか設定する。
	 @param[in] value シングルスレッドにするか指定する値。
	 */
	static void force_single_thread( bool value ) {
		if( ! g_shkz_force_single_thread ) {
			g_shkz_force_single_thread = static_cast<bool *>(::dlsym(RTLD_DEFAULT,"g_shkz_force_single_thread"));
			assert(g_shkz_force_single_thread);
		}
		*g_shkz_force_single_thread = value;
	}
private:
};
//
using parallel_ptr = std::unique_ptr<parallel_core>;
//
SHKZ_END_NAMESPACE
//
#endif
//