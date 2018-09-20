/*
**	module.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 31, 2017. 
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
#include <shiokaze/core/module.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/filesystem.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>
#include <thread>
#include <vector>
#include <mutex>
//
SHKZ_USING_NAMESPACE
//
static std::mutex module_mutex;
struct record_set {
	//
	struct handle_record {
		void *handle;
		void *func;
		unsigned count;
		handle_record () = default;
	};
	//
	unsigned num_open;
	std::vector<void *> handles_to_be_closed;
	std::map<std::string,record_set::handle_record> handles_open;
	void *pointer_to_self;
	record_set () = default;
};
static record_set asset;
static std::string simplify( std::string path ) {
#if 0
	std::string simple_path (path);
	std::string remove_str0 ("symlink-");
	std::string remove_str1 ("lib/libshiokaze_");
	simple_path.replace(simple_path.find(remove_str0),remove_str0.length(),"");
	simple_path.replace(simple_path.find(remove_str1),remove_str1.length(),"");
	return simple_path;
#else
	std::string prefix ("libshiokaze_");
	return path.substr(path.find(prefix)+prefix.length());
#endif
}
//
module::module() {}
module::~module() {
	//
	std::lock_guard<std::mutex> guard(module_mutex);
	//
	auto it = asset.handles_open.find(m_path);
	if( it == asset.handles_open.end()) {
		console::dump( "Could not grab pointer to \"%s\"\n", m_path.c_str());
		exit(0);
	}
	auto &record = it->second;
	record.count --;
	if( ! record.count ) {
		auto handle = record.handle;
		console::dump("\u2718 Unload scheduled \e[91m\"%s\"\e[39m\n", simplify(m_path).c_str());
		asset.handles_to_be_closed.push_back(handle);
		asset.handles_open.erase(it);
	}
}
//
unsigned module::close_all_handles () {
	//
	unsigned num_closed (0);
	while( ! asset.handles_to_be_closed.empty()) {
		auto handles_to_be_closed_copy (asset.handles_to_be_closed);
		asset.handles_to_be_closed.clear();
		for( auto handle : handles_to_be_closed_copy ) {
			++ num_closed;
			if( ::dlclose(handle)) {
				console::dump( "Could not close the handle: %s\n", ::dlerror() );
				exit(0);
			} else {
				assert( asset.num_open );
				-- asset.num_open;
			}
		}
	}
	return num_closed;
}
//
std::string module::module_libpath( std::string module_name ) {
	return filesystem::find_libpath(std::string("shiokaze_")+module_name);
}
//
module * module::alloc_module( configuration &config, std::string arg_name, std::string default_module_name, std::string description ) {
	std::string name (default_module_name);
	auto pos = name.find(':');
	if( pos != std::string::npos ) {
		arg_name = name.substr(0,pos);
		name = name.substr(pos+1);
	}
	config.get_string(arg_name,name,description);
	module *result = module::alloc_module(module::module_libpath(name));
	if( ! result ) {
		console::dump("Could not load library %s\n",name.c_str());
		exit(-1);
	}
	return result;
}
//
module * module::alloc_module( std::string path ) {
	//
	module *module_instance = nullptr;
	void* handle = nullptr;
	void* func = nullptr;
	//
	module_mutex.lock();
	//
	// Make sure to close all the remaining handles to avoid confusion
	close_all_handles();
	//
	auto it = asset.handles_open.find(path);
	if( it != asset.handles_open.end()) {
		auto &record = it->second;
		handle = record.handle;
		func = record.func;
		record.count ++;
		//
	} else {
		handle = ::dlopen(path.c_str(), RTLD_LAZY);
		++ asset.num_open;
		if( ! handle ) {
			console::dump("Could not open the file: %s (%s)\n", path.c_str(), ::dlerror());
			exit(0);
		} else {
			func = ::dlsym(handle,"create_instance");
			if( ! func ) {
				console::dump("(%s) Could not load the function \"create_instance\": %s\n", path.c_str(), ::dlerror());
				throw;
			} else {
				//
				record_set::handle_record record;
				record.handle = handle;
				record.func = func;
				record.count = 1;
				asset.handles_open[path] = record;
				//
				console::dump("\u2714 Loaded \e[32m\"%s\"\e[39m", simplify(path).c_str());
				void* license_func = ::dlsym(handle,"license");
				if( license_func ) {
					console::dump( " \e[95m(%s)\e[39m\n", reinterpret_cast<const char *(*)()>(license_func)());
				} else {
					console::dump("\n");
				}
			}
		}
	}
	module_mutex.unlock();
	if( func ) {
		module_instance = reinterpret_cast<module *(*)()>(func)();
		module_instance->m_path = path;
	}
	return module_instance;
}
//
unsigned module::count_open_modules() {
	return asset.num_open;
}