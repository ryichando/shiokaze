/*
**	parallel_driver.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 1, 2018.
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
#ifndef SHKZ_PARALLEL_DRIVER_H
#define SHKZ_PARALLEL_DRIVER_H
//
#include <algorithm>
#include <vector>
#include <shiokaze/core/credit.h>
#include <shiokaze/core/configurable.h>
#include <shiokaze/array/shape.h>
#include "parallel_core.h"
#include "loop_splitter.h"
//
SHKZ_BEGIN_NAMESPACE
//
#define shkz_default_parallel_name	"stdthread"
#define shkz_default_splitter_name	"sequential_splitter"
//
/** @file */
/// \~english @brief Class that facilitates the use of parallel_core class for parallel loop.
/// \~japanese @brief 並列ループのための parallel_core を使いやすくするクラス。
class parallel_driver : public configurable, public credit {
public:
	//
	LONG_NAME("Parallel Driver")
	ARGUMENT_NAME("Parallel")
	/**
	 \~english @brief Constructor for parallel_driver.
	 @param[in] parent Pointer to an instance of recursive_configurable.
	 @param[in] parallel_name Core name of the parallelization engine. "stdthread" is set as default.
	 @param[in] splitter_name Core name of the parallel enumeration splitter name. "sequential_splitter" is set as default.
	 \~japanese @brief parallel_driver のコンストラクタ。
	 @param[in] parent recursive_configurable のインスタンスへのポインタ。
	 @param[in] parallel_name 並列化エンジンのコア名前。"stdthread" がデフォルトとして設定される。
	 @param[in] splitter_name 並列化分散コアの名前。"sequential_splitter" がデフォルトとして設定される。
	 */
	parallel_driver ( recursive_configurable *parent, std::string parallel_name=shkz_default_parallel_name, std::string splitter_name=shkz_default_splitter_name ) : m_parallel_name(parallel_name), m_splitter_name(splitter_name) {
		parent->add_child(this);
	}
	/**
	 \~english @brief Constructor for parallel_driver.
	 @param[in] parallel_name Core name of the parallelization engine. "stdthread" is set as default.
	 @param[in] splitter_name Core name of the parallel enumeration splitter name. "sequential_splitter" is set as default.
	 \~japanese @brief parallel_driver のコンストラクタ。
	 @param[in] parallel_name 並列化エンジンのコア名前。"stdthread" がデフォルトとして設定される。
	 @param[in] splitter_name 並列化分散コアの名前。"sequential_splitter" がデフォルトとして設定される。
	 */
	parallel_driver ( std::string parallel_name=shkz_default_parallel_name, std::string splitter_name=shkz_default_splitter_name ) : m_parallel_name(parallel_name), m_splitter_name(splitter_name) {
		configurable::setup_now(this);
	}
	/**
	 \~english @brief Get the number of maximal threads set.
	 @return Number of threads set.
	 \~japanese @brief 最大のスレッドの数を得る。
	 @return スレッドの数を得る。
	 */
	int get_maximal_threads() const {
		return m_maximal_threads;
	}
	/**
	 \~english @brief Set the number of maximal threads set.
	 @param maximal_threads Number of threads to set.
	 \~japanese @brief 最大のスレッドの数をセットする。
	 @param maximal_threads セットするスレッドの数。
	 */
	void set_maximal_threads( int maximal_threads ) {
		m_maximal_threads = maximal_threads;
	}
	/**
	 \~english @brief Get a pointer to the internal instance of parallel_core.
	 @return Pointer to the internal parallel_core instance.
	 \~japanese @brief 内部の parallel_core のインスタンスへのポインターを獲る。
	 @return 内部の parallel_core インスタンスへのポインター。
	 */
	const parallel_core *get() const {
		return m_parallel_dispatcher.get();
	}
	/**
	 \~english @brief Run operations in parallel.
	 @param[in] functions List of operation functions that are ran in parallel.
	 \~japanese @brief 処理を並列に行う。
	 @param[in] functions 並列に行う処理関数のリスト。
	 */
	inline void run( const std::vector<std::function<void()> > &functions ) {
		if( m_maximal_threads > 1 ) {
			m_parallel_dispatcher->run(functions);
		} else {
			for( auto f : functions ) f();
		}
	}
	/**
	 \~english @brief Perform a parallel loop operation.
	 @param[in] size Size of the loop.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 並列処理を行う。
	 @param[in] size ループの大きさ。
	 @param[in] func 実際のループ処理を行う関数。
	 */
	inline void for_each( size_t size, std::function<void(size_t n, int thread_index)> func ) const {
		//
		if( size ) {
			int num_threads = m_maximal_threads;
			if( num_threads > size ) num_threads = size;
			if( num_threads > 1 ) {
				//
				assert(m_loop_splitter.get());
				//
				const void *context = m_loop_splitter->new_context(size,num_threads);
				auto start_func = m_loop_splitter->get_start_func(context);
				auto advance_func = m_loop_splitter->get_advance_func(context);
				//
				m_parallel_dispatcher->for_each(func,
					[&](int q) { return start_func(context,q); },
					[&](size_t &n, int q){ return advance_func(context,n,q); },
				num_threads);
				//
				m_loop_splitter->delete_context(context);
			} else {
				for( size_t n=0; n<size; ++n ) {
					func(n,0);
				}
			}
		}
	}
	/**
	 \~english @brief Perform a parallel loop operation.
	 @param[in] size Size of the loop.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 並列処理を行う。
	 @param[in] size ループの大きさ。
	 @param[in] func 実際のループ処理を行う関数。
	 */
	inline void for_each( size_t size, std::function<void(size_t n)> func ) const {
		for_each(size,[&]( size_t n, int thread_n ) {
			func(n);
		});
	}
	/**
	 \~english @brief Perform a two dimensional parallel loop operation.
	 @param[in] shape Two dimensional shape.
	 @param[in] ordering Ordering of the loop.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 2次元の並列処理を行う。
	 @param[in] shape ２次元の形状。
	 @param[in] ordering ループの順番。
	 @param[in] func 実際のループを処理する関数。
	 */
	inline void for_each2( const shape2 &shape, std::function<void(int i, int j, int thread_index)> func ) const {
		for_each(shape.count(),[&]( size_t n, int thread_index ) {
			int i = n % shape.w;
			int j = n / shape.w;
			func(i,j,thread_index);
		});
	}
	/**
	 \~english @brief Perform a two dimensional parallel loop operation.
	 @param[in] shape Two dimensional shape.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 2次元の並列処理を行う。
	 @param[in] shape ２次元の形状。
	 @param[in] func 実際のループを処理する関数。
	 */
	inline void for_each2( const shape2 &shape, std::function<void(int i, int j)> func ) const {
		for_each2(shape,[&]( int i, int j, int thread_index ) {
			func(i,j);
		});
	}
	/**
	 \~english @brief Perform a three dimensional parallel loop operation.
	 @param[in] shape Three dimensional shape.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 3次元の並列処理を行う。
	 @param[in] shape 3次元の形状。
	 @param[in] func 実際のループを処理する関数。
	 */
	inline void for_each3( const shape3 &shape, std::function<void(int i, int j, int k, int thread_index)> func ) const {
		size_t plane_count = shape.w * shape.h;
		for_each(shape.count(),[&]( size_t n, int thread_index ) {
			int k = n / plane_count;
			size_t m = n % plane_count;
			int i = m % shape.w;
			int j = m / shape.w;
			func(i,j,k,thread_index);
		});
	}
	/**
	 \~english @brief Perform a three dimensional parallel loop operation.
	 @param[in] shape Three dimensional shape.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 3次元の並列処理を行う。
	 @param[in] shape 3次元の形状。
	 @param[in] func 実際のループを処理する関数。
	 */
	inline void for_each3( const shape3 &shape, std::function<void(int i, int j, int k)> func ) const {
		for_each3(shape,[&]( int i, int j, int k, int thread_index ) {
			func(i,j,k);
		});
	}
	//
private:
	//
	std::string m_parallel_name, m_splitter_name;
	parallel_ptr m_parallel_dispatcher;
	loop_splitter_ptr m_loop_splitter;
	int m_maximal_threads {NUM_THREAD};
	//
	virtual void load( configuration &config ) override {
		configuration::auto_group group(config,*this);
		m_parallel_dispatcher = parallel_core::quick_load_module(config,m_parallel_name);
		m_loop_splitter = loop_splitter::quick_load_module(config,m_splitter_name);
	}
	//
	virtual void configure( configuration &config ) override {
		configuration::auto_group group(config,*this);
		config.get_integer("Threads",m_maximal_threads,"Number of maximal threads");
		m_parallel_dispatcher->recursive_configure(config);
		m_loop_splitter->recursive_configure(config);
	}
};
//
/// \~english @brief Class that facilitates the use of serial loop operations.
/// \~japanese @brief 逐次処理ループを使いやすくするクラス。
class serial {
public:
	/**
	 \~english @brief Perform a serial loop operation.
	 @param[in] size Size of the loop.
	 @param[in] func Function that processes a loop. If the function return \c true, the loop interrupts.
	 \~japanese @brief 逐次処理を行う。
	 @param[in] size ループの大きさ。
	 @param[in] func 実際のループを処理する関数。もし関数が \c true を返すと、ループ処理を中断する。
	 */
	static void interruptible_for_each( size_t size, std::function<bool(size_t n)> func ) {
		for( size_t n=0; n<size; ++n ) {
			if(func(n)) break;
		}
	}
	/**
	 \~english @brief Perform a serial loop operation.
	 @param[in] shape Two dimensional shape.
	 @param[in] func Function that processes a loop. If the function return \c true, the loop interrupts.
	 \~japanese @brief 逐次処理を行う。
	 @param[in] shape 2次元の形状。
	 @param[in] func 実際のループを処理する関数。もし関数が \c true を返すと、ループ処理を中断する。
	 */
	static void interruptible_for_each2( const shape2 &shape, std::function<bool(int i, int j)> func ) {
		interruptible_for_each(shape.count(),[&]( size_t n ) {
			int i = n % shape.w;
			int j = n / shape.w;
			return func(i,j);
		});
	}
	/**
	 \~english @brief Perform a serial loop operation.
	 @param[in] shape Three dimensional shape.
	 @param[in] func Function that processes a loop. If the function return \c true, the loop interrupts.
	 \~japanese @brief 逐次処理を行う。
	 @param[in] shape 3次元の形状。
	 @param[in] func 実際のループを処理する関数。もし関数が \c true を返すと、ループ処理を中断する。
	 */
	static void interruptible_for_each3( const shape3 &shape, std::function<bool(int i, int j, int k)> func ) {
		if( ! shape.empty()) {
			size_t plane_count = shape.w * shape.h;
			interruptible_for_each(shape.count(),[&]( size_t n ) {
				int k = n / plane_count;
				size_t m = n % plane_count;
				int i = m % shape.w;
				int j = m / shape.w;
				return func(i,j,k);
			});
		}
	}
	/**
	 \~english @brief Perform a serial loop operation.
	 @param[in] size Size of the loop.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 逐次処理を行う。
	 @param[in] size ループの大きさ。
	 @param[in] func 実際のループを処理する関数。
	 */
	static void for_each( size_t size, std::function<void(size_t n)> func ) {
		for( size_t n=0; n<size; ++n ) {
			func(n);
		}
	}
	/**
	 \~english @brief Perform a two dimensional serial loop operation.
	 @param[in] shape Two dimensional shape.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 2次元の逐次処理を行う。
	 @param[in] shape 2次元の形状。
	 @param[in] func 実際のループを処理する関数。
	 */
	static void for_each2( const shape2 &shape, std::function<void(int i, int j)> func ) {
		for_each(shape.count(),[&]( size_t n ) {
			int i = n % shape.w;
			int j = n / shape.w;
			func(i,j);
		});
	}
	/**
	 \~english @brief Perform a three dimensional serial loop operation.
	 @param[in] shape Two dimensional shape.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 3次元の逐次処理を行う。
	 @param[in] shape 3次元の形状。
	 @param[in] func 実際のループを処理する関数。
	 */
	static void for_each3( const shape3 &shape, std::function<void(int i, int j, int k)> func ) {
		size_t plane_count = shape.w * shape.h;
		for_each(shape.count(),[&]( size_t n ) {
			int k = n / plane_count;
			size_t m = n % plane_count;
			int i = m % shape.w;
			int j = m / shape.w;
			func(i,j,k);
		});
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//