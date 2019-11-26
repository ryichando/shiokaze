/*
**	graphics_utility.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 15, 2018.
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
#ifndef SHKZ_GRAPHICS_UTILITY_H
#define SHKZ_GRAPHICS_UTILITY_H
//
#include <cmath>
#include <shiokaze/graphics/graphics_engine.h>
#include <shiokaze/core/console.h>
#include <shiokaze/math/vec.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that provides various utility functions for graphics.
/// \~japanese @brief 描画のための様々なユーティリティ関数群を提供するクラス。
class graphics_utility {
private:
	using ge = graphics_engine;
public:
	/**
	 \~english @brief Draw a circle.
	 @param[in] g Graphics engine.
	 @param[in] p Pointer to position.
	 @param[in] r Radius.
	 @param[in] mode Drawing mode.
	 @param[in] num_v Number of vertices.
	 \~japanese @brief 円を描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] p 位置。
	 @param[in] r 半径。
	 @param[in] mode 描画モード。
	 @param[in] num_v 頂点数。
	 */
	template<class T> static void draw_circle ( graphics_engine &g, const T *p, double r, ge::MODE mode, unsigned num_v=20 ) {
		g.begin(mode);
		for( unsigned t=0; t<num_v; t++ ) {
			double theta = 2.0 * M_PI * t / (double)num_v;
			g.vertex2(p[0]+r*cos(theta),p[1]+r*sin(theta));
		}
		g.end();
	}
	/**
	 \~english @brief Draw an arrow.
	 @param[in] g Graphics engine.
	 @param[in] p0 Starting point.
	 @param[in] p1 End point.
	 \~japanese @brief 矢印を描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] p0 始点。
	 @param[in] p1 終点。
	 */
	template<class T> static void draw_arrow( graphics_engine &g, const T *p0, const T *p1 ) {
		double y_vec_x = p1[0]-p0[0];
		double y_vec_y = p1[1]-p0[1];
		double y_vec_len = hypot(y_vec_x,y_vec_y);
		if( y_vec_len ) {
			//
			y_vec_x /= y_vec_len;
			y_vec_y /= y_vec_len;
			//
			double x_vec_x = -y_vec_y;
			double x_vec_y = y_vec_x;
			double k = 0.25*y_vec_len;
			//
			g.begin(ge::MODE::LINES);
			g.vertex2v(p0);
			g.vertex2v(p1);
			g.end();
			//
			vec2d p2(p1[0]+k*0.8*x_vec_x-k*y_vec_x,p1[1]+k*0.8*x_vec_y-k*y_vec_y);
			vec2d p3(p1[0]-k*0.8*x_vec_x-k*y_vec_x,p1[1]-k*0.8*x_vec_y-k*y_vec_y);
			vec2d mid = (vec2d(p1[0],p1[1])+p2+p3)/3.0;
			g.begin(ge::MODE::TRIANGLES);
			g.vertex2v(p1);
			g.vertex2v(p2.v);
			g.vertex2v(mid.v);
			g.end();
			g.begin(ge::MODE::TRIANGLES);
			g.vertex2v(p1);
			g.vertex2v(p3.v);
			g.vertex2v(mid.v);
			g.end();
		}
	}
	/**
	 \~english @brief Draw a wired box of a unit size.
	 @param[in] g Graphics engine.
	 \~japanese @brief 単位サイズのワイヤーボックスを描く。
	 @param[in] g グラフィックスエンジン。
	 */
	static void draw_wired_box( graphics_engine &g, double scale=1.0 ) {
		const double p0[] = { 0.0, 0.0, 0.0 };
		const double p1[] = { scale, scale, scale };
		draw_wired_box(g,p0,p1);
	}
	/**
	 \~english @brief Draw a wired box.
	 @param[in] g Graphics engine.
	 @param[in] p0 Starting point.
	 @param[in] p1 End point.
	 \~japanese @brief ワイヤーボックスを描く。
	 @param[in] g グラフィックスエンジン。
	 @param[in] p0 始点。
	 @param[in] p1 終点。
	 */
	template<class T> static void draw_wired_box( graphics_engine &g, const T *p0, const T *p1 ) {
		//
		g.begin(ge::MODE::LINE_LOOP);
		g.vertex3(p0[0],p0[1],p0[2]);
		g.vertex3(p1[0],p0[1],p0[2]);
		g.vertex3(p1[0],p1[1],p0[2]);
		g.vertex3(p0[0],p1[1],p0[2]);
		g.end();
		//
		g.begin(ge::MODE::LINE_LOOP);
		g.vertex3(p0[0],p0[1],p1[2]);
		g.vertex3(p1[0],p0[1],p1[2]);
		g.vertex3(p1[0],p1[1],p1[2]);
		g.vertex3(p0[0],p1[1],p1[2]);
		g.end();
		//
		g.begin(ge::MODE::LINES);
		g.vertex3(p0[0],p0[1],p0[2]);
		g.vertex3(p0[0],p0[1],p1[2]);
		g.vertex3(p0[0],p1[1],p0[2]);
		g.vertex3(p0[0],p1[1],p1[2]);
		g.vertex3(p1[0],p1[1],p0[2]);
		g.vertex3(p1[0],p1[1],p1[2]);
		g.vertex3(p1[0],p0[1],p0[2]);
		g.vertex3(p1[0],p0[1],p1[2]);
		g.end();
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
//