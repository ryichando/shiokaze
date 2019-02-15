/*
**	fastmarch2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 3, 2017. 
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
#include <shiokaze/redistancer/redistancer2_interface.h>
#include <shiokaze/utility/meshutility2_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/array/shared_array2.h>
#include <shiokaze/utility/gridutility2_interface.h>
#include <list>
#include "matinv.h"
//
SHKZ_USING_NAMESPACE
//
typedef struct _node2 {
	//
	vec2d p;					// Position
	double levelset;			// Levelset (give sign info as input for levelset extrapolation)
	bool fixed;					// Fixed flag
	std::vector<_node2 *> p2p;	// Connection
	//
	///////// Just ignore below ///////////
	double new_levelset;
	bool operator()(const _node2* lhs, const _node2* rhs) const {
		return fabs(lhs->levelset) < fabs(rhs->levelset);
	}
} node2;
//
static bool fast_march( std::vector<node2 *> &nodes, double maxdist, double mindist, const parallel_driver &parallel ) {
	//
	// Check the sign of coefficients
	assert(mindist<=0);
	assert(maxdist>=0);
	//
	// Initialize
	for( size_t n=0; n<nodes.size(); n++ ) {
		node2 *node = nodes[n];
		node->new_levelset = 0.0;
	}
	//
	// Gather unfixed nodes
	std::list<node2 *> unfixed;
	for( size_t n=0; n<nodes.size(); n++ ) {
		node2 *node = nodes[n];
		if( ! node->p2p.empty() && ! node->fixed ) {
			node->levelset = node->new_levelset = (node->levelset > 0.0 ? maxdist : mindist);
			unfixed.push_back(node);
		}
	}
	//
	// Now repeat the propagation...
	while( true ) {
		//
		// Gather narrow band nodes
		std::list<node2 *> narrowList;
		for( typename std::list<node2 *>::iterator it=unfixed.begin(); it!=unfixed.end(); it++ ) {
			node2 *node = *it;
			if( ! node->p2p.empty() && ! node->fixed ) {
				// Sort order of connections
				std::sort(node->p2p.begin(),node->p2p.end(),node2());
				// Collect narrow bands
				node2 *neigh = node->p2p[0];
				if( neigh->fixed ) {
					if( neigh->levelset < maxdist && neigh->levelset > mindist ) {
						narrowList.push_back(node);
					}
				}
			}
		}
		//
		// If not found, just leave the loop
		if( narrowList.empty() ) break;
		//
		// Find the minimum edge length and min distance
		double ds (1.0);
		double dist (1.0);
		for( typename std::list<node2 *>::iterator it=narrowList.begin(); it!=narrowList.end(); it++ ) {
			node2 *node = *it;
			if( ! node->p2p.empty() ) {
				node2 *neigh = node->p2p[0];
				if( neigh->fixed ) {
					ds = std::min(ds,(node->p-neigh->p).len());
					dist = std::min(dist,std::abs(neigh->levelset));
				}
			}
		}
		//
		// Cut out the narrow bands to regularize the propagation speed
		for( typename std::list<node2 *>::iterator it=narrowList.begin(); it!=narrowList.end(); ) {
			node2 *node = *it;
			if( node->p2p.empty() ) continue;
			node2 *neigh = node->p2p[0];
			if( fabs(neigh->levelset) > dist+ds ) {
				it = narrowList.erase(it);
			} else {
				it ++;
			}
		}
		//
		// Tranfer to vector container
		std::vector<node2 *> narrowNodes;
		narrowNodes.insert(narrowNodes.end(),narrowList.begin(),narrowList.end());
		//
		// Propagate once
		parallel.for_each(narrowNodes.size(),[&](size_t pindex) {
			node2 *node = narrowNodes[pindex];
			if( ! node->p2p.empty() ) {
				//
				// Pick neighboring nodes
				std::vector<node2 *> tri(node->p2p.size()+1); tri[0] = node;
				for( size_t i=0; i<node->p2p.size(); i++ ) tri[1+i] = node->p2p[i];
				//
				// Find the number of valid connections
				size_t numValid=0;
				if( node->p2p.size() > 2 && tri[1]->fixed && tri[2]->fixed ) numValid = 2;
				else if( node->p2p.size() > 1 && tri[1]->fixed ) numValid = 1;
				//
				// Compute shape function if necessary
				double M[3][3];
				if( numValid == 2 ) {
					double A[3][3];
					for( int i=0; i<numValid+1; i++ ) for( int j=0; j<numValid+1; j++ ) {
						if( i < numValid ) A[i][j] = tri[j]->p[i];
						else A[i][j] = 1.0;
					}
					if( ! matinv<double>::invert3x3(A,M)) numValid = 1;
				}
				// Levelset extrapolation
				int sgn = tri[0]->levelset > 0.0 ? 1 : -1;
				if( numValid == 2 ) {
					// Build quadric equation
					vec2d det;
					vec2d coef;
					for( int dim=0; dim<numValid; dim++ ) {
						det[dim] = M[0][dim];
						coef[dim] = 0.0;
						for( int k=1; k<numValid+1; k++ ) {
							coef[dim] += M[k][dim]*tri[k]->new_levelset;
						}
					}
					// Compute quadric coefficients
					double A = det.norm2();
					double B = 2.*det*coef;
					double C = coef.norm2()-1.0;
					if( A ) {
						double D = B/A;
						node->new_levelset = sgn*0.5*sqrtf(std::max(1e-8,D*D-4.0*C/A))-0.5*D;
					} else {
						printf( "determinant was zero !\n" );
						exit(0);
					}
				} else if( numValid == 1 ) {
					// If only one neighbor is fixed, just copy that one
					node->new_levelset = tri[1]->new_levelset+sgn*(tri[1]->p-tri[0]->p).len();
				} else {
					node->new_levelset = node->levelset;
				}
			}
		});
		//
		// Fix the narrow bands
		parallel.for_each(narrowNodes.size(),[&](size_t n) {
			node2 *node = narrowNodes[n];
			if( ! node->p2p.empty() ) {
				node->levelset = std::max(mindist,std::min(maxdist,node->new_levelset));
				node->fixed = true;
			}
		});
	}
	return true;
}
//
class fastmarch2 : public redistancer2_interface {
private:
	//
	LONG_NAME("FastMarch 2D")
	ARGUMENT_NAME("FastMarch")
	//
	virtual void redistance( array2<double> &phi_array, unsigned width, double dx ) override {
		//
		double half_bandwidth = dx * width;
		typedef struct { bool known; double dist; } grid;
		//
		// Flood fill and dilate
		phi_array.flood_fill();
		phi_array.dilate(2);
		//
		// Generate contours
		shared_array2<std::vector<vec2d> > contours(phi_array.shape()-shape2(1,1));
		contours->activate_as(phi_array);
		contours->parallel_actives([&](int i, int j, auto &it, int tn) {
			vec2d lines[8];	double v[2][2]; int pnum; vec2d vertices[2][2];
			bool skip (false);
			for( int ni=0; ni<2; ni++ ) for( int nj=0; nj<2; nj++ ) {
				if( skip ) break;
				if( phi_array.active(i+ni,j+nj)) {
					v[ni][nj] = phi_array(i+ni,j+nj);
					vertices[ni][nj] = dx*vec2d(i+ni,j+nj);
				} else {
					skip = true;
					break;
				}
			}
			if( ! skip ) {
				m_meshutility->march_points(v,vertices,lines,pnum,false);
				if( pnum ) {
					for( unsigned n=0; n<pnum; ++n ) {
						auto ptr = it.ptr();
						if( ptr ) ptr->push_back(lines[n]);
						else it.set({lines[n]});
					}
				}
			}
		});
		//
		// Compute the closest distance
		shared_array2<grid *> grids(phi_array.shape());
		//
		grids->activate_as(phi_array);
		grids->parallel_actives([&](int i, int j, auto &it, int tn) {
			//
			double min_d = 1.0;
			vec2d origin = dx*vec2d(i,j);
			double sgn = (phi_array(i,j)>0.0 ? 1.0 : -1.0);
			auto ptr = it();
			//
			int w (1);
			for( int ni=i-w; ni<=i+w-1; ++ni ) for( int nj=j-w; nj<=j+w-1; ++nj ) {
				if( ! contours->shape().out_of_bounds(ni,nj)) {
					if( contours->active(ni,nj) ) {
						const std::vector<vec2d> &lines = contours()(ni,nj);
						for( int m=0; m<lines.size(); m+=2 ) {
							vec2d out = origin;
							m_meshutility->distance(lines[m],lines[m+1],out);
							double d = (out-origin).len();
							if( d < min_d ) {
								if( ! ptr ) {
									ptr = new grid;
									it.set(ptr);
								}
								ptr->dist = sgn * d;
								ptr->known = true;
								min_d = d;
							}
						}
					}
				}
			}
			if( ! ptr || ! ptr->known ) it.set_off();
		});
		//
		// Trim to just near the surface
		m_gridutility->mark_narrowband(phi_array,width);
		//
		// Setup network
		shared_array2<node2 *> node_array(phi_array.shape());
		node_array->activate_as(phi_array);
		node_array->parallel_actives([&](int i, int j, auto &it) {
			it.set(new node2);
		});
		//
		node_array->const_parallel_actives([&](int i, int j, const auto &it, int tn) {
			//
			vec2d p = dx*vec2d(i,j);
			const auto ptr = it();
			const auto grid_ptr = grids()(i,j);
			ptr->p = p;
			if( grid_ptr ) {
				ptr->fixed = grid_ptr->known;
				ptr->levelset = grid_ptr->dist;
			} else {
				double sgn = (phi_array(i,j)>0.0 ? 1.0 : -1.0);
				ptr->fixed = false;
				ptr->levelset = sgn * half_bandwidth;
			}
			//
			int q[][DIM2] = {{i-1,j},{i+1,j},{i,j-1},{i,j+1}};
			for( unsigned n=0; n<4; n++ ) {
				if( ! node_array->shape().out_of_bounds(q[n]) && node_array->active(q[n])) {
					ptr->p2p.push_back(node_array()(q[n]));
				}
			}
		});
		//
		std::vector<node2 *> nodes;
		node_array->const_serial_actives([&](const auto &it) {
			nodes.push_back(it());
		});
		//
		// Perform fast march
		fast_march(nodes,half_bandwidth,-half_bandwidth,m_parallel);
		//
		phi_array.parallel_actives([&](int i, int j, auto &it, int tn) {
			it.set(node_array()(i,j)->levelset);
		});
		//
		grids->serial_actives([&](const auto &it) {
			const auto ptr = it();
			if( ptr ) delete ptr;
		});
		node_array->serial_actives([&](auto &it) { delete it(); });
		phi_array.flood_fill();
	}
	//
	meshutility2_driver m_meshutility{this,"meshutility2"};
	gridutility2_driver m_gridutility{this,"gridutility2"};
	parallel_driver m_parallel{this};
	//
};
//
extern "C" module * create_instance() {
	return new fastmarch2;
}
//
extern "C" const char *license() {
	return "MIT";
}
//