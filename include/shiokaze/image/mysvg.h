/*
**	mysvg.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017. 
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
#ifndef	SHKZ_MYSVG_H
#define SHKZ_MYSVG_H
//
#include <shiokaze/math/vec.h>
#include <vector>
#include <cstdlib>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that handles writing SVG images.
/// \~japanese @brief SVG 形式の画像を書き出すクラス。
class mysvg {
public:
	/**
	 \~english @brief Write SVG header.
	 @param[in] fp Pointer to an instance of FILE.
	 \~japanese @brief SVG ヘッダーを書く。
	 @param[in] fp FILE インスタンスへのポインター。
	 */
	static void write_header( std::FILE *fp ) {
		std::fprintf( fp, "<?xml version=\"1.0\" standalone=\"no\"?>\n" );
		std::fprintf( fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n" );
		std::fprintf( fp, "<svg width=\"30cm\" height=\"30cm\" viewBox=\"0 0 1 1\"\n");
		std::fprintf( fp, "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n" );
	}
	/**
	 \~english @brief Write SVG footer.
	 @param[in] fp Pointer to an instance of FILE.
	 \~japanese @brief SVG フッターを書く。
	 @param[in] fp FILE インスタンスへのポインター。
	 */
	static void write_footer( std::FILE *fp ) {
		std::fprintf( fp, "</svg>\n" );
	}
	/**
	 \~english @brief Write a closed polygon.
	 @param[in] fp Pointer to an instance of FILE.
	 @param[in] polygon Polygon to write.
	 @param[in] color RGB color of the polygon.
	 \~japanese @brief 閉じたポリゴンを書く。
	 @param[in] fp FILE インスタンスへのポインター。
	 @param[in] polygon 書き出すポリゴン。
	 @param[in] color ポリゴンの RGB カラー。
	 */
	static void write_polygon( std::FILE *fp, const std::vector<vec2d> &polygon, double color[3] ) {
		unsigned rgb[3];
		for( unsigned n=0; n<3; ++n ) rgb[n] = 255.0*std::min(1.0,color[n]);
		std::fprintf(fp, "<polygon fill=\"rgb(%d,%d,%d)\" stroke=\"none\"\n", rgb[0], rgb[1], rgb[2]);
		std::fprintf(fp, "points= \"" );
		for( unsigned n=0; n<polygon.size(); ++n ) std::fprintf( fp, "%lf,%lf ", polygon[n][0], polygon[n][1] );
		std::fprintf(fp, "\"" );
		std::fprintf(fp, "/>\n");
	}
	/**
	 \~english @brief Write a line.
	 @param[in] fp Pointer to an instance of FILE.
	 @param[in] pos Line to write.
	 @param[in] color RGB color of the polygon.
	 @param[in] width Width of the line.
	 \~japanese @brief 閉じたポリゴンを書く。
	 @param[in] fp FILE インスタンスへのポインター。
	 @param[in] pos 線。
	 @param[in] color 線の RGB カラー。
	 @param[in] width 線の太さ。
	 */
	static void write_line( std::FILE *fp, vec2d pos[2], double color[3], double width ) {
		unsigned rgb[3];
		for( unsigned n=0; n<3; ++n ) rgb[n] = 255.0*std::min(1.0,color[n]);
		std::fprintf(fp, "<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" style=\"stroke:rgb(%d,%d,%d);stroke-width:%lf\"/>\n",
					 pos[0][0], pos[0][1], pos[1][0], pos[1][1], rgb[0], rgb[1], rgb[2], width );
	}
	/**
	 \~english @brief Write a circle.
	 @param[in] fp Pointer to an instance of FILE.
	 @param[in] pos Center of the circle.
	 @param[in] r Radius of the circle.
	 @param[in] line_color RGB color of the contour of the circle.
	 @param[in] fill_color Filling RGB color of the circle.
	 \~japanese @brief 閉じたポリゴンを書く。
	 @param[in] fp FILE インスタンスへのポインター。
	 @param[in] pos 円の中心。
	 @param[in] r 円の半径。
	 @param[in] line_color 輪郭の色。
	 @param[in] fill_color 塗る潰しの色。
	 */
	static void write_circle( std::FILE *fp, vec2d pos, double r, double width, double line_color[3], double fill_color[3] ) {
		unsigned line_rgb[3], fill_rgb[3];
		for( unsigned n=0; n<3; ++n ) line_rgb[n] = 255.0*std::min(1.0,line_color[n]);
		for( unsigned n=0; n<3; ++n ) fill_rgb[n] = 255.0*std::min(1.0,fill_color[n]);
		std::fprintf(fp,"<circle cx=\"%lf\" cy=\"%lf\" r=\"%lf\" stroke=\"rgb(%d,%d,%d)\" stroke-width=\"%lf\" fill=\"rgb(%d,%d,%d)\" />\n",
					 pos[0], pos[1], r, line_rgb[0], line_rgb[1], line_rgb[2], width, fill_rgb[0], fill_rgb[1], fill_rgb[2] );
	}
};
//
SHKZ_END_NAMESPACE
//
#endif
