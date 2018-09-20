/*
**	dilate2.h
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
#ifndef SHKZ_DILATE2_H
#define SHKZ_DILATE2_H
//
#include <shiokaze/array/shape.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/ordering/ordering_core.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
class dilate2 {
public:
	//
	static std::vector<size_t> dilate( const shape2 &shape, unsigned char *bit_mask, size_t bit_mask_size, const parallel_driver *parallel=nullptr );
	static std::vector<size_t> dilate( const shape2 &shape, const ordering_core *ordering, const void *context, unsigned char *bit_mask, size_t bit_mask_size, const parallel_driver *parallel=nullptr );
};
//
SHKZ_END_NAMESPACE
//
#endif
//