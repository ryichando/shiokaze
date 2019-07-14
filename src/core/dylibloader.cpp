/*
**	dylibloader.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 20, 2017. 
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
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/core/filesystem.h>
#include <shiokaze/core/console.h>
//
#include <iostream>
#include <cassert>
#include <dlfcn.h>
//
SHKZ_USING_NAMESPACE
//
dylibloader::dylibloader() {
	m_handle = nullptr;
}
//
dylibloader::~dylibloader() {
	close_library();
}
static std::string simplify( std::string path ) {
	return path.substr(path.find_last_of("/")+1);
}
//
bool dylibloader::open_library( std::string path ) {
	if( m_handle ) close_library();
	m_handle = ::dlopen(path.c_str(), RTLD_LAZY);
	if( ! m_handle ) {
		std::cout << "Could not open the file: path=" << path << " error=" << ::dlerror() << std::endl;
		exit(0);
	} else {
		m_path = path;
		console::dump("<Checkmark> Loaded <Green>\"%s\"<Default>", simplify(path).c_str());
		void* license_func = ::dlsym(m_handle,"license");
		if( license_func ) {
			console::dump( " <Light_Magenta>(%s)<Default>\n", reinterpret_cast<const char *(*)()>(license_func)());
		} else {
			console::dump("\n");
		}
	}
	return m_handle != nullptr;
}
//
void dylibloader::close_library() {
	if( m_handle ) {
		auto unload_func = reinterpret_cast<void(*)()>(load_symbol("unload"));
		if( unload_func ) {
			unload_func();
		}
		if( ::dlclose(m_handle)) {
			std::cout << "Could not close the handle: " << ::dlerror() << std::endl;
			exit(0);
		} else {
			m_handle = nullptr;
			console::dump("<Cross> Unloaded <Light_Red>\"%s\"<Default>\n", simplify(m_path).c_str());
		}
	}
	m_path = "";
}
//
void* dylibloader::load_symbol( std::string name ) const {
	if ( m_handle ) {
		return ::dlsym(m_handle,name.c_str());
	} else {
		return nullptr;
	}
}
//
void dylibloader::load( configuration &config ) {
	auto load_func = reinterpret_cast<void(*)(configuration &config)>(load_symbol("load"));
	if( load_func ) {
		load_func(config);
	}
}
//
void dylibloader::configure( configuration &config) {
	auto configure_func = reinterpret_cast<void(*)(configuration &config)>(load_symbol("configure"));
	if( configure_func ) {
		configure_func(config);
	}
}
//
void dylibloader::overwrite( configuration &config ) const {
	auto overwrite_func = reinterpret_cast<std::map<std::string,std::string>(*)()>(load_symbol("get_default_parameters"));
	if( overwrite_func ) {
		std::map<std::string,std::string> dictionary (config.get_dictionary());
		std::map<std::string,std::string> new_dictionary = (*overwrite_func)();
		for( auto it=new_dictionary.begin(); it!=new_dictionary.end(); ++it ) {
			if( dictionary.find(it->first) == dictionary.end()) {
				config.set_string(it->first,it->second);
			}
		}
	}
}
//
