/*
**	filesystem.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 6, 2017.
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
#include <shiokaze/core/filesystem.h>
#include <boost/filesystem.hpp>
#include <iostream>
//
SHKZ_USING_NAMESPACE
//
bool filesystem::is_exist( std::string path ) {
	return boost::filesystem::exists(path);
}
//
bool filesystem::has_parent( std::string path ) {
	const auto p = boost::filesystem::path(path);
	return p.has_parent_path();
}
//
std::string filesystem::parent_path( std::string path ) {
	const auto p = boost::filesystem::path(path);
	return p.parent_path().string();
}
//
bool filesystem::has_root( std::string path ) {
	const auto p = boost::filesystem::path(path);
	return p.has_root_path();
}
//
bool filesystem::create_directory( std::string path ) {
	return boost::filesystem::create_directory(path);
}
//
bool filesystem::create_directories( std::string path ) {
	return boost::filesystem::create_directories(path);
}
//
void filesystem::rename( std::string old_path, std::string new_path ) {
	boost::filesystem::rename(old_path,new_path);
}
//
void filesystem::remove_file( std::string path ) {
	 boost::filesystem::remove(path);
}
//
void filesystem::remove_dir_contents( std::string path ) {
	 boost::filesystem::remove_all(path);
}
//
std::string filesystem::find_resource_path( std::string directory, std::string name ) {
	//
	std::string path = "./resources/"+ directory + "/" + name;
	if(filesystem::is_exist(path)) return path;
	else {
		std::string private_path = "./private-resources/"+ directory + "/" + name;
		if(filesystem::is_exist(private_path)) return private_path;
		else {
			std::cout << "Could not locate the resource path=" << directory << "/" << name << std::endl;
			exit(0);
		}
	}
	return std::string();
}
//
std::string filesystem::resolve_libname( std::string name ) {
	//
#ifdef __APPLE__
	static const std::string extension ("dylib");
#else
	static const std::string extension ("so");
#endif
	std::string libname = std::string("lib")+name+SHKZ_SUFFIX+"."+extension;
	return libname;
}