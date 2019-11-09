/*
**	macsmoke3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017. 
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
#ifndef SHKZ_MACSMOKE3_H
#define SHKZ_MACSMOKE3_H
//
#include <shiokaze/array/array3.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/ui/drawable.h>
#include <shiokaze/advection/macadvection3_interface.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/utility/macstats3_interface.h>
#include <shiokaze/visualizer/gridvisualizer3_interface.h>
#include <shiokaze/visualizer/macvisualizer3_interface.h>
#include <shiokaze/projection/macproject3_interface.h>
#include <shiokaze/timestepper/timestepper_interface.h>
#include <shiokaze/utility/graphplotter_interface.h>
#include <shiokaze/core/dylibloader.h>
//
SHKZ_BEGIN_NAMESPACE
//
class macsmoke3 : public drawable {
public:
	//
	macsmoke3();
	LONG_NAME("MAC Smoke 3D")
	MODULE_NAME("macsmoke3")
	ARGUMENT_NAME("Smoke")
	//
protected:
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override;
	virtual void drag( double x, double y, double z, double u, double v, double w ) override;
	virtual void idle() override;
	virtual void draw( graphics_engine &g ) const override;
	virtual bool should_quit() const override { return m_timestepper->should_quit(); }
	virtual bool should_screenshot() const override { return m_timestepper->should_export_frame(); }
	virtual void load( configuration &config ) override;
	virtual void configure( configuration &config ) override;
	virtual void post_initialize() override;
	//
	macarray3<float> m_velocity{this};
	macarray3<float> m_external_force{this};
	array3<float> m_density{this};
	array3<float> m_accumulation{this};
	//
	array3<float> m_fluid{this};
	array3<float> m_solid{this};
	//
	std::vector<vec3d> m_dust_particles;
	//
	shape3 m_shape;
	double m_dx;
	bool m_force_exist;
	unsigned m_graph_id;
	//
	struct Parameters {
		bool mouse_interaction {false};
		bool use_dust {false};
		double minimal_density {0.01};
		unsigned r_sample {4};
		bool show_graph {false};
		unsigned extrapolated_width {3};
		double buoyancy_factor {2.0};
		bool render_density {false};
		unsigned render_sample_count {128};
		double volume_scale {40.0};
	};
	//
	Parameters m_param;
	//
	environment_setter arg_shape{this,"shape",&m_shape};
	environment_setter arg_dx{this,"dx",&m_dx};
	//
	macproject3_driver m_macproject{this,"macpressuresolver3"};
	macadvection3_driver m_macadvection{this,"macadvection3"};
	gridvisualizer3_driver m_gridvisualizer{this,"gridvisualizer3"};
	graphplotter_driver m_graphplotter{this,"graphplotter"};
	macstats3_driver m_macstats{this,"macstats3"};
	macvisualizer3_driver m_macvisualizer{this,"macvisualizer3"};
	timestepper_driver m_timestepper{this,"timestepper"};
	macutility3_driver m_macutility{this,"macutility3"};
	//
	parallel_driver m_parallel{this};
	dylibloader m_dylib;
	//
	virtual void inject_external_force( macarray3<float> &velocity );
	virtual void add_buoyancy_force( macarray3<float> &velocity, const array3<float> &density, double dt );
	virtual void advect_dust_particles( const macarray3<float> &velocity, double dt );
	virtual void add_source ( macarray3<float> &velocity, array3<float> &density, double time, double dt );
	virtual void rasterize_dust_particles( array3<float> &rasterized_density );
	virtual void draw_dust_particles( graphics_engine &g ) const;
	virtual void export_density () const;
	virtual void do_export_density( int frame ) const;
	virtual void add_to_graph();
	virtual void render_density( int frame ) const;
	//
};
//
SHKZ_END_NAMESPACE
//
#endif