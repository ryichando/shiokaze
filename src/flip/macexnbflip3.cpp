/*
**	macexnbflip3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 29, 2017. All rights reserved.
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
#include "macexnbflip3.h"
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <algorithm>
//
SHKZ_USING_NAMESPACE
//
template<typename T>
static void pointwise_gaussian_blur( const array3<T> &source, array3<T> &result, double r ) {
	//
	const int rs = std::floor(r * 2.57);
	const int L = 2*rs+1;
	//
	std::vector<double> exp_w(L*L*L);
	for(int qi=-rs; qi<rs+1; qi++) {
		for(int qj=-rs; qj<rs+1; qj++) {
			for(int qk=-rs; qk<rs+1; qk++) {
				double q2 = vec3d(qi,qj,qk).norm2();
				exp_w[qi+rs+L*(qj+rs)+(L*L)*(qk+rs)] = std::exp(-q2/(2.0*r*r))/std::pow(M_PI*2.0*r*r,DIM3/2.0);
			}
		}
	}
	result.activate_as(source);
	result.parallel_actives([&]( int i, int j, int k, auto &it, int tn ) {
		//
		T val (0.0);
		double wsum (0.0);
		for(int qi=-rs; qi<rs+1; qi++) {
			for(int qj=-rs; qj<rs+1; qj++) {
				for(int qk=-rs; qk<rs+1; qk++) {
					int ni = i+qi;
					int nj = j+qj;
					int nk = k+qk;
					T value;
					const double &wght = exp_w[qi+rs+L*(qj+rs)+(L*L)*(qk+rs)];
					if( ! source.shape().out_of_bounds(ni,nj,nk) && source.active(ni,nj,nk)) {
						value = source(ni,nj,nk);
					} else {
						value = source(i,j,k);
					}
					val += value * wght; wsum += wght;
				}
			}
		}
		it.set(val/wsum);
	});
}
//
void macexnbflip3::internal_sizing_func(array3<Real> &sizing_array,
							const bitarray3 &mask,
							const array3<Real> &fluid,
							const macarray3<Real> &velocity ) const {
	//
	shared_array3<vec3r> diff(m_shape);
	//
	if( m_param.mode==0 || m_param.mode==1 ) {
		//
		// 1. Make velocity array
		shared_array3<vec3r> velocity_array(m_shape);
		velocity.convert_to_full(velocity_array());
		//
		// 2. Apply gaussian blur to 1
		shared_array3<vec3r> gaussian_blured_velocity_array(m_shape);
		pointwise_gaussian_blur(velocity_array(),gaussian_blured_velocity_array(),m_param.radius);
		//
		// 3. Take diff between 1 and 2
		gaussian_blured_velocity_array->set_touch_only_actives(true);
		diff->copy(gaussian_blured_velocity_array());
		diff() -= velocity_array();
	}
	//
	// 4. Compute blurred levelset
	shared_array3<Real> fluid_blurred(fluid);
	if( m_param.mode==0 || m_param.mode==2 ) pointwise_gaussian_blur(fluid,fluid_blurred(),m_param.radius);
	//
	// 5. Set sizing_array
	sizing_array.activate_as_bit(mask);
	sizing_array.parallel_actives([&](int i, int j, int k, auto &it, int tn ) {
		//
		double value0, value1;
		//
		if( m_param.mode==0 || m_param.mode==1 ) value0 = std::max(0.0,m_param.amplification * std::min(1.0,diff()(i,j,k).len()) - m_param.threshold_u);
		if( m_param.mode==0 || m_param.mode==2 ) {
			double val = fluid(i,j,k);
			if( val < 0 && val > -0.5*m_dx ) {
				value1 = std::max(0.0, m_param.amplification * std::abs(fluid_blurred()(i,j,k)-val) / m_dx - m_param.threshold_g);
			} else {
				value1 = 0.0;
			}
		}
		//
		if( m_param.mode == 0 ) {
			it.set(std::max(value0,value1));
		} else if( m_param.mode == 1 ) {
			it.set(value0);
		} else if( m_param.mode == 2 ) {
			it.set(value1);
		}
	});
}
//
void macexnbflip3::configure( configuration &config ) {
	//
	macnbflip3::configure(config);
	//
	config.get_double("DiffuseRate",m_param.diffuse_rate,"Diffuse rate for sizing function");
	config.get_unsigned("DiffuseCount",m_param.diffuse_count,"Diffuse count for sizing function");
	config.get_double("Threshold_U",m_param.threshold_u,"Threshold velocity for sizing function evaluation");
	config.get_double("Threshold_G",m_param.threshold_g,"Threshold geometry for sizing function evaluation");
	config.get_double("Amplification",m_param.amplification,"Amplification velocity for sizing function evaluation");
	config.get_double("SizingBlurRadius",m_param.radius,"Gaussian blur radius for velocity sizing function");
	std::string mode_str("both");
	config.get_string("SizingMode",mode_str,"Sizing function combination mode (both,velocity,geometry)");
	if( mode_str == "both" ) {
		m_param.mode = 0;
	} else if( mode_str == "velocity" ) {
		m_param.mode = 1;
	} else if( mode_str == "geometry" ) { 
		m_param.mode = 2;
	} else {
		printf( "Unknown mode %s\n", mode_str.c_str());
		exit(0);
	}
}
//
void macexnbflip3::compute_sizing_func( const array3<Real> &fluid, const bitarray3 &mask, const macarray3<Real> &velocity, array3<Real> &sizing_array ) const {
	//
	auto diffuse = [&]( array3<Real> &array, int width, double rate ) {
		//
		for( unsigned count=0; count<width; count++ ) {
			//
			shared_array3<Real> array_save(array);
			array.parallel_actives([&](int i, int j, int k, auto &it, int tn ) {
				if( mask(i,j,k)) {
					double sum (0.0);
					int weight = 0;
					int query[][DIM3] = {{i+1,j,k},{i-1,j,k},{i,j+1,k},{i,j-1,k},{i,j,k-1},{i,j,k+1}};
					for( int nq=0; nq<6; nq++ ) {
						int qi = query[nq][0];
						int qj = query[nq][1];
						int qk = query[nq][2];
						if( ! m_shape.out_of_bounds(qi,qj,qk)) {
							if( mask(qi,qj,qk) && array_save()(qi,qj,qk) > array(i,j,k)) {
								sum += array_save()(qi,qj,qk);
								weight ++;
							}
						}
					}
					if( weight ) {
						it.set((1.0-rate)*array(i,j,k)+rate*sum/weight);
					}
				}
			});
		}
	};
	//
	// Evaluate sizing function
	shared_array3<Real> pop_array(m_shape);
	internal_sizing_func(pop_array(),mask,fluid,velocity);
	//
	sizing_array.clear(0.0);
	if( array_utility3::value_exist(pop_array())) {
		//
		// Diffuse
		if( m_param.diffuse_count ) {
			diffuse(pop_array(),m_param.diffuse_count,m_param.diffuse_rate);
		}
		// Assign initial value
		sizing_array.activate_as(pop_array());
		sizing_array.parallel_actives([&]( int i, int j, int k, auto &it, int tn ) {
			it.set(std::max((Real)0.0,std::min((Real)1.0,pop_array()(i,j,k))));
		});
	}
}
//
extern "C" module * create_instance() {
	return new macexnbflip3();
}
//