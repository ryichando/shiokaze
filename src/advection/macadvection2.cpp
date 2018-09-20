/*
**	macadvection2.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 15, 2017. 
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
#include <shiokaze/advection/macadvection2_interface.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/array_interpolator2.h>
#include <shiokaze/math/WENO2.h>
#include <limits>
//
SHKZ_USING_NAMESPACE
//
class macadvection2 : public macadvection2_interface {
private:
	//
	virtual void advect_scalar(	array2<double> &scalar,				// Cell-centered
								const macarray2<double> &velocity,	// Face-located
								double dt ) override {
		//
		shared_array2<double> scalar0(scalar);
		advect_cell(scalar0(),velocity,scalar,dt,m_param.use_maccormack,m_param.weno_interpolation);
	}
	//
	virtual void advect_vector(	macarray2<double> &u,				// Face-located
								const macarray2<double> &velocity,	// Face-located
								double dt ) override {
		//
		shared_macarray2<double> u0(u);
		advect_u(u0(),velocity,u,dt,m_param.use_maccormack,m_param.weno_interpolation);
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_bool("MacCormack",m_param.use_maccormack,"Whether to use MacCormack method");
		config.get_bool("WENO",m_param.weno_interpolation,"Whether to use WENO interpolation for advection");
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	using double2 = struct { double v[2] = {0.0, 0.0}; };
	void advect_semiLagrangian_u( const macarray2<double> &v_in, const macarray2<double> &v, macarray2<double> &v_out, macarray2<double2> *minMax, double dt, bool weno_interpolation ) {
		//
		v_out.clear();
		v_out.activate_as(v_in);
		auto v_in_accessors = v_in.get_const_accessors();
		//
		shared_macarray2<vec2d> face_full_velocity(m_shape);
		v_in.convert_to_full(face_full_velocity());
		//
		auto face_full_velocity_accessors = face_full_velocity->get_const_accessors();
		v_out.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
			//
			const vec2d &u = face_full_velocity_accessors[tn](dim,i,j);
			//
			if( ! u.empty() ) {
				vec2d p = vec2d(i,j)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO2::interpolate(v_in_accessors[tn].get(dim),p);
				} else {
					value = array_interpolator2::interpolate<double>(v_in_accessors[tn].get(dim),p);
				}
				it.set(value);
			} else {
				it.set(v_in_accessors[tn](dim,i,j));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(v_in);
			minMax->parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				//
				const vec2d &u = face_full_velocity_accessors[tn](dim,i,j);
				//
				if( ! u.empty() ) {
					vec2d p = vec2d(i,j)-dt*u/m_dx;
					vec2i indices[4];
					double coef[4];
					array_interpolator2::interpolate_coef(v_in[dim].shape(),p,indices,coef);
					double min_value = std::numeric_limits<double>::max();
					double max_value = std::numeric_limits<double>::min();
					for( int n=0; n<4; ++n ) {
						double value = v_in_accessors[tn](dim,indices[n]);
						min_value = std::min(min_value,value);
						max_value = std::max(max_value,value);
					}
					double2 d2;
					d2.v[0] = min_value;
					d2.v[1] = max_value;
					it.set(d2);
				} else {
					if( minMax ) {
						double2 d2;
						d2.v[0] = d2.v[1] = v_in_accessors[tn](dim,i,j);
						it.set(d2);
					}
				}
			});
		}
	}
	//
	void advect_u ( const macarray2<double> &v_in, const macarray2<double> &v, macarray2<double> &v_out, double dt, bool use_maccormack, bool weno_interpolation ) {
		//
		if( use_maccormack ) {
			//
			shared_macarray2<double> velocity_0(v_in.type()), velocity_1(v_in.type());
			shared_macarray2<double2> minMax_u(v_in.shape());
			//
			advect_semiLagrangian_u(v_in,v,velocity_0(),&minMax_u(),dt,weno_interpolation);
			advect_semiLagrangian_u(velocity_0(),v,velocity_1(),nullptr,-dt,weno_interpolation);
			//
			v_out.clear();
			v_out.activate_as(v_in);
			//
			auto minMax_u_accessors = minMax_u->get_const_accessors();
			auto v_in_accessors = v_in.get_const_accessors();
			auto v_0_accessors = velocity_0->get_const_accessors();
			auto v_1_accessors = velocity_1->get_const_accessors();
			//
			v_out.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				//
				double min_value = minMax_u_accessors[tn](dim,i,j).v[0];
				double max_value = minMax_u_accessors[tn](dim,i,j).v[1];
				double vel0 = v_0_accessors[tn](dim,i,j);
				double correction = 0.5*(v_in_accessors[tn](dim,i,j)-v_1_accessors[tn](dim,i,j));
				if( vel0+correction < min_value ) it.set(min_value);
				else if( vel0+correction > max_value ) it.set(max_value);
				else it.set(vel0+correction);
			});
		} else {
			advect_semiLagrangian_u(v_in,v,v_out,nullptr,dt,weno_interpolation);
		}
	}
	//
	void advect_semiLagrangian_cell ( const array2<double> &q_in, const macarray2<double> &v, array2<double> &q_out, array2<double2> *minMax, double dt, bool weno_interpolation ) {
		//
		shared_array2<vec2d> full_velocity(m_shape);
		v.convert_to_full(full_velocity());
		//
		auto full_velocity_accessors = full_velocity->get_const_accessors();
		auto q_in_accessors = q_in.get_const_accessors();
		//
		q_out.clear();
		q_out.activate_as(q_in);
		q_out.parallel_actives([&](int i, int j, auto &it, int tn) {
			//
			const vec2d &u = full_velocity_accessors[tn](i,j);
			//
			if( ! u.empty() ) {
				vec2d p = vec2d(i,j)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO2::interpolate(q_in_accessors[tn],p);
				} else {
					value = array_interpolator2::interpolate<double>(q_in_accessors[tn],p);
				}
				it.set(value);
			} else {
				it.set(q_in_accessors[tn](i,j));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(q_in);
			minMax->parallel_actives([&](int i, int j, auto &it, int tn) {
				//
				const vec2d &u = full_velocity_accessors[tn](i,j);
				//
				if( ! u.empty() ) {
					vec2d p = vec2d(i,j)-dt*u/m_dx;
					vec2i indices[4];
					double coef[4];
					array_interpolator2::interpolate_coef(q_in.shape(),p,indices,coef);
					double min_value = std::numeric_limits<double>::max();
					double max_value = std::numeric_limits<double>::min();
					for( int n=0; n<4; ++n ) {
						double value = q_in_accessors[tn](indices[n]);
						min_value = std::min(min_value,value);
						max_value = std::max(max_value,value);
					}
					double2 d2;
					d2.v[0] = min_value;
					d2.v[1] = max_value;
					it.set(d2);
				} else {
					double2 d2;
					d2.v[0] = d2.v[1] = q_in_accessors[tn](i,j);
					it.set(d2);
				}
			});
		}
	}
	//
	void advect_cell ( const array2<double> &q_in, const macarray2<double> &v, array2<double> &q_out, double dt, bool use_maccormack, bool weno_interpolation ) {
		//
		if( use_maccormack ) {
			//
			shared_array2<double> q_0(q_in.type()), q_1(q_in.type());
			shared_array2<double2> minMax_q(q_in.shape());
			//
			advect_semiLagrangian_cell(q_in,v,q_0(),&minMax_q(),dt,weno_interpolation);
			advect_semiLagrangian_cell(q_0(),v,q_1(),nullptr,-dt,weno_interpolation);
			//
			q_out.clear();
			q_out.activate_as(q_in);
			//
			auto minMax_q_accessors = minMax_q->get_const_accessors();
			auto q_in_accessors = q_in.get_const_accessors();
			auto q_0_accessors = q_0->get_const_accessors();
			auto q_1_accessors = q_1->get_const_accessors();
			//
			q_out.parallel_actives([&](int i, int j, auto &it, int tn) {
				//
				double min_value = minMax_q_accessors[tn](i,j).v[0];
				double max_value = minMax_q_accessors[tn](i,j).v[1];
				double q0 = q_0_accessors[tn](i,j);
				double correction = 0.5*(q_in_accessors[tn](i,j)-q_1_accessors[tn](i,j));
				if( q0+correction < min_value ) it.set(min_value);
				else if( q0+correction > max_value ) it.set(max_value);
				else it.set(q0+correction);
				//
			});
		} else {
			advect_semiLagrangian_cell(q_in,v,q_out,nullptr,dt,weno_interpolation);
		}
	}
	//
	struct Parameters {
		bool use_maccormack {true};
		bool weno_interpolation {false};
	};
	Parameters m_param;
	//
	shape2 m_shape;
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new macadvection2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//