/*
**	image_io.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 1, 2018.
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
#include <shiokaze/image/image_io_interface.h>
#include "lodepng/lodepng.h"
//
SHKZ_USING_NAMESPACE
//
class image_io : public image_io_interface {
public:
	//
	MODULE_NAME("image_io")
	//
	image_io() = default;
	//
protected:
	//
	virtual void set_image( unsigned width, unsigned height, const std::vector<unsigned char> &data ) override {
		size_t size = width * height * 4;
		m_data = data;
		m_width = width;
		m_height = height;
	}
	//
	virtual void get_image( unsigned &width, unsigned &height, std::vector<unsigned char> &data ) const override {
		//
		if( ! m_data.empty() ) {
			width = m_width;
			height = m_height;
			data = m_data;
			//
		} else {
			data.clear();
			width = height = 0;
		}
	}
	//
	// -- Regarding path extraction ---
	//
	// http://en.cppreference.com/w/cpp/string/basic_string/find_last_of
	// https://stackoverflow.com/questions/51949/how-to-get-file-extension-from-string-in-c
	//
	virtual bool write( std::string path ) const override {
		std::string file_extension = path.substr(path.find_last_of(".")+1);
		//
		if( file_extension == "png") {
			unsigned error = lodepng_encode32_file(path.c_str(),m_data.data(),m_width,m_height);
			if( ! error ) {
				return true;
			} else {
				printf( "Error: %s\n", lodepng_error_text(error));
				return false;
			}
		} else {
			printf( "Unsupported format %s\n", file_extension.c_str());
			return false;
		}
	}
	virtual bool read( std::string path ) override {
		//
		std::string file_extension = path.substr(path.find_last_of(".")+1);
		//
		if( file_extension == "png") {
			unsigned error = lodepng::decode(m_data,m_width,m_height,path.c_str());
			if( ! error ) {
				return true;
			} else {
				printf( "Error: %s\n", lodepng_error_text(error));
				return false;
			}
		} else {
			printf( "Unsupported format %s\n", file_extension.c_str());
			return false;
		}
	}
	//
private:
	//
	unsigned m_width;
	unsigned m_height;
	std::vector<unsigned char> m_data;
};
//
extern "C" module * create_instance() {
	return new image_io;
}
//
extern "C" const char *license() {
	return "zlib";
}
//