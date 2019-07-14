/*
**	macsmoke2.h
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
#ifndef SHKZ_MACSMOKE2_H
#define SHKZ_MACSMOKE2_H
//
#include <shiokaze/array/array2.h>
#include <shiokaze/array/macarray2.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/ui/drawable.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/advection/macadvection2_interface.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/utility/macstats2_interface.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/visualizer/macvisualizer2_interface.h>
#include <shiokaze/projection/macproject2_interface.h>
#include <shiokaze/timestepper/timestepper_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class macsmoke2 : public drawable {
public:
	//
	macsmoke2();
	LONG_NAME("MAC Smoke 2D")
	MODULE_NAME("macsmoke2")
	ARGUMENT_NAME("Smoke")
	//
	virtual void drag( int width, int height, double x, double y, double u, double v ) override;
	virtual void idle() override;
	virtual void setup_window( std::string &name, int &width, int &height ) const override;
	virtual void draw( graphics_engine &g, int width, int height ) const override;
	virtual bool should_quit() const override { return m_timestepper->should_quit(); }
	virtual bool should_screenshot() const override { return m_timestepper->should_export_frame(); }
	//
protected:
	//
	virtual void load( configuration &config ) override;
	virtual void configure( configuration &config ) override;
	virtual void post_initialize() override;
	//
	macarray2<float> m_velocity{this};
	macarray2<float> m_external_force{this};
	//
	array2<float> m_density{this};
	array2<float> m_accumulation{this};
	//
	array2<float> m_fluid{this};
	array2<float> m_solid{this};
	//
	std::vector<vec2d> m_dust_particles;
	//
	shape2 m_shape;
	double m_dx;
	bool m_force_exist;
	//
	struct Parameters {
		bool use_dust {false};
		double minimal_density {0.01};
		unsigned r_sample {4};
		unsigned extrapolated_width {3};
		double buoyancy_factor {2.0};
	};
	//
	Parameters m_param;
	//
	environment_setter arg_shape{this,"shape",&m_shape};
	environment_setter arg_dx{this,"dx",&m_dx};
	//
	macproject2_driver m_macproject{this,"macpressuresolver2"};
	macadvection2_driver m_macadvection{this,"macadvection2"};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	macstats2_driver m_macstats{this,"macstats2"};
	macvisualizer2_driver m_macvisualizer{this,"macvisualizer2"};
	timestepper_driver m_timestepper{this,"timestepper"};
	macutility2_driver m_macutility{this,"macutility2"};
	//
	parallel_driver m_parallel{this};
	dylibloader m_dylib;
	//
	virtual void inject_external_force( macarray2<float> &velocity );
	virtual void add_buoyancy_force( macarray2<float> &velocity, const array2<float> &density, double dt );
	virtual void advect_dust_particles( const macarray2<float> &velocity, double dt );
	virtual void add_source ( macarray2<float> &velocity, array2<float> &density, double time, double dt );
	virtual void rasterize_dust_particles( array2<float> &rasterized_density );
	virtual void draw_dust_particles( graphics_engine &g ) const;
};
//
SHKZ_END_NAMESPACE
//
#endif
