/*
**	macexnbflip2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 27, 2017.
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
#ifndef SHKZ_MACEXNBFLIP2_H
#define SHKZ_MACEXNBFLIP2_H
//
#include <shiokaze/core/dylibloader.h>
#include "macnbflip2.h"
//
SHKZ_BEGIN_NAMESPACE
//
class macexnbflip2 : public macnbflip2 {
protected:
	//
	LONG_NAME("MAC Extended Narrowband FLIP 2D")
	//
	virtual void configure( configuration &config ) override;
	virtual void sizing_func( array2<double> &sizing_array, const bitarray2 &mask, const macarray2<double> &velocity, double dt ) override;
	//
	struct Parameters {
		double decay_rate {10.0};
		unsigned diffuse_count {4};
		double diffuse_rate {0.75};
		double threshold_u {0.2};
		double threshold_g {1.5};
		double radius {1.0};
		double amplification{5.0};
		int mode {0};
	};
	//
	Parameters m_param;
	//
	virtual void internal_sizing_func(array2<double> &sizing_array,
							const bitarray2 &mask,
							const array2<double> &solid,
							const array2<double> &fluid,
							const macarray2<double> &velocity,
							double dt);
};
//
SHKZ_END_NAMESPACE
//
#endif
