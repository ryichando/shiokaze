/*
**	unstructured_fastmarch2.h
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
#ifndef SHKZ_UNSTRUCTURED_FASTMARCH2_H
#define SHKZ_UNSTRUCTURED_FASTMARCH2_H
//
#include <vector>
#include <algorithm>
#include <shiokaze/math/vec.h>
#include <list>
#include <numeric>
#include "matinv.h"
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/utility/meshutility2_interface.h>
//
SHKZ_BEGIN_NAMESPACE
//
class unstructured_fastmarch2 {
public:
	//
	static void fastmarch(
					const std::vector<vec2d> &positions,
					const std::vector<std::vector<size_t> > &_connections,
					std::vector<double> &levelset,
					std::vector<char> &fixed,
					double distance,
					const parallel_driver &parallel,
					const meshutility2_interface *meshutility ) {
		//
		// Copy the connection information
		std::vector<std::vector<size_t> > connections = _connections;
		//
		// Find intersections
		fixed.resize(positions.size());
		parallel.for_each(fixed.size(),[&]( size_t n ) {
			//
			if( ! fixed[n] ) {
				double levelset0 = levelset[n];
				for( unsigned k=0; k<connections[n].size(); ++k ) {
					size_t m = connections[n][k];
					double levelset1 = levelset[m];
					if( levelset0 * levelset1 < 0.0 ) {
						fixed[n] = true;
						break;
					}
				}
			}
		});
		//
		// Reset level set values
		parallel.for_each(fixed.size(),[&]( size_t n ) {
			if( ! fixed[n] ) levelset[n] = std::copysign(distance,levelset[n]);
		});
		//
		// Gather unfixed nodes
		std::list<size_t> unfixed;
		for( size_t n=0; n<positions.size(); n++ ) {
			if( ! connections[n].empty() && ! fixed[n] ) unfixed.push_back(n);
		}
		//
		// Now repeat the propagation...
		while( true ) {
			//
			// Gather narrow band nodes
			std::list<size_t> narrowlist;
			for( typename std::list<size_t>::iterator it=unfixed.begin(); it!=unfixed.end(); it++ ) {
				size_t n = *it;
				if( ! connections[n].empty() && ! fixed[n] ) {
					//
					// Sort connection by distance
					std::sort(connections[n].begin(), connections[n].end(), [&](size_t a, size_t b){ return std::abs(levelset[a]) < std::abs(levelset[b]); });
					size_t min_index = connections[n][0];
					//
					// Collect narrow bands
					if( fixed[min_index] ) {
						if( std::abs(levelset[min_index]) < distance ) {
							narrowlist.push_back(n);
						}
					}
				}
			}
			//
			// If not found, just leave the loop
			if( narrowlist.empty() ) break;
			//
			// Find the minimum edge length and min distance
			double ds (1.0);
			double dist (distance);
			for( typename std::list<size_t>::iterator it=narrowlist.begin(); it!=narrowlist.end(); it++ ) {
				size_t n = *it;
				if( ! connections[n].empty() ) {
					size_t k = connections[n][0];
					if( fixed[k] ) {
						ds = std::min(ds,(positions[n]-positions[k]).len());
						dist = std::min(dist,std::abs(levelset[k]));
					}
				}
			}
			//
			// Cut out the narrow bands to regularize the propagation speed
			for( typename std::list<size_t>::iterator it=narrowlist.begin(); it!=narrowlist.end(); ) {
				size_t n = *it;
				if( ! connections[n].empty() ) {
					it ++;
					continue;
				}
				size_t k = connections[n][0];
				if( std::abs(levelset[k]) > dist+ds ) {
					it = narrowlist.erase(it);
				} else {
					it ++;
				}
			}
			//
			// Tranfer to vector container
			std::vector<size_t> narrowbands;
			narrowbands.insert(narrowbands.end(),narrowlist.begin(),narrowlist.end());
			//
			// Propagate once
			parallel.for_each(narrowbands.size(),[&](size_t _n) {
				//
				size_t n = narrowbands[_n];
				if( ! connections[n].empty() ) {
					//
					// Pick neighboring nodes
					std::vector<size_t> tri(connections[n].size()+1); tri[0] = n;
					for( size_t i=0; i<connections[n].size(); i++ ) tri[1+i] = connections[n][i];
					//
					// Find the number of valid connections
					size_t num_valid (0);
					if( connections[n].size() > 2 && fixed[tri[1]] && fixed[tri[2]] ) num_valid = 2;
					else if( connections[n].size() > 1 && fixed[tri[1]] ) num_valid = 1;
					//
					// Compute shape function if necessary
					double M[3][3];
					for( unsigned i=0; i<3; i++ ) for( unsigned j=0; j<3; j++ ) M[i][j] = 0.0;
					if( num_valid == 2 ) {
						double A[3][3];
						for( int i=0; i<num_valid+1; i++ ) for( int j=0; j<num_valid+1; j++ ) {
							if( i < num_valid ) A[i][j] = positions[tri[j]][i];
							else A[i][j] = 1.0;
						}
						if( ! matinv<double>::invert3x3(A,M)) num_valid = 1;
					}
					//
					// Levelset extrapolation
					int sgn = levelset[tri[0]] > 0.0 ? 1 : -1;
					if( num_valid == 2 ) {
						//
						// Build quadric equation
						vec2d det;
						vec2d coef;
						for( int dim=0; dim<num_valid; dim++ ) {
							det[dim] = M[0][dim];
							coef[dim] = 0.0;
							for( int k=1; k<num_valid+1; k++ ) {
								coef[dim] += M[k][dim]*levelset[tri[k]];
							}
						}
						// Compute quadric coefficients
						double A = det.norm2();
						double B = 2.*det*coef;
						double C = coef.norm2()-1.0;
						assert(A);
						double D = B/A;
						levelset[n] = sgn*0.5*sqrtf(std::max(1e-8,D*D-4.0*C/A))-0.5*D;
					} else if( num_valid == 1 ) {
						//
						// If only one neighbor is fixed, just copy that one
						levelset[n] = levelset[tri[1]]+sgn*(positions[tri[1]]-positions[tri[0]]).len();
					}
				}
			});
			//
			// Fix the narrow bands
			for( size_t _n=0; _n<narrowbands.size(); ++_n ) {
				size_t n = narrowbands[_n];
				if( ! connections[n].empty() ) {
					levelset[n] = std::min(distance,levelset[n]);
					fixed[n] = true;
				}
			}
		}
	}
};
//
SHKZ_END_NAMESPACE
//
#endif