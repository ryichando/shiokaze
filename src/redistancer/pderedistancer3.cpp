/*
**	pderedistancer3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 5, 2017. 
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
#include <shiokaze/redistancer/redistancer3_interface.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/console.h>
//
SHKZ_USING_NAMESPACE
//
class pderedistancer3 : public redistancer3_interface {
public:
	//
	LONG_NAME("PDE Redistancer 3D")
	ARGUMENT_NAME("PDERedist")
	//
	virtual void redistance( array3<double> &phi_array, double dx ) override {
		//
		unsigned half_cells = phi_array.get_levelset_halfwidth();
		double half_bandwidth = half_cells * dx;
		auto smoothed_sgn = []( double value, double dx ) {
			return value / sqrt(value*value+dx*dx);
		};
		//
		m_gridutility->trim_narrowband(phi_array,half_cells);
		shared_array3<double> phi_array0 (phi_array);
		//
		shared_array3<double> smoothed_sgns (phi_array.type());
		auto phi_array0_accessors = phi_array0->get_const_accessors();
		smoothed_sgns->activate_as(phi_array);
		smoothed_sgns->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			it.set(smoothed_sgn(phi_array0_accessors[tn](i,j,k),dx));
		});
		//
		auto smoothed_sgns_accessors = smoothed_sgns->get_const_accessors();
		auto derivative = [&]( const array3<double> &phi_array, array3<double> &phi_array_derivative ) {
			//
			phi_array_derivative.clear();
			phi_array_derivative.activate_as(phi_array);
			//
			auto phi_array_accessors = phi_array.get_const_accessors();
			phi_array_derivative.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				//
				const double &sgn0 = smoothed_sgns_accessors[tn](i,j,k);
				const double &phi0 = phi_array0_accessors[tn](i,j,k);
				const double &phi = phi_array_accessors[tn](i,j,k);
				//
				// Evaluate upwind gradient
				vec3d gradient;
				const vec3i ijk (i,j,k);
				for( int dim : DIMS3 ) {
					int select_direction (0);
					double tmp_phi (sgn0*phi), phi_backward, phi_backward0, phi_forward, phi_forward0, g (0.0);
					if( ijk[dim] > 0 && phi_array_accessors[tn].active(i-(dim==0),j-(dim==1),k-(dim==2))) {
						phi_backward = phi_array_accessors[tn](i-(dim==0),j-(dim==1),k-(dim==2));
						if( sgn0*phi_backward < tmp_phi ) select_direction = -1;
					}
					if( ijk[dim] < phi_array.shape()[dim]-1 && phi_array_accessors[tn].active(i+(dim==0),j+(dim==1),k+(dim==2))) {
						phi_forward = phi_array_accessors[tn](i+(dim==0),j+(dim==1),k+(dim==2));
						if( sgn0*phi_forward < tmp_phi ) {
							if( select_direction == 0 ) select_direction = 1;
							else if( sgn0*phi_forward < sgn0*phi_backward ) {
								select_direction = 1;
							}
						}
					}
					if( select_direction == -1 ) {
						phi_backward0 = phi_array0_accessors[tn](i-(dim==0),j-(dim==1),k-(dim==2));
						double frac = utility::fraction(phi0,phi_backward0);
						if( frac == 1.0 || frac == 0.0 ) {
							g = (phi-phi_backward) / dx;
						} else {
							if( sgn0 < 0.0 ) g = phi / (dx * frac);
							else g = phi / (dx * (1.0-frac));
						}
					} else if( select_direction == 1 ) {
						phi_forward0 = phi_array0_accessors[tn](i+(dim==0),j+(dim==1),k+(dim==2));
						double frac = utility::fraction(phi0,phi_forward0);
						if( frac == 1.0 || frac == 0.0 ) {
							g = (phi_forward-phi) / dx;
						} else {
							if( sgn0 < 0.0 ) g = -phi / (dx * frac);
							else g = -phi / (dx * (1.0-frac));
						}
					}
					gradient[dim] = g;
				}
				//
				it.set(sgn0 * (1.0 - gradient.len()));
			});
		};
		//
		// Evolve by PDE
		double dt = m_param.rate * dx;
		for( int itcount=0; itcount<half_cells; ++itcount ) {
			//
			if( m_param.temporal_scheme == "Euler" ) {
				//
				shared_array3<double>	phi_array_derivative0(phi_array.type()),
										phi_array_save(phi_array);
				//
				phi_array.set_touch_only_actives(true);
				phi_array_derivative0->set_touch_only_actives(true);
				//
				derivative(phi_array,phi_array_derivative0());
				phi_array = phi_array_derivative0();
				phi_array *= dt;
				phi_array += phi_array_save();
				//
			} else if( m_param.temporal_scheme == "RK2" ) {
				//
				shared_array3<double>	phi_array_derivative0(phi_array.type()),
										phi_array_derivative1(phi_array.type()),
										phi_array_derivative_tmp(phi_array.type());
				//
				phi_array.set_touch_only_actives(true);
				phi_array_derivative0->set_touch_only_actives(true);
				phi_array_derivative1->set_touch_only_actives(true);
				phi_array_derivative_tmp->set_touch_only_actives(true);
				//
				derivative(phi_array,phi_array_derivative0());
				phi_array_derivative_tmp() = phi_array_derivative0();
				phi_array_derivative_tmp() *= dt;
				phi_array_derivative_tmp() += phi_array;
				//
				derivative(phi_array_derivative_tmp(),phi_array_derivative1());
				phi_array_derivative_tmp() = phi_array_derivative0();
				phi_array_derivative_tmp() += phi_array_derivative1();
				phi_array_derivative_tmp() *= 0.5 * dt;
				//
				phi_array += phi_array_derivative_tmp();
				//
			} else {
				console::dump( "Unknown scheme %s\n", m_param.temporal_scheme.c_str());
				exit(0);
			}
		}
		//
		phi_array.parallel_actives([&](auto &it) {
			if( std::abs(it()) > half_cells*dx ) it.set_off();
		});
		phi_array.flood_fill();
	}
	//
protected:
	//
	virtual void configure( configuration &config ) override {
		config.get_double("IntegrationRate",m_param.rate,"Levelset advance rate");
		config.get_string("RedistTemporalScheme",m_param.temporal_scheme,"Temporal integration scheme");
	}
	//
	struct Parameters {
		double rate {0.75};
		std::string temporal_scheme {"Euler"};
	};
	Parameters m_param;
	gridutility3_driver m_gridutility{this,"gridutility3"};
};
//
extern "C" module * create_instance() {
	return new pderedistancer3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//