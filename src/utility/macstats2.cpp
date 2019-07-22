/*
**	macstats2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 21, 2017. 
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
#include <shiokaze/utility/macstats2_interface.h>
#include <shiokaze/utility/macutility2_interface.h>
#include <shiokaze/array/array_utility2.h>
#include <shiokaze/array/array_interpolator2.h>
#include <shiokaze/core/console.h>
#include <algorithm>
#include <cstdio>
//
SHKZ_USING_NAMESPACE
using namespace array_utility2;
using namespace array_interpolator2;
//
class macstats2 : public macstats2_interface {
protected:
	//
	MODULE_NAME("macstats2")
	//
	virtual void dump_stats( const array2<float> &solid, const array2<float> &fluid, const macarray2<float> &velocity, const timestepper_interface *tmstepper ) const override {
		//
		unsigned num_active_fluid (0);
		if( levelset_exist(solid)) {
			fluid.const_serial_actives([&](int i, int j, const auto &it) {
				if( it() < 0.0 && interpolate<float>(solid,vec2i(i,j).cell()) > 0.0 ) num_active_fluid ++;
			});
		} else {
			fluid.const_serial_actives([&](const auto &it) {
				if( it() < 0.0 ) num_active_fluid ++;
			});
		}
		bool has_fluid = levelset_exist(fluid);
		if( has_fluid && m_param.export_path.size()) {
			FILE *fp = fopen((m_param.export_path+"/num_active_fluid.out").c_str(),"a");
			if( fp ) {
				fprintf(fp, "%e %d\n", tmstepper->get_current_time(), num_active_fluid );
				fclose(fp);
			}
		}
		//
		if( m_param.report_kinetic_energy ) {
			double kinetic_energy = m_macutility->get_kinetic_energy(solid,fluid,velocity);
			if( m_param.report_console ) {
				if( has_fluid ) console::dump( "Report: active fluid cells = %d, kinetic energy = %.3e\n", num_active_fluid, kinetic_energy );
				else console::dump( "Report: kinetic energy = %.3e\n", kinetic_energy );
			}
			if( m_param.export_path.size()) {
				FILE *fp = fopen((m_param.export_path+"/kinetic_enegy.out").c_str(),"a");
				if( fp ) {
					fprintf(fp, "%e %e\n", tmstepper->get_current_time(), kinetic_energy );
					fclose(fp);
				}
			}
		}
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("ReportConsole",m_param.report_console,"Whether to report in console");
		config.get_bool("ReportKineticEnergy",m_param.report_console,"Whether to report kinetic energy");
		config.get_string("StatsPath",m_param.export_path,"Stats export path");
	}
	virtual void initialize( const shape2 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	struct Parameters {
		std::string export_path;
		bool report_console {false};
		bool report_kinetic_energy {true};
	};
	Parameters m_param;
	//
	macutility2_driver m_macutility{this,"macutility2"};
	//
	shape2 m_shape;
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new macstats2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//