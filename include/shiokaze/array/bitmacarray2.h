/*
**	bitmacarray2.h
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
#ifndef SHKZ_BITMACARRAY2_H
#define SHKZ_BITMACARRAY2_H
//
#include <shiokaze/array/bitarray2.h>
#include <algorithm>
#include <array>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Two dimensional staggered bit grid class designed to be defined as instance member in recursive_configurable class.
/// \~japanese @brief recursive_configurable インスタンスのメンバーインスタンスとして定義可能なスタッガードビット2次元配列クラス。
class bitmacarray2 : public recursive_configurable {
public:
	/**
	 \~english @brief Constructor for bitmacarray2.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] shape Shape of the grid
	 @param[in] core_name Core module name. Default value is "lineararray_core2". Can be also "tiledarray_core2".
	 \~japanese @brief bitmacarray2 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] shape グリッドの形
	 @param[in] core_name コア子ジュールの名前。デフォルトは "lineararray_core2"。"tiledarray_core2" も可能。
	 */
	bitmacarray2 ( recursive_configurable *parent, const shape2 &shape, std::string core_name="") : m_shape(shape), m_array_0(this,shape.face(0),core_name), m_array_1(this,shape.face(1),core_name) {
			if( parent ) parent->add_child(this);
			else recursive_configurable::setup_now(this);
	}
	/**
	 \~english @brief Constructor for bitmacarray2.
	 @param[in] parent Pointer to a parent recursive_configurable instance. Can be nullptr.
	 @param[in] core_name Core module name. Default value is "lineararray_core2". Can be also "tiledarray_core2".
	 \~japanese @brief bitmacarray2 のコンストラクタ。
	 @param[in] parent 親 recursive_configurable のインスタンスへのポインタ。nullptr も可。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "lineararray_core2"。"tiledarray_core2" も可能。
	 */
	bitmacarray2( recursive_configurable *parent, std::string core_name="" ) : bitmacarray2(parent,shape2(0,0),core_name) {}
	/**
	 \~english @brief Constructor for bitmacarray2.
	 @param[in] core_name Core module name. Default value is "lineararray_core2". Can be also "tiledarray_core2".
	 \~japanese @brief bitmacarray2 のコンストラクタ。
	 @param[in] core_name コア子ジュールの名前。デフォルトは "lineararray_core2"。"tiledarray_core2" も可能。
	 */
	bitmacarray2 ( std::string core_name="") : bitmacarray2(nullptr,shape2(0,0),core_name) {}
	/**
	 \~english @brief Constructor for bitmacarray2.
	 @param[in] shape Shape of the grid
	 @param[in] core_name Core module name. Default value is "lineararray_core2". Can be also "tiledarray_core2".
	 \~japanese @brief bitmacarray2 のコンストラクタ。
	 @param[in] shape グリッドの形
	 @param[in] core_name コア子ジュールの名前。デフォルトは "lineararray_core2"。"tiledarray_core2" も可能。
	 */
	bitmacarray2( const shape2 &shape, std::string core_name="") : bitmacarray2(nullptr,shape,core_name) {}
	/**
	 \~english @brief Copy constructor for bitmacarray2.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitmacarray2 のコピーコンストラクタ。
	 @param[in] コピーする bitmacarray2 のインスタンスへの参照。
	 */
	bitmacarray2 ( const bitmacarray2& v ) : m_array_0(this), m_array_1(this) {
		copy(v);
	}
	/**
	 \~english @brief Deep copy operation for bitmacarray2.
	 @param[in] array Reference to an instance of macarray to copy from.
	 \~japanese @brief bitmacarray2 のディープコピー演算子。
	 @param[in] コピーする bitmacarray2 のインスタンスへの参照。
	 */
	bitmacarray2& operator=(const bitmacarray2 &array) {
		copy(array);
		return *this;
	}
	/**
	 \~english @brief Deep copy function for bitmacarray2.
	 @param[in] array Reference to an instance of array to copy from.
	 \~japanese @brief bitmacarray2 のディープコピー関数。
	 @param[in] コピーする bitmacarray2 のインスタンスへの参照。
	 */
	void copy( const bitmacarray2 &array ) {
		if( this != &array ) {
			set_type(array.type());
			for( int dim : DIMS2 ) (*this)[dim].copy(array[dim]);
		}
	}
	/**
	 \~english @brief Allocate grid memory with value.
	 @param[in] shape Shape of the grid.
	 @param[in] value Initial value
	 \~japanese @brief グリッドを値でメモリに展開する。
	 @param[in] shape グリッドの形。
	 @param[in] value 初期値。
	 */
	void initialize ( const shape2 &shape ) {
		m_shape = shape;
		for( int dim : DIMS2 )(*this)[dim].initialize(shape.face(dim));
	}
	/**
	 \~english @brief Function to count the number of active cells.
	 @return Active cell count.
	 \~japanese @brief アクティブセルの数を数える関数。
	 @return アクティブセルの数。
	 */
	size_t count () const {
		size_t sum (0);
		for( int dim : DIMS2 ) sum += (*this)[dim].count();
		return sum;
	}
	/**
	 \~english @brief Function to return the list of active cells positions.
	 @return The list of active cells positions.
	 \~japanese @brief アクティブセルの位置のリストを返す関数。
	 @return アクティブセルの位置のリスト。
	 */
	std::array<std::vector<vec2i>,DIM2> actives() const {
		std::array<std::vector<vec2i>,DIM2> result;
		m_parallel.for_each( DIM2, [&]( size_t dim ) {
			result[dim] = (*this)[dim].actives();
		});
		return result;
	}
	/**
	 \~english @brief Activate cells at the positons of active_entries.
	 @param[in] active_entries The list of target positions to activate.
	 @param[in] offset Offset applied to the active_entries.
	 \~japanese @brief active_entries と同じ場所のセルを offset だけずらして、アクティブにする。
	 @param[in] active_entries アクティブにするセルの場所のリスト。
	 @param[in] offset active_entries に適用されるオフセット。
	 */
	void activate( const std::array<std::vector<vec2i>,DIM2> &active_entries, const std::array<vec2i,DIM2> &offsets={vec2i(),vec2i()} ) {
		m_parallel.for_each( DIM2, [&]( size_t dim ) {
			(*this)[dim].activate(active_entries[dim],offsets[dim]);
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
	void activate_as( const bitmacarray2 &array, const std::array<vec2i,DIM2> &offsets={vec2i(),vec2i()} ) {
		m_parallel.for_each( DIM2, [&]( size_t dim ) {
			(*this)[dim].activate_as(array[dim],offsets[dim]);
		});
	}
	/**
	 \~english @brief Activate all the cells.
	 \~japanese @brief 全てのセルをアクティブにする。
	 */
	void activate_all() {
		m_parallel.for_each( DIM2, [&]( size_t dim ) {
			(*this)[dim].activate_all();
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
	void copy_active_as( const bitmacarray2 &array, const vec2i &offset=vec2i() ) {
		m_parallel.for_each( DIM2, [&]( size_t dim ) {
			(*this)[dim].copy_active_as(array[dim],offset);
		});
	}
	/**
	 \~english @brief Get the shape of the array.
	 @return Shape of the array.
	 \~japanese @brief グリッドの形を返す。
	 @return グリッドの形。
	 */
	shape2 shape() const { return m_shape; }
	/**
	 \~english @brief Get the shape of the staggered grid of a specified dimension.
	 @return Shape of the grid of an input dimension.
	 \~japanese @brief 指定された次元のスタッガードグリッドの形を返す。
	 @return 指定された次元でのグリッドの形。
	 */
	shape2 shape(int dim) const { return (*this)[dim].shape(); }
	/**
	 \~english @brief Clear out the grid.
	 *
	 Note that size, the memory allocation, background values and the information regarding level set or fillable left intact.
	 \~japanese @brief グリッドを初期化する。
	 *
	 グリッドの大きさやメモリ確保、レベルセット情報、バックグラウンド値は変更されない。
	 */
	void clear() {
		for( int dim : DIMS2 )(*this)[dim].clear();
	}
	/**
	 \~english @brief Return if the grid is different from an input array.
	 @param[in] array Target array to compare.
	 @return \c true if the array is different from the input array and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと違うかどうか返す。
	 @param[in] array 目標とする比べるグリッド。
	 @return もしグリッドが入力と違うグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator!=( const bitmacarray2 &v ) const {
		return ! (*this == v);
	}
	/**
	 \~english @brief Return if the grid is same to an input array.
	 @param[in] array Target array to compare.
	 @return \c true if the array is the same to the input and \c false otherwise.
	 \~japanese @brief グリッドが入力されたグリッドと同じかどうか返す。
	 @param[in] array 目標とする比べるグリッド。
	 @return もしグリッドが入力と同じグリッドなら \c true そうでなければ \c false を返す。
	 */
	bool operator==(const bitmacarray2 &v) const {
		for( int dim : DIMS2 ) {
			if((*this)[dim] != v[dim]) return false;
		}
		return true;
	}
	/**
	 \~english @brief Get the read-only reference to the staggered array of a specified dimension.
	 @param[in] dim Dimensiton of the grid.
	 \~japanese @brief 指定した次元の読み込み処理だけ可能なスタッガード格子の参照を得る。
	 @param[in] dim グリッドの次元。
	 */
	const bitarray2& operator[](int dim) const {
		return dim==0 ? m_array_0 : m_array_1;
	}
	/**
	 \~english @brief Get the reference to the staggered array of a specified dimension.
	 @param[in] dim Dimensiton of the grid.
	 \~japanese @brief 指定した次元のスタッガード格子の参照を得る。
	 @param[in] dim グリッドの次元。
	 */
	bitarray2& operator[](int dim) {
		return dim==0 ? m_array_0 : m_array_1;
	}
	/// \~english @brief Read-only value accessor.
	/// \~japanese @brief 数値の取得のみを代理するアクセッサー。
	class const_accessor {
	friend class bitmacarray2;
	public:
		/**
		 \~english @brief Get if the cell is active at the input position.
		 @param[in] dim dimension of the grid.
		 @param[in] i position on x coordiante.
		 @param[in] j position on y coordinate.
		 @return \c true if the cell at the input position is active and \c false if not.
		 \~japanese @brief 入力位置でセルがアクティブか得る。
		 @param[in] dim グリッドの次元。
		 @param[in] i x 座標上の位置。
		 @param[in] j y 座標上の位置。
		 @return 入力位置でのセルがアクティブなら \c true そうでないなら \c false を返す。
		 */
		bool operator()(int dim, int i, int j) {
			return get(dim)(i,j);
		}
		/**
		 \~english @brief Get if the cell is active at the input position.
		 @param[in] dim dimension of the grid.
		 @param[in] pi position on grid.
		 @return \c true if the cell at the input position is active and \c false if not.
		 \~japanese @brief 入力位置でセルがアクティブか得る。
		 @param[in] dim グリッドの次元。
		 @param[in] pi グリッドの位置。
		 @return 入力位置でのセルがアクティブなら \c true そうでないなら \c false を返す。
		 */
		bool operator()(int dim, const vec2i &pi) {
			return get(dim)(pi);
		}
		/**
		 \~english @brief Get the shape of the associated grid.
		 @return Shape of the grid.
		 \~japanese @brief 関連付けされたグリッドの大きさを得る。
		 @return グリッドの形。
		 */
		const shape2 shape() const {
			return array->shape();
		}
		/**
		 \~english @brief Get the shape of the associated staggered grid of a specified dimension.
		 @return Shape of the grid.
		 \~japanese @brief 関連付けされた次元のスタッガードグリッドの大きさを得る。
		 @return グリッドの形。
		 */
		const shape2 shape(int dim) const {
			return array->shape(dim);
		}
		/**
		 \~english @brief Get a const_accessor of the associated staggered grid of a specified dimension.
		 @return An instance of const_accessor.
		 \~japanese @brief 関連付けされた次元のスタッガードグリッドの const_accessor を得る。
		 @return const_accessor のインスタンス。
		 */
		typename bitarray2::const_accessor& get( int dim ) {
			return dim == 0 ? accessor_0 : accessor_1;
		}
	protected:
		const_accessor( const bitmacarray2 &array ) : array(&array),
			accessor_0(array[0].get_const_accessor()),
			accessor_1(array[1].get_const_accessor()) {}
		const bitmacarray2 *array;
		typename bitarray2::const_accessor accessor_0, accessor_1;
	};
	/// \~english @brief Serial writable accessor.
	/// \~japanese @brief シリアルに書き込み可能なアクセッサー。
	class serial_accessor : public const_accessor {
	friend class bitmacarray2;
	public:
		/**
		 \~english @brief Set bit at the input position.
		 @param[in] dim dimension of the grid.
		 @param[in] i position on x coordiante.
		 @param[in] j position on y coordinate.
		 \~japanese @brief 入力位置で値をビットをセットする。
		 @param[in] dim グリッドの次元。
		 @param[in] i x 座標上の位置。
		 @param[in] j y 座標上の位置。
		 */
		void set( int dim, int i, int j) {
			get(dim).set(i,j);
		}
		/**
		 \~english @brief Set the bit at the input position.
		 @param[in] dim dimension of the grid.
		 @param[in] pi position on grid.
		 @param[in] value Value to set
		 \~japanese @brief 入力位置で値をビットをセットする。
		 @param[in] dim グリッドの次元。
		 @param[in] pi グリッドの位置。
		 @param[in] value 設定する値。
		 */
		void set( int dim, const vec2i &pi ) {
			set(dim,pi[0],pi[1]);
		}
		/**
		 \~english @brief Inactivate cell at the input position.
		 @param[in] dim dimension of the grid.
		 @param[in] i position on x coordiante.
		 @param[in] j position on y coordinate.
		 \~japanese @brief 入力位置でセルを非アクティブする。
		 @param[in] dim グリッドの次元。
		 @param[in] i x 座標上の位置。
		 @param[in] j y 座標上の位置。
		 */
		void set_off( int dim, int i, int j ) {
			get(dim).set_off(i,j);
		}
		/**
		 \~english @brief Inactivate cell at the input position.
		 @param[in] dim dimension of the grid.
		 @param[in] pi position on grid.
		 \~japanese @brief 入力位置でセルを非アクティブする。
		 @param[in] dim グリッドの次元。
		 @param[in] pi グリッドの位置。
		 */
		void set_off( int dim, const vec2i &pi ) {
			set(dim,pi[0],pi[1]);
		}
		/**
		 \~english @brief Get a serial_accessor of the associated staggered grid of a specified dimension.
		 @return An instance of serial_accessor.
		 \~japanese @brief 関連付けされた次元のスタッガードグリッドの serial_accessor を得る。
		 @return serial_accessor のインスタンス。
		 */
		typename bitarray2::serial_accessor& get( int dim ) {
			return dim == 0 ? accessor_0 : accessor_1;
		}
	protected:
		serial_accessor( bitmacarray2 &array ) : const_accessor(array),
			accessor_0(array[0].get_serial_accessor()),
			accessor_1(array[1].get_serial_accessor()) {}
		typename bitarray2::serial_accessor accessor_0, accessor_1;
	};
	/**
	 \~english @brief Set the number of threads for parallel processing on this grid.
	 @param[in] number Number of threads.
	 \~japanese @brief 並列処理をするためのスレッドの数を設定する。
	 @param[in] number スレッドの数。
	 */
	void set_thread_num( int number ) {
		for( int dim : DIMS2 ) (*this)[dim].set_thread_num(number);
	}
	/**
	 \~english @brief Get the current number of threads for parallel processing on this grid.
	 @return number Number of threads.
	 \~japanese @brief 現在設定されている並列処理をするためのスレッドの数を得る。
	 @return number スレッドの数。
	 */
	int get_thread_num() const {
		return m_array_0.get_thread_num();
	}
	/**
	 \~english @brief Generate an instance of const_accessor for this grid.
	 @return Generated instance of const_accessor.
	 \~japanese @brief このグリッドの const_accessor のインスタンスを生成する。
	 @return 生成された const_accessor のインスタンス。
	 */
	const_accessor get_const_accessor() const {
		return const_accessor(*this);
	}
	/**
	 \~english @brief Generate list of instances of const_accessor for this grid.
	 @return Generated list of instances of const_accessor.
	 *
	 The size of list is the number of threads assigned for this grid.
	 \~japanese @brief このグリッドの const_accessor のインスタンスのリストを生成する。
	 *
	 リストの大きさは、このグリッドに関連付けされたスレッドの数。
	 @return 生成された const_accessor のインスタンスのリスト。
	 */
	std::vector<const_accessor> get_const_accessors( int number=0 ) const {
		if( ! number ) number = get_thread_num();
		return std::vector<const_accessor>(number,*this);
	}
	/**
	 \~english @brief Generate an instance of serial_accessor for this grid.
	 @return Generated instance of serial_accessor.
	 \~japanese @brief このグリッドの serial_accessor のインスタンスを生成する。
	 @return 生成された serial_accessor のインスタンス。
	 */
	serial_accessor get_serial_accessor() {
		return serial_accessor(*this);
	}
	//
	enum { ACTIVES = true, ALL = false };
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void parallel_actives( std::function<void(typename bitarray2::iterator& it)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void parallel_all( std::function<void(typename bitarray2::iterator& it)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void parallel_op( std::function<void(typename bitarray2::iterator& it)> func, bool type=ALL ) {
		parallel_op([func](int dim, int i, int j, typename bitarray2::iterator& it, int thread_index){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void parallel_actives( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void parallel_all( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void parallel_op( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func, bool type=ALL ) {
		parallel_op([func](int dim, int i, int j, typename bitarray2::iterator& it, int thread_index){
			func(dim,i,j,it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void parallel_actives( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it, int thread_index)> func ) { parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void parallel_all( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it, int thread_index)> func ) { parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void parallel_op( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it, int thread_index)> func, bool type=ALL ) {
		for( int dim : DIMS2 ) {
			(*this)[dim].parallel_op([&](int i, int j, typename bitarray2::iterator& it, int thread_index) {
				func(dim,i,j,it,thread_index);
			},type);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_parallel_actives( std::function<void(const typename bitarray2::const_iterator& it)> func ) const { const_parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_parallel_all( std::function<void(const typename bitarray2::const_iterator& it)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void const_parallel_op( std::function<void(const typename bitarray2::const_iterator& it)> func, bool type=ALL ) const {
		const_parallel_op([func](int dim, int i, int j, const typename bitarray2::const_iterator& it, int thread_index){
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_parallel_actives( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func ) const { const_parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_parallel_all( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void const_parallel_op( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func, bool type=ALL ) const {
		const_parallel_op([func](int dim, int i, int j, const typename bitarray2::const_iterator& it, int thread_index){
			func(dim,i,j,it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_parallel_actives( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it, int thread_index)> func ) const { const_parallel_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_parallel_all( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it, int thread_index)> func ) const { const_parallel_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in parallel by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルを並列に読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void const_parallel_op( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it, int thread_index)> func, bool type=ALL ) const {
		for( int dim : DIMS2 ) {
			(*this)[dim].const_parallel_op([&](int i, int j, const typename bitarray2::const_iterator& it, int thread_index) {
				func(dim,i,j,it,thread_index);
			},type);
		};
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void serial_actives( std::function<void(typename bitarray2::iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void serial_all( std::function<void(typename bitarray2::iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void serial_op( std::function<void(typename bitarray2::iterator& it)> func, bool type=ALL ) {
		serial_op([func](int dim, int i, int j, typename bitarray2::iterator& it) {
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void serial_actives( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void serial_all( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void serial_op( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func, bool type=ALL ) {
		for( int dim : DIMS2 ) {
			(*this)[dim].serial_op([&](int i, int j, typename bitarray2::iterator& it) {
				func(dim,i,j,it);
			},type);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_serial_actives( std::function<void(const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_serial_all( std::function<void(const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void const_serial_op( std::function<void(const typename bitarray2::const_iterator& it)> func, bool type=ALL ) const {
		const_serial_op([func](int dim, int i, int j, const typename bitarray2::const_iterator& it) {
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief アクティブなセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_serial_actives( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 \~japanese @brief 全てのセルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void const_serial_all( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに読み込みのみで処理する。
	 @param[in] func それぞれのセルを処理する関数。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void const_serial_op( std::function<void(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func, bool type=ALL ) const {
		for( int dim : DIMS2 ) {
			(*this)[dim].const_serial_op([&](int i, int j, const typename bitarray2::const_iterator& it) {
				func(dim,i,j,it);
			},type);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	inline void interruptible_serial_actives( std::function<bool(typename bitarray2::iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	inline void interruptible_serial_all( std::function<bool(typename bitarray2::iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void interruptible_serial_op( std::function<bool(typename bitarray2::iterator& it)> func, bool type=ALL ) {
		serial_op([func](int dim, int i, int j, typename bitarray2::iterator& it) {
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルをシリアルに処理する。\c true を返すと、ループを中断する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void interruptible_serial_actives( std::function<bool(int dim, int i, int j, typename bitarray2::iterator& it)> func ) { serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルをシリアルに処理する。\c true を返すと、ループを中断する。
	 @param[in] func それぞれのセルを処理する関数。
	 */
	inline void interruptible_serial_all( std::function<bool(int dim, int i, int j, typename bitarray2::iterator& it)> func ) { serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order.
	 @param[in] func Function that defines how each grid cell is processed.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルをシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void interruptible_serial_op( std::function<bool(int dim, int i, int j, typename bitarray2::iterator& it)> func, bool type=ALL ) {
		for( int dim : DIMS2 ) {
			(*this)[dim].serial_op([&](int i, int j, typename bitarray2::iterator& it) {
				func(dim,i,j,it);
			},type);
		}
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	inline void interruptible_const_serial_actives( std::function<bool(const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	inline void interruptible_const_serial_all( std::function<bool(const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void interruptible_const_serial_op( std::function<bool(const typename bitarray2::const_iterator& it)> func, bool type=ALL ) const {
		const_serial_op([func](int dim, int i, int j, const typename bitarray2::const_iterator& it) {
			func(it);
		},type);
	}
	/**
	 \~english @brief Loop over all the active cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief アクティブなセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	inline void interruptible_const_serial_actives( std::function<bool(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ACTIVES); }
	/**
	 \~english @brief Loop over all the cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 \~japanese @brief 全てのセルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 */
	inline void interruptible_const_serial_all( std::function<bool(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func ) const { const_serial_op(func,ALL); }
	/**
	 \~english @brief Loop over cells in serial order by read-only fashion.
	 @param[in] func Function that defines how each grid cell is processed. Stop the loop if return true.
	 @param[in] type Type of target cells. ACTIVE or ALL. Stop the loop if return true.
	 \~japanese @brief セルを読み込み可能に限定してシリアルに処理する。
	 @param[in] func それぞれのセルを処理する関数。\c true を返すと、ループを中断する。
	 @param[in] type ターゲットセルのタイプ. ACTIVE か ALL。
	 */
	inline void interruptible_const_serial_op( std::function<bool(int dim, int i, int j, const typename bitarray2::const_iterator& it)> func, bool type=ALL ) const {
		for( int dim : DIMS2 ) {
			(*this)[dim].const_serial_op([&](int i, int j, const typename bitarray2::const_iterator& it) {
				func(dim,i,j,it);
			},type);
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
	void dilate( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it, int thread_index)> func, int count=1 ) {
		for( int n=0; n<count; ++n ) {
			m_parallel.for_each(DIM2,[&]( size_t dim ) {
				operator[](dim).dilate([&](int i, int j, typename bitarray2::iterator& it, int thread_index) {
					func(dim,i,j,it,thread_index);
				});
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
	void dilate( std::function<void(int dim, int i, int j, typename bitarray2::iterator& it)> func, int count=1 ) {
		dilate([&](int dim, int i, int j, typename bitarray2::iterator& it, int thread_index) {
			func(dim,i,j,it);
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
			dilate([&](int dim, int i, int j, typename bitarray2::iterator& it){ it.set(); });
		}
	}
	/**
	 \~english @brief Get the core name of module of this grid.
	 @return Name of the core name.
	 \~japanese @brief グリッドのモジュールのコアネームを取得する。
	 @return コアネームの名前。
	 */
	std::string get_core_name() const {
		return m_array_0.get_core_name();
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
		 \~english @brief Type for x dimensional grid.
		 \~japanese @brief x 次元のグリッドのタイプ。
		 */
		typename bitarray2::type2 type0;
		/**
		 \~english @brief Type for y dimensional grid.
		 \~japanese @brief y 次元のグリッドのタイプ。
		 */
		typename bitarray2::type2 type1;
	};
	/**
	 \~english @brief Get the type of this grid.
	 @return Type of this grid.
	 \~japanese @brief このグリッドの type を取得する。
	 @return このグリッドの type。
	 */
	type2 type() const {
		return { get_core_name(), m_shape, m_array_0.type(), m_array_1.type() };
	}
	/**
	 \~english @brief Set the type of this grid.
	 @param[in] type An instance of type to set.
	 \~japanese @brief グリッドの type を設定する。
	 @param[in] type セットする type のインスタンス。
	 */
	void set_type( const type2 &type ) {
		m_shape = type.shape;
		m_array_0.set_type(type.type0);
		m_array_1.set_type(type.type1);
	}
private:
	parallel_driver m_parallel{this};
	bitarray2 m_array_0;
	bitarray2 m_array_1;
	shape2 m_shape;
};
//
SHKZ_END_NAMESPACE
//
#endif