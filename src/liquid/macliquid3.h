/*
**	macliquid3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 18, 2017. 
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
#ifndef SHKZ_MACLIQUID3_H
#define SHKZ_MACLIQUID3_H
//
#include <shiokaze/array/array3.h>
#include <shiokaze/array/macarray3.h>
#include <shiokaze/advection/macadvection3_interface.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/utility/macutility3_interface.h>
#include <shiokaze/utility/macstats3_interface.h>
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <shiokaze/meshexporter/meshexporter3_interface.h>
#include <shiokaze/visualizer/gridvisualizer3_interface.h>
#include <shiokaze/visualizer/macvisualizer3_interface.h>
#include <shiokaze/projection/macproject3_interface.h>
#include <shiokaze/surfacetracker/maclevelsetsurfacetracker3_interface.h>
#include <shiokaze/ui/drawable.h>
#include <shiokaze/timestepper/timestepper_interface.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/rigidbody/rigidworld3_interface.h>
#include <shiokaze/utility/graphplotter_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class macliquid3 : public drawable {
public:
	//
	LONG_NAME("MAC Liquid 3D")
	ARGUMENT_NAME("Liquid")
	//
	macliquid3();
	//
protected:
	//
	virtual void drag( double x, double y, double z, double u, double v, double w ) override;
	virtual void setup_window( std::string &name, int &width, int &height ) const override;
	virtual void idle() override;
	virtual void draw( graphics_engine &g ) const override;
	virtual bool should_quit() const override { return m_timestepper->should_quit(); }
	virtual bool should_screenshot() const override { return m_timestepper->should_export_frame(); }
	//
	virtual void load( configuration &config ) override;
	virtual void configure( configuration &config ) override;
	virtual void post_initialize() override;
	//
	macarray3<Real> m_velocity{this};
	macarray3<Real> m_external_force{this};
	array3<Real> m_fluid{this};
	array3<Real> m_solid{this};
	//
	shape3 m_shape;
	double m_dx;
	//
	shape3 m_doubled_shape;
	double m_half_dx;
	//
	bool m_force_exist;
	double m_initial_volume;
	unsigned m_prev_frame;
	unsigned m_graph_lists[4];
	//
	environment_setter arg_shape{this,"shape",&m_shape};
	environment_setter arg_dx{this,"dx",&m_dx};
	//
	macproject3_driver m_macproject{this,"macpressuresolver3"};
	macadvection3_driver m_macadvection{this,"macadvection3"};
	macsurfacetracker3_driver m_macsurfacetracker{this,"maclevelsetsurfacetracker3"};
	cellmesher3_driver m_mesher{this,"marchingcubes"};
	cellmesher3_driver m_highres_mesher{this,"marchingcubes"};
	meshexporter3_driver m_mesh_exporter{this,"meshexporter3"};
	timestepper_driver m_timestepper{this,"timestepper"};
	gridutility3_driver m_gridutility{this,"gridutility3"};
	macutility3_driver m_macutility{this,"macutility3"};
	macstats3_driver m_macstats{this,"macstats3"};
	gridvisualizer3_driver m_gridvisualizer{this,"gridvisualizer3"};
	macvisualizer3_driver m_macvisualizer{this,"macvisualizer3"};
	graphplotter_driver m_graphplotter{this,"graphplotter"};
	dylibloader m_dylib;
	//
	std::string m_export_path;
	//
	struct Parameters {
		vec3d gravity {0.0,-9.8,0.0};
		double surftens_k {0.0};
		bool show_graph {false};
		bool mouse_interaction {false};
		bool volume_correction {true};
		double volume_change_tol_ratio {0.03};
		bool render_mesh {false};
		bool render_transparent {false};
		unsigned render_sample_count {64};
		unsigned render_transparent_sample_count {512};
		vec3d target {0.5,0.15,0.5};
		vec3d origin {0.5,1.5,3.0};
	};
	//
	Parameters m_param;
	//
	virtual void inject_external_force( macarray3<Real> &velocity, double dt );
	virtual void set_volume_correction( macproject3_interface *macproject );
	virtual void extend_both( int w=2 );
	virtual void export_mesh() const;
	virtual void do_export_mesh( unsigned frame ) const;
	virtual void do_export_solid_mesh() const;
	virtual void render_mesh( unsigned frame ) const;
	virtual void add_to_graph();
	//
};
//
SHKZ_END_NAMESPACE
//
#endif