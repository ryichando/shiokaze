/*
**	macadvection3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 24, 2017. 
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
#include <shiokaze/advection/macadvection3_interface.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/math/WENO3.h>
#include <limits>
//
SHKZ_USING_NAMESPACE
//
class macadvection3 : public macadvection3_interface {
protected:
	//
	MODULE_NAME("macadvection3")
	//
	virtual void advect_scalar( array3<float> &scalar,				// Cell-centered
								const macarray3<float> &velocity,	// Face-located
								const array3<float> &fluid,		// Fluid level set
								double dt, std::string name="scalar") override {
		//
		shared_array3<float> scalar0(scalar);
		advect_cell(scalar0(),velocity,scalar,fluid,dt,m_param.use_maccormack,m_param.weno_interpolation,name);
	}
	//
	virtual void advect_vector( macarray3<float> &u,				// Face-located
								const macarray3<float> &velocity,	// Face-located
								const array3<float> &fluid,		// Fluid level set
								double dt, std::string name="vector" ) override {
		//
		shared_macarray3<float> u0(u);
		advect_u(u0(),velocity,u,fluid,dt,m_param.use_maccormack,m_param.weno_interpolation,name);
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_bool("MacCormack",m_param.use_maccormack,"Whether to use MacCormack method");
		config.get_bool("WENO",m_param.weno_interpolation,"Whether to use WENO interpolation for advection");
		config.get_unsigned( "TrimNarrowBand",m_param.trim_narrowband,"Narrow band count to turn to semi-Lagrangian advection");
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	using float2 = struct { float v[2] = { 0.0, 0.0 }; bool within_narrowband{false}; };
	void advect_semiLagrangian_u ( const macarray3<float> &v_in, const macarray3<float> &v, macarray3<float> &v_out, macarray3<float2> *minMax, const array3<float> &fluid, double dt, bool weno_interpolation ) {
		//
		v_out.clear();
		v_out.activate_as(v_in);
		//
		shared_macarray3<vec3f> face_full_velocity(m_shape);
		v_in.convert_to_full(face_full_velocity());
		//
		v_out.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
			//
			const vec3f &u = face_full_velocity()[dim](i,j,k);
			//
			if( ! u.empty() ) {
				vec3d p = vec3d(i,j,k)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO3::interpolate(v_in[dim],p);
				} else {
					value = array_interpolator3::interpolate<float>(v_in[dim],p);
				}
				it.set(value);
			} else {
				it.set(v_in[dim](i,j,k));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(v_in);
			minMax->parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				//
				const vec3d &u = face_full_velocity()[dim](i,j,k);
				//
				if( ! u.empty() ) {
					vec3d p = vec3d(i,j,k)-dt*u/m_dx;
					vec3d face_p = vec3i(i,j,k).face(dim)-dt*u/m_dx;
					vec3i indices[8];
					double coef[8];
					array_interpolator3::interpolate_coef(v_in[dim].shape(),p,indices,coef);
					double min_value = std::numeric_limits<double>::max();
					double max_value = std::numeric_limits<double>::min();
					for( int n=0; n<8; ++n ) {
						double value = v_in[dim](indices[n]);
						min_value = std::min(min_value,value);
						max_value = std::max(max_value,value);
					}
					float2 d2;
					bool within_narrowband = array_interpolator3::interpolate(fluid,face_p-vec3d(0.5,0.5,0.5)) > -m_dx * (double)m_param.trim_narrowband;
					d2.v[0] = min_value;
					d2.v[1] = max_value;
					d2.within_narrowband = within_narrowband;
					it.set(d2);
				} else {
					if( minMax ) {
						vec3d face_p = vec3i(i,j,k).face(dim);
						float2 d2;
						bool within_narrowband = array_interpolator3::interpolate(fluid,face_p-vec3d(0.5,0.5,0.5)) > -m_dx * (double)m_param.trim_narrowband;
						d2.v[0] = d2.v[1] = v_in[dim](i,j,k);
						d2.within_narrowband = within_narrowband;
						it.set(d2);
					}
				}
			});
		}
	}
	//
	void advect_u ( const macarray3<float> &v_in, const macarray3<float> &v, macarray3<float> &v_out, const array3<float> &fluid, double dt, bool use_maccormack, bool weno_interpolation, std::string name ) {
		//
		scoped_timer timer(this);
		//
		if( use_maccormack ) {
			//
			shared_macarray3<float> velocity_0(v_in.type()), velocity_1(v_in.type());
			shared_macarray3<float2> minMax_u(v_in.shape());
			//
			timer.tick(); console::dump( ">>> Advecting %s with the MacCormack advection (%s)...\n", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			timer.tick(); console::dump( "Forward advection...");
			advect_semiLagrangian_u(v_in,v,velocity_0(),&minMax_u(),fluid,dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("u_maccormack_forward_advection_"+name).c_str());
			timer.tick(); console::dump( "Backward advection...");
			advect_semiLagrangian_u(velocity_0(),v,velocity_1(),nullptr,fluid,-dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("u_maccormack_backward_advection_"+name).c_str());
			timer.tick(); console::dump( "Computing the final velocity...");
			//
			v_out.clear();
			v_out.activate_as(v_in);
			v_out.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				//
				if( minMax_u()[dim](i,j,k).within_narrowband) {
					it.set(velocity_0()[dim](i,j,k));
				} else {
					double min_value = minMax_u()[dim](i,j,k).v[0];
					double max_value = minMax_u()[dim](i,j,k).v[1];
					double vel0 = velocity_0()[dim](i,j,k);
					double correction = 0.5*(v_in[dim](i,j,k)-velocity_1()[dim](i,j,k));
					if( vel0+correction < min_value ) it.set(min_value);
					else if( vel0+correction > max_value ) it.set(max_value);
					else it.set(vel0+correction);
				}
			});
			//
			console::dump( "Done. Took %s\n", timer.stock("maccormack_final_velocity_compute_u_"+name).c_str());
			console::dump( "<<< MacCormack advection done. Took %s\n", timer.stock("maccormack_u_"+name).c_str());
		} else {
			timer.tick(); console::dump( "Advecting %s by the semi-lagrangian advection (%s)...", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			advect_semiLagrangian_u(v_in,v,v_out,nullptr,fluid,dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("semilagrangian_u_"+name).c_str());
		}
	}
	//
	void advect_semiLagrangian_cell ( const array3<float> &q_in, const macarray3<float> &v, array3<float> &q_out, array3<float2> *minMax, const array3<float> &fluid, double dt, bool weno_interpolation ) {
		//
		shared_array3<vec3f> full_velocity(m_shape);
		v.convert_to_full(full_velocity());
		//
		q_out.clear();
		q_out.activate_as(q_in);
		q_out.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			const vec3d &u = full_velocity()(i,j,k);
			//
			if( ! u.empty() ) {
				vec3d p = vec3d(i,j,k)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO3::interpolate(q_in,p);
				} else {
					value = array_interpolator3::interpolate<float>(q_in,p);
				}
				it.set(value);
			} else {
				it.set(q_in(i,j,k));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(q_in);
			minMax->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				//
				const vec3d &u = full_velocity()(i,j,k);
				//
				if( ! u.empty() ) {
					vec3d p = vec3d(i,j,k)-dt*u/m_dx;
					vec3i indices[8];
					double coef[8];
					array_interpolator3::interpolate_coef(q_in.shape(),p,indices,coef);
					double min_value = std::numeric_limits<float>::max();
					double max_value = std::numeric_limits<float>::min();
					for( int n=0; n<8; ++n ) {
						double value = q_in(indices[n]);
						min_value = std::min(min_value,value);
						max_value = std::max(max_value,value);
					}
					float2 d2;
					bool within_narrowband = array_interpolator3::interpolate(fluid,p) > -m_dx * (double)m_param.trim_narrowband;
					d2.v[0] = min_value;
					d2.v[1] = max_value;
					d2.within_narrowband = within_narrowband;
					it.set(d2);
				} else {
					float2 d2;
					bool within_narrowband = fluid(i,j,k) > -m_dx * (double)m_param.trim_narrowband;
					d2.v[0] = d2.v[1] = q_in(i,j,k);
					d2.within_narrowband = within_narrowband;
					it.set(d2);
				}
			});
		}
	}
	//
	void advect_cell ( const array3<float> &q_in, const macarray3<float> &v, array3<float> &q_out, const array3<float> &fluid, double dt, bool use_maccormack, bool weno_interpolation, std::string name ) {
		//
		scoped_timer timer(this);
		//
		if( use_maccormack ) {
			//
			shared_array3<float> q_0(q_in.type()), q_1(q_in.type());
			shared_array3<float2> minMax_q(q_in.shape());
			//
			timer.tick(); console::dump( ">>> Advecting %s with the MacCormack advection (%s)...\n", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			timer.tick(); console::dump( "Forward advection...");
			advect_semiLagrangian_cell(q_in,v,q_0(),&minMax_q(),fluid,dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("cell_maccormack_forward_advection_"+name).c_str());
			timer.tick(); console::dump( "Backward advection...");
			advect_semiLagrangian_cell(q_0(),v,q_1(),nullptr,fluid,-dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("cell_maccormack_backward_advection_"+name).c_str());
			timer.tick(); console::dump( "Computing the final velocity...");
			//
			q_out.clear();
			q_out.activate_as(q_in);
			q_out.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				//
				if( minMax_q()(i,j,k).within_narrowband) {
					it.set(q_0()(i,j,k));
				} else {
					double min_value = minMax_q()(i,j,k).v[0];
					double max_value = minMax_q()(i,j,k).v[1];
					double q0 = q_0()(i,j,k);
					double correction = 0.5*(q_in(i,j,k)-q_1()(i,j,k));
					if( q0+correction < min_value ) it.set(min_value);
					else if( q0+correction > max_value ) it.set(max_value);
					else it.set(q0+correction);
				}
			});
			console::dump( "Done. Took %s\n", timer.stock("cell_maccormack_final_velocity_compute_"+name).c_str());
			console::dump( "<<< MacCormack advection done. Took %s\n", timer.stock("maccormack_cell_"+name).c_str());
		} else {
			timer.tick(); console::dump( "Advecting %s by the semi-lagrangian advection (%s)...", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			advect_semiLagrangian_cell(q_in,v,q_out,nullptr,fluid,dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("semilagrangian_cell_"+name).c_str());
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
	shape3 m_shape;
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new macadvection3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//