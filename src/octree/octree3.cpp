/*
**	octree3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017.
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
#include <unordered_map>
#include <algorithm>
#include <shiokaze/octree/octree3.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/array/shape.h>
#include <shiokaze/utility/utility.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/core/console.h>
//
SHKZ_USING_NAMESPACE
using namespace console;
//
octree3::~octree3() {
	clear();
}
//
void octree3::copy( const octree3 &octree ) {
	clear();
	if( octree.m_root ) {
		m_maxdepth = octree.m_maxdepth;
		m_resolution = octree.m_resolution;
		m_root = new leaf3;
		copy(octree.m_root,m_root);
		m_terminals.resize(octree.m_terminals.size());
		//
		// Build terminal array
		unsigned index = 0;
		count_num_terminal(m_root,index);
		m_terminals.resize(index);
		index = 0;
		build_array(m_root,index);
		//
		// Copy nodes
		m_nodes = octree.m_nodes;
	}
}
//
void octree3::copy( leaf3 *src, leaf3 *dest ) {
	*dest = *src;
	shape3(2,2,2).for_each([&]( int i, int j, int k ) {
		if( src->children[i][j][k] ) {
			dest->children[i][j][k] = new leaf3;
			copy(src->children[i][j][k],dest->children[i][j][k]);
		}
	});
}
//
void octree3::build_octree( std::function<double(const vec3d &p)> hint, unsigned maxdepth ) {
	//
	// Clear the octree first
	clear();
	//
	// Set the maximal depth
	m_maxdepth = maxdepth;
	m_resolution = powf(2,maxdepth+1);
	//
	// Allocate root leaf
	m_root = alloc_leaf(vec3i(m_resolution/2,m_resolution/2,m_resolution/2),0,vec3i(0,0,0));
	//
	// Subdivide this root leaf...
	subdivide(m_root,hint,m_maxdepth);
	//
	// Build terminal array
	uint index = 0;
	count_num_terminal(m_root,index);
	m_terminals.resize(index);
	index = 0;
	build_array(m_root,index);
	//
	// Build corner nodes and its references
	build_nodes();
}
//
unsigned octree3::hit_test( vec3d p, leaf3 *leaf ) const {
	if( ! leaf ) {
		p = m_resolution * p;
		leaf = m_root;
	}
	vec3d center = vec3d(leaf->center[0],leaf->center[1],leaf->center[2]);
	vec3d p0 = center - leaf->dx*vec3d(0.5,0.5,0.5);
	vec3d p1 = center + leaf->dx*vec3d(0.5,0.5,0.5);
	bool hit = utility::box(p,p0,p1) <= 0.0;
	if( hit ) {
		if( leaf->subdivided ) {
			shape3(2,2,2).for_each([&]( int i, int j, int k ) {
				return hit_test(p,leaf->children[i][j][k]);
			});
		} else {
			return leaf->index;
		}
	}
	return m_root->index;
}
//
octree3::leaf3* octree3::alloc_leaf( vec3i center, unsigned depth, vec3i position ) {
	leaf3 *leaf = new leaf3;
	leaf->center = center;
	leaf->position = position;
	leaf->depth = depth;
	leaf->dx = m_resolution/pow(2,depth);
	leaf->subdivided = false;
	shape3(2,2,2).for_each([&]( int i, int j, int k ) {
		leaf->children[i][j][k] = nullptr;
		leaf->corners[i][j][k] = 0;
	});
	return leaf;
}
//
static vec3i compute_center_pos( vec3i center, unsigned dx, unsigned i, unsigned j, unsigned k ) {
	return center-(int)(0.25*dx)*vec3i(1,1,1)+(int)(0.5*dx)*vec3i(i,j,k);
}
//
static vec3i compute_corner_pos( vec3i center, unsigned dx, unsigned i, unsigned j, unsigned k ) {
	return center-(int)(0.5*dx)*vec3i(1,1,1)+(int)(dx)*vec3i(i,j,k);
}
//
bool octree3::check_subdivision( vec3i pos, unsigned dx, std::function<double(vec3d p)> hint, int threshold, int depth, unsigned max_nest, unsigned m_maxdepth ) const {
	if( ! max_nest ) return false;
	if( hint(vec3d(pos)/(double)m_resolution) < pow(2,-depth) ) {
		return true;
	}
	if( dx > threshold ) {
		// Compute the center position of this children
		for( int i=0; i<2; ++i ) for( int j=0; j<2; ++j ) for( int k=0; k<2; ++k ) {
			// Compute the levelset at this position
			vec3i sub_pos = compute_center_pos(pos,dx,i,j,k);
			if( check_subdivision(sub_pos,dx/2,hint,threshold,depth,max_nest-1,m_maxdepth)) return true;
		}
	}
	return false;
}
//
void octree3::subdivide( leaf3 *leaf, std::function<double(vec3d p)> hint, unsigned m_maxdepth ) {
	// See hint indicates a subdivision
	int threshold = 8;
	//
	// If to subdivide, do it
	if( check_subdivision(leaf->center,leaf->dx,hint,threshold,leaf->depth,3,m_maxdepth)) {
		unsigned depth = leaf->depth+1;
		if( depth <= m_maxdepth ) {
			leaf->subdivided = true;
			shape3(2,2,2).for_each([&]( int i, int j, int k ) {
				// Compute the center position for this children
				vec3i center = compute_center_pos(leaf->center,leaf->dx,i,j,k);
				leaf3 *child = alloc_leaf(center,depth,vec3i(i,j,k));
				leaf->children[i][j][k] = child;
				subdivide(child,hint,m_maxdepth);
			});
		}
	}
}
//
void octree3::count_num_terminal( leaf3 *leaf, unsigned &count ) {
	if( leaf->subdivided ) {
		shape3(2,2,2).for_each([&]( int i, int j, int k ) {
			count_num_terminal(leaf->children[i][j][k],count);
		});
	} else {
		count++;
	}
}
//
void octree3::build_array( leaf3 *leaf, unsigned &index ) {
	if( leaf->subdivided ) {
		shape3(2,2,2).for_each([&]( int i, int j, int k ) {
			build_array(leaf->children[i][j][k],index);
		});
	} else {
		m_terminals[index] = leaf;
		leaf->index = index;
		index ++;
	}
}
//
void octree3::build_nodes() {
	std::unordered_map<uint64_t,unsigned> node_dictionary;
	unsigned index = 0;
	for( unsigned n=0; n<m_terminals.size(); n++ ) {
		leaf3 *leaf = m_terminals[n];
		shape3(2,2,2).for_each([&]( int i, int j, int k ) {
			// Compute the center position for this children
			vec3i corner = compute_corner_pos(leaf->center,leaf->dx,i,j,k);
			uint64_t idx = compute_corner_index(corner);
			if( node_dictionary.find(idx) == node_dictionary.end() ) {
				node_dictionary[idx] = index;
				leaf->corners[i][j][k] = index;
				index ++;
			} else {
				leaf->corners[i][j][k] = node_dictionary[idx];
			}
		});
	}
	m_nodes.resize(index);
	for( unsigned n=0; n<m_terminals.size(); n++ ) {
		leaf3 *leaf = m_terminals[n];
		shape3(2,2,2).for_each([&]( int i, int j, int k ) {
			unsigned index = leaf->corners[i][j][k];
			m_nodes[index] = vec3d(compute_corner_pos(leaf->center,leaf->dx,i,j,k))/m_resolution;
		});
	}
}
//
uint64_t octree3::compute_corner_index( vec3i p ) const {
	uint64_t R = m_resolution;
	return p[0]+p[1]*R+p[2]*R*R;
}
//
void octree3::clear() {
	if( m_root ) {
		release_children(m_root);
		m_root = nullptr;
	}
	m_maxdepth = 1;
	m_terminals.clear();
	m_nodes.clear();
}
//
bool octree3::release_children( leaf3 *leaf ) {
	if( ! leaf ) return false;
	//
	// Make sure we release all the chilren first
	shape3(2,2,2).for_each([&]( int i, int j, int k ) {
		if( leaf->children[i][j][k] ) {
			release_children(leaf->children[i][j][k]);
			leaf->children[i][j][k] = nullptr;
		}
	});
	//
	// After that we deallocate this structure
	delete leaf;
	return true;
}
//
void octree3::draw_octree( graphics_engine &g ) const {
	if( m_root ) {
		g.color4(0.5,0.5,0.5,0.5);
		draw_octree(g,m_root);
	}
}
//
void octree3::draw_octree( graphics_engine &g, const leaf3 *leaf ) const {
	g.color4(0.5,0.5,0.5,0.5);
	shape3(2,2,2).for_each([&]( int i, int j, int k ) {
		if( leaf->subdivided ) {
			draw_octree(g,leaf->children[i][j][k]);
		} else {
			int dx = leaf->dx;
			vec3i bottom_left = leaf->center-0.5*vec3d(dx,dx,dx);
			vec3d p0 = vec3d(bottom_left)/m_resolution;
			vec3d p1 = (vec3d(bottom_left)+vec3d(dx,dx,dx))/m_resolution;
			graphics_utility::draw_wired_box(g,p0.v,p1.v);
		}
	});
}
//