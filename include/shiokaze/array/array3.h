/*
**	array3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 7, 2017.
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
#ifndef SHKZ_ARRAY3_H
#define SHKZ_ARRAY3_H
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
/** @file */
/// \~english @brief Three dimensional array class designed to be defined as instance member in recursive_configurable class.
/// \~japanese @brief recursive_configurable インスタンスのメンバーインスタンスとして定義可能な3次元配列クラス。
template<class T> class array3 : public recursive_configurable, public messageable {
public:
	/**
	 \~english @brief Constructor for array3.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] shape Shape of the grid
	 @param[in] value Background value (initial value).
	 @param[in] core_name Core module name. Default value is "tiledarray_core3".
	 \~japanese @brief array3 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] shape グリッドの形
	 @param[in] value バックグラウンド値 (初期値)。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray_core3"。
	 */
	array3( recursive_configurable *parent, const shape3 &shape, T value=T(), std::string core_name="" ) :
		m_core_name(core_name), m_shape(shape), m_background_value(value) {
			if( parent ) parent->add_child(this);
			else setup_now();
		}
	/**
	 \~english @brief Constructor for array3.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] core_name Core module name. Default value is "tiledarray_core3".
	 \~japanese @brief array3 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray_core3"。
	 */
	array3( recursive_configurable *parent, std::string core_name="" ) : array3(parent,shape3(0,0,0),T(),core_name) {}
	/**
	 \~english @brief Constructor for array3.
	 @param[in] core_name Core module name. Default value is "tiledarray_core3".
	 \~japanese @brief array3 のコンストラクタ。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray_core3"。
	 */
	array3( std::string core_name="" ) : array3(nullptr,shape3(0,0,0),T(),core_name) {}
	/**
	 \~english @brief Constructor for array3.
	 @param[in] shape Shape of the grid
	 @param[in] value Background value (initial value).
	 @param[in] core_name Core module name. Default value is "tiledarray_core3".
	 \~japanese @brief array3 のコンストラクタ。
	 @param[in] shape グリッドの形
	 @param[in] value バックグラウンド値 (初期値)。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "tiledarray_core3"。
	 */
	array3( const shape3 &shape, T value=T(), std::string core_name="" ) : array3(nullptr,shape,value,core_name) {}
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
			initialize(m_shape,m_background_value);
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
	 \~english @brief Copy constructor for array3.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief array3 のコピーコンストラクタ。
	 @param[in] コピーする array3 のインスタンスへの参照。
	 */
	array3( const array3 &array ) {
		m_core_name = array.m_core_name;
		setup_now();
		copy(array);
	}
	/**
	 \~english @brief Set whether to force grid manipulation only on active cells.
	 If true, operatios such `operator+=()` only acts on active cells.
	 @param[in] touch_only_actives Whether to turn this on or off.
	 \~japanese @brief グリッドの操作を、アクティブセルだけに限定するか設定する関数。もし設定されたなら、
	 例えば `operator+=()`はアクティブセルだけに作用する。
	 @param[in] touch_only_actives 操作をアクティブセルに限定するか
	 */
	void set_touch_only_actives( bool touch_only_actives ) {
		m_touch_only_actives = touch_only_actives;
	}
	/**
	 \~english @brief Deep copy operation for array3.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief array3 のディープコピー演算子。
	 @param[in] コピーする array3 のインスタンスへの参照。
	 */
	array3& operator=(const array3 &array) {
		if( this != &array ) {
			copy(array);
		}
		return *this;
	}
	/**
	 \~english @brief Deep copy function for array3.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief array3 のディープコピー関数。
	 @param[in] コピーする array3 のインスタンスへの参照。
	 */
	void copy( const array3 &array ) {
		if( this != &array ) {
			set_type(array.type());
			assert(m_core);
			if( array.m_core ) {
				m_core->copy(*array.get_core(),[&](void *target, const void *src) {
					new (target) T(*static_cast<const T *>(src));
				},m_parallel);
			}
		}
	}
	/**
	 \~english @brief Flatten this grid to a fully linearized one dimendional array, of which can be accessed like `array[i+j*w+k*(w*h)]`.
	 @return Flattened linearized array.
	 \~japanese @brief グリッドを線形化された一次元配列へ変換する関数。`array[i+j*w+k*(w*h)]`のようにアクセス可能。
	 @return 変換された一次元配列。
	 */
	std::vector<T> linearize() const {
		const shape3 &s = m_shape;
		std::vector<T> result(s.count(),m_background_value);
		const_parallel_actives([&]( int i, int j, int k, auto &it ) {
			result[i + s.w * (j + s.h * k)] = it();
		});
		const_parallel_inside([&]( int i, int j, int k, auto &it ) {
			if( ! it.active()) result[i + s.w * (j + s.h * k)] = it();
		});
		return result;
	}
	virtual ~array3() {
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
	 @param[in] value Initial value
	 \~japanese @brief グリッドを値でメモリに展開する。
	 @param[in] shape グリッドの形。
	 @param[in] value 初期値。
	 */
	void initialize( const shape3 &shape, T value=T()) {
		clear();
		m_core->initialize(shape.w,shape.h,shape.d,sizeof(T));
		m_shape = shape;
		m_background_value = value;
		m_fillable = false;
		m_levelset = false;
		m_is_initialized = true;
	}
	/**
	 \~english @brief Set the grid as level set.
	 @param[in] bandwidth_half half bandwidth size of the level set.
	 \~japanese @brief グリッドをレベルセットとして定義する。
	 @param[in] bandwidth_half レベルセットの半分のバンド幅。
	 */
	void set_as_levelset( double bandwidth_half ) {
		m_levelset = true;
		m_fillable = false;
		m_background_value = bandwidth_half;
		m_fill_value = -bandwidth_half;
	}
	/**
	 \~english @brief Set the grid as a grid that is fillable by flood fill.
	 @param[in] fill_value fill_value
	 \~japanese @brief グリッドを Flood Fill で塗りつぶし可能にする。
	 @param[in] fill_value 塗りつぶしの値。
	 */
	void set_as_fillable( const T& fill_value ) {
		m_levelset = false;
		m_fillable = true;
		m_fill_value = fill_value;
	}
	/**
	 \~english @brief Set the grid as fillable as same as an input array.
	 @param[in] array Input array to mimic the fillable properties.
	 \~japanese @brief グリッドを入力のグリッドと同じような塗りつぶし可能に設定する。
	 @param[in] array 目標となる塗りつぶしに関する情報をコピーするグリッド。
	 */
	void set_as_fillable_as( const array3 &array ) {
		set_as_fillable(array.m_fill_value);
	}
	/**
	 \~english @brief Set the grid as level set as same as an input array.
	 @param[in] array Input array to mimic the level set properties.
	 \~japanese @brief グリッドを入力のグリッドと同じようなレベルセットグリッドに設定する。
	 @param[in] array 目標となるレベルセットに関する情報をコピーするグリッド。
	 */
	void set_as_levelset_as( const array3 &array ) {
		set_as_levelset(array.m_background_value);
	}
	/**
	 \~english @brief Return if the grid is set fillable.
	 @return Whether the grid is set fillable.
	 \~japanese @brief グリッドが塗りつぶし可能に設定されているか取得する。
	 @return グリッドが塗りつぶし可能に設定されているか。
	 */
	bool is_fillable() const {
		return m_fillable;
	}
	/**
	 \~english @brief Return if the grid is set level set.
	 @return Whether the grid is set level set.
	 \~japanese @brief グリッドがレベルセットに設定されているか取得する関数。
	 @return グリッドがレベルセットに設定されているか。
	 */
	bool is_levelset() const {
		return m_levelset;
	}
	/**
	 \~english @brief Perform flood fill. Grid should be set either level set of fillable beforehand.
	 \~japanese @brief 塗りつぶし処理を行う。グリッドは事前にレベルセットかぶり潰し可能に設定されている必要がある。
	 */
	void flood_fill() {
		if( m_fillable ) {
			m_core->flood_fill([&](const void *value_ptr) {
				return *static_cast<const T *>(value_ptr) == m_fill_value;
			},m_parallel);
		} else if( m_levelset ) {
			m_core->flood_fill([&](const void *value_ptr) {
				return *static_cast<const T *>(value_ptr) < 0.0;
			},m_parallel);
		} else {
			printf( "Flood fill attempted without being set either levelset or fillable.\n");
			exit(0);
		}
	}
	/**
	 \~english @brief Function to return the list of filled cells.
	 @return The list of filled cells.
	 \~japanese @brief 塗りつぶされたセルのリストを取得する関数。
	 @return 塗りつぶされたセルのリスト。
	 */
	std::vector<vec3i> fills() const {
		std::vector<vec3i> result;
		const_serial_inside([&](int i, int j, int k, const auto &it) {
			result.push_back(vec3i(i,j,k));
		});
		return result;
	}
	/**
	 \~english @brief Function to get if a cell is filled.
	 @param[in] i x coordinate position.
	 @param[in] j y coordinate position.
	 @param[in] k z coordinate position.
	 @return \c true if the cell is filled \c false if not.
	 \~japanese @brief セルが塗りつぶされているか取得する関数。
	 @param[in] i x 座標の位置。
	 @param[in] j y 座標の位置。
	 @param[in] k z 座標の位置。
	 @return 塗りつぶされていれば \c true なければ \c false が返る。
	 */
	bool filled( int i, int j, int k ) const {
		bool filled;
		(*m_core)(i,j,k,filled);
		return filled;
	}
	/**
	 \~english @brief Function to get if a cell is filled.
	 @param[in] pi position.
	 @return \c true if the cell is filled \c false if not.
	 \~japanese @brief セルが塗りつぶされているか取得する関数。
	 @param[in] pi 位置。
	 @return 塗りつぶされていれば \c true なければ \c false が返る。
	 */
	bool filled( const vec3i &pi ) const {
		return filled(pi[0],pi[1],pi[2]);
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
	std::vector<vec3i> actives() const {
		std::vector<vec3i> result;
		const_serial_actives([&](int i, int j, int k, const auto &it) {
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
			if( ! shape().out_of_bounds(pi) && ! active(pi)) {
				set(pi,(*this)(pi));
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
	template <class Y> void activate_as( const array3<Y> &array, const vec3i &offset=vec3i() ) {
		array.const_serial_actives([&](int i, int j, int k, const auto &it) {
			const vec3i &pi = vec3i(i,j,k) + offset;
			if( ! this->shape().out_of_bounds(pi) && ! this->active(pi)) {
				this->set(pi,(*this)(pi));
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
	template <class Y> void activate_as_bit( Y &array, const vec3i &offset=vec3i() ) {
		array.const_serial_actives([&](int i, int j, int k) {
			const vec3i &pi = vec3i(i,j,k) + offset;
			if( ! this->shape().out_of_bounds(pi) && ! this->active(pi)) {
				this->set(pi,(*this)(pi));
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
	template <class Y> void activate_inside_as( const array3<Y> &array, const vec3i &offset=vec3i() ) {
		array.const_serial_inside([&](int i, int j, int k, const auto &it) {
			const vec3i &pi = vec3i(i,j,k) + offset;
			if( ! this->shape().out_of_bounds(pi) && ! this->active(pi)) {
				this->set(pi,(*this)(pi));
			}
		});
	}
	/**
	 \~english @brief Activate all the cells.
	 \~japanese @brief 全てのセルをアクティブにする。
	 */
	void activate_all() {
		parallel_all([&](auto &it) {
			it.set(it());
		});
	}
	/**
	 \~english @brief Activate all the filled cells.
	 \~japanese @brief 塗りつぶされた全てのセルをアクティブにする。
	 */
	void activate_inside() {
		activate(fills());
	}
	/**
	 \~english @brief Copy the states of active and inactive cells as same as input array with an offset.
	 @param[in] array Target input array from which the states to be copied.
	 @param[in] offset Offset
	 \~japanese @brief セルのアクティブと非アクティブステートの状態を入力のグリッドと同じようにセットする。
	 @param[in] array 目標となる状態をコピーする元となるグリッド。
	 @param[in] offset オフセット
	 */
	template <class Y> void copy_active_as( const array3<Y> &array, const vec3i &offset=vec3i() ) {
		parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			const vec3i &pi = vec3i(i,j,k) + offset;
			if( ! this->shape().out_of_bounds(pi) ) {
				if( this->active(pi) && ! array.active(pi)) {
					it.set_off();
				}
			}
		});
		activate_as(array,offset);
	}
	/**
	 \~english @brief Get the background value (alternatively, initial value) of the grid.
	 \~japanese @brief グリッドのバックグランドの値（あるいは初期値）を得る。
	 */
	T get_background_value () const { return m_background_value; }
	/**
	 \~english @brief Set the background value (alternatively, initial value) of the grid.
	 @param[in] value New background value
	 \~japanese @brief グリッドのバックグランドの値（あるいは初期値）を設定する。
	 @param[in] value 新しい初期値
	 */
	void set_background_value( const T& value ) { m_background_value = value; }
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
	 \~english @brief Clear out the grid with the new backgroud value.
	 *
	 Background values regarding level set and the fillable will be dicarded. Size and the memory allocation left intact.
	 \~japanese @brief グリッドを新しい初期値で初期化する。
	 *
	 レベルセットや塗る潰しに関する情報として使われるバックグランド値は破棄される。ただし、グリッドの大きさやメモリ確保は変更されない。
	 */
	void clear(const T &v) {
		m_background_value = v;
		clear();
	}
	/**
	 \~english @brief Set value on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 @param[in] value Value to set at the position.
	 \~japanese @brief グリッドの値を設定する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[in] value この位置で設定する値。
	 */
	void set( int i, int j, int k, const T& value ) {
		m_core->set(i,j,k,[&](void *value_ptr, bool &active){
			if( ! active ) new (value_ptr) T(value);
			else *static_cast<T *>(value_ptr) = value;
			active = true;
		});
	}
	/**
	 \~english @brief Set value on grid.
	 @param[in] pi position on grid
	 @param[in] value Value to set at the position.
	 \~japanese @brief グリッドの値を設定する。
	 @param[in] pi グリッドの位置。
	 @param[in] value この位置で設定する値。
	 */
	void set( const vec3i &pi, const T& value) {
		set(pi[0],pi[1],pi[2],value);
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
	bool active( int i, int j, int k ) const {
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
	bool active( const vec3i &pi ) const {
		return active(pi[0],pi[1],pi[2]);
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
			if( active ) (static_cast<T *>(value_ptr))->~T();
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
	 \~english @brief Increment value on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 @param[in] value Value to increment at the position.
	 \~japanese @brief グリッドの値を加算する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[in] value この位置で加算する値。
	 */
	void increment( int i, int j, int k, const T& value) {
		m_core->set(i,j,k,[&](void *value_ptr, bool &active){
			if( active ) *static_cast<T *>(value_ptr) += value;
			else {
				*static_cast<T *>(value_ptr) = m_background_value + value;
				active = true;
			}
		});
	}
	/**
	 \~english @brief Increment value on grid.
	 @param[in] pi position on grid
	 @param[in] value Value to increment at the position.
	 \~japanese @brief グリッドの値を加算する。
	 @param[in] pi グリッドでの位置。
	 @param[in] value この位置で加算する値。
	 */
	void increment( const vec3i &pi, const T& value ) {
		increment(pi[0],pi[1],pi[2],value);
	}
	/**
	 \~english @brief Subtract value on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 @param[in] value Value to subtract at the position.
	 \~japanese @brief グリッドの値を減算する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[in] value この位置で減算する値。
	 */
	void subtract( int i, int j, int k, const T& value) {
		m_core->set(i,j,k,[&](void *value_ptr, bool &active){
			if( active ) *static_cast<T *>(value_ptr) -= value;
			else {
				*static_cast<T *>(value_ptr) = m_background_value - value;
				active = true;
			}
		});
	}
	/**
	 \~english @brief Subtract value on grid.
	 @param[in] pi position on grid.
	 @param[in] value Value to subtract at the position.
	 \~japanese @brief グリッドの値を減算する。
	 @param[in] pi グリッドでの位置。
	 @param[in] value この位置で減算する値。
	 */
	void subtract( const vec3i &pi, const T& value ) {
		subtract(pi[0],pi[1],pi[2],value);
	}
	/**
	 \~english @brief Multiply value on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 @param[in] value Value to multiply at the position.
	 \~japanese @brief グリッドの値を乗算する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[in] value この位置で乗算する値。
	 */
	void multiply( int i, int j, int k, const T& value ) {
		m_core->set(i,j,k,[&](void *value_ptr, bool &active){
			if( active ) *static_cast<T *>(value_ptr) *= value;
			else {
				*static_cast<T *>(value_ptr) = m_background_value * value;
				active = true;
			}
		});
	}
	/**
	 \~english @brief Multiply value on grid.
	 @param[in] pi position on grid.
	 @param[in] value Value to multiply at the position.
	 \~japanese @brief グリッドの値を乗算する。
	 @param[in] pi グリッドの位置。
	 @param[in] value この位置で乗算する値。
	 */
	void multiply( const vec3i &pi, const T& value ) {
		multiply(pi[0],pi[1],pi[2],value);
	}
	/**
	 \~english @brief Divide by value on grid.
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 @param[in] value Value to divide at the position.
	 \~japanese @brief グリッドの値で割り算する。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 @param[in] value この位置で割り算する値。
	 */
	void devide( int i, int j, int k, const T& value ) {
		multiply(i,j,k,1.0/value);
	}
	/**
	 \~english @brief Divide by value on grid.
	 @param[in] pi position on grid.
	 @param[in] value Value to divide at the position.
	 \~japanese @brief グリッドの値で割り算する。
	 @param[in] pi グリッドの位置。
	 @param[in] value この位置で割り算する値。
	 */
	void devide( const vec3i &pi, const T& value) {
		devide(pi[0],pi[1],pi[2],value);
	}
	/**
	 \~english @brief Get the pointer to the value at a position on grid
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 \~japanese @brief グリッドの指定した位置での値へのポインターを得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 */
	T* ptr(unsigned i, unsigned j, unsigned k ) {
		bool filled (false);
		return const_cast<T *>(static_cast<const T *>((*m_core)(i,j,k,filled)));
	}
	/**
	 \~english @brief Get the const pointer to the value at a position on grid
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 \~japanese @brief グリッドの指定した位置での値への const なポインターを得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 */
	const T* ptr(unsigned i, unsigned j, unsigned k ) const {
		return const_cast<array3<T> *>(this)->ptr(i,j,k);
	}
	/**
	 \~english @brief Get the pointer to the value at a position on grid
	 @param[in] pi position on grid.
	 \~japanese @brief グリッドの指定した位置での値へのポインターを得る。
	 @param[in] pi グリッドの位置。
	 */
	T* ptr( const vec3i &pi ) {
		return ptr(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Get the const pointer to the value at a position on grid
	 @param[in] pi position on grid.
	 \~japanese @brief グリッドの指定した位置での値への const なポインターを得る。
	 @param[in] pi グリッドの位置。
	 */
	const T* ptr( const vec3i &pi ) const {
		return const_cast<array3<T> *>(this)->ptr(pi);
	}
	/**
	 \~english @brief Get the the value at a position on grid
	 @param[in] i position on x coordiante.
	 @param[in] j position on y coordinate.
	 @param[in] k position on z coordinate
	 \~japanese @brief グリッドの指定した位置での値を得る。
	 @param[in] i x 座標上の位置。
	 @param[in] j y 座標上の位置。
	 @param[in] k z 座標上の位置。
	 */
	const T& operator()(int i, int j, int k ) const {
		bool filled (false);
		const T* ptr = static_cast<const T *>((*m_core)(i,j,k,filled));
		if( ptr ) return *ptr;
		else return filled ? m_fill_value : m_background_value;
	}
	/**
	 \~english @brief Get the the value at a position on grid
	 @param[in] pi position on grid.
	 \~japanese @brief グリッドの指定した位置での値を得る。
	 @param[in] pi グリッドの位置。
	 */
	const T& operator()(const vec3i &pi ) const {
		return (*const_cast<array3<T> *>(this))(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Return if the grid is different from an input array.
	 @param[in] array Target array to compare.
	 @return \c true if the array is different from the input array and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと違うかどうか返す。
	 @param[in] array 目標とする比べるグリッド。
	 @return もしグリッドが入力と違うグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator!=( const array3<T> &array ) const {
		return ! (*this == array);
	}
	/**
	 \~english @brief Return if the grid is same to an input array.
	 @param[in] v Target array to compare.
	 @return \c true if the array is the same to the input and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと同じかどうか返す。
	 @param[in] v 目標とする比べるグリッド。
	 @return もしグリッドが入力と同じグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator==(const array3<T> &v) const {
		if( v.type() == type() ) {
			bool differnt (false);
			interruptible_const_serial_actives([&]( int i, int j, int k, const const_iterator& it) {
				if( it() != v(i,j,k)) {
					differnt = true;
					return true;
				} else {
					return false;
				}
			});
			if( ! differnt ) {
				interruptible_const_serial_inside([&]( int i, int j, int k, const const_iterator& it) {
					if( ! it.active()) {
						if( it() != v(i,j,k)) {
							differnt = true;
							return true;
						}
					}
					return false;
				});
			}
			return ! differnt;
		}
		return false;
	}
	/**
	 \~english @brief Set all the grid values with an input value.
	 @param[in] v Input value to set.
	 \~japanese @brief グリッドの全てのセルの値を入力値に設定する。
	 @param[in] v 設定する値。
	 */
	void operator=(const T &v) {
		parallel_op([&](iterator& it) {
			it.set(v);
		},m_touch_only_actives);
	}
	/**
	 \~english @brief Increment all the values with the values of an input array.
	 @param[in] v Input array.
	 \~japanese @brief グリッドの全てのセルの値を入力されたグリッドの値で加算する。
	 @param[in] v 入力のグリッド。
	 */
	void operator+=(const array3<T> &v) {
		assert(shape()==v.shape());
		parallel_op([&](int i ,int j, int k, iterator& it, int tn) {
			if( ! m_touch_only_actives || v.active(i,j,k)) {
				it.increment(v(i,j,k));
			}
		},m_touch_only_actives);
	}
	/**
	 \~english @brief Subtract all the values with the values of an input array.
	 @param[in] v Input array.
	 \~japanese @brief グリッドの全てのセルの値を入力されたグリッドの値で減算する。
	 @param[in] v 入力のグリッド。
	 */
	void operator-=(const array3<T> &v) {
		assert(shape()==v.shape());
		parallel_op([&](int i, int j, int k, iterator& it, int tn) {
			if( ! m_touch_only_actives || v.active(i,j,k)) {
				it.subtract(v(i,j,k));
			}
		},m_touch_only_actives);
	}
	/**
	 \~english @brief Increment all the grid values with an input value.
	 @param[in] v Value to increment.
	 \~japanese @brief グリッドの全てのセルの値を入力値だけ加算する。
	 @param[in] v 加算する値。
	 */
	void operator+=(const T &v) {
		parallel_op([&](iterator& it) {
			it.increment(v);
		},m_touch_only_actives);
	}
	/**
	 \~english @brief Subtract all the grid values with an input value.
	 @param[in] v Value to subtract.
	 \~japanese @brief グリッドの全てのセルの値を入力値だけ減算する。
	 @param[in] v 減算する値。
	 */
	void operator-=(const T &v) {
		parallel_op([&](iterator& it) {
			it.subtract(v);
		},m_touch_only_actives);
	}
	/**
	 \~english @brief Multiply all the grid values with an input value.
	 @param[in] v Value to multiply.
	 \~japanese @brief グリッドの全てのセルの値を入力値で乗算する。
	 @param[in] v 乗算する値。
	 */
	void operator*=(const T &v) {
		parallel_op([&](iterator& it) {
			it.multiply(v);
		},m_touch_only_actives);
	}
	/**
	 \~english @brief Divide all the grid values with an input value.
	 @param[in] v Value to divide.
	 \~japanese @brief グリッドの全てのセルの値を入力値で乗算する。
	 @param[in] v 乗算する値。
	 */
	void operator/=(const T &v) {
		parallel_op([&](iterator& it) {
			it.divide(v);
		},m_touch_only_actives);
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
	friend class array3<T>;
	public:
		/**
		 \~english @brief Set a value.
		 @param[in] value Value to set.
		 \~japanese @brief 値をセットする。
		 @param[in] value セットする値。
		 */
		void set( const T &value ) {
			if( ! m_active ) allocate(value);
			else *static_cast<T *>(m_value_ptr) = value;
			m_active = true;
		}
		/**
		 \~english @brief Inactivate a cell.
		 \~japanese @brief セルを非アクティブにする。
		 */
		void set_off() {
			if( m_active && m_value_ptr ) deallocate();
			m_active = false;
		}
		/**
		 \~english @brief Increment value.
		 @param[in] value Value to increment.
		 \~japanese @brief 値を加算する。
		 @param[in] value 加算する値。
		 */
		void increment( const T& value ) {
			if( m_active ) {
				*static_cast<T *>(m_value_ptr) += value;
			} else {
				allocate(m_background_value + value);
				m_active = true;
			}
		}
		/**
		 \~english @brief Subtract value.
		 @param[in] value Value to subtract.
		 \~japanese @brief 値を減算する。
		 @param[in] value 減算する値。
		 */
		void subtract( const T& value ) {
			if( m_active ) {
				*static_cast<T *>(m_value_ptr) -= value;
			} else {
				allocate(m_background_value - value);
				m_active = true;
			}
		}
		/**
		 \~english @brief Multiply value.
		 @param[in] value Value to multiply.
		 \~japanese @brief 値を乗算する。
		 @param[in] value 乗算する値。
		 */
		void multiply( const T& value ) {
			if( m_active ) {
				*static_cast<T *>(m_value_ptr) *= value;
			} else {
				allocate(m_background_value * value);
				m_active = true;
			}
		}
		/**
		 \~english @brief Divide by value.
		 @param[in] value Value to divide.
		 \~japanese @brief 値で割り算をする。
		 @param[in] value 割り算する値。
		 */
		void divide( const T& value ) {
			multiply(1.0/value);
		}
		/**
		 \~english @brief Get if a cell is active.
		 \~japanese @brief セルがアクティブが取得する。
		 */
		bool active() const {
			return m_active;
		}
		/**
		 \~english @brief Get if a cell is filled.
		 \~japanese @brief セルが塗りつぶされているか取得する。
		 */
		bool filled() const {
			return m_filled;
		}
		/**
		 \~english @brief Get the value.
		 @return Value on the cell.
		 \~japanese @brief 値を得る。
		 @return セルでの値。
		 */
		const T& operator()() const {
			if( m_active ) {
				return *static_cast<const T *>(m_value_ptr);
			} else {
				return m_background_value;
			}
		}
		/**
		 \~english @brief Get pointer to the value.
		 @return Pointer to the value on the cell.
		 \~japanese @brief 値へのポインターを得る。
		 @return セルでの値へのポインター。
		 */
		T* ptr() { return m_active ? static_cast<T *>(m_value_ptr) : nullptr; }
		/**
		 \~english @brief Get const pointer to the value.
		 @return Const pointer to the value on the cell.
		 \~japanese @brief 値への const なポインターを得る。
		 @return セルでの値への const なポインター。
		 */
		const T* ptr() const { return m_active ? static_cast<const T *>(m_value_ptr) : nullptr; }
	private:
		iterator( void *value_ptr, bool &_active, bool _filled, const T& m_background_value ) 
			: m_value_ptr(value_ptr), m_active(_active), m_filled(_filled), m_background_value(m_background_value) {}
		//
		void allocate ( const T& value ) {
			new (m_value_ptr) T(value);
		}
		void deallocate() {
			(static_cast<T *>(m_value_ptr))->~T();
		}
		bool &m_active;
		bool m_filled;
		void *m_value_ptr;
		const T& m_background_value;
	};
	/// \~english @brief Read-only iterator.
	/// \~japanese @brief 読み込みのみ可能なイテレーター。
	class const_iterator {
	friend class array3<T>;
	public:
		/**
		 \~english @brief Get if a cell is active.
		 \~japanese @brief セルがアクティブが取得する。
		 */
		bool active() const {
			return m_active;
		}
		/**
		 \~english @brief Get if a cell is filled.
		 \~japanese @brief セルが塗りつぶされているか取得する。
		 */
		bool filled() const {
			return m_filled;
		}
		/**
		 \~english @brief Get the value.
		 @return Value on the cell.
		 \~japanese @brief 値を得る。
		 @return セルでの値。
		 */
		const T& operator()() const {
			if( m_active ) {
				return *static_cast<const T *>(m_value_ptr);
			} else {
				return m_background_value;
			}
		}
		/**
		 \~english @brief Get const pointer to the value.
		 @return Const pointer to the value on the cell.
		 \~japanese @brief 値への const なポインターを得る。
		 @return セルでの値への const なポインター。
		 */
		const T* ptr() const { return m_active ? static_cast<const T *>(m_value_ptr) : nullptr; }
	private:
		const_iterator( const void *value_ptr, const bool &_active, bool _filled, const T& m_background_value ) 
			: m_value_ptr(value_ptr), m_active(_active), m_filled(_filled), m_background_value(m_background_value) {}
		const bool &m_active;
		bool m_filled;
		const void *m_value_ptr;
		const T& m_background_value;
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
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it,thread_n);
			},m_parallel);
		} else {
			m_core->parallel_all([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled, int thread_n ){
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it,thread_n);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(const const_iterator& it)> func) const { const_parallel_op(func,ACTIVES); }
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
	 \~english @brief Loop over all the filled cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 塗りつぶされたセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_inside( std::function<void(const const_iterator& it)> func) const {
		const_parallel_inside([func](int i, int j, int k, const const_iterator& it, int thread_index){
			func(it);
		});
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(int i, int j, int k, const const_iterator& it)> func ) const { const_parallel_op(func,ACTIVES); }
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
	 \~english @brief Loop over all the filled cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 塗りつぶされたセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_inside( std::function<void(int i, int j, int k, const const_iterator& it)> func) const {
		const_parallel_inside([func](int i, int j, int k, const const_iterator& it, int thread_index){
			func(i,j,k,it);
		});
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_actives( std::function<void(int i, int j, int k, const const_iterator& it, int thread_index)> func ) const { const_parallel_op(func,ACTIVES); }
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
				const_iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it,thread_n);
			},m_parallel);
		} else {
			m_core->const_parallel_all([&](int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled, int thread_n ){
				const_iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it,thread_n);
			},m_parallel);
		}
	}
	/**
	 \~english @brief Loop over all the filled cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 塗りつぶされたセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_parallel_inside( std::function<void(int i, int j, int k, const const_iterator& it, int thread_index)> func ) const {
		m_core->const_parallel_inside([&](int i, int j, int k, const void *value_ptr, const bool &active, int thread_n ){
			const_iterator it(value_ptr,active,true,m_fill_value);
			func(i,j,k,it,thread_n);
		},m_parallel);
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
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it);
				return false;
			});
		} else {
			m_core->serial_all([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled ){
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it);
				return false;
			});
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_actives( std::function<void(const const_iterator& it)> func ) const { const_serial_op(func,ACTIVES); }
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
	 \~english @brief Loop over filled the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 塗りつぶされたセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_inside( std::function<void(const const_iterator& it)> func ) const {
		const_serial_inside([func](int i, int j, int k, const const_iterator& it){
			func(it);
		});
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_actives( std::function<void(int i, int j, int k, const const_iterator& it)> func ) const { const_serial_op(func,ACTIVES); }
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
				const_iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it);
				return false;
			});
		} else {
			m_core->const_serial_all([&](int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled ){
				const_iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				func(i,j,k,it);
				return false;
			});
		}
	}
	/**
	 \~english @brief Loop over filled the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 塗りつぶされたセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	void const_serial_inside( std::function<void(int i, int j, int k, const const_iterator& it)> func ) const {
		m_core->const_serial_inside([&](int i, int j, int k, const void *value_ptr, const bool &active ){
			const_iterator it(value_ptr,active,true,m_fill_value);
			func(i,j,k,it);
			return false;
		});
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
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				return func(i,j,k,it);
			});
		} else {
			m_core->serial_all([&](int i, int j, int k, void *value_ptr, bool &active, const bool &filled ){
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				return func(i,j,k,it);
			});
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_actives( std::function<bool(const const_iterator& it)> func ) const { interruptible_const_serial_op(func,ACTIVES); }
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
	 \~english @brief Loop over all the filled cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief 塗りつぶされた全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	void interruptible_const_serial_inside( std::function<bool(const const_iterator& it)> func, bool type=ALL ) const {
		interruptible_const_serial_inside([func](int i, int j, int k, const const_iterator& it){
			return func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_actives( std::function<bool(int i, int j, int k, const const_iterator& it)> func ) const { interruptible_const_serial_op(func,ACTIVES); }
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
				const_iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				return func(i,j,k,it);
			});
		} else {
			m_core->const_serial_all([&](int i, int j, int k, const void *value_ptr, const bool &active, const bool &filled ){
				const_iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
				return func(i,j,k,it);
			});
		}
	}
	/**
	 \~english @brief Loop over all the filled cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 塗りつぶされた全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	void interruptible_const_serial_inside( std::function<bool(int i, int j, int k, const const_iterator& it)> func ) const {
		m_core->const_serial_inside([&](int i, int j, int k, const void *value_ptr, const bool &active ){
			const_iterator it(value_ptr,active,true,m_fill_value);
			return func(i,j,k,it);
		});
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
				iterator it(value_ptr,active,filled,filled ? m_fill_value : m_background_value);
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
			dilate([&](int i, int j, int k, iterator& it){ it.set(it()); });
		}
	}
	/**
	 \~english @brief Swap array.
	 @param[in] rhs Array to swap.
	 \~japanese @brief グリッドを交換する。
	 @param[in] rhs 交換するグリッド。
	 */
	void swap( array3& rhs ) {
		m_core.swap(rhs.m_core);
		std::swap(m_shape,rhs.m_shape);
		std::swap(m_background_value,rhs.m_background_value);
		std::swap(m_core_name,rhs.m_core_name);
		std::swap(m_touch_only_actives,rhs.m_touch_only_actives);
		std::swap(m_levelset,rhs.m_levelset);
		std::swap(m_fillable,rhs.m_fillable);
		std::swap(m_fill_value,rhs.m_fill_value);
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
		 \~english @brief Background value.
		 \~japanese @brief バックグランドの値。
		 */
		T background_value;
		/**
		 \~english @brief Fill value.
		 \~japanese @brief 塗りつぶしの値。
		 */
		T fill_value;
		/**
		 \~english @brief Is grid fillable.
		 \~japanese @brief 塗りつぶし可能なグリッドか。
		 */
		bool is_fillable;
		/**
		 \~english @brief Is grid level set.
		 \~japanese @brief レベルセットグリッドか。
		 */
		bool is_levelset;
		/**
		 \~english @brief Are grid operations only allowed on active cells.
		 \~japanese @brief 演算子によるグリッド操作がアクティブセルだけに影響を与えるべきか。
		 */
		bool touch_only_actives;
		/**
		 \~english @brief Comparison operator.
		 \~japanese @brief 比較オペラータ。
		 */
		bool operator==(const type3 &rhs) const {
			return 
				core_name == rhs.core_name &&
				shape == rhs.shape &&
				background_value == rhs.background_value &&
				fill_value && rhs.fill_value &&
				is_fillable == rhs.is_fillable &&
				is_levelset == rhs.is_levelset &&
				touch_only_actives == rhs.touch_only_actives;
		}
	};
	/**
	 \~english @brief Get the type of this grid.
	 @return Type of this grid.
	 \~japanese @brief このグリッドの type を取得する。
	 @return このグリッドの type。
	 */
	type3 type() const { return { get_core_name(),shape(),m_background_value,m_fill_value,m_fillable,m_levelset,m_touch_only_actives}; }
	/**
	 \~english @brief Set the type of this grid.
	 @param[in] type An instance of type to set.
	 \~japanese @brief グリッドの type を設定する。
	 @param[in] type セットする type のインスタンス。
	 */
	void set_type( const type3 &type ) {
		m_core_name = type.core_name;
		m_shape = type.shape;
		m_background_value = type.background_value;
		m_touch_only_actives = type.touch_only_actives;
		m_fillable = type.is_fillable;
		m_fill_value = type.fill_value;
		m_levelset = type.is_levelset;
	}
	//
private:
	//
	shape3 m_shape;
	parallel_driver m_parallel{this};
	T m_background_value {T()}, m_fill_value {T()};
	bool m_touch_only_actives {false}, m_fillable {false}, m_levelset {false}, m_is_initialized {false};
	char m_levelset_halfwidth {0};
	array3_ptr m_core;
	std::string m_core_name;
};
//
SHKZ_END_NAMESPACE
//
#endif
