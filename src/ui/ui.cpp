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
#include <shiokaze/image/image_io_interface.h>
#include <shiokaze/core/filesystem.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/shared_array3.h>
//
SHKZ_USING_NAMESPACE
//
#ifdef USE_OPENGL
//
static void error_callback(int error, const char* description) {
	::fprintf(stderr, "Error: %s\n", description);
}
//
static int convert_modifier( int mods ) {
	int result (0);
	if( mods & GLFW_MOD_SHIFT ) result |= UI_interface::MOD_SHIFT;
	if( mods & GLFW_MOD_CONTROL ) result |= UI_interface::MOD_CONTROL;
	if( mods & GLFW_MOD_ALT ) result |= UI_interface::MOD_ALT;
	if( mods & GLFW_MOD_SUPER ) result |= UI_interface::MOD_SUPER;
	if( mods & GLFW_MOD_CAPS_LOCK ) result |= UI_interface::MOD_CAPS_LOCK;
	if( mods & GLFW_MOD_NUM_LOCK ) result |= UI_interface::MOD_NUM_LOCK;
	return result;
}
//
static void key_callback(::GLFWwindow* window, int key, int scancode, int action, int mods) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->get_instance();
	//
	if ( key == GLFW_KEY_ESCAPE ) {
		::glfwSetWindowShouldClose(window,1);
	} else if( action == GLFW_PRESS && key == GLFW_KEY_SLASH ) {
		instance->set_running( ! instance->is_running());
	} else if( action == GLFW_PRESS && key == GLFW_KEY_R ) {
		instance->reinitialize();
	} else if( action == GLFW_PRESS && key == GLFW_KEY_PERIOD ) {
		instance->idle();
	} else {
		int a;
		if( action == GLFW_PRESS ) a = UI_interface::ACTION::PRESS;
		else if( action == GLFW_REPEAT ) a = UI_interface::ACTION::REPEAT;
		else if( action == GLFW_RELEASE ) a = UI_interface::ACTION::RELEASE;
		//
		UI_interface::event_structure event;
		event.type = UI_interface::event_structure::KEYBOARD;
		event.key = key;
		event.action = a;
		event.mods = convert_modifier(mods);
		instance->handle_event(event);
	}
}
//
static void cursor_position_callback(::GLFWwindow* window, double xpos, double ypos) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->get_instance();
	int width, window_width, height, window_height;
	::glfwGetWindowSize(window,&window_width,&window_height);
	::glfwGetFramebufferSize(window,&width,&height);
	Real scale_x = width / (Real)window_width;
	Real scale_y = height / (Real)window_height;
	Real x = scale_x*xpos;
	Real y = scale_x*ypos;
	g->m_mouse_pos = vec2d(x,y);
	//
	UI_interface::event_structure event;
	event.type = UI_interface::event_structure::CURSOR;
	event.x = x;
	event.y = y;
	instance->handle_event(event);
	//
	if( g->m_accumulation ) {
		vec2d f = (vec2d(x,y)-g->m_pos0) / (double)g->m_accumulation;
		UI_interface::event_structure event;
		event.type = UI_interface::event_structure::DRAG;
		event.x = x;
		event.y = y;
		event.u = f[0];
		event.v = f[1];
		instance->handle_event(event);
		++ g->m_accumulation;
	} else {
		g->m_pos0 = vec2d(x,y);
	}
}
//
static void mouse_scroll_callback(::GLFWwindow* window, double xoffset, double yoffset) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->get_instance();
	UI_interface::event_structure event;
	event.type = UI_interface::event_structure::SCROLL;
	event.x = xoffset;
	event.y = yoffset;
	instance->handle_event(event);
}
//
static void mouse_button_callback(::GLFWwindow* window, int button, int action, int mods) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->get_instance();
	int a;
	if( action == GLFW_PRESS ) a = UI_interface::ACTION::PRESS;
	else if( action == GLFW_RELEASE ) a = UI_interface::ACTION::RELEASE;
	//
	UI_interface::event_structure event;
	event.type = UI_interface::event_structure::MOUSE;
	event.x = g->m_mouse_pos[0];
	event.y = g->m_mouse_pos[1];
	event.button = button;
	event.action = a;
	event.mods = convert_modifier(mods);
	instance->handle_event(event);
	//
	if( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS ) {
		g->m_accumulation = 1;
	} else if( action == GLFW_RELEASE ) {
		g->m_accumulation = 0;
	}
}
//
static void window_size_callback(::GLFWwindow* window, int width, int height) {
	//
	ui *g = static_cast<ui *>(::glfwGetWindowUserPointer(window));
	drawable *instance = g->get_instance();
	::glfwGetFramebufferSize(window,&width,&height);
	//
	UI_interface::event_structure event;
	event.type = UI_interface::event_structure::RESIZE;
	event.width = width;
	event.height = height;
	instance->handle_event(event);
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
	//
	m_instance = instance;
	m_until = 0;
	m_window_scale = 1.0;
	m_show_logo = true;
}
//
ui::~ui() {
	if( m_graphics_instance ) delete m_graphics_instance;
	m_graphics_instance = nullptr;
}
//
static std::string group_name ("User Interface");
static std::string argument_name ("UI");
//
void ui::load( configuration &config ) {
	configuration::auto_group group(config,group_name,argument_name);
#ifdef USE_OPENGL
	m_graphics_instance = new graphics_gl;
#else
	m_graphics_instance = nullptr;
#endif
	m_show_logo = ! m_instance->hide_logo();
	config.get_bool("ShowLogo",m_show_logo,"Whether to show logo");
	if( config.exist("Screenshot") || m_show_logo ) {
		m_image_io = image_io_interface::quick_load_module(config,"image_io");
	}
}
//
void ui::configure( configuration &config ) {
	//
	if( console::get_root_path().size()) {
		m_screenshot_path = console::get_root_path() + "/screenshot";
		if( ! filesystem::is_exist(m_screenshot_path)) filesystem::create_directory(m_screenshot_path);
	}
	if( m_image_io ) {
		m_image_io->recursive_configure(config);
	}
	//
	configuration::auto_group group(config,group_name,argument_name);
	config.get_string("Screenshot",m_screenshot_path,"Screnshot path");
	config.get_string("ScreenshotLabel",m_screenshot_label,"Screnshot label");
	config.get_string("Legend",m_legend,"Legend annotation string");
	config.get_integer("RecordUntil",m_until,"Maximal screenshot frame to quit");
	config.get_double("WindowScale",m_window_scale,"Widnow size scale");
	config.get_bool("Paused",m_paused,"Paused on start");
	//
	if( m_screenshot_path.size()) {
		assert(filesystem::is_exist(m_screenshot_path));
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
	assert( m_instance );
	assert( m_graphics_instance );
	//
	graphics_gl &ge = *static_cast<graphics_gl *>(m_graphics_instance);
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
	int w_width (640);
	int w_height (400);
	//
	std::string window_name = m_instance->get_name();
	m_instance->setup_window(window_name,w_width,w_height);
	w_width *= m_window_scale;
	w_height *= m_window_scale;
	GLFWwindow *window = ::glfwCreateWindow(w_width,w_height,window_name.c_str(),nullptr,nullptr);
	if ( ! window ) {
		::glfwTerminate();
		return;
	}
	//
	// Get frame buffer size
	int width, height;
	::glfwGetFramebufferSize(window,&width,&height);
	double dpi_scaling = width / (double) w_width;
	ge.set_HiDPI_scaling_factor(dpi_scaling);
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
	// Set scroll callback
	::glfwSetScrollCallback(window, mouse_scroll_callback);
	//
	// Set window resize callback
	::glfwSetWindowSizeCallback(window, window_size_callback);
	//
	// Make the window's context current
	::glfwMakeContextCurrent(window);
	//
	// Initialize graphics
	ge.setup_graphics();
	//
	// Call resize event
	{
		UI_interface::event_structure event;
		event.type = UI_interface::event_structure::RESIZE;
		event.width = width;
		event.height = height;
		m_instance->handle_event(event);
	}
	//
	// Load logo
	GLuint texture;
	unsigned logo_width, logo_height;
	if( m_show_logo ) {
		//
		std::string image_path = filesystem::find_resource_path("ui","SHKZ_Logo.png");
		if( m_image_io && m_image_io->read(image_path) ) {
			std::vector<unsigned char> data;
			m_image_io->get_image( logo_width, logo_height, data );
			if( ! data.empty() ) {
				::glPixelStorei(GL_UNPACK_ALIGNMENT,1);
				::glGenTextures(1,&texture);
				::glBindTexture(GL_TEXTURE_2D,texture);
				::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				::glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				::glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,logo_width,logo_height,0,GL_RGBA,GL_UNSIGNED_BYTE,data.data());
				::glBindTexture(GL_TEXTURE_2D,0);
			} else {
				m_show_logo = false;
			}
		}
	}
	//
	m_instance->set_running(! m_paused);
	//
	// Loop until the user closes the window
	m_frame = 0; m_step = 0;
	UI_interface::CURSOR_TYPE current_cursor_type = UI_interface::ARROW_CURSOR;
	GLFWcursor* cursor (nullptr);
	while (! ::glfwWindowShouldClose(window)) {
		//
		// Change cursor if requested
		UI_interface::CURSOR_TYPE cursor_type = m_instance->get_current_cursor();
		if( cursor_type != current_cursor_type ) {
			current_cursor_type = cursor_type;
			::glfwDestroyCursor(cursor);
			if( cursor_type == UI_interface::CURSOR_TYPE::ARROW_CURSOR ) {
				cursor = nullptr;
			} else if( cursor_type == UI_interface::HAND_CURSOR ) {
				cursor = ::glfwCreateStandardCursor(GLFW_HAND_CURSOR);
			} else if( cursor_type == UI_interface::IBEAM_CURSOR ) {
				cursor = ::glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
			} else if( cursor_type == UI_interface::CROSSHAIR_CURSOR ) {
				cursor = ::glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
			} else if( cursor_type == UI_interface::HRESIZE_CURSOR ) {
				cursor = ::glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
			} else if( cursor_type == UI_interface::VRESIZE_CURSOR ) {
				cursor = ::glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
			}
			glfwSetCursor(window,cursor);
		}
		//
		// Run idle
		bool running = m_instance->is_running();
		if( running ) {
			m_instance->idle();
		}
		//
		// Call draw
		ge.clear();
		::glfwGetFramebufferSize(window,&width,&height);
		UI_interface::event_structure event;
		event.type = UI_interface::event_structure::DRAW;
		event.g = &ge;
		m_instance->handle_event(event);
		//
		// Draw logo
		if( m_show_logo ) {
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
		if( running ) {
			//
			if( ! m_legend.empty()) {
				ge.color4(1.0,1.0,1.0,1.0);
				push_screen_coord(width,height);
				ge.draw_string(vec2d(10,25).v, m_legend.c_str());
				pop_screen_coord();
			}
			//
			// Export screenshot if requested
			++ m_step;
			if( m_screenshot_path.size() && m_image_io ) {
				if( m_instance->should_screenshot()) {
					std::string path = m_screenshot_path + "/" + m_screenshot_label + "_" + std::to_string(m_frame++) + ".png";
					write_image(m_image_io.get(),path,width,height);
				}
				if( m_until && m_frame > m_until ) {
					console::dump("run \"avconv -r 60 -i %s_%%d.png -pix_fmt yuv420p -b:v 12000k video.mp4\" to compile the video.\n", m_screenshot_label.c_str());
					break;
				}
			}
		} else {
			ge.color4(1.0,1.0,1.0,1.0);
			push_screen_coord(width,height);
			ge.draw_string(vec2d(10,height-10).v, "Not running");
			pop_screen_coord();
		}
		//
		// Swap front and back buffers
		::glfwSwapBuffers(window);
		//
		// Poll for and process events
		::glfwPollEvents();
		//
		// Exit loop if requested
		if( m_instance->should_quit()) break;
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
	//
	using boost::posix_time::ptime;
	using boost::posix_time::second_clock;
	ptime now = second_clock::universal_time();
	//
#ifndef USE_OPENGL
	path_to_log = "log_" + boost::posix_time::to_iso_string(now);
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
		return "<Light_Magenta>"+str+"<Default>";
	};
	console::dump( "   Arguments: %s\n", colored_str(parser.get_arg_string()).c_str());
	//
	console::dump( "   Date = <Cyan>%s UTC<Default>\n", boost::posix_time::to_simple_string(now).c_str());
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
	bool has_display (false);
#ifdef USE_OPENGL
	has_display = true;
#endif
	//
	auto remove_fold_character = [&]( std::string &str ) { str.erase(std::remove(str.begin(),str.end(),'\n'),str.end()); };
	std::string git_version = console::run("git describe --tags --always"); remove_fold_character(git_version);
	std::string get_current_branch_name = console::run("git rev-parse --abbrev-ref HEAD"); remove_fold_character(get_current_branch_name);
	//
	console::dump( "   CPU = <Cyan>%s<Default>\n", cpu_name.size() > cpu_trim ? cpu_name.substr(cpu_trim,cpu_name.size()).c_str() : "(Unknown)");
	console::dump( "   Display availability = %s\n", has_display ? "Yes" : "No" );
	console::dump( "   %s version = <Cyan>%d.%d.%d<Default>\n",compiler_name, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ );
	console::dump( "   Build target = <Cyan>%s<Default>\n", SHKZ_BUILD_TARGET );
	console::dump( "   Available cores = <Cyan>%d<Default>\n", std::thread::hardware_concurrency() );
	console::dump( "   Git version = %s-%s\n", get_current_branch_name.c_str(),git_version.c_str());
	//
	// Set whether we use OpenGL engine
	bool use_OpenGL;
#ifdef USE_OPENGL
	use_OpenGL = true;
#else
	use_OpenGL = false;
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
	//bool paused (false);
#ifdef USE_OPENGL
	if( drawable_instance ) {
		config.get_bool("OpenGL",use_OpenGL,"Whether to use OpenGL visualizer");
	}
#endif
	//
	sysstats_ptr stats = sysstats_interface::quick_load_module(config,"sysstats");
	config.pop_group();
	//
	bool *has_graphical_flag = static_cast<bool *>(::dlsym(RTLD_DEFAULT,"g_shkz_has_graphical_interface"));
	assert( has_graphical_flag );
	//
	if( use_OpenGL && drawable_instance ) {
		//
		*has_graphical_flag = true;
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
		*has_graphical_flag = false;
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
			if( instance->is_running()) {
				instance->idle();
				stats->report_stats();
				stats->plot_graph();
			}
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
