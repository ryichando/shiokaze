/*
**	image_io_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on February 1, 2018.
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
#ifndef SHKZ_IMAGE_IO_INTERFACE_H
#define SHKZ_IMAGE_IO_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface that handles image writing and reading.
/// \~japanese @brief 画像の書き込みと読み込みを行うインターフェース。
class image_io_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(image_io_interface,"Image IO","ImageIO","RGB Image loader and writer module")
	/**
	 \~english @brief Set image.
	 @param[in] width Width of the image.
	 @param[in] height Height of the image.
	 @param[in] data Data of the image.
	 \~japanese @brief 画像をセットする。
	 @param[in] width 画像の横幅。
	 @param[in] height 画像の縦幅。
	 @param[in] data 画像のデータ。
	 */
	virtual void set_image( unsigned width, unsigned height, const std::vector<unsigned char> &data ) = 0;
	/**
	 \~english @brief Get image.
	 @param[out] width Width of the image.
	 @param[out] height Height of the image.
	 @param[out] data Data of the image.
	 \~japanese @brief 画像を取得する。
	 @param[out] width 画像の横幅。
	 @param[out] height 画像の縦幅。
	 @param[out] data 画像のデータ。
	 */
	virtual void get_image( unsigned &width, unsigned &height, std::vector<unsigned char> &data ) const = 0;
	/**
	 \~english @brief Write the currently set image to a file.
	 @param[in] path Path to the file.
	 \~japanese @brief 現在セットされている画像をファイルに書き込む。
	 @param[in] path ファイルへのパス。
	 */
	virtual bool write( std::string path ) const = 0;
	/**
	 \~english @brief Read image file and set.
	 @param[in] path Path to the file.
	 \~japanese @brief 画像ファイルを読み込んで、セットする。
	 @param[in] path ファイルへのパス。
	 */
	virtual bool read( std::string path ) = 0;
	//
};
//
using image_io_ptr = std::unique_ptr<image_io_interface>;
using image_io_driver = recursive_configurable_driver<image_io_interface>;
//
SHKZ_END_NAMESPACE
//
#endif
