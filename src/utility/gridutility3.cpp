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
#include <shiokaze/array/array_utility3.h>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/array/array_derivative3.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/console.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stack>
#include "mc_table3.h"
#define _USE_MATH_DEFINES
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
#define VERIFICATION_TEST		0
//
class gridutility3 : public gridutility3_interface {
private:
	//
	virtual void convert_to_cell( const array3<double> &nodal_array, array3<double> &result ) const override {
		//
		result.clear();
		for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) for( unsigned kk=0; kk<2; kk++ ) {
			result.activate_as(nodal_array,-vec3i(ii,jj,kk));
		}
		auto accessors = nodal_array.get_const_accessors(result.get_thread_num());
		result.parallel_actives([&](int i, int j, int k, auto& it, int tn) {
			double value (0.0);
			double wsum = 0.0;
			for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) for( unsigned kk=0; kk<2; kk++ ) {
				value += accessors[tn](i+ii,j+jj,k+kk);
				wsum ++;
			}
			value = value / wsum;
			it.set(value);
		});
	}
	virtual void combine_levelset( const array3<double> &solid, const array3<double> &fluid, array3<double> &combined, double solid_offset=0.0 ) const override {
		//
		if( array_utility3::levelset_exist(solid) ) {
			shared_array3<double> copy_solid(fluid.type());
			if( fluid.shape() == solid.shape()) {
				copy_solid->copy(solid);
			} else {
				convert_to_cell(solid,copy_solid());
			}
			copy_solid->set_as_levelset(m_dx);
			copy_solid->flood_fill();
			//
			auto fluid_accessors = fluid.get_const_accessors(combined.get_thread_num());
			auto copy_solid_accessors = copy_solid->get_const_accessors();
			//
			combined.activate_as(fluid);
			combined.activate_as(copy_solid());
			combined.parallel_actives([&](int i, int j, int k, auto& it, int tn) {
				it.set(std::max(fluid_accessors[tn](i,j,k),-solid_offset-copy_solid_accessors[tn](i,j,k)));
			});
			combined.set_as_levelset(m_dx);
			combined.flood_fill();
		} else {
			combined.copy(fluid);
		}
	}
	virtual void extrapolate_levelset( const array3<double> &solid, array3<double> &fluid, double threshold=0.0 ) const override {
		//
		if( array_utility3::levelset_exist(solid) ) {
			//
			shared_array3<double> combined(fluid.type());
			combine_levelset(solid,fluid,combined(),m_dx);
			fluid.copy(combined());
			shared_array3<double> old_fluid = shared_array3<double>(fluid);
			//
			auto solid_accessors = solid.get_const_accessors();
			auto old_fluid_accessors = old_fluid->get_const_accessors();
			bool is_fluid_nodal = fluid.shape() == m_shape.nodal();
			//
			const double limit_y = sin(M_PI/4.0);
			fluid.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
				vec3d p = is_fluid_nodal ? vec3d(i,j,k) : vec3i(i,j,k).cell();
				double solid_levelset = is_fluid_nodal ? solid_accessors[tn](i,j,k) : array_interpolator3::interpolate<double>(solid_accessors[tn],p);
				if( solid_levelset < threshold && solid_levelset > -m_param.extrapolation_toward_solid*m_dx ) {
					if( m_param.solid_wall_extrapolation ) {
						double derivative[DIM3];
						array_derivative3::derivative(solid_accessors[tn],p,derivative);
						vec3d normal = vec3d(derivative).normal();
						for( unsigned dim : DIMS3 ) {
							p[dim] = std::min(1.0,std::max(0.0,p[dim]));
						}
						if( normal.norm2() ) {
							vec3d index_p_n = vec3d(i,j,k)+(-solid_levelset/m_dx)*normal;
							double value = array_interpolator3::interpolate<double>(old_fluid_accessors[tn],index_p_n);
							if( m_param.horizontal_solid_extrapolation && normal[1] < limit_y ) {
								vec3d normal_horizontal = normal;
								normal_horizontal[1] = 0.0;
								normal_horizontal.normalize();
								if( normal_horizontal.norm2() ) {
									vec3d index_p_h = vec3d(i,j,k)+(-solid_levelset/m_dx)*normal_horizontal;
									value = std::min(value,array_interpolator3::interpolate<double>(old_fluid_accessors[tn],index_p_h));
								}
							}
							it.set(value);
						} else {
							it.set(fluid.get_background_value());
						}
					} else {
						double value = std::max(it(),solid_levelset);
						it.set(solid_levelset);
					}
				}
			});
			//
			auto fluid_accessors = fluid.get_const_accessors();
			fluid.dilate([&]( int i, int j, int k, auto &it, int tn) {
				if( it() >= 0.0 ) {
					vec3i query[] = {vec3i(i+1,j,k),vec3i(i-1,j,k),vec3i(i,j+1,k),
								 	 vec3i(i,j-1,k),vec3i(i,j,k-1),vec3i(i,j,k+1)};
					for( int nq=0; nq<6; nq++ ) {
						vec3i qi (query[nq]);
						if( ! m_shape.out_of_bounds(qi) ) {
							if( fluid_accessors[tn](qi) < 0.0 ) {
								it.set(m_dx);
								break;
							}
						}
					}
				}
			});
			//
			fluid.flood_fill();
		}
	}
	virtual void compute_gradient( const array3<double> &levelset, array3<vec3d> &gradient ) const override {
		//
		auto levelset_accessors = levelset.get_const_accessors();
		gradient.activate_as(levelset);
		gradient.dilate();
		gradient.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			vec3d grad;
			for( unsigned dim : DIMS3 ) {
				grad[dim] = 
					array_interpolator3::interpolate<double>(levelset_accessors[tn],vec3i(i,j,k).cell()+0.5*vec3d(dim==0,dim==1,dim==2)) -
					array_interpolator3::interpolate<double>(levelset_accessors[tn],vec3i(i,j,k).cell()-0.5*vec3d(dim==0,dim==1,dim==2));
			}
			it.set(grad/m_dx);
		});
	}
	virtual unsigned mark_topology( const array3<char> &flag, array3<unsigned> &topology_array ) const override {
		//
		auto markable = [&]( const vec3i &q ) { return flag(q) && ! topology_array(q); };
		auto recursive_mark = [&]( vec3i node, unsigned topology_index ) {
			std::stack<vec3i> queue;
			queue.push(node);
			while(! queue.empty()) {
				vec3i q = queue.top();
				queue.pop();
				topology_array.set(q,topology_index);
				vec3i nq;
				for( unsigned dim : DIMS3 ) {
					if( q[dim]<flag.shape()[dim]-1 && markable(nq=(q+vec3i(dim==0,dim==1,dim==2)))) queue.push(nq);
					if( q[dim]>0 && markable(nq=(q-vec3i(dim==0,dim==1,dim==2)))) queue.push(nq);
				}
			}
		};
		//
		unsigned topology_index (0);
		flag.const_serial_actives([&](int i, int j, int k, const auto &it) {
			if( markable(vec3i(i,j,k)) ) {
				recursive_mark(vec3i(i,j,k),++topology_index);
			}
		});
		return topology_index;
	}
	virtual void mark_narrowband( array3<double> &levelset, unsigned half_cells ) const override {
		//
		shared_array3<double> old_levelset(levelset);
		auto old_levelset_accessors = old_levelset->get_const_accessors();
		levelset.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			vec3i ijk (i,j,k);
			const double &phi = it();
			bool should_set_off(true);
			for( int dim : DIMS3 ) {
				if( ijk[dim] > 0 ) {
					vec3i np(i-(dim==0),j-(dim==1),k-(dim==2));
					if( old_levelset_accessors[tn].active(np) ) {
						if( phi * old_levelset_accessors[tn](np) < 0.0 ) {
							should_set_off = false;
							break;
						}
					}
				}
				if( ijk[dim] < levelset.shape()[dim]-1 ) {
					vec3i np(i+(dim==0),j+(dim==1),k+(dim==2));
					if( old_levelset_accessors[tn].active(np) ) {
						if( phi * old_levelset_accessors[tn](np) < 0.0 ) {
							should_set_off = false;
							break;
						}
					}
				}
			}
			if( should_set_off ) it.set_off();
		});
		//
		if( half_cells > 1 ) {
			auto levelset_accessors = levelset.get_const_accessors();
			for( int count=0; count<half_cells-1; ++count) levelset.dilate([&](int i, int j, int k, auto &it, int tn) {
				vec3i query[] = {vec3i(i-1,j,k),vec3i(i+1,j,k),vec3i(i,j-1,k),vec3i(i,j+1,k),vec3i(i,j,k-1),vec3i(i,j,k+1)};
				double extrapolated_value (0.0);
				for( int nq=0; nq<6; nq++ ) {
					const vec3i &qi = query[nq];
					if( ! levelset.shape().out_of_bounds(qi) ) {
						if( levelset_accessors[tn].active(qi) ) {
							const double &value = levelset_accessors[tn](qi);
							if( value < 0.0 ) {
								extrapolated_value = std::min(extrapolated_value,extrapolated_value-m_dx);
								break;
							} else {
								extrapolated_value = std::max(extrapolated_value,extrapolated_value+m_dx);
								break;
							}
						}
					}
				}
				it.set(extrapolated_value);
			});
		}
	}
	void mark_narrowband( const array3<double> &levelset, array3<char> &flag, unsigned half_cells ) const {
		//
		auto levelset_accessors = levelset.get_const_accessors();
		flag.clear(0);
		flag.activate_as(levelset);
		flag.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			vec3i ijk (i,j,k);
			const double &phi = levelset_accessors[tn](i,j,k);
			bool should_set_off(true);
			//
			for( int dim : DIMS3 ) {
				if( should_set_off && ijk[dim] > 0 ) {
					if( phi * levelset_accessors[tn](i-(dim==0),j-(dim==1),k-(dim==2)) < 0.0 ) {
						it.set(phi < 0.0 ? -1 : 1);
						should_set_off = false;
						break;
					}
				}
				if( should_set_off && ijk[dim] < levelset.shape()[dim]-1 ) {
					if( phi * levelset_accessors[tn](i+(dim==0),j+(dim==1),k+(dim==2)) < 0.0 ) {
						it.set(phi < 0.0 ? -1 : 1);
						should_set_off = false;
						break;
					}
				}
			}
			if( should_set_off ) it.set_off();
		});
		//
		if( half_cells > 1 ) {
			auto flag_accessors = flag.get_const_accessors();
			for( int count=0; count<half_cells-1; ++count) flag.dilate([&](int i, int j, int k, auto &it, int tn) {
				vec3i query[] = {vec3i(i-1,j,k),vec3i(i+1,j,k),vec3i(i,j-1,k),vec3i(i,j+1,k),vec3i(i,j,k-1),vec3i(i,j,k+1)};
				for( int nq=0; nq<6; nq++ ) {
					const vec3i &qi = query[nq];
					if( ! flag.shape().out_of_bounds(qi) ) {
						const char &value = flag_accessors[tn](qi);
						if( value ) {
							it.set(value < 0 ? -2-(char)count : 2+(char)count);
							break;
						}
					}
				}
			});
		}
	}
	virtual void trim_narrowband( array3<double> &levelset, unsigned half_cells ) const override {
		//
		shared_array3<char> flag(levelset.shape());
		mark_narrowband(levelset,flag(),levelset.get_levelset_halfwidth());
		auto flag_accessors = flag->get_const_accessors();
		levelset.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			if( ! flag_accessors[tn](i,j,k)) it.set_off();
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
	virtual double get_volume( const array3<double> &solid, const array3<double> &fluid ) const override {
		//
		shared_array3<double> combined = shared_array3<double>(fluid.type());
		combine_levelset(solid,fluid,combined());
		//
		auto combined_accessors = combined->get_const_accessors();
		std::vector<double> volume_buckets(combined_accessors.size(),0.0);
		auto shrunk_shape = combined->shape()-shape3(1,1,1);
		//
		auto accumulation_body = [&]( int i, int j, int k, int tn ) {
			double cell_fluid[2][2][2];
			for( int ii=0; ii<2; ++ii ) for( int jj=0; jj<2; ++jj ) for( int kk=0; kk<2; ++kk ) {
				cell_fluid[ii][jj][kk] = combined_accessors[tn](i+ii,j+jj,k+kk);
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
	virtual bool assign_visualizable_solid( const dylibloader &dylib, double dx, array3<double> &solid ) const override {
		//
		bool is_nodal = solid.shape() == m_shape.nodal();
		bool is_cell = solid.shape() == m_shape.cell();
		//
		auto solid_visualize_func = reinterpret_cast<double(*)(const vec3d &)>(dylib.load_symbol("solid_visualize"));
		solid.clear(1.0);
		//
		if( solid_visualize_func ) {
			solid.set_as_levelset(dx);
			solid.parallel_all([&](int i, int j, int k, auto &it) {
				double value = (*solid_visualize_func)( is_nodal ? dx*vec3i(i,j,k).nodal() : dx*vec3i(i,j,k).cell());
				if( std::abs(value) < 3.0*dx ) it.set(value);
			});
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