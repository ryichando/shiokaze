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
private:
	//
	virtual void advect_scalar( array3<double> &scalar,				// Cell-centered
								const macarray3<double> &velocity,	// Face-located
								double dt, std::string name="scalar") override {
		//
		shared_array3<double> scalar0(scalar);
		advect_cell(scalar0(),velocity,scalar,dt,m_param.use_maccormack,m_param.weno_interpolation,name);
	}
	//
	virtual void advect_vector( macarray3<double> &u,				// Face-located
								const macarray3<double> &velocity,	// Face-located
								double dt, std::string name="vector" ) override {
		//
		shared_macarray3<double> u0(u);
		advect_u(u0(),velocity,u,dt,m_param.use_maccormack,m_param.weno_interpolation,name);
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_bool("MacCormack",m_param.use_maccormack,"Whether to use MacCormack method");
		config.get_bool("WENO",m_param.weno_interpolation,"Whether to use WENO interpolation for advection");
	}
	//
	virtual void initialize( const shape3 &shape, double dx ) override {
		//
		m_shape = shape;
		m_dx = dx;
	}
	//
	using double2 = struct { double v[2] = { 0.0, 0.0 }; };
	void advect_semiLagrangian_u ( const macarray3<double> &v_in, const macarray3<double> &v, macarray3<double> &v_out, macarray3<double2> *minMax, double dt, bool weno_interpolation ) {
		//
		v_out.clear();
		v_out.activate_as(v_in);
		auto v_in_accessors = v_in.get_const_accessors();
		//
		shared_macarray3<vec3d> face_full_velocity(m_shape);
		v_in.convert_to_full(face_full_velocity());
		//
		auto face_full_velocity_accessors = face_full_velocity->get_const_accessors();
		v_out.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
			//
			const vec3d &u = face_full_velocity_accessors[tn](dim,i,j,k);
			//
			if( ! u.empty() ) {
				vec3d p = vec3d(i,j,k)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO3::interpolate(v_in_accessors[tn].get(dim),p);
				} else {
					value = array_interpolator3::interpolate<double>(v_in_accessors[tn].get(dim),p);
				}
				it.set(value);
			} else {
				it.set(v_in_accessors[tn](dim,i,j,k));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(v_in);
			minMax->parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				//
				const vec3d &u = face_full_velocity_accessors[tn](dim,i,j,k);
				//
				if( ! u.empty() ) {
					vec3d p = vec3d(i,j,k)-dt*u/m_dx;
					vec3i indices[8];
					double coef[8];
					array_interpolator3::interpolate_coef(v_in[dim].shape(),p,indices,coef);
					double min_value = std::numeric_limits<double>::max();
					double max_value = std::numeric_limits<double>::min();
					for( int n=0; n<8; ++n ) {
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
						d2.v[0] = d2.v[1] = v_in_accessors[tn](dim,i,j,k);
						it.set(d2);
					}
				}
			});
		}
	}
	//
	void advect_u ( const macarray3<double> &v_in, const macarray3<double> &v, macarray3<double> &v_out, double dt, bool use_maccormack, bool weno_interpolation, std::string name ) {
		//
		scoped_timer timer(this);
		//
		if( use_maccormack ) {
			//
			shared_macarray3<double> velocity_0(v_in.type()), velocity_1(v_in.type());
			shared_macarray3<double2> minMax_u(v_in.shape());
			//
			timer.tick(); console::dump( ">>> Advecting %s with the MacCormack advection (%s)...\n", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			timer.tick(); console::dump( "Forward advection...");
			advect_semiLagrangian_u(v_in,v,velocity_0(),&minMax_u(),dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("u_maccormack_forward_advection_"+name).c_str());
			timer.tick(); console::dump( "Backward advection...");
			advect_semiLagrangian_u(velocity_0(),v,velocity_1(),nullptr,-dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("u_maccormack_backward_advection_"+name).c_str());
			timer.tick(); console::dump( "Computing the final velocity...");
			//
			v_out.clear();
			v_out.activate_as(v_in);
			//
			auto minMax_u_accessors = minMax_u->get_const_accessors();
			auto v_in_accessors = v_in.get_const_accessors();
			auto v_0_accessors = velocity_0->get_const_accessors();
			auto v_1_accessors = velocity_1->get_const_accessors();
			//
			v_out.parallel_actives([&](int dim, int i, int j, int k, auto &it, int tn) {
				//
				double min_value = minMax_u_accessors[tn](dim,i,j,k).v[0];
				double max_value = minMax_u_accessors[tn](dim,i,j,k).v[1];
				double vel0 = v_0_accessors[tn](dim,i,j,k);
				double correction = 0.5*(v_in_accessors[tn](dim,i,j,k)-v_1_accessors[tn](dim,i,j,k));
				if( vel0+correction < min_value ) it.set(min_value);
				else if( vel0+correction > max_value ) it.set(max_value);
				else it.set(vel0+correction);
			});
			//
			console::dump( "Done. Took %s\n", timer.stock("maccormack_final_velocity_compute_u_"+name).c_str());
			console::dump( "<<< MacCormack advection done. Took %s\n", timer.stock("maccormack_u_"+name).c_str());
		} else {
			timer.tick(); console::dump( "Advecting %s by the semi-lagrangian advection (%s)...", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			advect_semiLagrangian_u(v_in,v,v_out,nullptr,dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("semilagrangian_u_"+name).c_str());
		}
	}
	//
	void advect_semiLagrangian_cell ( const array3<double> &q_in, const macarray3<double> &v, array3<double> &q_out, array3<double2> *minMax, double dt, bool weno_interpolation ) {
		//
		shared_array3<vec3d> full_velocity(m_shape);
		v.convert_to_full(full_velocity());
		//
		auto full_velocity_accessors = full_velocity->get_const_accessors();
		auto q_in_accessors = q_in.get_const_accessors();
		//
		q_out.clear();
		q_out.activate_as(q_in);
		q_out.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			const vec3d &u = full_velocity_accessors[tn](i,j,k);
			//
			if( ! u.empty() ) {
				vec3d p = vec3d(i,j,k)-dt*u/m_dx;
				double value;
				if( weno_interpolation ) {
					value = WENO3::interpolate(q_in_accessors[tn],p);
				} else {
					value = array_interpolator3::interpolate<double>(q_in_accessors[tn],p);
				}
				it.set(value);
			} else {
				it.set(q_in_accessors[tn](i,j,k));
			}
		});
		//
		if( minMax ) {
			minMax->clear();
			minMax->activate_as(q_in);
			minMax->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				//
				const vec3d &u = full_velocity_accessors[tn](i,j,k);
				//
				if( ! u.empty() ) {
					vec3d p = vec3d(i,j,k)-dt*u/m_dx;
					vec3i indices[8];
					double coef[8];
					array_interpolator3::interpolate_coef(q_in.shape(),p,indices,coef);
					double min_value = std::numeric_limits<double>::max();
					double max_value = std::numeric_limits<double>::min();
					for( int n=0; n<8; ++n ) {
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
					d2.v[0] = d2.v[1] = q_in_accessors[tn](i,j,k);
					it.set(d2);
				}
			});
		}
	}
	//
	void advect_cell ( const array3<double> &q_in, const macarray3<double> &v, array3<double> &q_out, double dt, bool use_maccormack, bool weno_interpolation, std::string name ) {
		//
		scoped_timer timer(this);
		//
		if( use_maccormack ) {
			//
			shared_array3<double> q_0(q_in.type()), q_1(q_in.type());
			shared_array3<double2> minMax_q(q_in.shape());
			//
			timer.tick(); console::dump( ">>> Advecting %s with the MacCormack advection (%s)...\n", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			timer.tick(); console::dump( "Forward advection...");
			advect_semiLagrangian_cell(q_in,v,q_0(),&minMax_q(),dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("cell_maccormack_forward_advection_"+name).c_str());
			timer.tick(); console::dump( "Backward advection...");
			advect_semiLagrangian_cell(q_0(),v,q_1(),nullptr,-dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("cell_maccormack_backward_advection_"+name).c_str());
			timer.tick(); console::dump( "Computing the final velocity...");
			//
			q_out.clear();
			q_out.activate_as(q_in);
			//
			auto minMax_q_accessors = minMax_q->get_const_accessors();
			auto q_in_accessors = q_in.get_const_accessors();
			auto q_0_accessors = q_0->get_const_accessors();
			auto q_1_accessors = q_1->get_const_accessors();
			//
			q_out.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				//
				double min_value = minMax_q_accessors[tn](i,j,k).v[0];
				double max_value = minMax_q_accessors[tn](i,j,k).v[1];
				double q0 = q_0_accessors[tn](i,j,k);
				double correction = 0.5*(q_in_accessors[tn](i,j,k)-q_1_accessors[tn](i,j,k));
				if( q0+correction < min_value ) it.set(min_value);
				else if( q0+correction > max_value ) it.set(max_value);
				else it.set(q0+correction);
				//
			});
			console::dump( "Done. Took %s\n", timer.stock("cell_maccormack_final_velocity_compute_"+name).c_str());
			console::dump( "<<< MacCormack advection done. Took %s\n", timer.stock("maccormack_cell_"+name).c_str());
		} else {
			timer.tick(); console::dump( "Advecting %s by the semi-lagrangian advection (%s)...", name.c_str(), weno_interpolation ? "WENO" : "Bilinear");
			advect_semiLagrangian_cell(q_in,v,q_out,nullptr,dt,weno_interpolation);
			console::dump( "Done. Took %s\n", timer.stock("semilagrangian_cell_"+name).c_str());
		}
	}
	//
	struct Parameters {
		bool use_maccormack {true};
		bool weno_interpolation {false};
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