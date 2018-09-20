/*
**	sysstats.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 21, 2017. 
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
#include <shiokaze/system/sysstats_interface.h>
#include <shiokaze/core/cmdparser.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/filesystem.h>
#include <string>
//
SHKZ_USING_NAMESPACE
//
class sysstats : public sysstats_interface {
public:
	//
	virtual void report_stats() const override {
		console::dump("Arguments: %s\n", arg_str.c_str());
	}
	//
	virtual void plot_graph() const override {
		if( plot_template.size()) {
			scoped_timer timer{this};
			global_timer::pause();
			std::string record_path = console::get_root_path() + "/record";
			std::string record_image_path = console::get_root_path() + "/record/graph_images";
			if( ! filesystem::is_exist(record_image_path)) {
				filesystem::create_directory(record_image_path);
			}
			std::string plot_command = console::format_str(plot_template.c_str(), console::get_root_path().c_str());
			timer.tick(); console::dump( "Plotting graph (%s)...", plot_command.c_str());
			console::system(plot_command);
			console::dump( "Done.\n" );
			global_timer::resume();
		}
	}
	//
protected:
	//
	virtual void configure ( configuration &config ) override {
		cmdparser parser(config.get_dictionary());
		arg_str = parser.get_arg_string();
	#ifdef __APPLE__
		plot_template = "";
	#else
		plot_template = "cd %s/record; ./plot.sh > /dev/null 2>&1";
	#endif
		config.get_string("PlotTemplate",plot_template,"Plot command template");
	}
	//
private:
	std::string arg_str;
	std::string plot_template;
};
//
extern "C" module * create_instance() {
	return new sysstats();
}
//
extern "C" const char *license() {
	return "MIT";
}
//