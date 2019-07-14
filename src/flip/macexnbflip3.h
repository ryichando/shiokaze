/*
**	macexnbflip3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 28, 2017.
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
#ifndef SHKZ_MACEXNBFLIP3_H
#define SHKZ_MACEXNBFLIP3_H
//
#include "macnbflip3.h"
//
SHKZ_BEGIN_NAMESPACE
//
class macexnbflip3 : public macnbflip3 {
protected:
	//
	LONG_NAME("MAC Extended Narrowband FLIP 3D")
	MODULE_NAME("macexnbflip3")
	//
	virtual void configure( configuration &config ) override;
	virtual void compute_sizing_func( const array3<float> &fluid, const bitarray3 &mask, const macarray3<float> &velocity, array3<float> &sizing_array ) const override;
	//
	struct Parameters {
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
private:
	//
	virtual void internal_sizing_func(array3<float> &sizing_array,
							const bitarray3 &mask,
							const array3<float> &fluid,
							const macarray3<float> &velocity ) const;
};
//
SHKZ_END_NAMESPACE
//
#endif
