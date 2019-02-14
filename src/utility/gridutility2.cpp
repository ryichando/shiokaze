/*
**	gridutility2.cpp
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
#include <shiokaze/utility/gridutility2_interface.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/array/array_utility2.h>
#include <shiokaze/array/array_interpolator2.h>
#include <shiokaze/array/array_derivative2.h>
#include <shiokaze/utility/utility.h>
#include <algorithm>
#include <cstdlib>
#include <stack>
#define _USE_MATH_DEFINES
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
class gridutility2 : public gridutility2_interface {
private:
	//
	virtual void convert_to_cell( const array2<double> &nodal_array, array2<double> &result ) const override {
		//
		result.clear();
		for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) {
			result.activate_as(nodal_array,-vec2i(ii,jj));
		}
		result.parallel_actives([&](int i, int j, auto& it, int tn) {
			double value (0.0);
			double wsum = 0.0;
			for( unsigned ii=0; ii<2; ii++ ) for( unsigned jj=0; jj<2; jj++ ) {
				value += nodal_array(i+ii,j+jj);
				wsum ++;
			}
			value = value / wsum;
			it.set(value);
		});
	}
	virtual void combine_levelset( const array2<double> &solid, const array2<double> &fluid, array2<double> &combined, double solid_offset=0.0 ) const override {
		//
		if( array_utility2::levelset_exist(solid) ) {
			//
			shared_array2<double> copy_solid(fluid.type());
			if( fluid.shape() == solid.shape()) {
				copy_solid->copy(solid);
			} else {
				convert_to_cell(solid,copy_solid());
			}
			copy_solid->set_as_levelset(m_dx);
			copy_solid->flood_fill();
			//
			combined.activate_as(fluid);
			combined.activate_as(copy_solid());
			combined.parallel_actives([&](int i, int j, auto& it, int tn) {
				it.set(std::max(fluid(i,j),-solid_offset-copy_solid()(i,j)));
			});
			combined.set_as_levelset(m_dx);
			combined.flood_fill();
			//
		} else {
			combined.copy(fluid);
		}
	}
	virtual void extrapolate_levelset( const array2<double> &solid, array2<double> &fluid, double threshold=0.0 ) const override {
		//
		if( array_utility2::levelset_exist(solid) ) {
			//
			shared_array2<double> combined(fluid.type());
			combine_levelset(solid,fluid,combined(),m_dx);
			fluid.copy(combined());
			shared_array2<double> old_fluid = shared_array2<double>(fluid);
			//
			bool is_fluid_nodal = fluid.shape() == m_shape.nodal();
			//
			const double limit_y = sin(M_PI/4.0);
			fluid.parallel_actives([&](int i, int j, auto &it, int tn) {
				vec2d p = is_fluid_nodal ? vec2d(i,j) : vec2i(i,j).cell();
				double solid_levelset = is_fluid_nodal ? solid(i,j) : array_interpolator2::interpolate<double>(solid,p);
				//
				if( solid_levelset < threshold && solid_levelset > -m_param.extrapolation_toward_solid*m_dx ) {
					if( m_param.solid_wall_extrapolation ) {
						double derivative[DIM2];
						array_derivative2::derivative(solid,p,derivative);
						vec2d normal = vec2d(derivative).normal();
						if( normal.norm2() ) {
							vec2d index_p_n = vec2d(i,j)+(-solid_levelset/m_dx)*normal;
							double value = array_interpolator2::interpolate<double>(old_fluid(),index_p_n);
							if( m_param.horizontal_solid_extrapolation && normal[1] < limit_y ) {
								vec2d normal_horizontal = normal;
								normal_horizontal[1] = 0.0;
								normal_horizontal.normalize();
								if( normal_horizontal.norm2() ) {
									vec2d index_p_h = vec2d(i,j)+(-solid_levelset/m_dx)*normal_horizontal;
									value = std::min(value,array_interpolator2::interpolate<double>(old_fluid(),index_p_h));
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
			fluid.dilate([&]( int i, int j, auto &it, int tn) {
				if( it() >= 0.0 ) {
					vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
					for( int nq=0; nq<4; nq++ ) {
						vec2i qi (query[nq]);
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
			fluid.flood_fill();
		}
	}
	virtual void compute_gradient( const array2<double> &levelset, array2<vec2d> &gradient ) const override {
		//
		gradient.activate_as(levelset);
		gradient.dilate();
		gradient.parallel_actives([&](int i, int j, auto &it, int tn) {
			vec2d grad;
			for( unsigned dim : DIMS2 ) {
				grad[dim] = 
					array_interpolator2::interpolate<double>(levelset,vec2i(i,j).cell()+0.5*vec2d(dim==0,dim==1)) -
					array_interpolator2::interpolate<double>(levelset,vec2i(i,j).cell()-0.5*vec2d(dim==0,dim==1));
			}
			it.set(grad/m_dx);
		});
	}
	virtual unsigned mark_topology( const array2<char> &flag, array2<unsigned> &topology_array ) const override {
		//
		auto markable = [&]( const vec2i &q ) { return flag(q) && ! topology_array(q); };
		auto recursive_mark = [&]( vec2i node, unsigned topology_index ) {
			std::stack<vec2i> queue;
			queue.push(node);
			while(! queue.empty()) {
				vec2i q = queue.top();
				queue.pop();
				topology_array.set(q,topology_index);
				vec2i nq;
				for( unsigned dim : DIMS2 ) {
					if( q[dim]<flag.shape()[dim]-1 && markable(nq=(q+vec2i(dim==0,dim==1)))) queue.push(nq);
					if( q[dim]>0 && markable(nq=(q-vec2i(dim==0,dim==1)))) queue.push(nq);
				}
			}
		};
		//
		unsigned topology_index (0);
		flag.const_serial_actives([&](int i, int j, const auto &it) {
			if( markable(vec2i(i,j)) ) {
				recursive_mark(vec2i(i,j),++topology_index);
			}
		});
		return topology_index;
	}
	virtual void mark_narrowband( array2<double> &levelset, unsigned half_cells ) const override {
		//
		shared_array2<double> old_levelset(levelset);
		levelset.parallel_actives([&](int i, int j, auto &it, int tn) {
			//
			vec2i ij (i,j);
			const double &phi = it();
			bool should_set_off(true);
			for( int dim : DIMS2 ) {
				if( ij[dim] > 0 ) {
					vec2i np(i-(dim==0),j-(dim==1));
					if( old_levelset->active(np) ) {
						if( phi * old_levelset()(np) < 0.0 ) {
							should_set_off = false;
							break;
						}
					}
				}
				if( ij[dim] < levelset.shape()[dim]-1 ) {
					vec2i np(i+(dim==0),j+(dim==1));
					if( old_levelset->active(np) ) {
						if( phi * old_levelset()(i+(dim==0),j+(dim==1)) < 0.0 ) {
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
			for( int count=0; count<half_cells-1; ++count) levelset.dilate([&](int i, int j, auto &it, int tn) {
				vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
				double extrapolated_value (0.0);
				for( int nq=0; nq<4; nq++ ) {
					const vec2i &qi = query[nq];
					if( ! levelset.shape().out_of_bounds(qi) ) {
						if( levelset.active(qi) ) {
							const double &value = levelset(qi);
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
	void mark_narrowband( const array2<double> &levelset, array2<char> &flag, unsigned half_cells ) const {
		//
		flag.clear(0);
		flag.activate_as(levelset);
		flag.parallel_actives([&](int i, int j, auto &it, int tn) {
			//
			vec2i ij (i,j);
			const double &phi = levelset(i,j);
			bool should_set_off(true);
			//
			for( int dim : DIMS2 ) {
				if( should_set_off && ij[dim] > 0 ) {
					if( phi * levelset(i-(dim==0),j-(dim==1)) < 0.0 ) {
						it.set(phi < 0.0 ? -1 : 1);
						should_set_off = false;
						break;
					}
				}
				if( should_set_off && ij[dim] < levelset.shape()[dim]-1 ) {
					if( phi * levelset(i+(dim==0),j+(dim==1)) < 0.0 ) {
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
			for( int count=0; count<half_cells-1; ++count) flag.dilate([&](int i, int j, auto &it, int tn) {
				vec2i query[] = {vec2i(i+1,j),vec2i(i-1,j),vec2i(i,j+1),vec2i(i,j-1)};
				for( int nq=0; nq<4; nq++ ) {
					const vec2i &qi = query[nq];
					if( ! flag.shape().out_of_bounds(qi) ) {
						const char &value = flag(qi);
						if( value ) {
							it.set(value < 0 ? -2-(char)count : 2+(char)count);
							break;
						}
					}
				}
			});
		}
	}
	virtual void trim_narrowband( array2<double> &levelset, unsigned half_cells ) const override {
		//
		shared_array2<char> flag(levelset.shape());
		mark_narrowband(levelset,flag(),half_cells);
		levelset.parallel_actives([&](int i, int j, auto &it, int tn) {
			if( ! flag()(i,j)) it.set_off();
		});
	}
	virtual double get_area( const array2<double> &solid, const array2<double> &fluid ) const override {
		//
		shared_array2<double> combined = shared_array2<double>(fluid.type());
		combine_levelset(solid,fluid,combined());
		//
		std::vector<double> volume_buckets(combined->get_thread_num(),0.0);
		auto shrunk_shape = combined->shape()-shape2(1,1);
		//
		auto accumulation_body = [&]( int i, int j, int tn ) {
			double cell_fluid[2][2];
			for( int ii=0; ii<2; ++ii ) for( int jj=0; jj<2; ++jj ) {
				cell_fluid[ii][jj] = combined()(i+ii,j+jj);
			}
			volume_buckets[tn] += utility::get_area(cell_fluid);
		};
		combined->const_parallel_inside([&](int i, int j, const auto &it, int tn) {
			if( ! shrunk_shape.out_of_bounds(i,j)) accumulation_body(i,j,tn);
		});
		combined->const_parallel_actives([&](int i, int j, const auto &it, int tn) {
			if( ! shrunk_shape.out_of_bounds(i,j) && ! it.filled()) accumulation_body(i,j,tn);
		});
		//
		double volume (0.0);
		for( auto e : volume_buckets ) volume += e;
		volume = (m_dx*m_dx) * volume;
		return volume;
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_bool("SolidWallExtrapolation", m_param.solid_wall_extrapolation,"Should extrapolate towards solid");
		config.get_bool("HorizontalSolidWallExtrapolation", m_param.horizontal_solid_extrapolation,"Should extrapolate horizontally");
		config.get_double("ExtrapolationDepth",m_param.extrapolation_toward_solid,"Solid extrapolation depth");
	}
	virtual void initialize( const shape2 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	struct Parameters {
		//
		bool solid_wall_extrapolation {true};
		bool horizontal_solid_extrapolation {true};
		double extrapolation_toward_solid {1.0};
	};
	//
	Parameters m_param;
	double m_dx;
	shape2 m_shape;
};
//
extern "C" module * create_instance() {
	return new gridutility2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//