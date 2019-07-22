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
#include <shiokaze/surfacetracker/maclevelsetsurfacetracker2_interface.h>
#include <shiokaze/advection/macadvection2_interface.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/utility/gridutility2_interface.h>
#include <shiokaze/redistancer/redistancer2_interface.h>
#include <shiokaze/array/shared_array2.h>
#include <cstdio>
//
SHKZ_USING_NAMESPACE
//
class maclevelsetsurfacetracker2 : public maclevelsetsurfacetracker2_interface {
protected:
	//
	LONG_NAME("MAC Levelset Surface Tracker 2D")
	MODULE_NAME("maclevelsetsurfacetracker2")
	//
	virtual void advect( array2<float> &fluid, const array2<float> &solid, const macarray2<float> &u, double dt ) override {
		//
		if( dt ) {
			shared_array2<float> fluid_save(fluid);
			m_macadvection->advect_scalar(fluid,u,fluid_save(),dt);
		}
		m_redistancer->redistance(fluid,m_param.levelset_half_bandwidth_count);
		m_gridutility->extrapolate_levelset(solid,fluid);
	}
	//
	virtual void load( configuration &config ) override {
		m_macadvection.set_name("Levelset Advection 2D","LevelsetAdvection");
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_unsigned("LevelsetHalfWidth",m_param.levelset_half_bandwidth_count,"Level set half bandwidth");
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	macadvection2_driver m_macadvection{this,"macadvection2"};
	redistancer2_driver m_redistancer{this,"pderedistancer2"};
	gridutility2_driver m_gridutility{this,"gridutility2"};
	//
	struct Parameters {
		unsigned levelset_half_bandwidth_count {3};
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