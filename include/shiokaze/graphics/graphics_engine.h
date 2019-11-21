/*
**	graphics_engine.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 13, 2018. 
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
#ifndef SHKZ_GRAPHICS_ENGINE_H
#define SHKZ_GRAPHICS_ENGINE_H
//
#include <shiokaze/core/common.h>
#include <string>
#include <map>
#include <functional>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for handling drawing operations.
/// \~japanese @brief 描画処理を行うインターフェース。
class graphics_engine {
public:
	/**
	 \~english @brief Default destructor.
	 \~japanese @brief デフォルトデストラクタ。
	 */
	virtual ~graphics_engine() = default;
	/**
	 \~english @brief Initialize graphics engine.
	 \~japanese @brief グラフィックスエンジンを初期化する。
	 */
	virtual void setup_graphics ( std::map<std::string,const void *> params=std::map<std::string,const void *>() ) = 0;
	//
	/// \~english @brief List of features that can be specified to get_supported().
	/// \~japanese @brief get_supported() で指定出来る機能一覧
	enum class FEATURE {
		/// \~english @brief Support for opacity (alpha) drawing
		/// \~japanese @brief 透明描画を行えるか
		OPACITY,
		/// \~english @brief Support for 3D perspective.
		/// \~japanese @brief 3次元投影を行えるか
		_3D
	};
	virtual bool get_supported ( FEATURE feature ) const = 0;
	/**
	 \~english @brief Set view port.
	 \~japanese @brief ビューポートを設定する。
	**/
	virtual void set_viewport( unsigned x, unsigned y, unsigned width, unsigned height ) = 0;
	/**
	 \~english @brief Get view port.
	 \~japanese @brief ビューポートを得る。
	**/
	virtual void get_viewport( unsigned &x, unsigned &y, unsigned &width, unsigned &height ) const = 0;
	/**
	 \~english @brief Configure 2D coordinate view.
	 @param[in] left Left edge x coordinate.
	 @param[in] right Right edge x coordinate.
	 @param[in] bottom Bottom edge y coordinate.
	 @param[in] top Top edge y coordinate.
	 \~japanese @brief 2次元座標系を設定する。
	 @param[in] left 左端の X 軸の座標。
	 @param[in] right 右端の X 軸の座標。
	 @param[in] bottom 下端の Y 軸の座標。
	 @param[in] top 上端の Y 軸の座標。
	 */
	virtual void set_2D_coordinate( double left, double right, double bottom, double top ) = 0;
	/**
	 \~english @brief Set up a camera with a target position, origin position and fov.
	 @param[in] target Target position.
	 @param[in] position Camera position.
	 @param[in] top Top unit vector.
	 @param[in] fov Field of view.
	 @param[in] near Near clipping.
	 @param[in] far Far clipping.
	 \~japanese @brief 視野角、カメラの位置、ターゲット位置を指定してカメラを向ける。
	 @param[in] target ターゲット位置。
	 @param[in] position カメラの位置。
	 @param[in] top 上方向のユニットベクトル。
	 @param[in] fov 視野角。
	 @param[in] near ニアクリッピング。
	 @param[in] far ファークリッピング。
	 */
	virtual void look_at( const double target[3], const double position[3], const double up[3], double fov, double near, double far ) = 0;
	/**
	 \~english @brief Clear out the canvas.
	 \~japanese @brief キャンバスをクリアする。
	 */
	virtual void clear() = 0;
	/**
	 \~english @brief Get the background color.
	 \~japanese @brief 背景色を得る。
	 */
	virtual void get_background_color( double color[3] ) const = 0;
	/**
	 \~english @brief Get the foreground color.
	 \~japanese @brief 前景色を得る。
	 */
	virtual void get_foreground_color( double color[3] ) const = 0;
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	void color3( double r, double g, double b ) {
		double v[] = { r, g, b };
		color3v(v);
	}
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	void color4( double r, double g, double b, double a ) {
		double v[] = { r, g, b, a };
		color4v(v);
	}
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	template <class T> void color3v( const T *v ) {
		double v_plus_alpha[] = { v[0], v[1], v[2], 1.0 };
		color4v(v_plus_alpha);
	}
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	virtual void color4v( const double *v ) = 0;
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	void color4v( const float *v ) {
		double p[] = { v[0], v[1], v[2], v[3] };
		color4v(p);
	}
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	void vertex2( double x, double y ) {
		double v[] = { x, y, 0.0 };
		vertex3v(v);
	}
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	void vertex3( double x, double y, double z ) {
		double v[] = { x, y, z};
		vertex3v(v);
	}\
	//
	enum class MODE { POINTS, LINES, LINE_STRIP, LINE_LOOP, TRIANGLES, TRIANGLE_STRIP, TRIANGLE_FAN };
	/**
	 \~english @brief Equivalebt to glBegin. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml
	 \~japanese @brief glBegin と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml を参照。
	 */
	virtual void begin( MODE mode ) = 0;
	/**
	 \~english @brief Equivalebt to glEnd. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnd.xml
	 \~japanese @brief glEnd と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnd.xml を参照。
	 */
	virtual void end() = 0;
	/**
	 \~english @brief Equivalebt to glPointSize. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPointSize.xml
	 \~japanese @brief glPointSize と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPointSize.xml を参照。
	 */
	virtual void point_size( double size ) = 0;
	/**
	 \~english @brief Equivalebt to glLineWidth. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLineWidth.xml
	 \~japanese @brief glLineWidth と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLineWidth.xml を参照。
	 */
	virtual void line_width( double width ) = 0;
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	template <class T> void vertex2v( const T *v ) {
		double v_3d_added[] = { v[0], v[1], 0.0 };
		vertex3v(v_3d_added);
	}
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	virtual void vertex3v( const double *v ) = 0;
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	void vertex3v( const float *v ) {
		double p[] = { v[0], v[1], v[2] };
		vertex3v(p);
	}
	/**
	 \~english @brief Draw a string at the current position.
	 @param[in] p Position.
	 @param[in] str String.
	 \~japanese @brief 現在の場所に文字を描く。
	 @param[in] p 位置。
	 @param[in] str 文字列。
	 */
	virtual void draw_string( const double *v, std::string str ) = 0;
};
//
SHKZ_END_NAMESPACE
//
#endif
