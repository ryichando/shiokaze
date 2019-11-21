/*
**	bitarray3.h
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
#ifndef SHKZ_BITARRAY3_H
#define SHKZ_BITARRAY3_H
//
#include <shiokaze/math/vec.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <utility>
#include <shiokaze/math/shape.h>
#include "array_core3.h"
//
SHKZ_BEGIN_NAMESPACE
//
template <class T> class array3;
/** @file */
/// \~english @brief Three dimensional bit grid class designed to be defined as instance member in recursive_configurable class.
/// \~japanese @brief recursive_configurable インスタンスのメンバーインスタンスとして定義可能な3次元ビット配列クラス。
class bitarray3 : public recursive_configurable, public messageable {
public:
	/**
	 \~english @brief Constructor for bitarray3.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] shape Shape of the grid
	 @param[in] core_name Core module name. Default value is "tiledarray3".
	 \~japanese @brief bitarray3 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] shape グリッドの形
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray3"。
	 */
	bitarray3( recursive_configurable *parent, const shape3 &shape, std::string core_name="" ) :
		m_core_name(core_name), m_shape(shape) {
			if( parent ) parent->add_child(this);
			else setup_now();
		}
	/**
	 \~english @brief Constructor for bitarray3.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] core_name Core module name. Default value is "tiledarray3".
	 \~japanese @brief bitarray3 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray3"。
	 */
	bitarray3( recursive_configurable *parent, std::string core_name="" ) : bitarray3(parent,shape3(0,0,0),core_name) {}
	/**
	 \~english @brief Constructor for bitarray3.
	 @param[in] core_name Core module name. Default value is "tiledarray3".
	 \~japanese @brief bitarray3 のコンストラクタ。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray3"。
	 */
	bitarray3( std::string core_name="" ) : bitarray3(nullptr,shape3(0,0,0),core_name) {}
	/**
	 \~english @brief Constructor for bitarray3.
	 @param[in] shape Shape of the grid
	 @param[in] core_name Core module name. Default value is "tiledarray3".
	 \~japanese @brief bitarray3 のコンストラクタ。
	 @param[in] shape グリッドの形
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray3"。
	 */
	bitarray3( const shape3 &shape, std::string core_name="" ) : bitarray3(nullptr,shape,core_name) {}
	//
private:
	//
	virtual void load( configuration &config ) override {
		if( m_core_name.empty()) {
			m_core_name = shkz_default_array_core3;
		} else {
			auto pos = m_core_name.find('*');
			if( pos != std::string::npos ) {
				m_core_name.erase(pos,1);
				m_core_name.insert(pos,shkz_default_array_core3);
			}
		}
		m_core = array_core3::quick_load_module(config,m_core_name);
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
	 \~english @brief Copy constructor for bitarray3.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitarray3 のコピーコンストラクタ。
	 @param[in] コピーする bitarray3 のインスタンスへの参照。
	 */
	bitarray3( const bitarray3 &array ) {
		m_core_name = array.m_core_name;
		setup_now();
		copy(array);
	}
	/**
	 \~english @brief Deep copy operation for bitarray3.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitarray3 のディープコピー演算子。
	 @param[in] コピーする bitarray3 のインスタンスへの参照。
	 */
	bitarray3& operator=(const bitarray3 &array) {
		if( this != &array ) {
			copy(array);
		}
		return *this;
	}
	/**
	 \~english @brief Deep copy function for bitarray3.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitarray3 のディープコピー関数。
	 @param[in] コピーする bitarray3 のインスタンスへの参照。
	 */
	void copy( const bitarray3 &array ) {
		if( this != &array ) {
			set_type(array.type());
			assert(m_core);
			if( array.m_core ) {
				m_core->copy(*array.get_core(),[&](void *target, const void *src){},m_parallel);
			}
		}
	}
	virtual ~bitarray3() {
		clear();
	}
	/**
	 \~english @brief Get the shape of the array.
	 @return Shape of the array.
	 \~japanese @brief グリッドの形を返す。
	 @return グリッドの形。
	 */
	shape3 shape() const { return m_shape; }
	/**
	 \~english @brief Allocate grid memory with value.
	 @param[in] shape Shape or the grid.
	 \~japanese @brief グリッドを値でメモリに展開する。
	 @param[in] shape グリッドの形。
	 */
	void initialize( const shape3 &shape ) {
		clear();
		m_core->initialize(shape.w,shape.h,shape.d,0);
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
	std::vector<vec3i> actives() const {
		std::vector<vec3i> result;
		const_serial_actives([&](int i, int j, int k) {
			result.push_back(vec3i(i,j,k));
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
	void activate( const std::vector<vec3i> &active_entries, const vec3i &offset=vec3i() ) {
		for( const auto &e : active_entries ) {
			const vec3i &pi = e + offset;
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
	void activate_as( const bitarray3 &array, const vec3i &offset=vec3i() ) {
		array.const_serial_actives([&](int i, int j, int k) {
			const vec3i &pi = vec3i(i,j,k) + offset;
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
	template <class Y> void activate_as( const array3<Y> &array, const vec3i &offset=vec3i() ) {
		array.const_serial_actives([&](int i, int j, int k, const auto &it ) {
			const vec3i &pi = vec3i(i,j,k) + offset;
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
	void copy_active_as( const bitarray3 &array, const vec3i &offset=vec3i() ) {
		parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			const vec3i &pi = vec3i(i,j,k) + offset;
			if( ! this->shape().out_of_bounds(pi) ) {
				if( (*this)(pi) && ! array(pi)) {
					it.set_off();
				}
			}
		});
		activate_as(array,offset);
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
	 \~english @brief Set value on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 \~japanese @brief グリッドの値を設定する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 */
	void set( int i, int j, int k ) {
		m_core->set(i,j,k,[&](void *value_ptr, bool &active){
			active = true;
		});
	}
	/**
	 \~english @brief Set value on grid.
	 @param[in] pi position on grid
	 \~japanese @brief グリッドの値を設定する。
	 @param[in] pi グリッドの位置。
	 */
	void set( const vec3i &pi ) {
		set(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Get if a position on grid is active.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 @return \c true if active \c flase if inactive.
	 \~japanese @brief グリッドのある位置がアクティブか得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @return アクティブなら \c true 非アクティブなら \c false。
	 */
	bool operator()( int i, int j, int k ) const {
		bool filled (false);
		return (*m_core)(i,j,k,filled) != nullptr;
	}
	/**
	 \~english @brief Get if a position on grid is active.
	 @param[in] pi position on grid.
	 @return \c true if active \c flase if inactive.
	 \~japanese @brief グリッドのある位置がアクティブか得る。
	 @param[in] pi x グリッドでの位置。
	 @return アクティブなら \c true 非アクティブなら \c false。
	 */
	bool operator()( const vec3i &pi ) const {
		return (*this)(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Set a position on grid inactive.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 \~japanese @brief グリッドの指定された位置を非アクティブにする。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 */
	void set_off( int i, int j, int k ) {
		m_core->set(i,j,k,[&](void *value_ptr, bool &active){
			active = false;
		});
	}
	/**
	 \~english @brief Set a position on grid inactive.
	 @param[in] pi position on grid
	 \~japanese @brief グリッドの指定された位置を非アクティブにする。
	 @param[in] pi グリッドでの位置。
	 */
	void set_off( const vec3i &pi ) {
		set_off(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Return if the grid is different from an input array.
	 @param[in] array Target array to compare.
	 @return \c true if the array is different from the input array and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと違うかどうか返す。
	 @param[in] array 目標とする比べるグリッド。
	 @return もしグリッドが入力と違うグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator!=( const bitarray3 &array ) const {
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
	bool operator==(const bitarray3 &v) const {
		if( v.type() == type() ) {
			bool differnt (false);
			interruptible_const_serial_actives([&]( int i, int j, int k) {
				if( ! v(i,j,k)) {
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
	friend class bitarray3;
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
		iterator( bool &_active ) : m_active(_active) {}
		bool &m_active;
	};
	/// \~english @brief Read-only iterator.
	/// \~japanese @brief 読み込みのみ可能なイテレーター。
	class const_iterator {
	friend class bitarray3;
	public:
		/**
		 \~english @brief Get if a cell is active.
		 \~japanese @brief セルがアクティブが取得する。
		 */
		bool operator()() const {
			return m_active;
		}
	private:
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
		parallel_op([func](int i, int j, int k, iterator& it, int thread_index){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_actives( std::function<void(int i, int j, int k, iterator& it)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_all( std::function<void(int i, int j, int k, iterator& it)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void parallel_op( std::function<void(int i, int j, int k, iterator& it)> func, bool type=ALL ) {
		parallel_op([func](int i, int j, int k, iterator& it, int thread_index){
			func(i,j,k,it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_actives( std::function<void(int i, int j, int k, iterator& it, int thread_index)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void parallel_all( std::function<void(int i, int j, int k, iterator& it, int thread_index)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void parallel_op( std::function<void(int i, int j, int k, iterator& it, int thread_index)> func, bool type=ALL ) {
		if( type == ACTIVES ) {
			m_core->parallel_actives([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_n ){
				iterator it(active);
				func(i,j,k,it,thread_n);
			},m_parallel);
		} else {
			m_core->parallel_all([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_n ){
				iterator it(active);
				func(i,j,k,it,thread_n);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_all( std::function<void(const const_iterator& it)> func) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_parallel_op( std::function<void(const const_iterator& it)> func, bool type=ALL ) const {
		const_parallel_op([func](int i, int j, int k, const const_iterator& it, int thread_index){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(int i, int j, int k)> func ) const {
		const_parallel_op([&](int i, int j, int k, const const_iterator& it) { func(i,j,k); }, ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_all( std::function<void(int i, int j, int k, const const_iterator& it)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_parallel_op( std::function<void(int i, int j, int k, const const_iterator& it)> func, bool type=ALL ) const {
		const_parallel_op([func](int i, int j, int k, const const_iterator& it, int thread_index){
			func(i,j,k,it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(int i, int j, int k, int thread_index)> func ) const {
		const_parallel_op([&](int i, int j, int k, const const_iterator& it, int thread_index){ return func(i,j,k,thread_index); }, ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_all( std::function<void(int i, int j, int k, const const_iterator& it, int thread_index)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_parallel_op( std::function<void(int i, int j, int k, const const_iterator& it, int thread_index)> func, bool type=ALL ) const {
		if( type == ACTIVES ) {
			m_core->const_parallel_actives([&](int i, int j, int k, const void *value_ptr, const bool &filled, int thread_n ){
				bool active(true);
				const_iterator it(active);
				func(i,j,k,it,thread_n);
			},m_parallel);
		} else {
			m_core->const_parallel_all([&](int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_n ){
				const_iterator it(active);
				func(i,j,k,it,thread_n);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_actives( std::function<void(iterator& it)> func) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_all( std::function<void(iterator& it)> func) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void serial_op( std::function<void(iterator& it)> func, bool type=ALL ) {
		serial_op([func](int i, int j, int k, iterator& it){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_actives( std::function<void(int i, int j, int k, iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void serial_all( std::function<void(int i, int j, int k, iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void serial_op( std::function<void(int i, int j, int k, iterator& it)> func, bool type=ALL ) {
		if( type == ACTIVES ) {
			m_core->serial_actives([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				func(i,j,k,it);
				return false;
			});
		} else {
			m_core->serial_all([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				func(i,j,k,it);
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
		const_serial_op([func](int i, int j, int k, const const_iterator& it){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_actives( std::function<void(int i, int j, int k)> func ) const {
		const_serial_op([&](int i, int j, int k, const const_iterator& it) { func(i,j,k); }, ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_all( std::function<void(int i, int j, int k, const const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void const_serial_op( std::function<void(int i, int j, int k, const const_iterator& it)> func, bool type=ALL ) const {
		if( type == ACTIVES ) {
			m_core->const_serial_actives([&](int i, int j, int k, const void *value_ptr, const bool &filled ){
				bool active(true);
				const_iterator it(active);
				func(i,j,k,it);
				return false;
			});
		} else {
			m_core->const_serial_all([&](int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled ){
				const_iterator it(active);
				func(i,j,k,it);
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
		interruptible_serial_op([func](int i, int j, int k, iterator& it){
			return func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルをシリアルに処理する。\c true を返すと、ループを中断する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void interruptible_serial_actives( std::function<bool(int i, int j, int k, iterator& it)> func ) { interruptible_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルをシリアルに処理する。\c true を返すと、ループを中断する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void interruptible_serial_all( std::function<bool(int i, int j, int k, iterator& it)> func ) { interruptible_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_serial_op( std::function<bool(int i, int j, int k, iterator& it)> func, bool type=ALL ) {
		if( type == ACTIVES ) {
			m_core->serial_actives([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				return func(i,j,k,it);
			});
		} else {
			m_core->serial_all([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled ){
				iterator it(active);
				return func(i,j,k,it);
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
		const_serial_op([func](int i, int j, int k, const const_iterator& it){
			return func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_actives( std::function<bool(int i, int j, int k)> func ) const {
		interruptible_const_serial_op([&](int i, int j, int k, const const_iterator& it){ return func(i,j,k); }, ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_all( std::function<bool(int i, int j, int k, const const_iterator& it)> func ) const { interruptible_const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_const_serial_op( std::function<bool(int i, int j, int k, const const_iterator& it)> func, bool type=ALL ) const {
		if( type == ACTIVES ) {
			m_core->const_serial_actives([&](int i, int j, int k, const void *value_ptr, const bool &filled ){
				bool active(true);
				const_iterator it(active);
				return func(i,j,k,it);
			});
		} else {
			m_core->const_serial_all([&](int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled ){
				const_iterator it(active);
				return func(i,j,k,it);
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
	void dilate( std::function<void(int i, int j, int k, iterator& it, int thread_index )> func, int count=1 ) {
		for( int n=0; n<count; ++n ) {
			m_core->dilate([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_index) {
				iterator it(active);
				func(i,j,k,it,thread_index);
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
	void dilate( std::function<void(int i, int j, int k, iterator& it)> func, int count=1 ) {
		dilate([&](int i, int j, int k, iterator& it, int thread_index) {
			func(i,j,k,it);
		});
	}
	/**
	 \~english @brief Dilate cells.
	 @param[in] count Number of dilation count.
	 \~japanese @brief 拡張する。
	 @param[in] count 拡張の回数。
	 */
	void dilate( int count=1 ) {
		for( int n=0; n<count; ++n ) {
			dilate([&](int i, int j, int k, iterator& it){ it.set(); });
		}
	}
	/**
	 \~english @brief Swap array.
	 @param[in] rhs Array to swap.
	 \~japanese @brief グリッドを交換する。
	 @param[in] rhs 交換するグリッド。
	 */
	void swap( bitarray3& rhs ) {
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
	const array_core3 * get_core() const {
		return m_core.get();
	}
	/**
	 \~english @brief Get pointer to the core module.
	 @return Pointer to the core module.
	 \~japanese @brief コアモジュールのポインターを取得する。
	 @return コアモジュールへのポインタ。
	 */
	array_core3 * get_core() {
		return m_core.get();
	}
	/// \~english @brief Collection of properties of this grid.
	/// \~japanese @brief このグリッドのプロパティー集。
	struct type3 {
		/**
		 \~english @brief Core name of the module.
		 \~japanese @brief モジュールのコアネーム。
		 */
		std::string core_name;
		/**
		 \~english @brief Shape of the grid.
		 \~japanese @brief 格子の形。
		 */
		shape3 shape;
		/**
		 \~english @brief Check equality.
		 @return \c true if equal \c false otherwise.
		 \~japanese @brief 同値の確認。
		 @return 同じなら \c true を、そうでなければ \c false を返す。
		 */
		bool operator==( const type3 &type ) const {
			return core_name == type.core_name && shape == type.shape;
		}
	};
	/**
	 \~english @brief Get the type of this grid.
	 @return Type of this grid.
	 \~japanese @brief このグリッドの type を取得する。
	 @return このグリッドの type。
	 */
	type3 type() const { return { get_core_name(),shape() }; }
	/**
	 \~english @brief Set the type of this grid.
	 @param[in] type An instance of type to set.
	 \~japanese @brief グリッドの type を設定する。
	 @param[in] type セットする type のインスタンス。
	 */
	void set_type( const type3 &type ) {
		m_core_name = type.core_name;
		m_shape = type.shape;
	}
	//
private:
	//
	shape3 m_shape;
	parallel_driver m_parallel{this};
	array3_ptr m_core;
	bool m_is_initialized {false};
	std::string m_core_name;
};
//
SHKZ_END_NAMESPACE
//
#endif
