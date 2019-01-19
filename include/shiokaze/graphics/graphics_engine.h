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
	 \~english @brief Initialize graphics engine with a background color.
	 @param[in] r Red channel.
	 @param[in] g Green channel.
	 @param[in] b Blue channel.
	 @param[in] a Alpha channel.
	 \~japanese @brief 背景色を指定して、グラフィックスエンジンを初期化する。
	 @param[in] r 赤色成分。
	 @param[in] g 緑色成分。
	 @param[in] b 青色成分。
	 @param[in] a アルファ成分。
	 */
	virtual void setup_graphics ( double r=0.0, double g=0.0, double b=0.0, double a=0.0 ) = 0;
	/**
	 \~english @brief Configure the perspective view.
	 @param[in] width Pixel coordinate width.
	 @param[in] height Pixel coordinate height.
	 @param[in] dim View dimension number.
	 \~japanese @brief パースペクティブの描画設定を行う。
	 @param[in] width ピクセル単位の横幅。
	 @param[in] height ピクセル単位の縦幅。
	 @param[in] dim 描画する空間の次元数。
	 */
	virtual void configure_view( unsigned width, unsigned height, unsigned dim ) = 0;
	/**
	 \~english @brief Clear out the canvas.
	 \~japanese @brief キャンバスをクリアする。
	 */
	virtual void clear() = 0;
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
	void color3v( const double *v ) {
		double v_plus_alpha[] = { v[0], v[1], v[2], 1.0 };
		color4v(v_plus_alpha);
	}
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	virtual void color4v( const double *v ) = 0;
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
	void vertex2v( const double *v ) {
		double v_3d_added[] = { v[0], v[1], 0.0 };
		vertex3v(v_3d_added);
	}
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	virtual void vertex3v( const double *v ) = 0;
	/**
	 \~english @brief Draw a string at the current position.
	 @param[in] p Position.
	 @param[in] str String.
	 \~japanese @brief 現在の場所に文字を描く。
	 @param[in] p 位置。
	 @param[in] str 文字列。
	 */
	virtual void draw_string( const double *v, std::string str ) const = 0;
};
//
SHKZ_END_NAMESPACE
//
#endif
