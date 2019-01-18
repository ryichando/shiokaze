/*
**	mactallgridliquid2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 17, 2017. 
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
#ifndef SHKZ_MACTALLGRIDLIQUID2_H
#define SHKZ_MACTALLGRIDLIQUID2_H
//
#include <shiokaze/array/array2.h>
#include <shiokaze/array/macarray2.h>
#include <shiokaze/ui/drawable.h>
#include <shiokaze/utility/gridutility2_interface.h>
#include <shiokaze/advection/macadvection2_interface.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/utility/macstats2_interface.h>
#include <shiokaze/visualizer/macvisualizer2_interface.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/surfacetracker/macsurfacetracker2_interface.h>
#include <shiokaze/timestepper/timestepper_interface.h>
#include <shiokaze/core/dylibloader.h>
#include <shiokaze/math/RCMatrix_interface.h>
#include <shiokaze/linsolver/RCMatrix_solver.h>
#include "upsampler2.h"
//
SHKZ_BEGIN_NAMESPACE
//
class mactallgridliquid2 : public drawable {
public:
	//
	mactallgridliquid2();
	LONG_NAME("MAC Tall Grid Liquid 2D")
	ARGUMENT_NAME("TallGridLiquid")
	//
	virtual void drag( int width, int height, double x, double y, double u, double v ) override;
	virtual void idle() override;
	virtual void cursor( int width, int height, double x, double y ) override;
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
	virtual void project(double dt);
	//
	macarray2<double> m_velocity{this};
	macarray2<double> m_external_force{this};
	array2<double> m_fluid{this};
	array2<double> m_solid{this};
	array2<double> m_pressure{this};
	//
	environment_setter arg_shape{this,"shape",&m_shape};
	environment_setter arg_dx{this,"dx",&m_dx};
	//
	macadvection2_driver m_macadvection{this,"macadvection2"};
	macsurfacetracker2_driver m_macsurfacetracker{this,"maclevelsetsurfacetracker2"};
	timestepper_driver m_timestepper{this,"timestepper"};
	gridutility2_driver m_gridutility{this,"gridutility2"};
	macutility2_driver m_macutility{this,"macutility2"};
	macstats2_driver m_macstats{this,"macstats2"};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	macvisualizer2_driver m_macvisualizer{this,"macvisualizer2"};
	RCMatrix_factory_driver<size_t,double> m_factory{this,"RCMatrix"};
	RCMatrix_solver_driver<size_t,double> m_solver{this,"pcg"};
	dylibloader m_dylib;
	//
	upsampler2 upsampler;
	//
	shape2 m_shape;
	double m_dx;
	double m_initial_volume;
	bool m_force_exist;
	vec2d m_cursor;
	RCMatrix_ptr<size_t,double > m_UtLhsU;
	//
	struct Parameters {
		vec2d gravity;
		bool volume_correction;
		double volume_change_tol_ratio;
	};
	//
	Parameters m_param;
	//
	virtual void inject_external_force( macarray2<double> &velocity, double dt );
	virtual void extend_both();
};
//
SHKZ_END_NAMESPACE
//
#endif