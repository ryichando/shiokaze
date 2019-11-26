/*
**	ordering_core.h
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
#ifndef SHKZ_ORDERING_CORE_H
#define SHKZ_ORDERING_CORE_H
//
#include <shiokaze/math/shape.h>
#include <shiokaze/core/recursive_configurable_module.h>
#include <functional>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Core interface providing the way how cells of two and three dimensional grids are enumurated. "lineordering", "blockordering", and "zordering" are provided as implementations.
/// \~japanese @brief 2次元と3次元グリッドのセルの数え方を定義するインターフェース。"lineordering" "blockordering" "zordering" の3つが実装として提供される。
class ordering_core : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(ordering_core,"Loop Order Encoder/Decoder","Order","Loop order encoder/decoder")
	/**
	 \~english @brief Allocate a new context of a two dimensional shape.
	 @param[in] shape Shape.
	 \~japanese @brief 2次元形状のコンテクストを作成する。
	 @param[in] shape 形。
	 */
	virtual const void* new_context( const shape2& shape ) const = 0;
	/**
	 \~english @brief Allocate a new context of a three dimensional shape.
	 @param[in] shape Shape.
	 \~japanese @brief 3次元形状のコンテクストを作成する。
	 @param[in] shape 形。
	 */
	virtual const void* new_context( const shape3& shape ) const = 0;
	/**
	 \~english @brief Deallocate the context generated by new_context.
	 @param[in] context Context generated by new_context.
	 \~japanese @brief new_context で作成されたコンテクストを解放する。
	 @param[in] context new_context で作成されたコンテクスト。
	 */
	virtual void delete_context( const void *context ) const = 0;
	/**
	 \~english @brief Get encoder function for a two dimensional context.
	 @param[in] context Context generated by new_context.
	 @return Encoder function.
	 \~japanese @brief 2次元コンテクストからエンコード関数を取得する。
	 @param[in] context new_context で作成されたコンテクスト。
	 @return エンコード関数。
	 */
	virtual std::function<size_t(const void *context, int i, int j)> get_encoder_func2( const void *context ) const = 0;
	/**
	 \~english @brief Get encoder function for a three dimensional context.
	 @param[in] context Context generated by new_context.
	 @return Encoder function.
	 \~japanese @brief 3次元コンテクストからエンコード関数を取得する。
	 @param[in] context new_context で作成されたコンテクスト。
	 @return エンコード関数。
	 */
	virtual std::function<size_t(const void *context, int i, int j, int k)> get_encoder_func3( const void *context ) const = 0;
	/**
	 \~english @brief Structure for two dimensional decoder function.
	 \~japanese @brief 2次元のデコーダー関数の構造体。
	 */
	struct decoder_func2 {
		/**
		 \~english @brief Decoder function.
		 \~japanese @brief デコーダー関数。
		 */
		std::function<void(const void *context, size_t n, int &i, int &j)> func;
		/**
		 \~english @brief Start and the end of the range that can be decoded. Range of i such that range[0] <= i < range[1] is valid.
		 \~japanese @brief デコード可能な範囲の初めと終わり。range[0] <= i < range[1] となる i が有効な範囲。
		 */
		size_t range[2];
	};
	/**
	 \~english @brief Structure for three dimensional decoder function.
	 \~japanese @brief 3次元のデコーダー関数の構造体。
	 */
	struct decoder_func3 {
		/**
		 \~english @brief Decoder function.
		 \~japanese @brief デコーダー関数。
		 */
		std::function<void(const void *context, size_t n, int &i, int &j, int &k)> func;
		/**
		 \~english @brief Start and the end of the range that can be decoded. Range of i such that range[0] <= i < range[1] is valid.
		 \~japanese @brief デコード可能な範囲の初めと終わり。range[0] <= i < range[1] となる i が有効な範囲。
		 */
		size_t range[2];
	};
	/**
	 \~english @brief Get the two dimensional decoder function with respect to the context generated by new_context.
	 @param[in] context Context generated by new_context.
	 @return Collection of decoder functions.
	 \~japanese @brief new_context で作成されたコンテクストに対する2次元のデコーダー関数を取得する。
	 @param[in] context new_context で作成されたコンテクスト。
	 @return デコード関数群。
	 */
	virtual std::vector<decoder_func2> get_decoder_func2( const void *context ) const = 0;
	/**
	 \~english @brief Get the three dimensional decoder function with respect to the context generated by new_context.
	 @param[in] context Context generated by new_context.
	 @return Collection of decoder functions.
	 \~japanese @brief new_context で作成されたコンテクストに対する3次元のデコーダー関数を取得する。
	 @param[in] context new_context で作成されたコンテクスト。
	 @return デコード関数群。
	 */
	virtual std::vector<decoder_func3> get_decoder_func3( const void *context ) const = 0;
	/**
	 \~english @brief Decode an index and map to a two dimensional index space using a collection of decoder functions.
	 @param[in] decoders Collection of decoder functions generated by get_decoder_func2.
	 @param[in] context Context generated by new_context.
	 @param[in] n Index to decode.
	 @param[out] i x coordinate index value.
	 @param[out] j y coordinate index value.
	 \~japanese @brief デコーダー関数群を用いてインデックスをデコードし、2次元インデックス空間に変換する。
	 @param[in] decoders get_decoder_func2 で生成されたデコーダー関数群。
	 @param[in] context new_context で作成されたコンテクスト。
	 @param[in] n デコードを行うインデックスの値。
	 @param[out] i x 座標のインデックスの出力。
	 @param[out] j y 座標のインデックスの出力。
	 */
	void decode( const std::vector<decoder_func2> &decoders, const void *context, size_t n, int &i, int &j) const {
		for( const auto &d : decoders ) {
			if( n >= d.range[0] && n < d.range[1] ) {
				d.func(context,n,i,j);
				return;
			}
		}
		throw;
	}
	/**
	 \~english @brief Decode an index and map to a three dimensional index space using a collection of decoder functions.
	 @param[in] decoders Collection of decoder functions generated by get_decoder_func2.
	 @param[in] context Context generated by new_context.
	 @param[in] n Index to decode.
	 @param[out] i x coordinate index value.
	 @param[out] j y coordinate index value.
	 @param[out] k z coordinate index value.
	 \~japanese @brief デコーダー関数群を用いてインデックスをデコードし、3次元インデックス空間に変換する。
	 @param[in] decoders get_decoder_func3 で生成されたデコーダー関数群。
	 @param[in] context new_context で作成されたコンテクスト。
	 @param[in] n デコードを行うインデックスの値。
	 @param[out] i x 座標のインデックスの出力。
	 @param[out] j y 座標のインデックスの出力。
	 @param[out] k z 座標のインデックスの出力。
	 */
	void decode( const std::vector<decoder_func3> &decoders, const void *context, size_t n, int &i, int &j, int &k) const {
		for( const auto &d : decoders ) {
			if( n >= d.range[0] && n < d.range[1] ) {
				d.func(context,n,i,j,k);
				return;
			}
		}
		throw;
	}
};
//
using ordering_ptr = std::unique_ptr<ordering_core>;
//
SHKZ_END_NAMESPACE
//
#endif
//