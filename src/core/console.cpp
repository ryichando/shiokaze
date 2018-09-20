/*
**	console.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 16, 2018.
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
#include <shiokaze/core/console.h>
#include <shiokaze/timestepper/timestepper_interface.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/filesystem.h>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <mutex>
//
SHKZ_BEGIN_NAMESPACE
//
#define MAX_BUFFER_SIZE		512
//
static std::mutex g_console_mutex;
static std::string g_root_path;
static char g_buffer[MAX_BUFFER_SIZE];
static int g_nestLevel (0);
static double g_time {0.0};
//
namespace console {
	//
	std::string run( std::string format, ...) {
		va_list args;
		va_start(args,format);
		vsnprintf(g_buffer,MAX_BUFFER_SIZE,format.c_str(),args);
		va_end(args);
		FILE* pipe = popen(g_buffer, "r");
		if (!pipe) return "ERROR";
		std::string result;
		while(!feof(pipe)) {
			if(fgets(g_buffer,MAX_BUFFER_SIZE,pipe) != nullptr) {
				result += g_buffer;
			}
		}
		pclose(pipe);
		return result;
	}
	//
	int system( std::string format, ...) {
		va_list args;
		va_start(args,format);
		vsnprintf(g_buffer,MAX_BUFFER_SIZE,format.c_str(),args);
		va_end(args);
		return ::system(g_buffer);
	}
	//
	std::string tstr(double msec) {
		if( msec < 1000.0 ) {
			// Milli seconds
			snprintf(g_buffer,MAX_BUFFER_SIZE,"%.2f msec",msec);
		} else if( msec < 1000.0 * 60 * 3 ) {
			// Seconds
			snprintf(g_buffer,MAX_BUFFER_SIZE,"%.3f sec",msec/1000.0);
		} else if( msec < 1000.0 * 60 * 60 * 3 ) {
			// Minutes
			snprintf(g_buffer,MAX_BUFFER_SIZE,"%.3f minutes",msec/(60.0*1000.0));
		} else if( msec < 1000.0 * 60 * 60 * 60 * 3 ) {
			// Hours
			snprintf(g_buffer,MAX_BUFFER_SIZE,"%.3f hours",msec/(60*60.0*1000.0));
		} else {
			// Days
			snprintf(g_buffer,MAX_BUFFER_SIZE,"%.3f days",msec/(60*60.0*24*1000.0));
		}
		return g_buffer;
	}
	//
	std::string nth(int num) {
		switch(num) {
			case 0:
				snprintf(g_buffer,MAX_BUFFER_SIZE,"0th");
				break;
			case 1:
				snprintf(g_buffer,MAX_BUFFER_SIZE,"1st");
				break;
			case 2:
				snprintf(g_buffer,MAX_BUFFER_SIZE,"2nd");
				break;
			case 3:
				snprintf(g_buffer,MAX_BUFFER_SIZE,"3rd");
				break;
			default:
				snprintf(g_buffer,MAX_BUFFER_SIZE,"%dth",num);
				break;
		}
		return g_buffer;
	}
	//
	std::string size_str( size_t bytes ) {
		g_buffer[0]=0;
		if( bytes < 1024 ) {
			snprintf(g_buffer,MAX_BUFFER_SIZE,"%d bytes",(int)bytes);
		} else {
			double kbytes = bytes/1024;
			if( kbytes < 1024 ) {
				snprintf(g_buffer,MAX_BUFFER_SIZE,"%.2f KB",kbytes);
			} else {
				double mbytes = kbytes/1024.0;
				if( mbytes < 1024 ) {
					snprintf(g_buffer,MAX_BUFFER_SIZE,"%.2f MB",mbytes);
				} else {
					double gbytes = mbytes/1024.0;
					snprintf(g_buffer,MAX_BUFFER_SIZE,"%.2f GB",gbytes);
				}
			}
		}
		return g_buffer;
	}
	//
	std::string format_str( std::string format, ...) {
		va_list args;
		va_start(args,format);
		vsnprintf(g_buffer,MAX_BUFFER_SIZE,format.c_str(),args);
		va_end(args);
		return g_buffer;
	}
	//
	static void inc_nest() {
		++ g_nestLevel;
		if( g_nestLevel > 5 ) {
			printf( "Too nested dump !\n" );
			throw;
		}
	}
	//
	static void dec_nest() {
		-- g_nestLevel;
		if( g_nestLevel < 0 ) {
			printf( "Nest level broken !\n" );
			throw;
		}
	}
	//
	void set_root_path( std::string path ) {
		g_root_path = path;
	}
	//
	std::string get_root_path() {
		return g_root_path;
	}
	//
	void dump(std::string format, ...) {
		//
		static bool ended_with_return (false);
		std::lock_guard<std::mutex> guard(g_console_mutex);
		global_timer::pause();
		//
		unsigned len = format.size();
		va_list args;
		if( len > 2 ) {
			if( format[0]=='<' && format[1]=='<' && format[2]=='<' ) {
				dec_nest();
			}
		}
		if( ! g_root_path.empty() ) {
			FILE *console = fopen((g_root_path+"/console.out").c_str(),"a");
			if( console ) {
				if( ended_with_return ) {
					for( unsigned i=0; i<g_nestLevel; i++ ) fprintf(console,"   ");
				}
				va_start(args, format);
				vfprintf(console, format.c_str(), args);
				va_end(args);
				fclose(console);
			} else {
				printf("Could not open the root dir %s\n", g_root_path.c_str());
				throw;
			}
		}
		if( ended_with_return ) {
			for( unsigned i=0; i<g_nestLevel; i++ ) printf("   ");
		}
		va_start(args,format);
		vprintf(format.c_str(),args);
		va_end(args);
		ended_with_return = format[len-1] == '\n';
		if( len > 2 ) {
			if( format[0]=='>' && format[1]=='>' && format[2]=='>' ) {
				inc_nest();
			}
		}
		global_timer::resume();
	}
	//
	void set_time ( double time ) {
		g_time = time;
	}
	void write( std::string name, double number ) {
		if( ! g_root_path.empty()) {
			std::lock_guard<std::mutex> guard(g_console_mutex);
			static bool firstTime = true;
			std::string record_path = g_root_path+"/record";
			if( firstTime ) {
				if( ! filesystem::is_exist(record_path)) filesystem::create_directory(record_path);
				system("cp scripts/plot.sh %s/plot.sh",record_path.c_str());
				firstTime = false;
			}
			if( ! g_root_path.empty() ) {
				FILE *console = fopen(format_str("%s/%s.out",record_path.c_str(),name.c_str()).c_str(),"a");
				if( console ) {
					if( number == (int)number ) {
						fprintf( console, "%lf %d\n", g_time, (int)number );
					} else {
						fprintf( console, "%lf %e\n", g_time, number );
					}
					fclose(console);
				}
			}
		}
	}
	//
}
//
SHKZ_END_NAMESPACE
//
