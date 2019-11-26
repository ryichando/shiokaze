/*
**	color.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 7, 2017.
**
**	Transformed from:
**	https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
**
*/
//
#ifndef SHKZ_COLOR_H
#define SHKZ_COLOR_H
//
#include <cmath>
#include <ostream>
#include <vector>
#include <shiokaze/core/common.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that converts color spaces.
/// \~japanese @brief 色空間を変換するクラス。
class color {
public:
	/**
	 \~english @brief Structre that defines RGB color.
	 \~japanese @brief RGB カラーを定義する構造体。
	 */
	struct rgb {
		/**
		 \~english @brief Red color (0.0-1.0).
	 	\~japanese @brief 赤色 (0.0-1.0)。
		 */
		double r;
		/**
		 \~english @brief Green color (0.0-1.0).
	 	\~japanese @brief 緑色 (0.0-1.0)。
		 */
		double g;
		/**
		 \~english @brief Blue color (0.0-1.0).
	 	\~japanese @brief 青色 (0.0-1.0)。
		 */
		double b;
	};
	/**
	 \~english @brief Structre that defines HSV color.
	 \~japanese @brief HSV カラーを定義する構造体。
	 */
	struct hsv {
		/**
		 \~english @brief Angle in degrees (0.0-360.0).
	 	\~japanese @brief 角度 (0.0-360.0)。
		 */
		double h;
		/**
		 \~english @brief Saturation (0.0-1.0).
	 	\~japanese @brief 彩度 (0.0-1.0)。
		 */
		double s;
		/**
		 \~english @brief Brightness (0.0-1.0).
	 	\~japanese @brief 明度 (0.0-1.0)。
		 */
		double v;
	};
	/**
	 \~english @brief Convert RGB to HSV.
	 @param[in] rgb RGB color.
	 @return HSV color.
	 \~japanese @brief RGB を HSV に変換する。
	 @param[in] rgb RGB カラー。
	 @return HSV カラー。
	 */
	static hsv rgb2hsv(rgb in) {
		hsv         out;
		double      min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min  < in.b ? min  : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max  > in.b ? max  : in.b;

		out.v = max;                                // v
		delta = max - min;
		if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
			out.s = (delta / max);                  // s
		} else {
			// if max is 0, then r = g = b = 0
			// s = 0, v is undefined
			out.s = 0.0;
			out.h = NAN;                            // its now undefined
			return out;
		}
		if( in.r >= max )                           // > is bogus, just keeps compilor happy
			out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
		else
			if( in.g >= max )
				out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
			else
				out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

		out.h *= 60.0;                              // degrees

		if( out.h < 0.0 )
			out.h += 360.0;

		return out;
	}
	/**
	 \~english @brief Convert HSV to RGB.
	 @param[in] hsv HSV color.
	 @return RGB color.
	 \~japanese @brief HSV を RGB に変換する。
	 @param[in] hsv HSVカラー。
	 @return RGB カラー。
	 */
	static rgb hsv2rgb(hsv in) {
		double      hh, p, q, t, ff;
		long        i;
		rgb         out;

		if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
			out.r = in.v;
			out.g = in.v;
			out.b = in.v;
			return out;
		}
		hh = in.h;
		if(hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = in.v * (1.0 - in.s);
		q = in.v * (1.0 - (in.s * ff));
		t = in.v * (1.0 - (in.s * (1.0 - ff)));

		switch(i) {
			case 0:
				out.r = in.v;
				out.g = t;
				out.b = p;
				break;
			case 1:
				out.r = q;
				out.g = in.v;
				out.b = p;
				break;
			case 2:
				out.r = p;
				out.g = in.v;
				out.b = t;
				break;

			case 3:
				out.r = p;
				out.g = q;
				out.b = in.v;
				break;
			case 4:
				out.r = t;
				out.g = p;
				out.b = in.v;
				break;
			case 5:
			default:
				out.r = in.v;
				out.g = p;
				out.b = q;
				break;
		}
		return out;
	}
	/**
	 \~english @brief Convert heat to RGB color.
	 @param[in] heat Heat.
	 @param[out] rgb_result RGB color output.
	 \~japanese @brief 熱を RGB カラーに変換する。
	 @param[in] heat 熱。
	 @param[out] rgb_result RGB カラーの出力。
	 */
	static void heatcolor( double heat, double rgb_result[3] ) {
		hsv hsv_color;
		heat = std::min(1.0,std::max(0.0,heat));
		hsv_color.h = 230.0 * (1.0-heat);
		hsv_color.s = 0.5;
		hsv_color.v = 0.8;
		rgb rgb_color = color::hsv2rgb(hsv_color);
		rgb_result[0] = rgb_color.r;
		rgb_result[1] = rgb_color.g;
		rgb_result[2] = rgb_color.b;
	}
};
//
SHKZ_END_NAMESPACE
#endif