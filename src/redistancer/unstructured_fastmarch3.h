/*
**	unstructured_fastmarch3.h
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
#ifndef SHKZ_UNSTRUCTURED_FASTMARCH3_H
#define SHKZ_UNSTRUCTURED_FASTMARCH3_H
//
#include <vector>
#include <algorithm>
#include <shiokaze/math/vec.h>
#include <list>
#include <numeric>
#include <functional>
#include "matinv.h"
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/utility/meshutility3_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class unstructured_fastmarch3 {
public:
	//
	static void fastmarch(
					std::function<vec3r( size_t i )> position_func,
					std::function<void( size_t i, std::function<void( size_t j )> func )> iterate_connections,
					std::vector<Real> &levelset,
					std::vector<char> &fixed,
					double distance,
					const parallel_driver &parallel,
					const meshutility3_interface *meshutility ) {
		//
		// Initialize
		parallel.for_each(fixed.size(),[&](size_t n) {
			if( ! fixed[n] ) levelset[n] = std::copysign(distance,levelset[n]);
		});
		//
		// Propagate
		size_t prev_count_unfixed (0);
		while( true ) {
			//
			// Set front distance
			std::vector<double> min_dx_slot(parallel.get_thread_num(),distance);
			parallel.for_each(fixed.size(),[&](size_t n, int tid) {
				if( ! fixed[n] ) {
					iterate_connections(n,[&]( size_t m ) {
						if( fixed[m]) {
							min_dx_slot[tid] = std::min(min_dx_slot[tid],
								std::abs(levelset[m])+2.0*(position_func(m)-position_func(n)).len());
						}
					});
				}
			});
			//
			double front_distance (distance);
			for( const auto &e : min_dx_slot ) front_distance = std::min(front_distance,e);
			//
			std::vector<char> fixed_save (fixed);
			std::vector<Real> levelset_save (levelset);
			parallel.for_each(fixed.size(),[&](size_t n) {
				//
				if( ! fixed_save[n] ) {
					//
					// Pick neighboring nodes
					std::vector<size_t> tri;
					tri.push_back(n);
					bool has_connection (false);
					iterate_connections(n,[&]( size_t m ) {
						has_connection = true;
						if( fixed_save[m]) {
							if( std::abs(levelset_save[m]) < front_distance &&
								levelset_save[n] * levelset_save[m] > 0.0 &&
								std::abs(levelset_save[m]) < std::abs(levelset_save[n]) ) tri.push_back(m);
						}
					});
					if( ! has_connection ) {
						fixed[n] = true;
					}
					//
					// Find the number of valid connections
					size_t num_valid = tri.size();
					if( num_valid > 1 ) {
						//
						// Sort by distance
						std::vector<char> order_map(num_valid);
						std::iota(order_map.begin(),order_map.end(), 0);
						std::sort(order_map.begin()+1,order_map.end(),[&](char a, char b){
							return std::abs(levelset_save[tri[a]]) < std::abs(levelset_save[tri[b]]);
						});
						//
						// Compute shape function if necessary
						double M[4][4];
						for( char i=0; i<4; i++ ) for( char j=0; j<4; j++ ) M[i][j] = 0.0;
						if( num_valid > 2 ) {
							//
							bool succeeded = true;
							if( num_valid >= 4 ) {
								//
								num_valid = 4;
								double A4[4][4];
								double M4[4][4];
								for( char i=0; i<4; i++ ) for( char j=0; j<4; j++ ) {
									if( i < 3 ) A4[i][j] = position_func(tri[order_map[j]])[i];
									else A4[i][j] = 1.0;
								}
								succeeded = matinv<double>::invert4x4(A4,M4);
								if( succeeded ) {
									for( char i=0; i<4; i++ ) for( char j=0; j<4; j++ ) M[i][j] = M4[i][j];
								} else num_valid = 3;
							}
							if( num_valid == 3 ) {
								double A3[3][3];
								double M3[3][3];
								// Project triangle onto 2D beforehand
								vec3d proj_p[3] = { position_func(tri[order_map[0]]), position_func(tri[order_map[1]]), position_func(tri[order_map[2]]) };
								project_triangle(proj_p);
								for( char i=0; i<3; i++ ) for( char j=0; j<3; j++ ) {
									if( i < 2 ) A3[i][j] = proj_p[j][i];
									else A3[i][j] = 1.0;
								}
								succeeded = matinv<double>::invert3x3(A3,M3);
								if( succeeded ) {
									for( char i=0; i<3; i++ ) for( char j=0; j<3; j++ ) M[i][j] = M3[i][j];
								} else num_valid = 2;
							}
						}
						//
						// Levelset extrapolation
						Real sgn = levelset_save[n] > 0.0 ? 1 : -1;
						if( num_valid > 2 ) {
							//
							// Build quadric equation
							vec3d det;
							vec3d coef;
							for( char dim=0; dim<num_valid-1; dim++ ) {
								det[dim] = M[0][dim];
								for( size_t k=1; k<num_valid; k++ ) {
									coef[dim] += M[k][dim]*levelset_save[tri[order_map[k]]];
								}
							}
							// Compute quadric coefficients
							double A = det.norm2();
							double B = 2.*det*coef;
							double C = coef.norm2()-1.0;
							assert(A);
							double D = B/A;
							levelset[n] = sgn*0.5*sqrtf(std::max(1e-8,D*D-4.0*C/A))-0.5*D;
						}
						//
						if( num_valid == 2 ) {
							//
							// If only one neighbor is fixed, just copy that one
							levelset[n] = levelset_save[tri[order_map[1]]]+sgn*(position_func(tri[order_map[1]])-position_func(n)).len();
						}
						//
						fixed[n] = true;
						//
						// Clamp
						Real levelset_min (1.0);
						Real levelset_max (-1.0);
						for( unsigned n=1; n<tri.size(); ++n ) {
							levelset_min = std::min(levelset_min,levelset_save[tri[n]]);
							levelset_max = std::max(levelset_max,levelset_save[tri[n]]);
						}
						//
						if( sgn < 0.0 ) levelset[n] = std::min(levelset[n],levelset_max);
						else levelset[n] = std::max(levelset[n],levelset_min);
					}
				}
			});
			//
			size_t count_unfixed (0);
			for( const auto &e : fixed ) if( ! e ) count_unfixed ++;
			if( ! count_unfixed || prev_count_unfixed == count_unfixed ) break;
			prev_count_unfixed = count_unfixed;
		}
	}
	//
private:
	//
	static bool project_triangle( vec3d *points, unsigned num=3 ) {
		//
		// Compute normal vector of this triangle
		for( unsigned i=0; i<3; i++ ) for( unsigned j=i+1; j<3; j++ ) {
			if( ! (points[i]-points[j]).norm2() ) return false;
		}
		//
		vec3d n = ((points[2]-points[0])^(points[1]-points[0])).normal();
		vec3d e[2];
		e[0] = (points[1]-points[0]).normal();
		e[1] = n ^ e[0];
		//
		// Take a dot product
		std::vector<vec3d> old_points(num);
		for( unsigned i=0; i<num; i++ ) old_points[i] = points[i];
		for( unsigned i=0; i<num; i++ ) {
			points[i][0] = e[0] * (old_points[i]-old_points[0]);
			points[i][1] = e[1] * (old_points[i]-old_points[0]);
			points[i][2] = 0.0;
		}
		return true;
	}
};
//
SHKZ_END_NAMESPACE
//
#endif