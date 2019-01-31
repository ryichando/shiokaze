/*
**	ui.cpp
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
#include <stdio.h>
#include <dlfcn.h>
#include <cassert>
#include <algorithm>
#include <cmath>
//
#include "ui.h"
#include <thread>
//
#ifdef USE_OPENGL
#include "graphics_gl.h"
#include <shiokaze/graphics/graphics_utility.h>
#endif
//
#include <shiokaze/ui/drawable.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/cmdparser.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/system/sysstats_interface.h>
#include <shiokaze/image/image_io_interface.h>
#include <shiokaze/core/filesystem.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/shared_array3.h>
//
SHKZ_USING_NAMESPACE
//
static bool space_pressed (false), keyup_pressed (false), keydown_pressed (false);
static double camera_r (2.5);
static double camera_angle_xz (-0.65 * 3.1514);
static double camera_angle_y (1.0);
//
static vec3d camera_target, camera_position;
static void set_camera_pos() {
	camera_target = vec3d(0.5,0.35,0.5);
	camera_position = camera_target + vec3d(camera_r*cos(camera_angle_xz),camera_angle_y,camera_r*sin(camera_angle_xz));
}
#ifdef USE_OPENGL
//
static void error_callback(int error, const char* description) {
	::fprintf(stderr, "Error: %s\n", description);
}
//
static void key_callback(::GLFWwindow* window, int key, int scancode, int action, int mods) {
	//
	if( key == GLFW_KEY_SPACE  ) {
		space_pressed = action;
	} if( key == GLFW_KEY_UP  ) {
		keyup_pressed = action;
	} if( key == GLFW_KEY_DOWN  ) {
		keydown_pressed = action;
	} else {
		ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
		drawable *instance = g->getInstance();
		//
		if( action == GLFW_PRESS ) {
			if (key == GLFW_KEY_ESCAPE ) {
				::glfwSetWindowShouldClose(window,1);
			} else if( key == GLFW_KEY_SPACE ) {
				if( g->strong_pause_step == g->step ) g->strong_pause_step = 0;
				else instance->keyboard(key);
			} else {
				instance->keyboard(key);
			}
		}
	}
}
//
static void transform_coord_pos ( int width, int height, double *p ) {
	p[0] = p[0] / (double)width;
	p[1] = (1.0-p[1]/height) * (height / (double)width);
}
//
static void transform_coord_force ( int width, int height, double *u ) {
	double ratio = height / (double)width;
	u[1] = -u[1] * ratio;
}
//
static void cursor_position_callback(::GLFWwindow* window, double xpos, double ypos) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->getInstance();
	//
	g->mouse_p[0] = xpos * g->width / (double) g->w_width;
	g->mouse_p[1] = ypos * g->height / (double) g->w_height;
	//
	vec2d tranformed_pos (g->mouse_p);
	transform_coord_pos(g->width, g->height, tranformed_pos.v );
	instance->cursor(g->width, g->height, tranformed_pos[0], tranformed_pos[1]);
	//
	if( g->prev_mouse_p[0] && g->mouse_pressed ) {
		//
		g->force = g->mouse_p - g->prev_mouse_p;
		g->force_accumulation += g->force;
		g->mouse_accumulation ++;
		//
		double u = g->force_accumulation[0] / g->mouse_accumulation;
		double v = g->force_accumulation[1] / g->mouse_accumulation;
		//
		vec2d tranformed_force(u,v);
		transform_coord_force(g->width, g->height, tranformed_force.v );
		//
		instance->drag( g->width, g->height, tranformed_pos[0], tranformed_pos[1], tranformed_force[0], tranformed_force[1] );
		//
	}
	//
	g->prev_mouse_p = g->mouse_p;
}
//
static void mouse_button_callback(::GLFWwindow* window, int button, int action, int mods) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->getInstance();
	//
	vec2d tranformed_pos (g->mouse_p);
	transform_coord_pos(g->width, g->height, tranformed_pos.v );
	instance->mouse(g->width,g->height,tranformed_pos[0],tranformed_pos[1],button,action);
	//
	g->mouse_pressed = action;
	if( ! action ) {
		g->mouse_accumulation = 0;
		g->force_accumulation = vec2d();
	}
}
//
static void window_size_callback(::GLFWwindow* window, int _width, int _height) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->getInstance();
	graphics_engine *ge = g->getGraphicsEngine();
	//
	g->w_width = _width;
	g->w_height = _height;
	::glfwGetFramebufferSize(window,&g->width,&g->height);
	//
	instance->resize(*ge,g->width,g->height);
}
//
static void write_image( image_io_interface *image_io, std::string path, int width, int height ) {
	//
	std::vector<unsigned char> buffer_rgb(width*height*3);
	::glFlush();
	::glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	::glReadPixels( 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer_rgb.data() );
	//
	std::vector<unsigned char> buffer_rgba(width*height*4);
	for( int i=0; i<width; ++i ) for( int j=0; j<height; ++j ) {
		for( int c=0; c<3; ++c ) buffer_rgba[4*(i+(height-j-1)*width)+c] = buffer_rgb[3*(i+j*width)+c];
		buffer_rgba[4*(i+j*width)+3] = 255;
	}
	//
	image_io->set_image( width, height, buffer_rgba );
	image_io->write(path);
}
//
#endif
//
static module* alloc_module ( configuration &config ) {
	//
	std::string module_name ("demo-example");
	config.get_string("Target",module_name,"Target runnable module");
	//
	std::string path (module::module_libpath(module_name));
	module *instance = module::alloc_module(path);
	assert(instance);
	return instance;
}
//
ui::ui ( drawable *instance ) {	
	this->instance = instance;
	until = 0;
	strong_pause_step = 0;
	w_width = 600;
	w_height = 600;
	w_scale = 1.0;
	rotate_speed = 1.0;
	mouse_accumulation = 0;
	mouse_pressed = false;
	show_logo = true;
}
//
ui::~ui() {
	if( graphics_instance ) delete graphics_instance;
	graphics_instance = nullptr;
}
//
static std::string group_name ("User Interface");
static std::string argument_name ("UI");
//
void ui::load( configuration &config ) {
	configuration::auto_group group(config,group_name,argument_name);
#ifdef USE_OPENGL
	graphics_instance = new graphics_gl;
#else
	graphics_instance = nullptr;
#endif
	show_logo = ! instance->hide_logo();
	config.get_bool("ShowLogo",show_logo,"Whether to show logo");
	if( config.exist("Screenshot") || show_logo ) {
		image_io = image_io_interface::quick_load_module(config,"image_io");
	}
}
//
void ui::configure( configuration &config ) {
	//
	if( console::get_root_path().size()) {
		screenshot_path = console::get_root_path() + "/screenshot";
		if( ! filesystem::is_exist(screenshot_path)) filesystem::create_directory(screenshot_path);
	}
	if( image_io ) {
		image_io->recursive_configure(config);
	}
	//
	configuration::auto_group group(config,group_name,argument_name);
	config.get_string("Screenshot",screenshot_path,"Screnshot path");
	config.get_integer("RecordUntil",until,"Maximal screenshot frame to quit");
	config.get_integer("PauseStep", strong_pause_step,"Timestep to pause");
	config.get_double("WindowScale",w_scale,"Widnow size scale");
	config.get_double("RotationSpeed",rotate_speed,"View rotation speed");
	//
	if( screenshot_path.size()) {
		assert(filesystem::is_exist(screenshot_path));
	}
}
//
static void push_screen_coord ( unsigned width, unsigned height ) {
#if	USE_OPENGL
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);	
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0,width,height,0.0,-1.0,1.0);
#endif
}
//
static void pop_screen_coord () {
#if USE_OPENGL
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
#endif
}
//
void ui::run () {
	//
#if USE_OPENGL
	//
	// Make sure that the instance is not null
	assert( instance );
	assert( graphics_instance );
	//
	graphics_gl &ge = *static_cast<graphics_gl *>(graphics_instance);
	//
	// Set error callback
	::glfwSetErrorCallback(error_callback);
	//
	// Initialize the library
	if (!glfwInit()) {
		return;
	}
	//
	// Create a windowed mode window and its OpenGL context
	std::string window_name = instance->get_name();
	instance->setup_window(window_name,w_width,w_height);
	w_width *= w_scale;
	w_height *= w_scale;
	GLFWwindow *window = ::glfwCreateWindow(w_width, w_height, window_name.c_str(), nullptr, nullptr);
	if ( ! window ) {
		::glfwTerminate();
		return;
	}
	//
	// Get frame buffer size
	::glfwGetFramebufferSize(window,&width,&height);
	double dpi_scaling = width / (double) w_width;
	ge.setHiDPIScalingFactor(dpi_scaling);
	//
	// Set camera origin
	set_camera_pos();
	ge.set_camera(camera_target.v,camera_position.v);
	//
	// Enable multi sampling for nicer view
	::glfwWindowHint(GLFW_SAMPLES,4);
	//
	// Set pointer to this instance
	::glfwSetWindowUserPointer(window, this);
	//
	// Set keyboard callback
	::glfwSetKeyCallback(window, key_callback);
	//
	// Set cursor callback
	::glfwSetCursorPosCallback(window, cursor_position_callback);
	//
	// Set mouse callback
	::glfwSetMouseButtonCallback(window, mouse_button_callback);
	//
	// Set window resize callback
	::glfwSetWindowSizeCallback(window, window_size_callback);
	//
	// Make the window's context current
	::glfwMakeContextCurrent(window);
	//
	// Initialize graphics
	instance->setup_graphics(ge);
	//
	// Call resize event
	instance->resize(ge,width,height);
	//
	// Load logo
	GLuint texture;
	unsigned logo_width, logo_height;
	if( show_logo ) {
		//
		std::string image_path = filesystem::find_resource_path("ui","SHKZ_Logo.png");
		if( image_io && image_io->read(image_path) ) {
			std::vector<unsigned char> data;
			image_io->get_image( logo_width, logo_height, data );
			if( ! data.empty() ) {
				::glPixelStorei(GL_UNPACK_ALIGNMENT,1);
				::glGenTextures(1,&texture);
				::glBindTexture(GL_TEXTURE_2D,texture);
				::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				::glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,logo_width,logo_height,0,GL_RGBA,GL_UNSIGNED_BYTE,data.data());
				::glBindTexture(GL_TEXTURE_2D,0);
			} else {
				show_logo = false;
			}
		}
	}
	//
	// Loop until the user closes the window
	frame = 0;	step = 0;
	while (! ::glfwWindowShouldClose(window)) {
		//
		// Run idle
		bool running = instance->is_running();
		if( strong_pause_step && strong_pause_step == step ) {
			running = false;
		}
		if( running ) instance->idle();
		//
		// Call view change if that is the case
		bool refresh (false);
		double speed = rotate_speed * 0.05;
		if( keyup_pressed ) {
			camera_angle_y += 0.5 * speed;
			refresh = true;
		} else if ( keydown_pressed ) {
			camera_angle_y -= 0.5 * speed;
			refresh = true;
		} else if( space_pressed ) {
			camera_angle_xz += speed;
			refresh = true;
		}
		//
		if( refresh ) {
			set_camera_pos();
			ge.set_camera(camera_target.v,camera_position.v);
			instance->view_change(ge,width,height);
		}
		//
		// Call draw
		ge.clear();
		instance->draw(ge,width,height);
		//
		// Draw an arrow if mouse dragging
		if( mouse_accumulation ) {
			ge.color4(1.0,1.0,1.0,1.0);
			ge.line_width(2.0);
			double k = 10.0;
			push_screen_coord(width,height);
			graphics_utility::draw_arrow(ge,mouse_p.v,(mouse_p+k*force_accumulation/mouse_accumulation).v);
			pop_screen_coord();
			ge.line_width(1.0);
		}
		//
		// Draw logo
		if( show_logo ) {
			//
			vec2d sub_pos, sub_window, position;
			if( dpi_scaling == 1.0 ) {
				sub_pos = vec2d( 22 / (double) logo_width, 152 / (double) logo_height );
				sub_window = vec2d( 42 / (double ) logo_width, 22 / (double) logo_height );
				position = vec2d(width-sub_window[0]*logo_width-5,height-sub_window[1]*logo_height-5);
			} else {
				sub_pos = vec2d( 20 / (double) logo_width, 104 / (double) logo_height );
				sub_window = vec2d( 68 / (double) logo_width, 28 / (double) logo_height );
				position = vec2d(width-sub_window[0]*logo_width-10,height-sub_window[1]*logo_height-10);
			}
			//
			push_screen_coord(width,height);
			::glColor4d(0.0,0.0,0.0,0.5);
			::glBegin(GL_QUADS);
				::glVertex2i(position[0],position[1]);
				::glVertex2i(position[0],position[1]+sub_window[1]*logo_height);
				::glVertex2i(position[0]+sub_window[0]*logo_width,position[1]+sub_window[1]*logo_height);
				::glVertex2i(position[0]+sub_window[0]*logo_width,position[1]);
			::glEnd();
			::glColor4d(1.0,1.0,1.0,1.0);
			::glBindTexture(GL_TEXTURE_2D,texture);
			::glEnable(GL_TEXTURE_2D);
			::glBlendFunc(GL_ONE,GL_ONE);
			::glBegin(GL_QUADS);
				::glTexCoord2f(sub_pos[0],sub_pos[1]);
				::glVertex2i(position[0],position[1]);
				::glTexCoord2f(sub_pos[0],sub_pos[1]+sub_window[1]);
				::glVertex2i(position[0],position[1]+sub_window[1]*logo_height);
				::glTexCoord2f(sub_pos[0]+sub_window[0],sub_pos[1]+sub_window[1]);
				::glVertex2i(position[0]+sub_window[0]*logo_width,position[1]+sub_window[1]*logo_height);
				::glTexCoord2f(sub_pos[0]+sub_window[0],sub_pos[1]);
				::glVertex2i(position[0]+sub_window[0]*logo_width,position[1]);
			::glEnd();
			::glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			::glDisable(GL_TEXTURE_2D);
			::glBindTexture(GL_TEXTURE_2D,0);
			pop_screen_coord();
		}
		//
		if( ! running ) {
			//
			ge.color4(1.0,1.0,1.0,1.0);
			push_screen_coord(width,height);
			if( strong_pause_step && strong_pause_step == step ) {
				ge.draw_string(vec2d(10,height-10).v, "Not running. Press [space] to resume.");
			} else {
				ge.draw_string(vec2d(10,height-10).v, "Not running");
			}
			pop_screen_coord();
			//
		} else {
			//
			// Export screenshot if requested
			++ step;
			if( screenshot_path.size() && image_io ) {
				if( instance->should_screenshot()) {
					std::string path = screenshot_path + "/screenshot_" + std::to_string(frame++) + ".png";
					write_image(image_io.get(),path,width,height);
				}
				if( until && step >= until ) {
					console::dump("run \"avconv -r 60 -i screenshot_%d.png -pix_fmt yuv420p -b:v 12000k video.mp4\" to compile the video.\n");
					break;
				}
			}
		}
		//
		// Swap front and back buffers
		::glfwSwapBuffers(window);
		//
		// Poll for and process events
		::glfwPollEvents();
		//
		// Exit loop if requested
		if( instance->should_quit()) break;
	}
	//
	::glfwTerminate();
#endif
}
//
int ui::run ( int argc, const char* argv[] ) {
	//
	// Load module
	cmdparser parser(argc,argv);
	configuration &config = configurable::set_global_configuration(parser);
	//
	credit root_credit("Root","Root");
	config.push_group(group_name,argument_name);
	//
	// Check needs help
	bool help (false);
	for( int n=0; n<argc; ++n ) {
		std::string arg(argv[n]);
		std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
		if( arg == "help" ) {
			help = true;
			break;
		}
	}
	//
	// Set root path
	std::string path_to_log;
#ifndef USE_OPENGL
	path_to_log = "log";
#endif
	//
	config.get_string("Log",path_to_log,"Path to the directory to export log files");
	//
	if( ! help ) {
		if( path_to_log.size()) {
			// Some safety check
			if( path_to_log[0] == '/' ) {
				console::dump( "Absolute path \"%s\" not allowed.\n", path_to_log.c_str());
				exit(0);
			} else if( path_to_log.substr(0,2) == ".." ) {
				console::dump( "Parent path \"%s\" not allowed.\n", path_to_log.c_str());
				exit(0);
			} else {
				if( filesystem::is_exist(path_to_log)) filesystem::remove_dir_contents(path_to_log);
				filesystem::create_directory(path_to_log);
				console::set_root_path(path_to_log);
			}
		}
	}
	config.print_splash();
	//
	auto colored_str = []( std::string str ) {
		return "\e[95m"+str+"\e[39m";
	};
	console::dump( "   Arguments: %s\n", colored_str(parser.get_arg_string()).c_str());
	//
	using boost::posix_time::ptime;
	using boost::posix_time::second_clock;
	ptime now = second_clock::universal_time();
	console::dump( "   Date = \e[36m%s UTC\e[39m\n", boost::posix_time::to_simple_string(now).c_str());
	//
	std::string cpu_name;
	int cpu_trim (0);
#ifdef __APPLE__
	cpu_name = console::run("sysctl -n machdep.cpu.brand_string");
	cpu_trim = 0;
#else
	cpu_name = console::run("cat /proc/cpuinfo | grep 'model name' | uniq");
	cpu_trim = cpu_name.find(":")+2;
#endif
	cpu_name = cpu_name.substr(0,cpu_name.size()-1);
	//
	const char *compiler_name (nullptr);
#if defined(__clang__)
	compiler_name = "Clang";
#elif defined(__GNUC__)
	compiler_name = "GCC";
#elif defined(__ICC)
	compiler_name = "ICC";
#endif
	//
	console::dump( "   CPU = \e[36m%s\e[39m\n", cpu_name.size() > cpu_trim ? cpu_name.substr(cpu_trim,cpu_name.size()).c_str() : "(Unknown)");
	console::dump( "   OpenGL availability = %s\n", USE_OPENGL ? "Yes" : "No" );
	console::dump( "   %s version = \e[36m%d.%d.%d\e[39m\n",compiler_name, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ );
	console::dump( "   Build target = \e[36m%s\e[39m\n", SHKZ_BUILD_TARGET );
	console::dump( "   Available cores = \e[36m%d\e[39m\n", std::thread::hardware_concurrency() );
	//
	// Set whether we use OpenGL engine
	bool useOpenGL;
#ifdef USE_OPENGL
	useOpenGL = true;
#else
	useOpenGL = false;
#endif
	//
	configuration::print_bar("Loading Simulation");
	runnable *instance;
	drawable *drawable_instance;
	{
		configuration::auto_group group(config,root_credit);
		instance = dynamic_cast<runnable *>(alloc_module(config));
		assert(instance);
		instance->recursive_load(config);
		drawable_instance = dynamic_cast<drawable *>(instance);
	}
	//
	bool paused (false);
#ifdef USE_OPENGL
	if( drawable_instance ) {
		config.get_bool("OpenGL",useOpenGL,"Whether to use OpenGL visualizer");
		config.get_bool("Paused",paused,"Should pause on start");
	}
#endif
	//
	sysstats_ptr stats = sysstats_interface::quick_load_module(config,"sysstats");
	config.pop_group();
	instance->set_running( ! paused );
	//
	if( useOpenGL && drawable_instance ) {
		//
		ui userinterface(drawable_instance);
		userinterface.load(config);
		//
#ifdef USE_OPENGL
		::glutInit(&argc,(char **)argv);
#endif
		//
		userinterface.configure(config);
		stats->recursive_configure(config);
		{
			configuration::auto_group group(config,root_credit);
			instance->recursive_configure(config);
		}
		userinterface.configure(config);
		//
		if( help ) {
			config.print_help();
			exit(0);
		} else {
			config.print_variables();
		}
		//
		config.check_touched();
		stats->recursive_initialize();
		//
		configuration::auto_group group(config,root_credit);
		instance->recursive_initialize();
		userinterface.run();
		//
	} else {
		//
		stats->recursive_configure(config);
		{
			configuration::auto_group group(config,root_credit);
			instance->recursive_configure(config);
		}
		//
		if( help ) {
			config.print_help();
			exit(0);
		} else {
			config.print_variables();
		}
		config.check_touched();
		stats->recursive_initialize();
		//
		configuration::auto_group group(config,root_credit);
		instance->recursive_initialize();
		while( true ) {
			if( instance->is_running()) instance->idle();
			//
			stats->report_stats();
			stats->plot_graph();
			//
			if( instance->should_quit()) break;
		}
	}
	//
	delete instance;
	instance = nullptr;
	stats.reset();
	//
	shared_array_core2::clear();
	shared_array_core3::clear();
	//
	module::close_all_handles();
	configuration::print_bar("");
	//
	assert( ! module::count_open_modules());
	return 0;
}
//
extern "C" int run( int argc, const char* argv[] ) {
	return ui::run(argc,argv);
}
