/*
**	shared_array_core2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 15, 2018.
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
#include <shiokaze/array/shared_array_core2.h>
#include <map>
#include <thread>
#include <vector>
//
SHKZ_USING_NAMESPACE
//
struct array_table2 {
	array_table2() = default;
	shape2 shape;
	size_t class_hash;
	std::string core_name;
	array_table2( const shape2 &shape, size_t class_hash, std::string core_name ) : shape(shape), class_hash(class_hash), core_name(core_name) {}
	bool operator<(const array_table2& rhs) const { 
		return hash() < rhs.hash();
	}
	bool operator==(const array_table2& rhs) const {
		return rhs.shape==shape && rhs.class_hash==class_hash && rhs.core_name==core_name;
	}
	size_t hash() const {
		return shape.hash() ^ class_hash ^ std::hash<std::string>{}(core_name);
	}
};
//
struct shared_array_data2 {
	shared_array_data2() = default;
	shared_array_data2( std::function<void(void *)> dealloc_func, array_table2 key ) : dealloc_func(dealloc_func), key(key) {}
	array_table2 key;
	std::vector<void *> arrays;
	std::function<void(void *)> dealloc_func;
	int being_borrowed {0};
};
//
static std::thread::id g_main_thread_id = std::this_thread::get_id();
static std::map<array_table2,shared_array_data2> g_array_map;
static std::map<void *,shared_array_data2 *> g_pointer_map;
//
static void thread_check () {
	if( g_main_thread_id != std::this_thread::get_id() ) {
		printf( "shared_array_core2: Calling from a multithread is not allowed.\n");
		exit(0);
	}
}
//
size_t shared_array_core2::get_total_grid_count () {
	//
	thread_check();
	//
	size_t count (0);
	for( auto &it : g_array_map ) {
		count += it.second.arrays.size();
	}
	return count;
}
//
void * shared_array_core2::borrow_shared( const shape2 &shape, size_t class_hash, std::string core_name, std::function<void *(const shape2 &shape, std::string core_name)> alloc_func, std::function<void( void *ptr )> dealloc_func ) {
	//
	thread_check();
	//
	const auto key = array_table2(shape,class_hash,core_name);
	const auto it = g_array_map.find(key);
	shared_array_data2 *container (nullptr);
	if( it == g_array_map.end()) {
		g_array_map[key] = shared_array_data2(dealloc_func,key);
		container = &g_array_map[key];
	} else {
		container = &it->second;
	}
	void *result (nullptr);
	if( container->arrays.size()) {
		result = container->arrays.back();
		container->arrays.pop_back();
		assert( g_pointer_map.find(result) == g_pointer_map.end());
		g_pointer_map[result] = container;
	} else {
		result = alloc_func(shape,core_name);
		g_pointer_map[result] = container;
	}
	container->being_borrowed ++;
	return result;
}
//
void shared_array_core2::return_shared( void *array ) {
	//
	thread_check();
	//
	const auto it = g_pointer_map.find(array);
	if( it == g_pointer_map.end()) {
		throw;
	} else {
		it->second->arrays.push_back(array);
		it->second->being_borrowed --;
		g_pointer_map.erase(it);
	}
}
//
size_t shared_array_core2::clear() {
	//
	thread_check();
	//
	size_t count (0);
	for( auto it=g_array_map.begin(); it!=g_array_map.end(); ) {
		size_t size = it->second.arrays.size();
		for( auto &e : it->second.arrays ) {
			it->second.dealloc_func(e);
		}
		it->second.arrays.clear();
		if( ! it->second.being_borrowed ) {
			it = g_array_map.erase(it);
			count += size;
		} else {
			++ it;
		}
	}
	return count;
}
//