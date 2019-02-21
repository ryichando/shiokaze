/*
**	maclevelsetsurfacetracker2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 28, 2017. 
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
#include <shiokaze/surfacetracker/macsurfacetracker2_interface.h>
#include <shiokaze/advection/macadvection2_interface.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/utility/gridutility2_interface.h>
#include <shiokaze/visualizer/gridvisualizer2_interface.h>
#include <shiokaze/redistancer/redistancer2_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/array/shared_array2.h>
#include <cstdio>
//
SHKZ_USING_NAMESPACE
//
class maclevelsetsurfacetracker2 : public macsurfacetracker2_interface {
private:
	//
	LONG_NAME("MAC Levelset Surface Tracker 2D")
	//
	virtual void assign( const array2<double> &solid, const array2<double> &fluid ) override {
		m_solid.copy(solid);
		m_fluid.copy(fluid);
	}
	virtual void advect( const macarray2<double> &u, double dt ) override {
		//
		shared_array2<double> fluid_save(m_fluid);
		m_macadvection->advect_scalar(m_fluid,u,fluid_save(),dt);
		m_redistancer->redistance(m_fluid,m_param.levelset_half_bandwidth_count);
		m_gridutility->extrapolate_levelset(m_solid,m_fluid);
	}
	virtual void get( array2<double> &fluid ) override { fluid.copy(m_fluid); }
	virtual void draw( graphics_engine &g ) const override {
		g.color4(0.5,0.6,1.0,0.5);
		m_gridvisualizer->draw_levelset(g,m_fluid);
		if( m_param.draw_actives ) m_gridvisualizer->draw_active(g,m_fluid);
	}
	//
	virtual void load( configuration &config ) override {
		m_macadvection.set_name("Levelset Advection 2D","LevelsetAdvection");
	}
	virtual void configure( configuration &config ) override {
		config.get_bool("DrawActives",m_param.draw_actives,"Whether to draw active narrow band");
		config.get_unsigned("LevelsetHalfWidth",m_param.levelset_half_bandwidth_count,"Level set half bandwidth");
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
		//
		m_fluid.initialize(shape.cell());
		m_solid.initialize(shape.nodal());
	}
	//
	array2<double> m_solid{this}, m_fluid{this};
	macadvection2_driver m_macadvection{this,"macadvection2"};
	redistancer2_driver m_redistancer{this,"pderedistancer2"};
	gridutility2_driver m_gridutility{this,"gridutility2"};
	macutility2_driver m_macutility{this,"macutility2"};
	gridvisualizer2_driver m_gridvisualizer{this,"gridvisualizer2"};
	parallel_driver m_parallel{this};
	//
	struct Parameters {
		bool draw_actives {true};
		unsigned levelset_half_bandwidth_count {2};
	};
	//
	Parameters m_param;
	shape2 m_shape;
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new maclevelsetsurfacetracker2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//