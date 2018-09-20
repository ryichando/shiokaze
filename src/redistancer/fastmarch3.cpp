/*
**	fastmarch3.cpp
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
#include <shiokaze/pointgridhash/pointgridhash3_interface.h>
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <shiokaze/redistancer/redistancer3_interface.h>
#include <shiokaze/utility/meshutility3_interface.h>
#include <shiokaze/utility/gridutility3_interface.h>
#include <shiokaze/parallel/parallel_driver.h>
#include <shiokaze/core/console.h>
#include <shiokaze/array/shared_array3.h>
#include <list>
#include "matinv.h"
//
SHKZ_USING_NAMESPACE
//
typedef struct _node3 {
	//
	vec3d p;					// Position
	double levelset;			// Levelset (give sign info as input for levelset extrapolation)
	bool fixed;					// Fixed flag
	std::vector<_node3 *> p2p;	// Connection
	//
	///////// Just ignore below ///////////
	double new_levelset;
	bool operator()(const _node3* lhs, const _node3* rhs) const {
		return fabs(lhs->levelset) < fabs(rhs->levelset);
	}
} node3;
//
static bool projectTriangle( vec3d *points, unsigned num=3 ) {
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
//
static bool fastMarch( std::vector<node3 *> &nodes, double maxdist, double mindist, const parallel_driver &parallel ) {
	//
	// Check the sign of coefficients
	assert(mindist<=0);
	assert(maxdist>=0);
	//
	// Initialize
	for( size_t n=0; n<nodes.size(); n++ ) {
		node3 *node = nodes[n];
		node->new_levelset = 0.0;
	}
	//
	// Gather unfixed nodes
	std::list<node3 *> unfixed;
	for( size_t n=0; n<nodes.size(); n++ ) {
		node3 *node = nodes[n];
		if( ! node->fixed ) {
			node->levelset = (node->levelset > 0.0 ? maxdist : mindist);
			unfixed.push_back(node);
		}
	}
	//
	// Now repeat the propagation...
	while( true ) {
		//
		// Gather narrow band nodes
		std::list<node3 *> narrowList;
		for( typename std::list<node3 *>::iterator it=unfixed.begin(); it!=unfixed.end(); it++ ) {
			node3 *node = *it;
			if( ! node->p2p.empty() && ! node->fixed ) {
				// Sort order of connections
				std::sort(node->p2p.begin(),node->p2p.end(),node3());
				// Collect narrow bands
				node3 *neigh = node->p2p[0];
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
		double ds = 1.0;
		double dist = 1.0;
		for( typename std::list<node3 *>::iterator it=narrowList.begin(); it!=narrowList.end(); it++ ) {
			node3 *node = *it;
			if( ! node->p2p.empty() ) {
				node3 *neigh = node->p2p[0];
				if( neigh->fixed ) {
					ds = std::min(ds,(node->p-neigh->p).len());
					dist = std::min(dist,fabs(neigh->levelset));
				}
			}
		}
		//
		// Cut out the narrow bands to regularize the propagation speed
		for( typename std::list<node3 *>::iterator it=narrowList.begin(); it!=narrowList.end(); ) {
			node3 *node = *it;
			if( ! node->p2p.empty() ) {
				node3 *neigh = node->p2p[0];
				if( fabs(neigh->levelset) > dist+ds ) {
					it = narrowList.erase(it);
				} else {
					it ++;
				}
			}
		}
		//
		// Tranfer to vector container
		std::vector<node3 *> narrowNodes;
		narrowNodes.insert(narrowNodes.end(),narrowList.begin(),narrowList.end());
		//
		// Propagate once
		parallel.for_each(narrowNodes.size(),[&](size_t n) {
			//
			node3 *node = narrowNodes[n];
			if( ! node->p2p.empty() ) {
				//
				// Pick neighboring nodes
				std::vector<node3 *> tri(node->p2p.size()+1); tri[0] = node;
				for( size_t i=0; i<node->p2p.size(); i++ ) tri[1+i] = node->p2p[i];
				//
				// Find the number of valid connections
				size_t numValid=0;
				if( node->p2p.size() > 3 && tri[1]->fixed && tri[2]->fixed && tri[3]->fixed ) numValid = 3;
				else if( node->p2p.size() > 2 && tri[1]->fixed && tri[2]->fixed ) numValid = 2;
				else if( node->p2p.size() > 1 && tri[1]->fixed ) numValid = 1;
				//
				// Compute shape function if necessary
				double M[4][4];
				for( unsigned i=0; i<4; i++ ) for( unsigned j=0; j<4; j++ ) M[i][j] = 0.0;
				if( numValid >= 2 ) {
					bool succeeded = true;
					if( numValid == 3 ) {
						double A4[4][4];
						double M4[4][4];
						for( unsigned i=0; i<4; i++ ) for( unsigned j=0; j<4; j++ ) {
							if( i < 3 ) A4[i][j] = tri[j]->p[i];
							else A4[i][j] = 1.0;
						}
						succeeded = matinv<double>::invert4x4(A4,M4);
						if( succeeded ) {
							for( unsigned i=0; i<4; i++ ) for( unsigned j=0; j<4; j++ ) M[i][j] = M4[i][j];
						} else numValid = 2;
					}
					if( numValid == 2 ) {
						double A3[3][3];
						double M3[3][3];
						// Project triangle onto 2D beforehand
						vec3d proj_p[3] = { tri[0]->p, tri[1]->p, tri[2]->p };
						projectTriangle(proj_p);
						for( unsigned i=0; i<3; i++ ) for( unsigned j=0; j<3; j++ ) {
							if( i < 2 ) A3[i][j] = proj_p[j][i];
							else A3[i][j] = 1.0;
						}
						succeeded = matinv<double>::invert3x3(A3,M3);
						if( succeeded ) {
							for( unsigned i=0; i<3; i++ ) for( unsigned j=0; j<3; j++ ) M[i][j] = M3[i][j];
						} else numValid = 1;
					}
				}
				// Levelset extrapolation
				int sgn = tri[0]->levelset > 0.0 ? 1 : -1;
				if( numValid >= 2 ) {
					// Build quadric equation
					vec3d det;
					vec3d coef;
					for( unsigned dim=0; dim<numValid; dim++ ) {
						det[dim] = M[0][dim];
						coef[dim] = 0.0;
						for( size_t k=1; k<numValid+1; k++ ) {
							coef[dim] += M[k][dim]*tri[k]->new_levelset;
						}
					}
					// Compute quadric coefficients
					double A = det.norm2();
					double B = 2.*det*coef;
					double C = coef.norm2()-1.0;
					if( A ) {
						double D = B/A;
						node->new_levelset = sgn*0.5*sqrtf(fmax(1e-8,D*D-4.0*C/A))-0.5*D;
					} else {
						console::dump( "determinant was zero !\n" );
						exit(0);
					}
				} else if( numValid == 1 ) {
					// If only one neighbor is fixed, just copy that one
					node->new_levelset = tri[1]->new_levelset+sgn*(tri[1]->p-tri[0]->p).len();
				} else {
					node->new_levelset = node->new_levelset;
				}
			}
		});
		//
		// Fix the narrow bands
		parallel.for_each(narrowNodes.size(),[&](size_t n) {
			node3 *node = narrowNodes[n];
			if( ! node->p2p.empty() ) {
				node->levelset = fmax(mindist,fmin(maxdist,node->new_levelset));
				node->fixed = true;
			}
		});
	}
	return true;
}
//
class fastmarch3 : public redistancer3_interface {
private:
	//
	LONG_NAME("FastMarch 3D")
	ARGUMENT_NAME("FastMarch")
	//
	virtual void redistance( array3<double> &phi_array, double dx ) override {
		//
		unsigned half_cells = phi_array.get_levelset_halfwidth();
		double half_bandwidth = dx * (double)half_cells;
		typedef struct { bool known; double dist; } grid;
		//
		// Generate a mesh
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		m_mesher->generate_mesh(phi_array,vertices,faces);
		//
		// Sort triangles
		std::vector<vec3d> face_centers(faces.size());
		m_parallel.for_each(faces.size(),[&](size_t n) {
			vec3d center = (vertices[faces[n][0]]+vertices[faces[n][1]]+vertices[faces[n][2]]) / 3.0;
			face_centers[n] = center;
		});
		m_hashtable->sort_points(face_centers);
		//
		// Compute the closest distance
		shared_array3<grid *> grids(phi_array.shape());
		//
		auto grids_accessors = grids->get_const_accessors();
		auto phi_array_accessors = phi_array.get_const_accessors();
		//
		grids->activate_as(phi_array);
		grids->parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			//
			double min_d = 1e9;
			vec3d origin = dx*vec3d(i,j,k);
			double sgn = (phi_array_accessors[tn](i,j,k) > 0.0 ? 1.0 : -1.0);
			auto ptr = it();
			//
			std::vector<size_t> face_neighbors = m_hashtable->get_cell_neighbors(vec3i(i,j,k),pointgridhash3_interface::USE_NODAL);
			for( unsigned n=0; n<face_neighbors.size(); ++n ) {
				const unsigned &idx = face_neighbors[n];
				vec3d p = dx*vec3d(i+0.5,j+0.5,k+0.5);
				vec3d out;
				double d = m_meshutility->point_triangle_distance(p,vertices[faces[idx][0]],vertices[faces[idx][1]],vertices[faces[idx][2]],out);
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
			if( ! ptr || ! ptr->known ) it.set_off();
		});
		//
		// Trim to just near the surface
		m_gridutility->mark_narrowband(phi_array,half_cells);
		//
		// Setup a network
		shared_array3<node3 *> nodeArray(phi_array.shape());
		nodeArray->activate_as(phi_array);
		nodeArray->parallel_actives([&](auto &it) {
			it.set(new node3);
		});
		//
		auto nodeArray_accessors = nodeArray->get_const_accessors();
		nodeArray->const_parallel_actives([&](int i, int j, int k, const auto &it, int tn) {
			//
			vec3d p = dx*vec3d(i,j,k);
			const auto ptr = nodeArray_accessors[tn](i,j,k);
			const auto grid_ptr = grids_accessors[tn](i,j,k);
			//
			ptr->p = p;
			if( grid_ptr ) {
				ptr->fixed = grid_ptr->known;
				ptr->levelset = grid_ptr->dist;
			} else {
				double sgn = (phi_array_accessors[tn](i,j,k)>0.0 ? 1.0 : -1.0);
				ptr->fixed = false;
				ptr->levelset = sgn * half_bandwidth;
			}
			//
			int q[][DIM3] = {{i-1,j,k},{i+1,j,k},{i,j-1,k},{i,j+1,k},{i,j,k-1},{i,j,k+1}};
			for( unsigned n=0; n<6; n++ ) {
				if( ! nodeArray->shape().out_of_bounds(q[n]) && nodeArray_accessors[tn].active(q[n])) {
					it()->p2p.push_back(nodeArray_accessors[tn](q[n]));
				}
			}
		});
		//
		std::vector<node3 *> nodes;
		nodeArray->const_serial_actives([&](const auto &it) {
			nodes.push_back(it());
		});
		//
		// Perform fast march
		fastMarch(nodes,half_bandwidth,-half_bandwidth,m_parallel);
		//
		phi_array.parallel_actives([&](int i, int j, int k, auto &it, int tn) {
			it.set(nodeArray_accessors[tn](i,j,k)->levelset);
		});
		//
		grids->serial_actives([&](auto &it) {
			const auto ptr = it();
			if( ptr ) delete ptr;
		});
		nodeArray->serial_actives([&](auto &it) { delete it(); });
		//
		phi_array.flood_fill();
	}
	//
	cellmesher3_driver m_mesher{this,"marchingcubes"};
	meshutility3_driver m_meshutility{this,"meshutility3"};
	pointgridhash3_driver m_hashtable{this,"pointgridhash3"};
	gridutility3_driver m_gridutility{this,"gridutility3"};
	parallel_driver m_parallel{this};
	//
};
//
extern "C" module * create_instance() {
	return new fastmarch3;
}
//
extern "C" const char *license() {
	return "MIT";
}
//