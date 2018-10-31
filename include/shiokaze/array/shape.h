/*
**	shape.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 10, 2018.
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
#ifndef SHKZ_SHAPE_H
#define SHKZ_SHAPE_H
//
#include <shiokaze/math/vec.h>
#include <algorithm>
#include <cstdint>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Structure that defines shape such as width, height.
/// \~japanese @brief 縦、横といった形状を定義する構造体。
//
/// \~english @brief Structure that defines a two dimensional shape such as width, height.
/// \~japanese @brief 縦、横といった2次元の形状を定義する構造体。
struct shape2 {
	/**
	 \~english @brief Constructor for shape2.
	 @param[in] gn Array that defines numbers of the shape.
	 \~japanese @brief shape2 のコンストラクタ。
	 @param[in] gn 形状の形の数を定義した配列。
	 */
	shape2( const unsigned gn[DIM2] ) : w(gn[0]), h(gn[1]) {}
	/**
	 \~english @brief Constructor for shape2.
	 @param[in] w Width.
	 @param[in] h Height.
	 \~japanese @brief shape2 のコンストラクタ。
	 @param[in] w 幅。
	 @param[in] h 高さ。
	 */
	shape2( unsigned w, unsigned h ) : w(w), h(h) {}
	/**
	 \~english @brief Constructor for shape2.
	 \~japanese @brief shape2 のコンストラクタ。
	 */
	shape2() : w(0), h(0) {}
	/**
	 \~english @brief Get if this shape is equal to the input shape.
	 @param[in] shape Shape to compare.
	 \~japanese @brief この形状が入力の形状と同じが取得する。
	 @param[in] shape 比較する形状。
	 */
	bool operator==( const shape2 &shape ) const {
		return w==shape.w && h==shape.h;
	}
	/**
	 \~english @brief Get if this shape is different from the input shape.
	 @param[in] shape Shape to compare.
	 \~japanese @brief この形状が入力の形状と違うか取得する。
	 @param[in] shape 比較する形状。
	 */
	bool operator!=( const shape2 &shape ) const {
		return ! (*this == shape);
	}
	/**
	 \~english @brief Get the number of a specified dimensional of this shape.
	 @param[in] idx Number of dimension.
	 \~japanese @brief 形状の指定した次元の大きさを得る。
	 @param[in] idx 次元の数。
	 */
	unsigned operator[](unsigned idx) const {
		if( idx == 0 ) return w;
		else if( idx == 1 ) return h;
		else return 0;
	}
	/**
	 \~english @brief Get the writable number of a specified dimensional of this shape.
	 @param[in] idx Number of dimension.
	 \~japanese @brief 形状の指定した書き込み可能な次元の大きさを得る。
	 @param[in] idx 次元の数。
	 */
	unsigned& operator[](unsigned idx) {
		static unsigned dummy (0);
		if( idx == 0 ) return w;
		else if( idx == 1 ) return h;
		else return dummy;
	}
	/**
	 \~english @brief Get an expanded shape by the input shape.
	 @param[in] rhs Input shape.
	 @return Expanded shape.
	 \~japanese @brief 入力の形状だけ拡張された形状を取得する。
	 @param[in] rhs 入力の形状。
	 @return 拡張された形状。
	 */
	shape2 operator+(const shape2& rhs) const { 
		return shape2(w+rhs.w,h+rhs.h);
	}
	/**
	 \~english @brief Expand this shape by the input shape.
	 @param[in] rhs Input shape.
	 \~japanese @brief 入力の形状だけ形状を拡張する。
	 @param[in] rhs 入力の形状。
	 */
	void operator+=(const shape2& rhs) { 
		w += rhs.w;
		h += rhs.h;
	}
	/**
	 \~english @brief Get an shrunk shape by the input shape.
	 @param[in] rhs Input shape.
	 @return Shrunk shape.
	 \~japanese @brief 入力の形状だけ縮小された形状を取得する。
	 @param[in] rhs 入力の形状。
	 @return 縮小された形状。
	 */
	shape2 operator-(const shape2& rhs) const { 
		return shape2(w-rhs.w,h-rhs.h);
	}
	/**
	 \~english @brief Shrink the shape by the input shape.
	 @param[in] rhs Input shape.
	 \~japanese @brief 入力の形状だけ形状を縮小する。
	 @param[in] rhs 入力の形状。
	 */
	void operator-=(const shape2& rhs) { 
		w -= rhs.w;
		h -= rhs.h;
	}
	/**
	 \~english @brief Get a scaled shape by the input number.
	 @param[in] s Scaling factor.
	 @return Scaled shape.
	 \~japanese @brief 入力の形状だけスケーリングされた形状を取得する。
	 @param[in] s スケーリングファクター。
	 @return スケーリングされた形状。
	 */
	shape2 operator*(double s) const { 
		return shape2(s*w,s*h);
	}
	/**
	 \~english @brief Scale this shape by the input number.
	 @param[in] s Scaling factor.
	 \~japanese @brief 入力の形状だけ形状をスケーリングする。
	 @param[in] s スケーリングファクター。
	 */
	void operator*=(double v) { 
		w *= v;
		h *= v;
	}
	/**
	 \~english @brief Get a scaled (by division) shape by the input number.
	 @param[in] s Scaling factor.
	 @return Scaled shape.
	 \~japanese @brief 入力の形状だけ(割り算によって)スケーリングされた形状を取得する。
	 @param[in] s スケーリングファクター。
	 @return スケーリングされた形状。
	 */
	shape2 operator/(double s) const { 
		return shape2(w/s,h/s);
	}
	/**
	 \~english @brief Scale this shape (by division) by the input number.
	 @param[in] s Scaling factor.
	 \~japanese @brief 入力の形状だけ形状を(割り算によって)スケーリングする。
	 @param[in] s スケーリングファクター。
	 */
	void operator/=(double v) { 
		w /= v;
		h /= v;
	}
	/**
	 \~english @brief Get the dimensional numbers of this shape.
	 @param[out] w Width.
	 @param[out] h Height.
	 \~japanese @brief この形状の次元に関する数字を得る。
	 @param[out] w 幅。
	 @param[out] h 高さ。
	 */
	void get( unsigned &w, unsigned &h ) const {
		w = this->w;
		h = this->h;
	}
	/**
	 \~english @brief Get the dimensional numbers of this shape.
	 @param[out] gn One dimensional array that stores width and height in this order.
	 \~japanese @brief この形状の次元に関する数字を得る。
	 @param[out] gn 幅、高さの順番で数字を格納する一次元配列。
	 */
	void get( unsigned gn[DIM2] ) const {
		gn[0] = w;
		gn[1] = h;
	}
	/**
	 \~english @brief Compare the hash from the input shape.
	 @param[in] rhs Shape to compare the hash.
	 \~japanese @brief 入力の形状とハッシュを調べる。
	 @param[in] rhs ハッシュを比較する形状。
	 */
	bool operator<(const shape2& rhs) const { 
		return hash() < rhs.hash();
	}
	/**
	 \~english @brief Get the hash number of this shape.
	 \~japanese @brief この形状のハッシュの数字を取得する。
	 */
	size_t hash() const {
		return w ^ (h << 1);
	}
	/**
	 \~english @brief Get the shape for the cell-centered grid from this shape.
	 \~japanese @brief この形状からセルセンターで定義されたグリッドの形状を取得する。
	 */
	shape2 cell() const { return shape2(w,h); }
	/**
	 \~english @brief Get the shape for the nodal defined grid from this shape.
	 \~japanese @brief この形状からノードで定義されたグリッドの形状を取得する。
	 */
	shape2 nodal() const { return shape2(w+1,h+1); }
	/**
	 \~english @brief Get the shape for the staggered grid of a specified dimension from this shape.
	 @param[in] dim Input dimension.
	 \~japanese @brief この形状からスタッガードグリッドの入力次元の数字のグリッドの形状を取得する。
	 @param[in] dim 入力の次元。
	 */
	shape2 face(int dim) const { return shape2(w+(dim==0),h+(dim==1)); }
	/**
	 \~english @brief Find the nearest cell position from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief 小数点を含む位置情報の入力から一番近いセルの位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec2i find_cell( const vec2d &p ) const {
		return cell().clamp(p);
	}
	/**
	 \~english @brief Find the nearest nodal position from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief 小数点を含む位置情報の入力から一番近いノード(節点)の位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec2i find_node( const vec2d &p ) const {
		return nodal().clamp(p+vec2d(0.5,0.5));
	}
	/**
	 \~english @brief Find the nearest facial position (in the context of staggered grid) from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief スタッガード格子の文脈で、小数点を含む位置情報の入力から一番近い面の位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec2i find_face( const vec2d &p, unsigned dim ) const {
		return face(dim).clamp(p+0.5*vec2d(dim==0,dim==1));
	}
	/**
	 \~english @brief Get the length of grid cell size.
	 \~japanese @brief グリッドのセルの長さを得る。
	 */
	double dx() const {
		double dx(1.0);
		for( unsigned dim : DIMS2 ) dx = std::min(dx,1.0/(*this)[dim]);
		return dx;
	}
	/**
	 \~english @brief Get the new constrained position within the index space of this shape.
	 @param[in] pi Input position.
	 @return Constrained new position.
	 \~japanese @brief この形状のインデックス空間に制約された新しい位置を取得する。
	 @param[in] pi 入力の位置。
	 @return 制約された新しい位置。
	 */
	vec2i clamp( const vec2i &pi ) const { 
		return clamp(pi[0],pi[1]);
	}
	/**
	 \~english @brief Get the new constrained position within the index space of this shape.
	 @param[in] i Input position on x coordinate.
	 @param[in] j Input position on y coordinate.
	 @return Constrained new position.
	 \~japanese @brief この形状のインデックス空間に範囲に限定された新しい位置を取得する。
	 @param[in] i x 方向の入力の位置。
	 @param[in] j y 方向の入力の位置。
	 @return 制約された新しい位置。
	 */
	vec2i clamp( int i, int j ) const {
		if( i < 0 ) i = 0;
		if( i > w-1 ) i = w-1;
		if( j < 0 ) j = 0;
		if( j > h-1 ) j = h-1;
		return vec2i(i,j);
	}
	/**
	 \~english @brief Get if the position is outside of the index space of this shape.
	 @param[in] i Input position on x coordinate.
	 @param[in] j Input position on y coordinate.
	 @return \c true if the input position is outside of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の範囲外にあるか取得する。
	 @param[in] i x 方向の入力の位置。
	 @param[in] j y 方向の入力の位置。
	 @return もしインデックス空間の範囲外にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool out_of_bounds( int i, int j ) const {
		return ( i < 0 || i >= w || j < 0 || j >= h );
	}
	/**
	 \~english @brief Get if the position is outside of the index space of this shape.
	 @param[in] pi Input position.
	 @return \c true if the input position is outside of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の範囲外にあるか取得する。
	 @param[in] pi Input position.
	 @return もしインデックス空間の範囲外にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool out_of_bounds( const vec2i &pi ) const {
		return out_of_bounds(pi[0],pi[1]);
	}
	/**
	 \~english @brief Get if the position lies on the edge of the index space of this shape.
	 @param[in] i Input position on x coordinate.
	 @param[in] j Input position on y coordinate.
	 @return \c true if the input position lies on the edge of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の境界にあるか取得する。
	 @param[in] i x 方向の入力の位置。
	 @param[in] j y 方向の入力の位置。
	 @return もしインデックス空間の境界にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool on_edge( int i, int j ) const {
		return i==0 || j==0 || i==w-1 || j==h-1;
	}
	/**
	 \~english @brief Get if the position lies on the edge of the index space of this shape.
	 @param[in] pi Input position.
	 @return \c true if the input position lies on the edge of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の境界にあるか取得する。
	 @param[in] pi Input position.
	 @return もしインデックス空間の境界にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool on_edge( const vec2i &pi ) const {
		return on_edge(pi[0],pi[1]);
	}
	/**
	 \~english @brief Count the number of cells of the grid of this shape.
	 @return The number of total elements.
	 \~japanese @brief この形で構成されるグリッドの全ての要素の数を数える。
	 @return 全要素の数。
	 */
	size_t count() const {
		return w*h;
	}
	/**
	 \~english @brief Get if all the lengthes of this shape is zero.
	 @return \c true if the shape is empty \c false otherwise.
	 \~japanese @brief この形の大きさがどの向きでもゼロかどうか調べる。
	 @return もし空白であれば \c true を返し、そうでなければ \c false を返す。
	 */
	bool empty() const {
		return w == 0 && h == 0;
	}
	/**
	 \~english @brief Perform a two dimensional serial loop operation.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 2次元の逐次処理を行う。
	 @param[in] func 実際のループを処理する関数。
	 */
	void for_each( std::function<void(int i, int j)> func ) const {
		for( int j=0; j<h; ++j ) for( int i=0; i<w; ++i ) {
			func(i,j);
		}
	}
	/**
	 \~english @brief Perform a serial loop operation.
	 @param[in] func Function that processes a loop. If the function return \c true, the loop interrupts.
	 \~japanese @brief 逐次処理を行う。
	 @param[in] func 実際のループを処理する関数。もし関数が \c true を返すと、ループ処理を中断する。
	 */
	void interruptible_for_each( std::function<bool(int i, int j)> func ) const {
		for( int j=0; j<h; ++j ) for( int i=0; i<w; ++i ) {
			if(func(i,j)) return;
		}
	}
	/**
	 \~english @brief Width of the shape.
	 \~japanese @brief 形状の幅。
	 */
	unsigned w;
	/**
	 \~english @brief Height of the shape.
	 \~japanese @brief 形状の高さ。
	 */
	unsigned h;
};
//
static inline shape2 operator*(double s, const shape2 &shape) {
	return shape*s;
}
static inline shape2 operator/(double s, const shape2 &shape) {
	return shape/s;
}
//
/// \~english @brief Structure that defines a three dimensional shape such as width, height and depth.
/// \~japanese @brief 縦、横、奥行きといった3次元の形状を定義する構造体。
struct shape3 {
	/**
	 \~english @brief Constructor for shape3.
	 @param[in] gn Array that defines numbers of the shape.
	 \~japanese @brief shape3 のコンストラクタ。
	 @param[in] gn 形状の形の数を定義した配列。
	 */
	shape3( const unsigned gn[DIM2] ) : w(gn[0]), h(gn[1]), d(gn[2]) {}
	/**
	 \~english @brief Constructor for shape2.
	 @param[in] w Width.
	 @param[in] h Height.
	 @param[in] d Depth.
	 \~japanese @brief shape2 のコンストラクタ。
	 @param[in] w 幅。
	 @param[in] h 高さ。
	 @param[in] h 奥行き。
	 */
	shape3( unsigned w, unsigned h, unsigned d) : w(w), h(h), d(d) {}
	/**
	 \~english @brief Constructor for shape3.
	 \~japanese @brief shape3 のコンストラクタ。
	 */
	shape3() : w(0), h(0), d(0) {}
	/**
	 \~english @brief Get if this shape is equal to the input shape.
	 @param[in] shape Shape to compare.
	 \~japanese @brief この形状が入力の形状と同じが取得する。
	 @param[in] shape 比較する形状。
	 */
	bool operator==( const shape3 &shape ) const {
		return w==shape.w && h==shape.h && d==shape.d;
	}
	/**
	 \~english @brief Get if this shape is different from the input shape.
	 @param[in] shape Shape to compare.
	 \~japanese @brief この形状が入力の形状と違うか取得する。
	 @param[in] shape 比較する形状。
	 */
	bool operator!=( const shape3 &shape ) const {
		return ! (*this == shape);
	}
	/**
	 \~english @brief Get the number of a specified dimensional of this shape.
	 @param[in] idx Number of dimension.
	 \~japanese @brief 形状の指定した次元の大きさを得る。
	 @param[in] idx 次元の数。
	 */
	unsigned operator[](unsigned idx) const {
		if( idx == 0 ) return w;
		else if( idx == 1 ) return h;
		else if( idx == 2 ) return d;
		else return 0;
	}
	/**
	 \~english @brief Get the writable number of a specified dimensional of this shape.
	 @param[in] idx Number of dimension.
	 \~japanese @brief 形状の指定した書き込み可能な次元の大きさを得る。
	 @param[in] idx 次元の数。
	 */
	unsigned& operator[](unsigned idx) {
		static unsigned dummy (0);
		if( idx == 0 ) return w;
		else if( idx == 1 ) return h;
		else if( idx == 2 ) return d;
		else return dummy;
	}
	/**
	 \~english @brief Get an expanded shape by the input shape.
	 @param[in] rhs Input shape.
	 @return Expanded shape.
	 \~japanese @brief 入力の形状だけ拡張された形状を取得する。
	 @param[in] rhs 入力の形状。
	 @return 拡張された形状。
	 */
	shape3 operator+(const shape3& rhs) const { 
		return shape3(w+rhs.w,h+rhs.h,d+rhs.d);
	}
	/**
	 \~english @brief Expand this shape by the input shape.
	 @param[in] rhs Input shape.
	 \~japanese @brief 入力の形状だけ形状を拡張する。
	 @param[in] rhs 入力の形状。
	 */
	void operator+=(const shape3& rhs) { 
		w += rhs.w;
		h += rhs.h;
		d += rhs.d;
	}
	/**
	 \~english @brief Get an shrunk shape by the input shape.
	 @param[in] rhs Input shape.
	 @return Shrunk shape.
	 \~japanese @brief 入力の形状だけ縮小された形状を取得する。
	 @param[in] rhs 入力の形状。
	 @return 縮小された形状。
	 */
	shape3 operator-(const shape3& rhs) const { 
		return shape3(w-rhs.w,h-rhs.h,d-rhs.d);
	}
	/**
	 \~english @brief Shrink the shape by the input shape.
	 @param[in] rhs Input shape.
	 \~japanese @brief 入力の形状だけ形状を縮小する。
	 @param[in] rhs 入力の形状。
	 */
	void operator-=(const shape3& rhs) { 
		w -= rhs.w;
		h -= rhs.h;
		d -= rhs.d;
	}
	/**
	 \~english @brief Get a scaled shape by the input number.
	 @param[in] s Scaling factor.
	 @return Scaled shape.
	 \~japanese @brief 入力の形状だけスケーリングされた形状を取得する。
	 @param[in] s スケーリングファクター。
	 @return スケーリングされた形状。
	 */
	shape3 operator*(double s) const { 
		return shape3(s*w,s*h,s*d);
	}
	/**
	 \~english @brief Scale this shape by the input number.
	 @param[in] s Scaling factor.
	 \~japanese @brief 入力の形状だけ形状をスケーリングする。
	 @param[in] s スケーリングファクター。
	 */
	void operator*=(double v) { 
		w *= v;
		h *= v;
		d *= v;
	}
	/**
	 \~english @brief Get a scaled (by division) shape by the input number.
	 @param[in] s Scaling factor.
	 @return Scaled shape.
	 \~japanese @brief 入力の形状だけ(割り算によって)スケーリングされた形状を取得する。
	 @param[in] s スケーリングファクター。
	 @return スケーリングされた形状。
	 */
	shape3 operator/(double s) const { 
		return shape3(w/s,h/s,d/s);
	}
	/**
	 \~english @brief Scale this shape (by division) by the input number.
	 @param[in] s Scaling factor.
	 \~japanese @brief 入力の形状だけ形状を(割り算によって)スケーリングする。
	 @param[in] s スケーリングファクター。
	 */
	void operator/=(double v) { 
		w /= v;
		h /= v;
		d /= v;
	}
	/**
	 \~english @brief Get the dimensional numbers of this shape.
	 @param[out] w Width.
	 @param[out] h Height.
	 @param[out] d Depth.
	 \~japanese @brief この形状の次元に関する数字を得る。
	 @param[out] w 幅。
	 @param[out] h 高さ。
	 @param[out] d 奥行き。
	 */
	void get( unsigned &w, unsigned &h, unsigned &d ) const {
		w = this->w;
		h = this->h;
		d = this->d;
	}
	/**
	 \~english @brief Get the dimensional numbers of this shape.
	 @param[out] gn One dimensional array that stores width, height and depth in this order.
	 \~japanese @brief この形状の次元に関する数字を得る。
	 @param[out] gn 幅、高さ、奥行きの順番で数字を格納する一次元配列。
	 */
	void get( unsigned gn[DIM3] ) const {
		gn[0] = w;
		gn[1] = h;
		gn[2] = d;
	}
	/**
	 \~english @brief Compare the hash from the input shape.
	 @param[in] rhs Shape to compare the hash.
	 \~japanese @brief 入力の形状とハッシュを調べる。
	 @param[in] rhs ハッシュを比較する形状。
	 */
	bool operator < (const shape2& rhs) const { 
		return hash() < rhs.hash();
	}
	/**
	 \~english @brief Get the hash number of this shape.
	 \~japanese @brief この形状のハッシュの数字を取得する。
	 */
	size_t hash() const {
		return w ^ (h << 1) ^ (d << 2);
	}
	/**
	 \~english @brief Get the shape for the cell-centered grid from this shape.
	 \~japanese @brief この形状からセルセンターで定義されたグリッドの形状を取得する。
	 */
	shape3 cell() const { return shape3(w,h,d); }
	/**
	 \~english @brief Get the shape for the nodal defined grid from this shape.
	 \~japanese @brief この形状からノードで定義されたグリッドの形状を取得する。
	 */
	shape3 nodal() const { return shape3(w+1,h+1,d+1); }
	/**
	 \~english @brief Get the shape for the staggered grid of a specified dimension from this shape.
	 @param[in] dim Input dimension.
	 \~japanese @brief この形状からスタッガードグリッドの入力次元の数字のグリッドの形状を取得する。
	 @param[in] dim 入力の次元。
	 */
	shape3 face(int dim) const { return shape3(w+(dim==0),h+(dim==1),d+(dim==2)); }
	/**
	 \~english @brief Get the shape of edge of a specified dimension from this shape.
	 @param[in] dim Input dimension.
	 \~japanese @brief この形状からエッジの形状を取得する。
	 @param[in] dim 入力の次元。
	 */
	shape3 edge(int dim) const { return shape3(w+(dim!=0),h+(dim!=1),d+(dim!=2)); }
	/**
	 \~english @brief Find the nearest cell position from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief 小数点を含む位置情報の入力から一番近いセルの位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec3i find_cell( const vec3d &p ) const {
		return shape3(w,h,d).clamp(p);
	}
	/**
	 \~english @brief Find the nearest nodal position from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief 小数点を含む位置情報の入力から一番近いノード(節点)の位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec3i find_node( const vec3d &p ) const {
		return nodal().clamp(p+vec3d(0.5,0.5,0.5));
	}
	/**
	 \~english @brief Find the nearest edge position from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief 小数点を含む位置情報の入力から一番近いエッジの位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec3i find_edge( const vec3d &p, unsigned dim ) const {
		return edge(dim).clamp(p+0.5*vec3d(dim!=0,dim!=1,dim!=2));
	}
	/**
	 \~english @brief Find the nearest facial position (in the context of staggered grid) from the input fractional position.
	 @param[in] p Fractional input position.
	 \~japanese @brief スタッガード格子の文脈で、小数点を含む位置情報の入力から一番近い面の位置を取得する。
	 @param[in] p 入力の小数点を含む位置。
	 */
	vec3i find_face( const vec3d &p, unsigned dim ) const {
		return face(dim).clamp(p+0.5*vec3d(dim==0,dim==1,dim==2));
	}
	/**
	 \~english @brief Get the length of grid cell size.
	 \~japanese @brief グリッドのセルの長さを得る。
	 */
	double dx() const {
		double dx(1.0);
		for( unsigned dim : DIMS3 ) dx = std::min(dx,1.0/(*this)[dim]);
		return dx;
	}
	/**
	 \~english @brief Get the new constrained position within the index space of this shape.
	 @param[in] pi Input position.
	 @return Constrained new position.
	 \~japanese @brief この形状のインデックス空間に制約された新しい位置を取得する。
	 @param[in] pi 入力の位置。
	 @return 制約された新しい位置。
	 */
	vec3i clamp( const vec3i &pi ) const { 
		return clamp(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Get the new constrained position within the index space of this shape.
	 @param[in] i Input position on x coordinate.
	 @param[in] j Input position on y coordinate.
	 @return Constrained new position.
	 \~japanese @brief この形状のインデックス空間に範囲に限定された新しい位置を取得する。
	 @param[in] i x 方向の入力の位置。
	 @param[in] j y 方向の入力の位置。
	 @return 制約された新しい位置。
	 */
	vec3i clamp( int i, int j, int k ) const {
		if( i < 0 ) i = 0;
		if( i > w-1 ) i = w-1;
		if( j < 0 ) j = 0;
		if( j > h-1 ) j = h-1;
		if( k < 0 ) k = 0;
		if( k > d-1 ) k = d-1;
		return vec3i(i,j,k);
	}
	/**
	 \~english @brief Get if the position is outside of the index space of this shape.
	 @param[in] i Input position on x coordinate.
	 @param[in] j Input position on y coordinate.
	 @param[in] k Input position on z coordinate.
	 @return \c true if the input position is outside of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の範囲外にあるか取得する。
	 @param[in] i x 方向の入力の位置。
	 @param[in] j y 方向の入力の位置。
	 @param[in] k z 方向の入力の位置。
	 @return もしインデックス空間の範囲外にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool out_of_bounds( int i, int j, int k ) const {
		return ( i < 0 || i >= w || j < 0 || j >= h || k < 0 || k >= d );
	}
	/**
	 \~english @brief Get if the position is outside of the index space of this shape.
	 @param[in] pi Input position.
	 @return \c true if the input position is outside of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の範囲外にあるか取得する。
	 @param[in] pi Input position.
	 @return もしインデックス空間の範囲外にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool out_of_bounds( const vec3i &pi ) const {
		return out_of_bounds(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Get if the position lies on the edge of the index space of this shape.
	 @param[in] i Input position on x coordinate.
	 @param[in] j Input position on y coordinate.
	 @param[in] k Input position on z coordinate.
	 @return \c true if the input position lies on the edge of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の境界にあるか取得する。
	 @param[in] i x 方向の入力の位置。
	 @param[in] j y 方向の入力の位置。
	 @param[in] k z 方向の入力の位置。
	 @return もしインデックス空間の境界にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool on_edge( int i, int j, int k ) const {
		return i==0 || j==0 || i==w-1 || j==h-1 || k==d-1 || k==d-1;
	}
	/**
	 \~english @brief Get if the position lies on the edge of the index space of this shape.
	 @param[in] pi Input position.
	 @return \c true if the input position lies on the edge of the index space, \c false otherwise.
	 \~japanese @brief 入力の位置がこの形状のインデックス空間の境界にあるか取得する。
	 @param[in] pi Input position.
	 @return もしインデックス空間の境界にあれば \c true を返し、範囲内にあれば \c false を返す。
	 */
	bool on_edge( const vec3i &pi ) const {
		return on_edge(pi[0],pi[1],pi[2]);
	}
	/**
	 \~english @brief Count the number of cells of the grid of this shape.
	 @return The number of total elements.
	 \~japanese @brief この形で構成されるグリッドの全ての要素の数を数える。
	 @return 全要素の数。
	 */
	size_t count() const {
		return w*h*d;
	}
	/**
	 \~english @brief Get if all the lengthes of this shape is zero.
	 @return \c true if the shape is empty \c false otherwise.
	 \~japanese @brief この形の大きさがどの向きでもゼロかどうか調べる。
	 @return もし空白であれば \c true を返し、そうでなければ \c false を返す。
	 */
	bool empty() const {
		return w == 0 && h == 0 && d == 0;
	}
	/**
	 \~english @brief Perform a three dimensional serial loop operation.
	 @param[in] func Function that processes a loop.
	 \~japanese @brief 3次元の逐次処理を行う。
	 @param[in] func 実際のループを処理する関数。
	 */
	void for_each( std::function<void(int i, int j, int k)> func ) const {
		for( int k=0; k<d; ++k ) for( int j=0; j<h; ++j ) for( int i=0; i<w; ++i ) {
			func(i,j,k);
		}
	}
	/**
	 \~english @brief Perform a serial loop operation.
	 @param[in] func Function that processes a loop. If the function return \c true, the loop interrupts.
	 \~japanese @brief 逐次処理を行う。
	 @param[in] func 実際のループを処理する関数。もし関数が \c true を返すと、ループ処理を中断する。
	 */
	void interruptible_for_each( std::function<bool(int i, int j, int k)> func ) const {
		for( int k=0; k<d; ++k ) for( int j=0; j<h; ++j ) for( int i=0; i<w; ++i ) {
			if(func(i,j,k)) return;
		}
	}
	/**
	 \~english @brief Width of the shape.
	 \~japanese @brief 形状の幅。
	 */
	unsigned w;
	/**
	 \~english @brief Height of the shape.
	 \~japanese @brief 形状の高さ。
	 */
	unsigned h;
	/**
	 \~english @brief Depth of the shape.
	 \~japanese @brief 形状の奥行き。
	 */
	unsigned d;
};
//
static inline shape3 operator*(double s, const shape3 &shape) {
	return shape*s;
}
static inline shape3 operator/(double s, const shape3 &shape) {
	return shape/s;
}
//
SHKZ_END_NAMESPACE
//
#endif