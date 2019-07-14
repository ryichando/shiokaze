/*
**	environment.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 6, 2017. 
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
#include <shiokaze/system/environment_interface.h>
#include <shiokaze/core/console.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
//
using boost::posix_time::ptime;
using boost::posix_time::second_clock;
//
SHKZ_USING_NAMESPACE
//
static ::std::string run( const char *format, ...) {
	char command[512];
	va_list args;
	va_start(args, format);
	vsnprintf(command,512,format,args);
	va_end(args);
	//
	FILE* pipe = popen(command, "r");
	if (!pipe) return "ERROR";
	char buffer[128];
	//
	::std::string result = "";
	while(!feof(pipe)) {
		if(fgets(buffer,128,pipe) != nullptr) {
			result += buffer;
		}
	}
	fflush(pipe);
	pclose(pipe);
	return result;
}
//
class environment : public environment_interface {
public:
	//
	MODULE_NAME("environment")
	//
	virtual std::string today_UTC() const override {
		ptime now = second_clock::universal_time();
		return boost::posix_time::to_simple_string(now);
	}
	virtual std::string cpu_name() const override {
		std::string cpu_name;
		int cpu_trim (0);
	#ifdef __APPLE__
		cpu_name = run("sysctl -n machdep.cpu.brand_string");
		cpu_trim = 0;
	#else
		cpu_name = run("cat /proc/cpuinfo | grep 'model name' | uniq");
		cpu_trim = cpu_name.find(":")+2;
	#endif
		cpu_name = cpu_name.substr(0,cpu_name.size()-1);
		return cpu_name.size() > cpu_trim ? cpu_name.substr(cpu_trim,cpu_name.size()) : "(Unknown)";
	}
	virtual std::string get_gcc_version() const override {
		std::string version_str;
		version_str += std::to_string(__GNUC__) + ".";
		version_str += std::to_string(__GNUC_MINOR__) + ".";
		version_str += std::to_string(__GNUC_PATCHLEVEL__);
		return version_str;
	}
	virtual std::string get_git_revnumber() const override {
		return console::run("git describe --tags");
	}
	virtual unsigned get_num_threads() const override {
		return std::thread::hardware_concurrency();
	}
	//
};
//
extern "C" module * create_instance() {
	return new environment();
}
//
extern "C" const char *license() {
	return "MIT";
}
