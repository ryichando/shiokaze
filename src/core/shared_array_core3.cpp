/*
**	shared_array_core3.cpp
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
#include <shiokaze/array/shared_array_core3.h>
#include <map>
#include <mutex>
#include <vector>
//
SHKZ_USING_NAMESPACE
//
struct array_table3 {
	shape3 shape;
	size_t class_hash;
	std::string core_name;
	array_table3( const shape3 &shape, size_t class_hash, std::string core_name ) : shape(shape), class_hash(class_hash), core_name(core_name) {}
	bool operator<(const array_table3& rhs) const { 
		return hash() < rhs.hash();
	}
	bool operator==(const array_table3& rhs) const {
		return rhs.shape==shape && rhs.class_hash==class_hash && rhs.core_name==core_name;
	}
	size_t hash() const {
		return shape.hash() ^ class_hash ^ std::hash<std::string>{}(core_name);
	}
};
//
struct shared_array_data3 {
	shared_array_data3() = default;
	shared_array_data3( std::function<void(void *)> dealloc_func ) : dealloc_func(dealloc_func) {}
	std::vector<void *> arrays;
	std::function<void(void *)> dealloc_func;
};
//
static std::mutex shared_array3_mutex;
static std::map<array_table3,shared_array_data3> array_map;
static std::map<void *,shared_array_data3 *> pointer_map;
//
void * shared_array_core3::borrow_shared( const shape3 &shape, size_t class_hash, std::string core_name, std::function<void *(const shape3 &shape, std::string core_name)> alloc_func, std::function<void( void *ptr )> dealloc_func ) {
	//
	std::lock_guard<std::mutex> guard(shared_array3_mutex);
	const auto key = array_table3(shape,class_hash,core_name);
	const auto it = array_map.find(key);
	shared_array_data3 *container (nullptr);
	if( it == array_map.end()) {
		array_map[key] = shared_array_data3(dealloc_func);
		container = &array_map[key];
	} else {
		container = &it->second;
	}
	void *result (nullptr);
	if( container->arrays.size()) {
		result = container->arrays.back();
		container->arrays.pop_back();
		assert( pointer_map.find(result) == pointer_map.end());
		pointer_map[result] = container;
	} else {
		result = alloc_func(shape,core_name);
		pointer_map[result] = container;
	}
	return result;
}
//
void shared_array_core3::return_shared( void *array) {
	std::lock_guard<std::mutex> guard(shared_array3_mutex);
	const auto it = pointer_map.find(array);
	if( it == pointer_map.end()) {
		throw;
	} else {
		it->second->arrays.push_back(array);
		pointer_map.erase(it);
	}
}
//
void shared_array_core3::clear() {
	for( auto &it : array_map ) {
		for( auto &e : it.second.arrays ) {
			it.second.dealloc_func(e);
		}
		it.second.arrays.clear();
		it.second.dealloc_func = nullptr;
	}
	array_map.clear();
}
//