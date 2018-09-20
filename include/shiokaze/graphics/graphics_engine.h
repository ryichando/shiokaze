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
	virtual void setup_graphics ( double r=0.0, double g=0.0, double b=0.0, double a=0.0 ) const = 0;
	/**
	 \~english @brief Equivalebt to glViewport. See https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glViewport.xml
	 \~japanese @brief glViewport と同じ。https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glViewport.xml を参照。
	 */
	virtual void viewport(unsigned x, unsigned y, unsigned width, unsigned height) const = 0;
	/**
	 \~english @brief Equivalebt to glOrtho. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml
	 \~japanese @brief glOrtho と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml を参照。
	 */
	virtual void ortho(double left, double right, double bottom, double top, double nearZ, double farZ) const = 0;
	//
	enum class BUFFER_TYPE { COLOR_BUFFER_BIT, DEPTH_BUFFER_BIT, STENCIL_BUFFER_BIT };
	/**
	 \~english @brief Equivalebt to glClear. See https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glClear.xml
	 \~japanese @brief glClear と同じ。https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glClear.xml を参照。
	 */
	virtual void clear ( BUFFER_TYPE type=BUFFER_TYPE::COLOR_BUFFER_BIT ) const = 0;
	/**
	 \~english @brief Equivalebt to glClearStencil. See https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glClearStencil.xml
	 \~japanese @brief glClearStencil と同じ。https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glClearStencil.xml を参照。
	 */
	virtual void clear_stencil( int s ) const = 0;
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
	virtual void configure_view( unsigned width, unsigned height, unsigned dim ) const = 0;
	/**
	 \~english @brief Temporarily set coordinate to 2D screen coordinate.
	 @param[in] width Pixel coordinate width.
	 @param[in] height Pixel coordinate height.
	 \~japanese @brief 座標系を二次元スクリーンスペースに一時的に設定する。
	 @param[in] width ピクセル単位の横幅。
	 @param[in] height ピクセル単位の縦幅。
	 */
	virtual void push_screen_coord ( unsigned width, unsigned height ) const = 0;
	/**
	 \~english @brief Unset the coordinate set by push_screen_coord and get back to the coordinate before calling push_screen_coord.
	 \~japanese @brief push_screen_coord で設定した座標系を解除し、push_screen_coord を呼ぶ前の座標系に戻す。
	 */
	virtual void pop_screen_coord () const = 0;
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	virtual void color3( double r, double g, double b ) const = 0;
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	virtual void color4( double r, double g, double b, double a ) const = 0;
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	virtual void color3v( const double *v ) const = 0;
	/**
	 \~english @brief Equivalebt to glColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml
	 \~japanese @brief glColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glColor.xml を参照。
	 */
	virtual void color4v( const double *v ) const = 0;
	/**
	 \~english @brief Equivalebt to glRasterPos. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml
	 \~japanese @brief glRasterPos と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml を参照。
	 */
	virtual void raster3( double x, double y, double z ) const = 0;
	/**
	 \~english @brief Equivalebt to glRasterPos. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml
	 \~japanese @brief glRasterPos と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml を参照。
	 */
	virtual void raster2( double x, double y ) const = 0;
	/**
	 \~english @brief Equivalebt to glRasterPos. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml
	 \~japanese @brief glRasterPos と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml を参照。
	 */
	virtual void raster3v( const double *v ) const = 0;
	/**
	 \~english @brief Equivalebt to glRasterPos. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml
	 \~japanese @brief glRasterPos と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glRasterPos.xml を参照。
	 */
	virtual void raster2v( const double *v ) const = 0;
	//
	enum class CAPABILITY { BLEND, DEPTH_TEST, STENCIL_TEST, COLOR_LOGIC_OP };
	/**
	 \~english @brief Equivalebt to glEnable. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnable.xml
	 \~japanese @brief glEnable と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnable.xml を参照。
	 */
	virtual void enable( CAPABILITY cap ) const = 0;
	/**
	 \~english @brief Equivalebt to glDisable. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glDisable.xml
	 \~japanese @brief glDisable と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glDisable.xml を参照。
	 */
	virtual void disable( CAPABILITY cap ) const = 0;
	//
	enum class MODE { POINTS, LINES, LINE_STRIP, LINE_LOOP, TRIANGLES, TRIANGLE_STRIP,
		TRIANGLE_FAN, QUADS, QUAD_STRIP, POLYGON };
	/**
	 \~english @brief Equivalebt to glBegin. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml
	 \~japanese @brief glBegin と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml を参照。
	 */
	virtual void begin( MODE mode ) const = 0;
	/**
	 \~english @brief Equivalebt to glEnd. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnd.xml
	 \~japanese @brief glEnd と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glEnd.xml を参照。
	 */
	virtual void end() const = 0;
	/**
	 \~english @brief Equivalebt to glPointSize. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPointSize.xml
	 \~japanese @brief glPointSize と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPointSize.xml を参照。
	 */
	virtual void point_size( double size ) const = 0;
	/**
	 \~english @brief Equivalebt to glLineWidth. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLineWidth.xml
	 \~japanese @brief glLineWidth と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLineWidth.xml を参照。
	 */
	virtual void line_width( double width ) const = 0;
	//
	enum class ACTION { KEEP, ZERO, REPLACE, INCR, INCR_WRAP, DECR, DECR_WRAP, INVERT };
	/**
	 \~english @brief Equivalebt to glStencilOp. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glStencilOp.xml
	 \~japanese @brief glStencilOp と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glStencilOp.xml を参照。
	 */
	virtual void stencil_op( ACTION fail, ACTION pass ) const = 0;
	//
	enum class FUNC { NEVER, LESS, LEQUAL, GREATER, GEQUAL, EQUAL, NOTEQUAL, ALWAYS };
	/**
	 \~english @brief Equivalebt to glStencilFunc. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glStencilFunc.xml
	 \~japanese @brief glStencilFunc と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glStencilFunc.xml を参照。
	 */
	virtual void stencil_func( FUNC func, int ref, unsigned mask ) const = 0;
	//
	enum class OPERATION { CLEAR, SET, COPY, COPY_INVERTED, NOOP, INVERT, 
		AND, NAND, OR, NOR, XOR, EQUIV, AND_REVERSE, AND_INVERTED, 
		OR_REVERSE, OR_INVERTED };
	virtual void logic_op( OPERATION op ) const = 0;
	//
	enum class FACTOR {	ZERO, ONE,
						SRC_COLOR, ONE_MINUS_SRC_COLOR,
						DST_COLOR, ONE_MINUS_DST_COLOR,
						SRC_ALPHA, ONE_MINUS_SRC_ALPHA,
						DST_ALPHA, ONE_MINUS_DST_ALPHA };
	/**
	 \~english @brief Equivalebt to glBlendFunc. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBlendFunc.xml
	 \~japanese @brief glBlendFunc と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBlendFunc.xml を参照。
	 */
	virtual void blend_func( FACTOR sfactor, FACTOR dfactor ) const = 0;
	/**
	 \~english @brief Equivalebt to glBlendColor. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBlendColor.xml
	 \~japanese @brief glBlendColor と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBlendColor.xml を参照。
	 */
	virtual void blend_color( double r, double g, double b, double a ) const = 0;
	//
	enum class MAT_MODE { MODELVIEW, PROJECTION };
	/**
	 \~english @brief Equivalebt to glMatrixMode. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMatrixMode.xml
	 \~japanese @brief glMatrixMode と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMatrixMode.xml を参照。
	 */
	virtual void matrix_mode( MAT_MODE mode ) const = 0;
	/**
	 \~english @brief Equivalebt to glLoadIdentity. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLoadIdentity.xml
	 \~japanese @brief glLoadIdentity と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glLoadIdentity.xml を参照。
	 */
	virtual void load_identity() const = 0;
	/**
	 \~english @brief Equivalebt to glPushMatrix. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml
	 \~japanese @brief glPushMatrix と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPushMatrix.xml を参照。
	 */
	virtual void push_matrix() const = 0;
	/**
	 \~english @brief Equivalebt to glPopMatrix. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPopMatrix.xml
	 \~japanese @brief glPopMatrix と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glPopMatrix.xml を参照。
	 */
	virtual void pop_matrix() const = 0;
	/**
	 \~english @brief Equivalebt to glTranslate. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml
	 \~japanese @brief glTranslate と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glTranslate.xml を参照。
	 */
	virtual void translate( double x, double y, double z ) const = 0;
	/**
	 \~english @brief Equivalebt to glScale. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glScale.xml
	 \~japanese @brief glScale と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glScale.xml を参照。
	 */
	virtual void scale( double x, double y, double z ) const = 0;
	/**
	 \~english @brief Equivalebt to glMultMatrix. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMultMatrix.xml
	 \~japanese @brief glMultMatrix と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glMultMatrix.xml を参照。
	 */
	virtual void multiply( const double *m ) const = 0;
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	virtual void vertex3( double x, double y, double z ) const = 0;
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	virtual void vertex2( double x, double y ) const = 0;
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	virtual void vertex3v( const double *v ) const = 0;
	/**
	 \~english @brief Equivalebt to glVertex. See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml
	 \~japanese @brief glVertex と同じ。https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glVertex.xml を参照。
	 */
	virtual void vertex2v( const double *v ) const = 0;
	/**
	 \~english @brief Draw a string at the current position.
	 @param[in] p Position.
	 @param[in] str String.
	 \~japanese @brief 現在の場所に文字を描く。
	 @param[in] p 位置。
	 @param[in] str 文字列。
	 */
	virtual void draw_string( const double *p, std::string str ) const = 0;
	/**
	 \~english @brief Get DPI factor of the monitor.
	 @return DPI factor.
	 \~japanese @brief モニターの DPI を取得する。
	 @return DPI ファクター。
	 */
	virtual double get_HiDPI_scaling_factor() const = 0;
	//
};
//
SHKZ_END_NAMESPACE
//
#endif
