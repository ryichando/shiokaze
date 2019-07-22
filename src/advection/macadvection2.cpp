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
protected:
	//
	MODULE_NAME("macadvection2")
	//
	virtual void advect_scalar(	array2<float> &scalar,				// Cell-centered
								const macarray2<float> &velocity,	// Face-located
								const array2<float> &fluid,			// Fluid level set
								double dt ) override {
		//
		shared_array2<float> scalar0(scalar);
		advect_cell(scalar0(),velocity,scalar,fluid,dt,m_param.use_maccormack,m_param.weno_interpolation);
	}
	//
	virtual void advect_vector(	macarray2<float> &u,				// Face-located
								const macarray2<float> &velocity,	// Face-located
								const array2<float> &fluid,			// Fluid level set
								double dt ) override {
		//
		shared_macarray2<float> u0(u);
		advect_u(u0(),velocity,u,fluid,dt,m_param.use_maccormack,m_param.weno_interpolation);
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_bool("MacCormack",m_param.use_maccormack,"Whether to use MacCormack method");
		config.get_bool("WENO",m_param.weno_interpolation,"Whether to use WENO interpolation for advection");
		config.get_unsigned( "TrimNarrowBand",m_param.trim_narrowband,"Narrow band count to turn to semi-Lagrangian advection");
	}
	//
	virtual void initialize( const shape2 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	using float2 = struct { float v[2] = {0.0, 0.0}; bool within_narrowband{false}; };
	void advect_semiLagrangian_u( const macarray2<float> &v_in, const macarray2<float> &v, macarray2<float> &v_out, macarray2<float2> *minMax, const array2<float> &fluid, double dt, bool weno_interpolation ) {
		//
		v_out.clear();
		v_out.activate_as(v_in);
		//
		shared_macarray2<vec2f> face_full_velocity(m_shape);
		v_in.convert_to_full(face_full_velocity());
		//
		v_out.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
			//
			const vec2f &u = face_full_velocity()[dim](i,j);
			//
			if( ! u.empty() ) {
				vec2d p = vec2d(i,j)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO2::interpolate(v_in[dim],p);
				} else {
					value = array_interpolator2::interpolate<float>(v_in[dim],p);
				}
				it.set(value);
			} else {
				it.set(v_in[dim](i,j));
			}
		});
		//
		if( minMax ) {
			//
			minMax->clear();
			minMax->activate_as(v_in);
			//
			minMax->parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				//
				const vec2d &u = face_full_velocity()[dim](i,j);
				//
				if( ! u.empty() ) {
					vec2d p = vec2d(i,j)-dt*u/m_dx;
					vec2d face_p = vec2i(i,j).face(dim)-dt*u/m_dx;
					vec2i indices[4];
					double coef[4];
					array_interpolator2::interpolate_coef(v_in[dim].shape(),p,indices,coef);
					double min_value = std::numeric_limits<double>::max();
					double max_value = std::numeric_limits<double>::min();
					for( int n=0; n<4; ++n ) {
						double value = v_in[dim](indices[n]);
						min_value = std::min(min_value,value);
						max_value = std::max(max_value,value);
					}
					float2 d2;
					bool within_narrowband = array_interpolator2::interpolate(fluid,face_p-vec2d(0.5,0.5)) > -m_dx * (double)m_param.trim_narrowband;
					d2.v[0] = min_value;
					d2.v[1] = max_value;
					d2.within_narrowband = within_narrowband;
					it.set(d2);
				} else {
					if( minMax ) {
						vec2d face_p = vec2i(i,j).face(dim);
						float2 d2;
						bool within_narrowband = array_interpolator2::interpolate(fluid,face_p-vec2d(0.5,0.5)) > -m_dx * (double)m_param.trim_narrowband;
						d2.v[0] = d2.v[1] = v_in[dim](i,j);
						d2.within_narrowband = within_narrowband;
						it.set(d2);
					}
				}
			});
		}
	}
	//
	void advect_u ( const macarray2<float> &v_in, const macarray2<float> &v, macarray2<float> &v_out, const array2<float> &fluid, double dt, bool use_maccormack, bool weno_interpolation ) {
		//
		if( use_maccormack ) {
			//
			shared_macarray2<float> velocity_0(v_in.type()), velocity_1(v_in.type());
			shared_macarray2<float2> minMax_u(v_in.shape());
			//
			advect_semiLagrangian_u(v_in,v,velocity_0(),&minMax_u(),fluid,dt,weno_interpolation);
			advect_semiLagrangian_u(velocity_0(),v,velocity_1(),nullptr,fluid,-dt,weno_interpolation);
			//
			v_out.clear();
			v_out.activate_as(v_in);
			//
			v_out.parallel_actives([&](int dim, int i, int j, auto &it, int tn) {
				//
				if( minMax_u()[dim](i,j).within_narrowband) {
					it.set(velocity_0()[dim](i,j));
				} else {
					double min_value = minMax_u()[dim](i,j).v[0];
					double max_value = minMax_u()[dim](i,j).v[1];
					double vel0 = velocity_0()[dim](i,j);
					double correction = 0.5*(v_in[dim](i,j)-velocity_1()[dim](i,j));
					if( vel0+correction < min_value ) it.set(min_value);
					else if( vel0+correction > max_value ) it.set(max_value);
					else it.set(vel0+correction);
				}
			});
		} else {
			advect_semiLagrangian_u(v_in,v,v_out,nullptr,fluid,dt,weno_interpolation);
		}
	}
	//
	void advect_semiLagrangian_cell ( const array2<float> &q_in, const macarray2<float> &v, array2<float> &q_out, array2<float2> *minMax, const array2<float> &fluid, double dt, bool weno_interpolation ) {
		//
		shared_array2<vec2f> full_velocity(m_shape);
		v.convert_to_full(full_velocity());
		//
		q_out.clear();
		q_out.activate_as(q_in);
		q_out.parallel_actives([&](int i, int j, auto &it, int tn) {
			//
			const vec2d &u = full_velocity()(i,j);
			//
			if( ! u.empty() ) {
				vec2d p = vec2d(i,j)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO2::interpolate(q_in,p);
				} else {
					value = array_interpolator2::interpolate<float>(q_in,p);
				}
				it.set(value);
			} else {
				it.set(q_in(i,j));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(q_in);
			//
			minMax->parallel_actives([&](int i, int j, auto &it, int tn) {
				//
				const vec2d &u = full_velocity()(i,j);
				//
				if( ! u.empty() ) {
					vec2d p = vec2d(i,j)-dt*u/m_dx;
					vec2i indices[4];
					double coef[4];
					array_interpolator2::interpolate_coef(q_in.shape(),p,indices,coef);
					double min_value = std::numeric_limits<float>::max();
					double max_value = std::numeric_limits<float>::min();
					for( int n=0; n<4; ++n ) {
						double value = q_in(indices[n]);
						min_value = std::min(min_value,value);
						max_value = std::max(max_value,value);
					}
					float2 d2;
					bool within_narrowband = array_interpolator2::interpolate(fluid,p) > -m_dx * (double)m_param.trim_narrowband;
					d2.v[0] = min_value;
					d2.v[1] = max_value;
					d2.within_narrowband = within_narrowband;
					it.set(d2);
				} else {
					float2 d2;
					bool within_narrowband = fluid(i,j) > -m_dx * (double)m_param.trim_narrowband;
					d2.v[0] = d2.v[1] = q_in(i,j);
					d2.within_narrowband = within_narrowband;
					it.set(d2);
				}
			});
		}
	}
	//
	void advect_cell ( const array2<float> &q_in, const macarray2<float> &v, array2<float> &q_out, const array2<float> &fluid, double dt, bool use_maccormack, bool weno_interpolation ) {
		//
		if( use_maccormack ) {
			//
			shared_array2<float> q_0(q_in.type()), q_1(q_in.type());
			shared_array2<float2> minMax_q(q_in.shape());
			//
			advect_semiLagrangian_cell(q_in,v,q_0(),&minMax_q(),fluid,dt,weno_interpolation);
			advect_semiLagrangian_cell(q_0(),v,q_1(),nullptr,fluid,-dt,weno_interpolation);
			//
			q_out.clear();
			q_out.activate_as(q_in);
			//
			q_out.parallel_actives([&](int i, int j, auto &it, int tn) {
				//
				if( minMax_q()(i,j).within_narrowband) {
					it.set(q_0()(i,j));
				} else {
					double min_value = minMax_q()(i,j).v[0];
					double max_value = minMax_q()(i,j).v[1];
					double q0 = q_0()(i,j);
					double correction = 0.5*(q_in(i,j)-q_1()(i,j));
					if( q0+correction < min_value ) it.set(min_value);
					else if( q0+correction > max_value ) it.set(max_value);
					else it.set(q0+correction);
				}
			});
		} else {
			advect_semiLagrangian_cell(q_in,v,q_out,nullptr,fluid,dt,weno_interpolation);
		}
	}
	//
	struct Parameters {
		bool use_maccormack {true};
		bool weno_interpolation {false};
		unsigned trim_narrowband {1};
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