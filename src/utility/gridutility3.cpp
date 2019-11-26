/*
**	gridutility3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 15, 2017.
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
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/array/shared_bitarray3.h>
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/array_derivative3.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/console.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include "../cellmesher/mc_table.h"
#define _USE_MATH_DEFINES
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
#define VERIFICATION_TEST		0
//
class gridutility3 : public gridutility3_interface {
protected:
	//
	virtual void convert_to_cell( const array3<Real> &nodal_array, array3<Real> &result ) const override {
		//
		result.clear();
		for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) for( unsigned kk=0; kk<2; kk++ ) {
			result.activate_as(nodal_array,-vec3i(ii,jj,kk));
		}
		result.parallel_actives([&](int i, int j, int k, auto& it, int tn) {
			double value (0.0);
			double wsum = 0.0;
			for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) for( unsigned kk=0; kk<2; kk++ ) {
				value += nodal_array(i+ii,j+jj,k+kk);
				wsum ++;
			}
			value = value / wsum;
			it.set(value);
		});
	}
	virtual void combine_levelset( const array3<Real> &solid, const array3<Real> &fluid, array3<Real> &combined, double solid_offset=0.0 ) const override {
		//
		if( array_utility3::levelset_exist(solid) ) {
			shared_array3<Real> copy_solid(fluid.type());
			if( fluid.shape() == solid.shape()) {
				copy_solid->copy(solid);
			} else {
				convert_to_cell(solid,copy_solid());
			}
			copy_solid->flood_fill();
			//
			combined.activate_as(fluid);
			combined.activate_as(copy_solid());
			combined.parallel_actives([&](int i, int j, int k, auto& it, int tn) {
				it.set(std::max(fluid(i,j,k),-(Real)solid_offset-copy_solid()(i,j,k)));
			});
			combined.set_type(fluid.type());
			combined.flood_fill();
		} else {
			combined.copy(fluid);
		}
	}
	virtual void extrapolate_levelset( const array3<Real> &solid, array3<Real> &fluid, double threshold=0.0 ) const override {
		//
		if( array_utility3::levelset_exist(solid) ) {
			//
			shared_array3<Real> old_fluid = shared_array3<Real>(fluid);
			bool is_fluid_nodal = fluid.shape() == m_shape.nodal();
			//
			const double limit_y = sin(M_PI/4.0);
			fluid.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				vec3d p = is_fluid_nodal ? vec3d(i,j,k) : vec3i(i,j,k).cell<double>();
				double solid_levelset = is_fluid_nodal ? solid(i,j,k) : array_interpolator3::interpolate<Real>(solid,p);
				if( solid_levelset < threshold ) {
					if( m_param.solid_wall_extrapolation ) {
						Real derivative[DIM3];
						array_derivative3::derivative(solid,p,derivative);
						vec3d normal = vec3d(derivative).normal();
						if( normal.norm2() ) {
							vec3d index_p_n = vec3d(i,j,k)+(-solid_levelset/m_dx)*normal;
							Real value = array_interpolator3::interpolate<Real>(old_fluid(),index_p_n);
							if( m_param.horizontal_solid_extrapolation && normal[1] < limit_y ) {
								vec3d normal_horizontal = normal;
								normal_horizontal[1] = 0.0;
								normal_horizontal.normalize();
								if( normal_horizontal.norm2() ) {
									vec3d index_p_h = vec3d(i,j,k)+(-solid_levelset/m_dx)*normal_horizontal;
									value = std::min(value,array_interpolator3::interpolate<Real>(old_fluid(),index_p_h));
								}
							}
							it.set(value);
						} else {
							it.set(fluid.get_background_value());
						}
					} else {
						Real value = std::max(it(),(Real)solid_levelset);
						it.set(solid_levelset);
					}
				}
			});
			//
			fluid.dilate([&]( int i, int j, int k, auto &it, int tn) {
				if( it() >= 0.0 ) {
					vec3i query[] = {vec3i(i+1,j,k),vec3i(i-1,j,k),vec3i(i,j+1,k),
								 	 vec3i(i,j-1,k),vec3i(i,j,k-1),vec3i(i,j,k+1)};
					for( int nq=0; nq<6; nq++ ) {
						vec3i qi (query[nq]);
						if( ! m_shape.out_of_bounds(qi) ) {
							if( fluid(qi) < 0.0 ) {
								it.set(m_dx);
								break;
							}
						}
					}
				}
			});
			//
			shared_array3<Real> combined(fluid.type());
			combine_levelset(solid,fluid,combined(),m_dx);
			fluid.copy(combined());
			fluid.flood_fill();
		}
	}
	virtual void compute_gradient( const array3<Real> &levelset, array3<vec3d> &gradient ) const override {
		//
		gradient.activate_as(levelset);
		gradient.dilate();
		gradient.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			vec3d grad;
			for( unsigned dim : DIMS3 ) {
				grad[dim] = 
					array_interpolator3::interpolate<Real>(levelset,vec3i(i,j,k).cell()+0.5*vec3d(dim==0,dim==1,dim==2)) -
					array_interpolator3::interpolate<Real>(levelset,vec3i(i,j,k).cell()-0.5*vec3d(dim==0,dim==1,dim==2));
			}
			it.set(grad/m_dx);
		});
	}
	virtual void trim_narrowband( array3<Real> &levelset ) const override {
		//
		shared_bitarray3 flag(levelset.shape());
		flag->activate_as<Real>(levelset);
		flag->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			vec3i ijk (i,j,k);
			const double &phi = levelset(i,j,k);
			bool should_set_off(true);
			//
			for( int dim : DIMS3 ) {
				if( should_set_off && ijk[dim] > 0 ) {
					if( levelset.active(i-(dim==0),j-(dim==1),k-(dim==2)) && phi * levelset(i-(dim==0),j-(dim==1),k-(dim==2)) < 0.0 ) {
						it.set();
						should_set_off = false;
						break;
					}
				}
				if( should_set_off && ijk[dim] < levelset.shape()[dim]-1 ) {
					if( levelset.active(i+(dim==0),j+(dim==1),k+(dim==2)) && phi * levelset(i+(dim==0),j+(dim==1),k+(dim==2)) < 0.0 ) {
						it.set();
						should_set_off = false;
						break;
					}
				}
			}
			if( should_set_off ) it.set_off();
		});
		//
		levelset.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			if( ! flag()(i,j,k)) it.set_off();
		});
	}
	//
	struct Triangle {
		vec3d p[3];
	};
	std::vector<Triangle> Polygonise( const double fluid[2][2][2] ) const {
		//
		std::vector<Triangle> result;
		double value[8];
		int flag (0);
		for(int n=0; n<8; ++n ) {
			value[n] = fluid[a2fVertexOffset[n][0]][a2fVertexOffset[n][1]][a2fVertexOffset[n][2]];
			if(value[n] < 0.0) flag |= 1 << n;
		}
		//
		int edge_flag = aiCubeEdgeFlags[flag];
		if( edge_flag ) {
			std::vector<vec3d> edge_vertices(12);
			for( int n=0; n<12; ++n ) {
				if(edge_flag & (1<<n)) {
					//
					size_t edge0 = a2iEdgeConnection[n][0];
					size_t edge1 = a2iEdgeConnection[n][1];
					//
					vec3d p1 = vec3d(
						a2fVertexOffset[edge0][0],
						a2fVertexOffset[edge0][1],
						a2fVertexOffset[edge0][2]);
					//
					vec3d p2 = vec3d(
						a2fVertexOffset[edge1][0],
						a2fVertexOffset[edge1][1],
						a2fVertexOffset[edge1][2]);
					//
					double v1 = value[edge0];
					double v2 = value[edge1];
					double fraction = utility::fraction(v1,v2);
					vec3d p;
					if( v1 < 0.0 ) {
						p = fraction*p2+(1.0-fraction)*p1;
					} else {
						p = fraction*p1+(1.0-fraction)*p2;
					}
					edge_vertices[n] = p;
				}
			}
			for( int n=0; n<=5; ++n) {
				if(a2iTriangleConnectionTable[flag][3*n] < 0) break;
				Triangle tri;
				for( int m=0; m<3; ++m) {
					tri.p[m] = edge_vertices[a2iTriangleConnectionTable[flag][3*n+m]];
				}
				result.push_back(tri);
			}
		}
		return result;
	}
	virtual double get_cell_volume( const double fluid[2][2][2] ) const override {
		//
		bool has_fluid (false);
		for( int ii=0; ii<2; ++ii ) for( int jj=0; jj<2; ++jj ) for( int kk=0; kk<2; ++kk ) {
			if( fluid[ii][jj][kk] < 0.0 ) {
				has_fluid = true;
				break;
			}
		}
		//
		double volume (0.0);
		if( has_fluid ) {
			//
	#if VERIFICATION_TEST
			double dummy[2][2][2];
			dummy[0][0][0] = -1.0;
			dummy[0][0][1] = 1.0;
			dummy[0][1][0] = -1.0;
			dummy[1][0][0] = -1.0;
			dummy[0][1][1] = 1.0;
			dummy[1][1][0] = -1.0;
			dummy[1][0][1] = 1.0;
			dummy[1][1][1] = 1.0;
			//
			std::vector<Triangle> triangles = Polygonise(dummy);
	#else
			std::vector<Triangle> triangles = Polygonise(fluid);
	#endif
			for( int i=0; i<triangles.size(); ++i ) {
				Triangle &triangle = triangles[i];
				vec3d vec1 = triangle.p[1]-triangle.p[0];
				vec3d vec2 = triangle.p[2]-triangle.p[0];
				vec3d area_vector = 0.5 * (vec1 ^ vec2);
				vec3d center = (triangle.p[0]+triangle.p[1]+triangle.p[2]) / 3.0;
				volume += (center * area_vector) / 3.0;
				//
	#if VERIFICATION_TEST
				console::dump( "area_vector = (%f,%f,%f)\n", area_vector[0], area_vector[1], area_vector[2]);
				exit(0);
	#endif
			}
			//
			// X flux
			double face_flux[2][2];
			face_flux[0][0] = fluid[1][0][0];
			face_flux[0][1] = fluid[1][0][1];
			face_flux[1][1] = fluid[1][1][1];
			face_flux[1][0] = fluid[1][1][0];
			volume += utility::get_area(face_flux) / 3.0;
			//
			// Y flux
			face_flux[0][0] = fluid[0][1][0];
			face_flux[0][1] = fluid[0][1][1];
			face_flux[1][1] = fluid[1][1][1];
			face_flux[1][0] = fluid[1][1][0];
			volume += utility::get_area(face_flux) / 3.0;
			//
			// Z flux
			face_flux[0][0] = fluid[0][0][1];
			face_flux[0][1] = fluid[0][1][1];
			face_flux[1][1] = fluid[1][1][1];
			face_flux[1][0] = fluid[1][0][1];
			volume += utility::get_area(face_flux) / 3.0;
		}
		//
		return volume;
	}
	virtual double get_volume( const array3<Real> &solid, const array3<Real> &fluid ) const override {
		//
		shared_array3<Real> combined(fluid.type());
		combine_levelset(solid,fluid,combined());
		//
		std::vector<Real> volume_buckets(combined->get_thread_num(),0.0);
		auto shrunk_shape = combined->shape()-shape3(1,1,1);
		//
		auto accumulation_body = [&]( int i, int j, int k, int tn ) {
			double cell_fluid[2][2][2];
			for( int ii=0; ii<2; ++ii ) for( int jj=0; jj<2; ++jj ) for( int kk=0; kk<2; ++kk ) {
				cell_fluid[ii][jj][kk] = combined()(i+ii,j+jj,k+kk);
				volume_buckets[tn] += get_cell_volume(cell_fluid);
			}
		};
		//
		combined->const_parallel_inside([&](int i, int j, int k, const auto &it, int tn) {
			if( ! shrunk_shape.out_of_bounds(i,j,k)) accumulation_body(i,j,k,tn);
		});
		combined->const_parallel_actives([&](int i, int j, int k, const auto &it, int tn) {
			if( ! shrunk_shape.out_of_bounds(i,j,k) && ! it.filled() ) accumulation_body(i,j,k,tn);
		});
		//
		double volume (0.0);
		for( auto e : volume_buckets ) volume += e;
		volume = (m_dx*m_dx*m_dx) * volume;
		return volume;
		//
	}
	virtual bool assign_visualizable_solid( const dylibloader &dylib, double dx, array3<Real> &solid ) const override {
		//
		bool is_nodal = solid.shape() == m_shape.nodal();
		bool is_cell = solid.shape() == m_shape.cell();
		//
		auto solid_visualize_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("solid_visualize"));
		solid.clear(1.0);
		//
		if( solid_visualize_func ) {
			solid.parallel_all([&](int i, int j, int k, auto &it) {
				double value = (*solid_visualize_func)( is_nodal ? dx*vec3i(i,j,k).nodal() : dx*vec3i(i,j,k).cell());
				if( std::abs(value) < 3.0*dx ) it.set(value);
			});
			solid.set_as_levelset(dx);
			solid.flood_fill();
			return true;
		} else {
			return false;
		}
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("SolidWallExtrapolation", m_param.solid_wall_extrapolation,"Should extrapolate towards solid");
		config.get_bool("HorizontalSolidWallExtrapolation", m_param.horizontal_solid_extrapolation,"Should extrapolate horizontally");
		config.get_double("ExtrapolationDepth",m_param.extrapolation_toward_solid,"Solid extrapolation depth");
	}
	virtual void initialize( const shape3 &shape, double dx ) override {
		m_dx = dx;
		m_shape = shape;
	}
	struct Parameters {
		//
		bool solid_wall_extrapolation {true};
		bool horizontal_solid_extrapolation {true};
		double extrapolation_toward_solid {1.0};
	};
	//
	Parameters m_param;
	double m_dx;
	shape3 m_shape;
};
//
extern "C" module * create_instance() {
	return new gridutility3();
}
//
extern "C" const char *license() {
	return "MIT";
}