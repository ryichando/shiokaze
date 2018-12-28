/*
**	octree2.cpp
**
**	This is part of Shiokaze fluid solver, a research-oriented fluid solver designed for collaborative projects.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017. All rights reserved.
**	Unauthorized use, redistributing and copying of this file, via any medium are strictly prohibited.
*/
//
#include <unordered_map>
#include <algorithm>
#include <shiokaze/array/shape.h>
#include <shiokaze/octree/octree2.h>
#include <shiokaze/core/console.h>
#include <shiokaze/utility/utility.h>
//
SHKZ_USING_NAMESPACE
//
octree2::~octree2() {
	clear();
}
//
void octree2::copy( const octree2 &octree ) {
	clear();
	if( octree.m_root ) {
		m_maxdepth = octree.m_maxdepth;
		m_resolution = octree.m_resolution;
		m_root = new leaf2;
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
void octree2::copy( leaf2 *src, leaf2 *dest ) {
	*dest = *src;
	shape2(2,2).for_each([&]( int i, int j ) {
		if( src->children[i][j] ) {
			dest->children[i][j] = new leaf2;
			copy(src->children[i][j],dest->children[i][j]);
		}
	});
}
//
void octree2::build_octree( std::function<double(const vec2d &p)> hint, unsigned maxdepth ) {
	// Clear the octree first
	clear();
	//
	// Set the maximal depth
	m_maxdepth = maxdepth;
	m_resolution = pow(2,maxdepth+1);
	//
	// Allocate root leaf
	m_root = alloc_leaf(vec2i(m_resolution/2,m_resolution/2),0,vec2i(0,0));
	//
	// Subdivide this root leaf...
	subdivide(m_root,hint,maxdepth);
	//
	// Build terminal array
	unsigned index = 0;
	count_num_terminal(m_root,index);
	m_terminals.resize(index);
	index = 0;
	build_array(m_root,index);
	//
	// Build corner nodes and its references
	build_nodes();
}
//
unsigned octree2::hit_test( vec2d p, leaf2 *leaf ) const {
	if( ! leaf ) {
		p = m_resolution * p;
		leaf = m_root;
	}
	vec2d center = vec2d(leaf->center[0],leaf->center[1]);
	vec2d p0 = center - leaf->dx*vec2d(0.5,0.5);
	vec2d p1 = center + leaf->dx*vec2d(0.5,0.5);
	bool hit = utility::box(p,p0,p1) <= 0.0;
	if( hit ) {
		if( leaf->subdivided ) {
			shape2(2,2).for_each([&]( int i, int j ) {
				return hit_test(p,leaf->children[i][j]);
			});
		} else {
			return leaf->index;
		}
	}
	return m_root->index;
}
//
octree2::leaf2* octree2::alloc_leaf( vec2i center, unsigned depth, vec2i position ) {
	leaf2 *leaf = new leaf2;
	leaf->center = center;
	leaf->position = position;
	leaf->depth = depth;
	leaf->dx = m_resolution/pow(2,depth);
	leaf->subdivided = false;
	shape2(2,2).for_each([&]( int i, int j ) {
		leaf->children[i][j] = nullptr;
		leaf->corners[i][j] = 0;
	});
	return leaf;
}
//
static vec2i compute_center_pos( vec2i center, unsigned dx, unsigned i, unsigned j ) {
	return center-(int)(0.25*dx)*vec2i(1,1)+(int)(0.5*dx)*vec2i(i,j);
}
//
static vec2i compute_corner_pos( vec2i center, unsigned dx, unsigned i, unsigned j ) {
	return center-(int)(0.5*dx)*vec2i(1,1)+(int)(dx)*vec2i(i,j);
}
//
bool octree2::check_subdivision( vec2i pos, unsigned dx, std::function<double(vec2d p)> hint, int threshold, int depth, unsigned max_nest, unsigned maxdepth ) const {
	if( ! max_nest ) return false;
	if( hint(vec2d(pos)/(double)m_resolution) < pow(2,-depth) ) {
		return true;
	}
	if( dx > threshold ) {
		// Compute the center position of this children
		for( int i=0; i<2; ++i ) for( int j=0; j<2; ++j ) {
			// Compute the levelset at this position
			vec2i sub_pos = compute_center_pos(pos,dx,i,j);
			if( check_subdivision(sub_pos,dx/2,hint,threshold,depth,max_nest-1,maxdepth)) return true;
		}
	}
	return false;
}
//
void octree2::subdivide( leaf2 *leaf, std::function<double(vec2d p)> hint, unsigned maxdepth ) {
	// See hint indicates a subdivision
	int threshold = 8;
	//
	// If to subdivide, do it
	if( check_subdivision(leaf->center,leaf->dx,hint,threshold,leaf->depth,3,maxdepth)) {
		unsigned depth = leaf->depth+1;
		if( depth <= maxdepth ) {
			leaf->subdivided = true;
			shape2(2,2).for_each([&]( int i, int j ) {
				// Compute the center position for this children
				vec2i center = compute_center_pos(leaf->center,leaf->dx,i,j);
				leaf2 *child = alloc_leaf(center,depth,vec2i(i,j));
				leaf->children[i][j] = child;
				subdivide(child,hint,maxdepth);
			});
		}
	}
}
//
void octree2::count_num_terminal( leaf2 *leaf, unsigned &count ) {
	if( leaf->subdivided ) {
		shape2(2,2).for_each([&]( int i, int j ) {
			count_num_terminal(leaf->children[i][j],count);
		});
	} else {
		count++;
	}
}
//
void octree2::build_array( leaf2 *leaf, unsigned &index ) {
	if( leaf->subdivided ) {
		shape2(2,2).for_each([&]( int i, int j ) {
			build_array(leaf->children[i][j],index);
		});
	} else {
		m_terminals[index] = leaf;
		leaf->index = index;
		index ++;
	}
}
//
void octree2::build_nodes() {
	std::unordered_map<uint64_t,unsigned> node_dictionary;
	unsigned index = 0;
	for( uint n=0; n<m_terminals.size(); n++ ) {
		leaf2 *leaf = m_terminals[n];
		shape2(2,2).for_each([&]( int i, int j ) {
			// Compute the center position for this children
			vec2i corner = compute_corner_pos(leaf->center,leaf->dx,i,j);
			uint64_t idx = compute_corner_index(corner);
			if( node_dictionary.find(idx) == node_dictionary.end() ) {
				node_dictionary[idx] = index;
				leaf->corners[i][j] = index;
				index ++;
			} else {
				leaf->corners[i][j] = node_dictionary[idx];
			}
		});
	}
	m_nodes.resize(index);
	for( uint n=0; n<m_terminals.size(); n++ ) {
		leaf2 *leaf = m_terminals[n];
		shape2(2,2).for_each([&]( int i, int j ) {
			unsigned index = leaf->corners[i][j];
			m_nodes[index] = vec2d(compute_corner_pos(leaf->center,leaf->dx,i,j))/m_resolution;
		});
	}
}
//
uint64_t octree2::compute_corner_index( vec2i p ) const {
	return p[0]+p[1]*m_resolution;
}
//
void octree2::clear() {
	if( m_root ) {
		release_children(m_root);
		m_root = nullptr;
	}
	m_maxdepth = 1;
	m_terminals.clear();
	m_nodes.clear();
}
//
bool octree2::release_children( leaf2 *leaf ) {
	if( ! leaf ) return false;
	// Make sure we release all the chilren first
	shape2(2,2).for_each([&]( int i, int j ) {
		if( leaf->children[i][j] ) {
			release_children(leaf->children[i][j]);
			leaf->children[i][j] = nullptr;
		}
	});
	// After that we deallocate this structure
	delete leaf;
	return true;
}
//
void octree2::draw_octree( const graphics_engine &g ) const {
	if( m_root ) {
		g.color4(0.5,0.5,0.5,0.5);
		draw_octree(g,m_root);
	}
}
//
void octree2::draw_octree( const graphics_engine &g, const leaf2 *leaf ) const {
	shape2(2,2).for_each([&]( int i, int j ) {
		if( leaf->subdivided ) {
			draw_octree(g,leaf->children[i][j]);
		} else {
			unsigned dx = leaf->dx;
			vec2i center = leaf->center;
			g.begin(graphics_engine::MODE::LINE_LOOP);
			g.vertex2((center[0]-0.5*dx)/(double)m_resolution,(center[1]-0.5*dx)/(double)m_resolution);
			g.vertex2((center[0]-0.5*dx)/(double)m_resolution,(center[1]+0.5*dx)/(double)m_resolution);
			g.vertex2((center[0]+0.5*dx)/(double)m_resolution,(center[1]+0.5*dx)/(double)m_resolution);
			g.vertex2((center[0]+0.5*dx)/(double)m_resolution,(center[1]-0.5*dx)/(double)m_resolution);
			g.end();
		}
	});
}