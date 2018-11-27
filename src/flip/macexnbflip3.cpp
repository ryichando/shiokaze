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
	auto source_acccessors = source.get_const_accessors();
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
					if( ! source.shape().out_of_bounds(ni,nj,nk) ) {
						const double &wght = exp_w[qi+rs+L*(qj+rs)+(L*L)*(qk+rs)];
						const T &value = source_acccessors[tn](ni,nj,nk);
						val += value * wght;  wsum += wght;
					}
				}
			}
		}
		if( wsum ) it.set(val/wsum);
		else it.set_off();
	});
}
//
void macexnbflip3::internal_sizing_func(array3<double> &sizing_array,
							const bitarray3 &mask,
							const array3<double> &solid,
							const array3<double> &fluid,
							const macarray3<double> &velocity,
							double dt) {
	//
	auto mask_accessors = mask.get_const_accessors();
	shared_array3<vec3d> diff(m_shape);
	//
	if( m_param.mode==0 || m_param.mode==1 ) {
		//
		// 1. Make velocity array
		shared_array3<vec3d> velocity_array(m_shape);
		velocity.convert_to_full(velocity_array());
		//
		// 2. Apply gaussian blur to 1
		shared_array3<vec3d> gaussian_blured_velocity_array(m_shape);
		pointwise_gaussian_blur(velocity_array(),gaussian_blured_velocity_array(),m_param.radius);
		//
		// 3. Take diff between 1 and 2
		gaussian_blured_velocity_array->set_touch_only_actives(true);
		diff->copy(gaussian_blured_velocity_array());
		diff() -= velocity_array();
	}
	//
	// 4. Compute blurred levelset
	shared_array3<double> fluid_blurred(fluid);
	if( m_param.mode==0 || m_param.mode==2 ) pointwise_gaussian_blur(fluid,fluid_blurred(),m_param.radius);
	//
	// 5. Set sizing_array
	auto diff_accessors = diff->get_const_accessors();
	auto fluid_accessors = fluid.get_const_accessors();
	auto fluid_blurred_accessors = fluid_blurred->get_const_accessors();
	//
	sizing_array.activate_as(mask);
	sizing_array.parallel_actives([&](int i, int j, int k, auto &it, int tn ) {
		//
		double value0, value1;
		//
		if( m_param.mode==0 || m_param.mode==1 ) value0 = std::max(0.0,m_param.amplification * std::min(1.0,diff_accessors[tn](i,j,k).len()) - m_param.threshold_u);
		if( m_param.mode==0 || m_param.mode==2 ) {
			double val = fluid_accessors[tn](i,j,k);
			if( val < 0 && val > -0.5*m_dx ) {
				value1 = std::max(0.0, m_param.amplification * std::abs(fluid_blurred_accessors[tn](i,j,k)-val) / m_dx - m_param.threshold_g);
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
	config.get_double("DecayRate",m_param.decay_rate,"Decay rate for tracer particles");
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
void macexnbflip3::sizing_func( array3<double> &sizing_array, const bitarray3 &mask, const macarray3<double> &velocity, double dt ) {
	//
	auto diffuse = [&]( array3<double> &array, int width, double rate ) {
		//
		auto mask_accessors = mask.get_const_accessors();
		auto array_accessors = array.get_const_accessors();
		//
		for( unsigned count=0; count<width; count++ ) {
			//
			shared_array3<double> array_save(array);
			auto array_save_accessors = array_save->get_const_accessors();
			//
			array.parallel_actives([&](int i, int j, int k, auto &it, int tn ) {
				if( mask_accessors[tn](i,j,k)) {
					double sum (0.0);
					int weight = 0;
					int query[][DIM3] = {{i+1,j,k},{i-1,j,k},{i,j+1,k},{i,j-1,k},{i,j,k-1},{i,j,k+1}};
					for( int nq=0; nq<6; nq++ ) {
						int qi = query[nq][0];
						int qj = query[nq][1];
						int qk = query[nq][2];
						if( ! m_shape.out_of_bounds(qi,qj,qk)) {
							if( mask_accessors[tn](qi,qj,qk) && array_save_accessors[tn](qi,qj,qk) > array_accessors[tn](i,j,k)) {
								sum += array_save_accessors[tn](qi,qj,qk);
								weight ++;
							}
						}
					}
					if( weight ) {
						it.set((1.0-rate)*array_accessors[tn](i,j,k)+rate*sum/weight);
					}
				}
			});
		}
	};
	//
	// Decay particle sizing value
	if( m_particles.size()) {
		for( unsigned n=0; n<m_particles.size(); ++n ) {
			m_particles[n].sizing_value -= m_param.decay_rate * dt;
		}
	}
	//
	// Evaluate sizing function
	shared_array3<double> pop_array(m_shape);
	auto pop_array_accessors = pop_array->get_const_accessors();
	internal_sizing_func(pop_array(),m_narrowband_mask,m_solid,m_fluid,velocity,dt);
	//
	sizing_array.clear(0.0);
	if( array_utility3::value_exist(pop_array())) {
		//
		// Diffuse
		if( m_param.diffuse_count ) {
			diffuse(pop_array(),m_param.diffuse_count,m_param.diffuse_rate);
		}
		// Pick max
		if( m_particles.size()) {
			m_parallel.for_each(m_particles.size(),[&]( size_t n, int tn ) {
				Particle &p = m_particles[n];
				p.sizing_value = std::max(p.sizing_value,array_interpolator3::interpolate<double>(pop_array_accessors[tn],p.p/m_dx-vec3d(0.5,0.5,0.5)));
			});
		}
		// Assign initial value
		sizing_array.activate_as(pop_array());
		sizing_array.parallel_actives([&]( int i, int j, int k, auto &it, int tn ) {
			it.set(std::max(0.0,std::min(1.0,pop_array_accessors[tn](i,j,k))));
		});
	}
	//
	// Assign sizing value
	if( m_particles.size()) {
		auto sizing_array_accessor = sizing_array.get_serial_accessor();
		for( unsigned n=0; n<m_particles.size(); ++n ) {
			vec3i pi = m_shape.clamp(m_particles[n].p/m_dx);
			sizing_array_accessor.set(pi,std::max(sizing_array_accessor(pi),std::min(1.0,m_particles[n].sizing_value)));
		}
	}
}
//
extern "C" module * create_instance() {
	return new macexnbflip3();
}
//