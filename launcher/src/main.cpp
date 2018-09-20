/*
**	main.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 31, 2017.
**
**	Shiokaze is free software; you can redistribute it and/or modify
**	it under the terms of the GNU General Public License Version 3
**	as published by the Free Software Foundation.
**
**	Shiokaze is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
//
#include <iostream>
#include <dlfcn.h>
#include <thread>
#include <shiokaze/core/filesystem.h>
//
SHKZ_USING_NAMESPACE
//
int main ( int argc, const char* argv[] ) {
	//
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	//
	std::thread dummy([](){}); dummy.join(); // Dummy code to enforce linking against thread
	const auto handle = ::dlopen(filesystem::find_libpath("shiokaze_ui").c_str(),RTLD_LAZY);
	int result (0);
	if( ! handle ) {
		std::cout << "Could not open the library: " << ::dlerror() << std::endl;
	} else {
		const auto run_func = ::dlsym(handle,"run");
		if( ! run_func ) {
			std::cout << "Could not load the function: " << ::dlerror() << std::endl;
		} else {
			const auto func = reinterpret_cast<int(*)(int argc, const char* argv[])>(run_func);
			result = func(argc,argv);
			if(::dlclose(handle)) {
				std::cout << "Could not close the handle: " << ::dlerror() << std::endl;
			}
		}
	}
	//
	return result;
}
//