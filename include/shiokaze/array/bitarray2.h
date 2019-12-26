/*
**	bitarray2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on August 8, 2018.
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
#ifndef SHKZ_BITARRAY2_H
#define SHKZ_BITARRAY2_H
//
#include <shiokaze/math/vec.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <utility>
#include <shiokaze/math/shape.h>
#include "array_core2.h"
//
SHKZ_BEGIN_NAMESPACE
//
template <class T> class array2;
/** @file */
/// \~english @brief Two dimensional bit grid class designed to be defined as instance member in recursive_configurable class.
/// \~japanese @brief recursive_configurable インスタンスのメンバーインスタンスとして定義可能な2次元ビット配列クラス。
class bitarray2 : public recursive_configurable, public messageable {
public:
	/**
	 \~english @brief Constructor for bitarray2.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] shape Shape of the grid
	 @param[in] core_name Core module name. Default value is "tiledarray2".
	 \~japanese @brief bitarray2 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] shape グリッドの形
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray2"。
	 */
	bitarray2( recursive_configurable *parent, const shape2 &shape, std::string core_name="" ) :
		m_core_name(core_name), m_shape(shape) {
			if( parent ) parent->add_child(this);
			else setup_now();
	}
	/**
	 \~english @brief Constructor for bitarray2.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] core_name Core module name. Default value is "tiledarray2".
	 \~japanese @brief bitarray2 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray2"。
	 */
	bitarray2( recursive_configurable *parent, std::string core_name="" ) : bitarray2(parent,shape2(0,0),core_name) {}
	/**
	 \~english @brief Constructor for bitarray2.
	 @param[in] core_name Core module name. Default value is "tiledarray2".
	 \~japanese @brief bitarray2 のコンストラクタ。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray2"。
	 */
	bitarray2( std::string core_name="" ) : bitarray2(nullptr,shape2(0,0),core_name) {}
	/**
	 \~english @brief Constructor for bitarray2.
	 @param[in] shape Shape of the grid
	 @param[in] core_name Core module name. Default value is "tiledarray2".
	 \~japanese @brief bitarray2 のコンストラクタ。
	 @param[in] shape グリッドの形
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray2"。
	 */
	bitarray2( const shape2 &shape, std::string core_name="" ) : bitarray2(nullptr,shape,core_name) {}
	//
private:
	//
	virtual void load( configuration &config ) override {
		if( m_core_name.empty()) {
			m_core_name = shkz_default_array_core2;
		} else {
			auto pos = m_core_name.find('*');
			if( pos != std::string::npos ) {
				m_core_name.erase(pos,1);
				m_core_name.insert(pos,shkz_default_array_core2);
			}
		}
		m_core = array_core2::quick_load_module(config,m_core_name);
	}
	//
	virtual void configure( configuration &config ) override {
		m_core->recursive_configure(config);
	}
	//
	virtual void post_initialize() override {
		if( shape().count() && ! m_is_initialized ) {
			initialize(m_shape);
		}
	}
	//
public:
	/**
	 \~english @brief Send a message to the core module.
	 @param[in] message Message
	 @param[in] ptr Pointer to some value.
	 @return \c true if handled \c false otherwise.
	 \~japanese @brief コアモジュールにメッセージを送る
	 @param[in] message メッセージ
	 @param[in] ptr あるポインターの値
	 @return もし処理されたら \c true を、処理されなかったら \c false
	 */
	virtual bool send_message( std::string message, void *ptr ) override {
		return get_core()->send_message(message,ptr);
	}
	/**
	 \~english @brief Send a message to the core module.
	 @param[in] message Message
	 @param[in] ptr Pointer to some value.
	 @return \c true if handled \c false otherwise.
	 \~japanese @brief コアモジュールにメッセージを送る
	 @param[in] message メッセージ
	 @param[in] ptr あるポインターの値
	 @return もし処理されたら \c true を、処理されなかったら \c false
	 */
	virtual bool const_send_message( std::string message, void *ptr ) const override {
		return get_core()->const_send_message(message,ptr);
	}
	/**
	 \~english @brief Copy constructor for bitarray2.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitarray2 のコピーコンストラクタ。
	 @param[in] コピーする bitarray2 のインスタンスへの参照。
	 */
	bitarray2( const bitarray2 &array ) {
		m_core_name = array.m_core_name;
		setup_now();
		copy(array);
	}
	/**
	 \~english @brief Deep copy operation for bitarray2.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitarray2 のディープコピー演算子。
	 @param[in] コピーする bitarray2 のインスタンスへの参照。
	 */
	bitarray2& operator=(const bitarray2 &array) {
		if( this != &array ) {
			copy(array);
		}
		return *this;
	}
	/**
	 \~english @brief Deep copy function for bitarray2.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitarray2 のディープコピー関数。
	 @param[in] コピーする bitarray2 のインスタンスへの参照。
	 */
	void copy( const bitarray2 &array ) {
		if( this != &array ) {
			set_type(array.type());
			assert(m_core);
			if( array.m_core ) {
				m_core->copy(*array.get_core(),[&](void *target, const void *src){},m_parallel);
			}
		}
	}
	virtual ~bitarray2() {
		clear();
	}
	/**
	 \~english @brief Get the shape of the array.
	 @return Shape of the array.
	 \~japanese @brief グリッドの形を返す。
	 @return グリッドの形。
	 */
	shape2 shape() const { return m_shape; }
	/**
	 \~english @brief Allocate grid memory with value.
	 @param[in] shape Shape of the grid.
	 \~japanese @brief グリッドを値でメモリに展開する。
	 @param[in] shape グリッドの形。
	 */
	void initialize( const shape2 &shape ) {
		clear();
		m_core->initialize(shape.w,shape.h,0);
		m_shape = shape;
		m_is_initialized = true;
	}
	/**
	 \~english @brief Function to count the number of active cells.
	 @return Active cell count.
	 \~japanese @brief アクティブセルの数を数える関数。
	 @return アクティブセルの数。
	 */
	size_t count () const { return m_core->count(m_parallel); }
	/**
	 \~english @brief Function to return the list of active cells positions.
	 @return The list of active cells positions.
	 \~japanese @brief アクティブセルの位置のリストを返す関数。
	 @return アクティブセルの位置のリスト。
	 */
	std::vector<vec2i> actives() const {
		std::vector<vec2i> result;
		const_serial_actives([&](int i, int j) {
			result.push_back(vec2i(i,j));
		});
		return result;
	}
	/**
	 \~english @brief Activate cells at the positons of active_entries with an offset.
	 @param[in] active_entries The list of target positions to activate.
	 @param[in] offset Offset applied to the active_entries.
	 \~japanese @brief active_entries と同じ場所のセルを offset だけずらして、アクティブにする。
	 @param[in] active_entries アクティブにするセルの場所のリスト。
	 @param[in] offset active_entries に適用されるオフセット。
	 */
	void activate( const std::vector<vec2i> &active_entries, const vec2i &offset=vec2i() ) {
		for( const auto &e : active_entries ) {
			const vec2i &pi = e + offset;
			if( ! shape().out_of_bounds(pi) && ! (*this)(pi)) {
				set(pi);
			}
		}
	}
	/**
	 \~english @brief Activate cells at the same positons where an input array is active with an offset.
	 @param[in] array Target array.
	 @param[in] offset Offset applied to the target array.
	 \~japanese @brief 入力のグリッドのアクティブセルと同じ場所のセルを offset だけずらして、アクティブにする。
	 @param[in] array 目標となるグリッド。
	 @param[in] offset 目標となるグリッドに適用されるオフセット。
	 */
	template <class Y> void activate_as( const array2<Y> &array, const vec2i &offset=vec2i() ) {
		array.const_serial_actives([&](int i, int j, const auto &it) {
			const vec2i &pi = vec2i(i,j) + offset;
			if( ! this->shape().out_of_bounds(pi) && ! (*this)(pi)) {
				this->set(pi);
			}
		});
	}
	/**
	 \~english @brief Activate cells at the same positons where an input array is active with an offset.
	 @param[in] array Target array.
	 @param[in] offset Offset applied to the target array.
	 \~japanese @brief 入力のグリッドのアクティブセルと同じ場所のセルを offset だけずらして、アクティブにする。
	 @param[in] array 目標となるグリッド。
	 @param[in] offset 目標となるグリッドに適用されるオフセット。
	 */
	template <class Y> void activate_as_bit( const Y &array, const vec2i &offset=vec2i() ) {
		array.const_serial_actives([&](int i, int j) {
			const vec2i &pi = vec2i(i,j) + offset;
			if( ! this->shape().out_of_bounds(pi) && ! (*this)(pi)) {
				this->set(pi);
			}
		});
	}
	/**
	 \~english @brief Activate cells at the same positons where an input array is filled with an offset.
	 @param[in] array Target array.
	 @param[in] offset Offset applied to the target array.
	 \~japanese @brief 入力のグリッドの塗りつぶされたセルと同じ場所のセルを offset だけずらして、アクティブにする。
	 @param[in] array 目標となるグリッド。
	 @param[in] offset 目標となるグリッドに適用されるオフセット。
	 */
	template <class Y> void activate_inside_as( const array2<Y> &array, const vec2i &offset=vec2i() ) {
		array.const_serial_inside([&](int i, int j, const auto &it) {
			const vec2i &pi = vec2i(i,j) + offset;
			if( ! this->shape().out_of_bounds(pi) && ! (*this)(pi)) {
				this->set(pi);
			}
		});
	}
	/**
	 \~english @brief Activate all the cells.
	 \~japanese @brief 全てのセルをアクティブにする。
	 */
	void activate_all() {
		parallel_all([&](auto &it) {
			it.set();
		});
	}
	/**
	 \~english @brief Copy the states of active and inactive cells as same as input array with an offset.
	 @param[in] array Target input array from which the states to be copied.
	 @param[in] offset Offset
	 \~japanese @brief セルのアクティブと非アクティブステートの状態を入力のグリッドと同じようにセットする。
	 @param[in] array 目標となる状態をコピーする元となるグリッド。
	 @param[in] offset オフセット
	 */
	void copy_active_as( const bitarray2 &array, const vec2i &offset=vec2i() ) {
		parallel_actives([&](int i, int j, auto &it, int tn) {
			const vec2i &pi = vec2i(i,j) + offset;
			if( ! this->shape().out_of_bounds(pi) ) {
				if( (*this)(pi) && ! array(pi)) {
					it.set_off();
				}
			}
		});
		activate_as_bit(array,offset);
	}
	/**
	 \~english @brief Clear out the grid.
	 *
	 Note that size, the memory allocation, background values and the information regarding level set or fillable left intact.
	 \~japanese @brief グリッドを初期化する。
	 *
	 グリッドの大きさやメモリ確保、レベルセット情報、バックグラウンド値は変更されない。
	 */
	void clear() {
		parallel_actives([&](iterator& it) {
			it.set_off();
		});
	}
	/**
	 \~english @brief Set bit on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 \~japanese @brief グリッドのビットを設定する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 */
	void set( int i, int j ) {
		m_core->set(i,j,[&](void *value_ptr, bool &active){
			active = true;
		});
	}
	/**
	 \~english @brief Set bit on grid.
	 @param[in] pi position on grid
	 @param[in] value Value to set at the position.
	 \~japanese @brief グリッドのビットを設定する。
	 @param[in] pi グリッドの位置。
	 @param[in] value この位置で設定する値。
	 */
	void set( const vec2i &pi ) {
		set(pi[0],pi[1]);
	}
	/**
	 \~english @brief Get if a position on grid is active.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @return \c true if active \c flase if inactive.
	 \~japanese @brief グリッドのある位置がアクティブか得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @return アクティブなら \c true 非アクティブなら \c false。
	 */
	bool operator()( int i, int j ) const {
		bool filled;
		return (*m_core)(i,j,filled) != nullptr;
	}
	/**
	 \~english @brief Get if a position on grid is active.
	 @param[in] pi position on grid.
	 @return \c true if active \c flase if inactive.
	 \~japanese @brief グリッドのある位置がアクティブか得る。
	 @param[in] pi x グリッドでの位置。
	 @return アクティブなら \c true 非アクティブなら \c false。
	 */
	bool operator()( const vec2i &pi ) const {
		return (*this)(pi[0],pi[1]);
	}
	/**
	 \~english @brief Get if a position on grid is active. (i,j) can be safely out of the domain.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @return \c true if active \c flase if inactive.
	 \~japanese @brief グリッドのある位置がアクティブか得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @return アクティブなら \c true 非アクティブなら \c false。(i,j) は領域外でも安全に取得可能。
	 */
	bool safe_get( int i, int j ) const {
		if( ! m_shape.out_of_bounds(i,j)) {
			return (*this)(i,j);
		}
		return false;
	}
	/**
	 \~english @brief Get if a position on grid is active. pi can be safely out of the domain.
	 @param[in] pi position on grid.
	 @return \c true if active \c flase if inactive.
	 \~japanese @brief グリッドのある位置がアクティブか得る。
	 @param[in] pi x グリッドでの位置。
	 @return アクティブなら \c true 非アクティブなら \c false。 pi は領域外でも安全に取得可能。
	 */
	bool safe_get( const vec2i &pi ) const {
		return (*this)(pi[0],pi[1]);
	}
	/**
	 \~english @brief Set a position on grid inactive.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 \~japanese @brief グリッドの指定された位置を非アクティブにする。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 */
	void set_off( int i, int j ) {
		m_core->set(i,j,[&](void *value_ptr, bool &active){
			active = false;
		});
	}
	/**
	 \~english @brief Set a position on grid inactive.
	 @param[in] pi position on grid
	 \~japanese @brief グリッドの指定された位置を非アクティブにする。
	 @param[in] pi グリッドでの位置。
	 */
	void set_off( const vec2i &pi ) {
		set_off(pi[0],pi[1]);
	}
	/**
	 \~english @brief Return if the grid is different from an input array.
	 @param[in] array Target array to compare.
	 @return \c true if the array is different from the input array and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと違うかどうか返す。
	 @param[in] array 目標とする比べるグリッド。
	 @return もしグリッドが入力と違うグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator!=( const bitarray2 &array ) const {
		return ! (*this == array);
	}
	/**
	 \~english @brief Return if the grid is same to an input array.
	 @param[in] array Target array to compare.
	 @return \c true if the array is the same to the input and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと同じかどうか返す。
	 @param[in] array 目標とする比べるグリッド。
	 @return もしグリッドが入力と同じグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator==(const bitarray2 &v) const {
		if( v.type() == type() ) {
			bool differnt (false);
			interruptible_const_serial_actives([&]( int i, int j ) {
				if( ! v(i,j)) {
					differnt = true;
					return true;
				} else {
					return false;
				}
			});
			return ! differnt;
		}
		return false;
	}
	/**
	 \~english @brief Set the number of threads for parallel processing on this grid.
	 @param[in] number Number of threads.
	 \~japanese @brief 並列処理をするためのスレッドの数を設定する。
	 @param[in] number スレッドの数。
	 */
	void set_thread_num( int number ) {
		m_parallel.set_thread_num(number);
	}
	/**
	 \~english @brief Get the current number of threads for parallel processing on this grid.
	 @return number Number of threads.
	 \~japanese @brief 現在設定されている並列処理をするためのスレッドの数を得る。
	 @return number スレッドの数。
	 */
	int get_thread_num() const {
		return m_parallel.get_thread_num();
	}
	/// \~english @brief Writable iterator.
	/// \~japanese @brief 書き込み可能なイテレーター。
	class iterator {
	friend class bitarray2;
	public:
		/**
		 \~english @brief Set a value.
		 @param[in] value Value to set.
		 \~japanese @brief 値をセットする。
		 @param[in] value セットする値。
		 */
		void set() {
			m_active = true;
		}
		/**
		 \~english @brief Inactivate a cell.
		 \~japanese @brief セルを非アクティブにする。
		 */
		void set_off() {
			m_active = false;
		}
		/**
		 \~english @brief Get if a cell is active.
		 \~japanese @brief セルがアクティブが取得する。
		 */
		bool operator()() const {
			return m_active;
		}
	private:
		//
		iterator( bool &_active ) : m_active(_active) {}
		bool &m_active;
	};
	/// \~english @brief Read-only iterator.
	/// \~japanese @brief 読み込みのみ可能なイテレーター。
	class const_iterator {
	friend class bitarray2;
	public:
		/**
		 \~english @brief Get if a cell is active.
		 \~japanese @brief セルがアクティブが取得する。
		 */
		bool operator()() const {
			return m_active;
		}
	private:
		//
		const_iterator( const bool &_active ) : m_active(_active) {}
		const bool &m_active;
	};
	//
	enum { ACTIVES = true, ALL = false };
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_actives( std::function<void(iterator& it)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_all( std::function<void(iterator& it)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void parallel_op( std::function<void(iterator& it)> func, bool type=ALL ) {
		parallel_op([func](int i, int j, iterator& it, int thread_index){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_actives( std::function<void(int i, int j, iterator& it)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_all( std::function<void(int i, int j, iterator& it)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void parallel_op( std::function<void(int i, int j, iterator& it)> func, bool type=ALL ) {
		parallel_op([func](int i, int j, iterator& it, int thread_index){
			func(i,j,it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_actives( std::function<void(int i, int j, iterator& it, int thread_index)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_all( std::function<void(int i, int j, iterator& it, int thread_index)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void parallel_op( std::function<void(int i, int j, iterator& it, int thread_index)> func, bool type=ALL ) {
		if( type == ACTIVES ) {
			m_core->parallel_actives([&](int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_n ){
				iterator it(active);
				func(i,j,it,thread_n);
			},m_parallel);
		} else {
			m_core->parallel_all([&](int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_n ){
				iterator it(active);
				func(i,j,it,thread_n);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_all( std::function<void(const const_iterator& it)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_parallel_op( std::function<void(const const_iterator& it)> func, bool type=ALL ) const {
		const_parallel_op([func](int i, int j, const const_iterator& it, int thread_index){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(int i, int j)> func ) const { const_parallel_op([&](int i, int j, const const_iterator& it) {
		func(i,j); },ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_all( std::function<void(int i, int j, const const_iterator& it)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_parallel_op( std::function<void(int i, int j, const const_iterator& it)> func, bool type=ALL ) const {
		const_parallel_op([func](int i, int j, const const_iterator& it, int thread_index){
			func(i,j,it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(int i, int j, int thread_index)> func ) const { const_parallel_op(
		[&](int i, int j, const const_iterator& it, int thread_index) { func(i,j,thread_index); },ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_all( std::function<void(int i, int j, const const_iterator& it, int thread_index)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_parallel_op( std::function<void(int i, int j, const const_iterator& it, int thread_index)> func, bool type=ALL ) const {
		if( type == ACTIVES ) {
			m_core->const_parallel_actives([&](int i, int j, const void *value_ptr, const bool &filled, int thread_n ){
				bool active(true);
				const_iterator it(active);
				func(i,j,it,thread_n);
			},m_parallel);
		} else {
			m_core->const_parallel_all([&](int i, int j, const void *value_ptr, const bool &active, const bool &filled, int thread_n ){
				const_iterator it(active);
				func(i,j,it,thread_n);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_actives( std::function<void(iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_all( std::function<void(iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void serial_op( std::function<void(iterator& it)> func, bool type=ALL ) {
		serial_op([func](int i, int j, iterator& it){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_actives( std::function<void(int i, int j, iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_all( std::function<void(int i, int j, iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void serial_op( std::function<void(int i, int j, iterator& it)> func, bool type=ALL ) {
		if( type == ACTIVES ) {
			m_core->serial_actives([&](int i, int j, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				func(i,j,it);
				return false;
			});
		} else {
			m_core->serial_all([&](int i, int j, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				func(i,j,it);
				return false;
			});
		}
	}
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_all( std::function<void(const const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_serial_op( std::function<void(const const_iterator& it)> func, bool type=ALL ) const {
		const_serial_op([func](int i, int j, const const_iterator& it){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_actives( std::function<void(int i, int j)> func ) const {
		const_serial_op([&]( int i, int j, const const_iterator& it ) {func(i,j);},ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_all( std::function<void(int i, int j, const const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_serial_op( std::function<void(int i, int j, const const_iterator& it)> func, bool type=ALL ) const {
		if( type == ACTIVES ) {
			m_core->const_serial_actives([&](int i, int j, const void *value_ptr, const bool &filled ){
				bool active(true);
				const_iterator it(active);
				func(i,j,it);
				return false;
			});
		} else {
			m_core->const_serial_all([&](int i, int j, const void *value_ptr, const bool &active, const bool &filled ){
				const_iterator it(active);
				func(i,j,it);
				return false;
			});
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_serial_actives( std::function<bool(iterator& it)> func ) { interruptible_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_serial_all( std::function<bool(iterator& it)> func ) { interruptible_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_serial_op( std::function<bool(iterator& it)> func, bool type=ALL ) {
		interruptible_serial_op([func](int i, int j, iterator& it){
			return func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルをシリアルに処理する。\c true を返すと、ループを中断する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void interruptible_serial_actives( std::function<bool(int i, int j, iterator& it)> func ) { interruptible_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルをシリアルに処理する。\c true を返すと、ループを中断する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void interruptible_serial_all( std::function<bool(int i, int j, iterator& it)> func ) { interruptible_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_serial_op( std::function<bool(int i, int j, iterator& it)> func, bool type=ALL ) {
		if( type == ACTIVES ) {
			m_core->serial_actives([&](int i, int j, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				return func(i,j,it);
			});
		} else {
			m_core->serial_all([&](int i, int j, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				return func(i,j,it);
			});
		}
	}
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_all( std::function<bool(const const_iterator& it)> func ) const { interruptible_const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_const_serial_op( std::function<bool(const const_iterator& it)> func, bool type=ALL ) const {
		interruptible_const_serial_op([func](int i, int j, const const_iterator& it){
			return func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_actives( std::function<bool(int i, int j)> func ) const {
		interruptible_const_serial_op([&](int i, int j, const const_iterator& it) {return func(i,j);},ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_all( std::function<bool(int i, int j, const const_iterator& it)> func ) const { interruptible_const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_const_serial_op( std::function<bool(int i, int j, const const_iterator& it)> func, bool type=ALL ) const {
		if( type == ACTIVES ) {
			m_core->const_serial_actives([&](int i, int j, const void *value_ptr, const bool &filled ){
				bool active(true);
				const_iterator it(active);
				return func(i,j,it);
			});
		} else {
			m_core->const_serial_all([&](int i, int j, const void *value_ptr, const bool &active, const bool &filled ){
				const_iterator it(active);
				return func(i,j,it);
			});
		}
	}
	/**
	 \~english @brief Dilate cells.
	 @param[in] func Function that specifies what value to assign on dilated cells.
	 @param[in] count Number of dilation count.
	 \~japanese @brief 拡張する。
	 @param[in] func 拡張されたセルにどのような値を与えるか指定する関数。
	 @param[in] count 拡張の回数。
	 */
	void dilate( std::function<void(int i, int j, iterator& it, int thread_index)> func, int count=1 ) {
		while( count -- ) {
			m_core->dilate([&](int i, int j, void *value_ptr, bool &active, const bool &filled, int thread_index) {
				iterator it(active);
				func(i,j,it,thread_index);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Dilate cells.
	 @param[in] func Function that specifies what value to assign on dilated cells.
	 @param[in] count Number of dilation count.
	 \~japanese @brief 拡張する。
	 @param[in] func 拡張されたセルにどのような値を与えるか指定する関数。
	 @param[in] count 拡張の回数。
	 */
	void dilate( std::function<void(int i, int j, iterator& it)> func, int count=1 ) {
		dilate([&](int i, int j, iterator& it, int thread_index) {
			func(i,j,it);
		},count);
	}
	/**
	 \~english @brief Dilate cells.
	 @param[in] count Number of dilation count.
	 \~japanese @brief 拡張する。
	 @param[in] count 拡張の回数。
	 */
	void dilate( int count=1 ) {
		dilate([&](int i, int j, iterator& it){ it.set(); },count);
	}
	/**
	 \~english @brief Erode cells.
	 @param[in] func Function that specifies whether to inactivate the cell.
	 @param[in] count Number of erode count.
	 \~japanese @brief 縮小する。
	 @param[in] func 拡張されたセルを非アクティブにするか指定する関数。
	 @param[in] count 縮小の回数。
	 */
	void erode( std::function<bool(int i, int j, int thread_index)> func, int count=1 ) {
		//
		std::vector<std::vector<vec2i> > off_positions(get_thread_num());
		//
		while( count -- ) {
			const_parallel_actives([&](int i, int j, int tn) {
				bool exit_loop (false);
				for( int dim : DIMS2 ) {
					for( int dir=-1; dir<=1; dir+=2 ) {
						const vec2i &pi = vec2i(i,j) + dir*vec2i(dim==0,dim==1);
						if( ! this->shape().out_of_bounds(pi) && ! (*this)(pi)) {
							if( func(i,j,tn)) {
								off_positions[tn].push_back(vec2i(i,j));
								exit_loop = true;
								break;
							}
						}
					}
					if( exit_loop ) break;
				}
			});
			for( const auto &bucket : off_positions ) for( const auto &pi : bucket ) {
				set_off(pi);
			}
		}
	}
	/**
	 \~english @brief Erode cells.
	 @param[in] func Function that specifies whether to inactivate the cell.
	 @param[in] count Number of erode count.
	 \~japanese @brief 縮小する。
	 @param[in] func 拡張されたセルを非アクティブにするか指定する関数。
	 @param[in] count 縮小の回数。
	 */
	void erode( std::function<bool(int i, int j)> func, int count=1 ) {
		erode([&](int i, int j, int thread_index) {
			return func(i,j);
		},count);
	}
	/**
	 \~english @brief Erode cells.
	 @param[in] count Number of erode count.
	 \~japanese @brief 縮小する。
	 @param[in] count 縮小の回数。
	 */
	void erode( int count=1 ) {
		erode([&](int i, int j, int thread_index) { return true; },count);
	}
	/**
	 \~english @brief Swap array.
	 @param[in] rhs Array to swap.
	 \~japanese @brief グリッドを交換する。
	 @param[in] rhs 交換するグリッド。
	 */
	void swap( bitarray2& rhs ) {
		m_core.swap(rhs.m_core);
		std::swap(m_shape,rhs.m_shape);
	}
	/**
	 \~english @brief Get the instance of `parallel_driver` of this grid.
	 @return Instance of `parallel_driver` of this grid.
	 \~japanese @brief このグリッドの `parallel_driver` のインスタンスを取得する。
	 @return `parallel_driver` のインスタンス。
	 */
	parallel_driver & get_parallel_driver() {
		return m_parallel;
	}
	/**
	 \~english @brief Get the const instance of `parallel_driver` of this grid.
	 @return Const instance of `parallel_driver` of this grid.
	 \~japanese @brief このグリッドの const  な `parallel_driver` のインスタンスを取得する。
	 @return Const な `parallel_driver` のインスタンス。
	 */
	const parallel_driver & get_parallel_driver() const {
		return m_parallel;
	}
	/**
	 \~english @brief Set the core name of module of this grid.
	 @param[in] Name of the core name.
	 \~japanese @brief グリッドのモジュールのコアネームを取得する。
	 @param[in] コアネームの名前。
	 */
	void set_core_name( std::string core_name ) {
		m_core_name = core_name;
	}
	/**
	 \~english @brief Get the core name of module of this grid.
	 @return Name of the core name.
	 \~japanese @brief グリッドのモジュールのコアネームを取得する。
	 @return コアネームの名前。
	 */
	std::string get_core_name() const {
		return m_core_name;
	}
	/**
	 \~english @brief Get pointer to the core module.
	 @return Pointer to the core module.
	 \~japanese @brief コアモジュールのポインターを取得する。
	 @return コアモジュールへのポインタ。
	 */
	const array_core2 * get_core() const {
		return m_core.get();
	}
	/**
	 \~english @brief Get pointer to the core module.
	 @return Pointer to the core module.
	 \~japanese @brief コアモジュールのポインターを取得する。
	 @return コアモジュールへのポインタ。
	 */
	array_core2 * get_core() {
		return m_core.get();
	}
	/// \~english @brief Collection of properties of this grid.
	/// \~japanese @brief このグリッドのプロパティー集。
	struct type2 {
		/**
		 \~english @brief Core name of the module.
		 \~japanese @brief モジュールのコアネーム。
		 */
		std::string core_name;
		/**
		 \~english @brief Shape of the grid.
		 \~japanese @brief 格子の形。
		 */
		shape2 shape;
		/**
		 \~english @brief Check equality.
		 @return \c true if equal \c false otherwise.
		 \~japanese @brief 同値の確認。
		 @return 同じなら \c true を、そうでなければ \c false を返す。
		 */
		bool operator==( const type2 &type ) const {
			return core_name == type.core_name && shape == type.shape;
		}
	};
	/**
	 \~english @brief Get the type of this grid.
	 @return Type of this grid.
	 \~japanese @brief このグリッドの type を取得する。
	 @return このグリッドの type。
	 */
	type2 type() const { return { get_core_name(),shape() }; }
	/**
	 \~english @brief Set the type of this grid.
	 @param[in] type An instance of type to set.
	 \~japanese @brief グリッドの type を設定する。
	 @param[in] type セットする type のインスタンス。
	 */
	void set_type( const type2 &type ) {
		m_core_name = type.core_name;
		m_shape = type.shape;
	}
	//
private:
	//
	shape2 m_shape;
	parallel_driver m_parallel{this};
	bool m_is_initialized {false};
	array2_ptr m_core;
	std::string m_core_name;
};
//
SHKZ_END_NAMESPACE
//
#endif
