/*
**	octree3.h
**
**	This is part of Shiokaze fluid solver, a research-oriented fluid solver designed for collaborative projects.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on April 10, 2017. All rights reserved.
**	Unauthorized use, redistributing and copying of this file, via any medium are strictly prohibited.
*/
//
#ifndef SHKZ_OCTREE3_H
#define SHKZ_OCTREE3_H
//
#include <shiokaze/math/vec.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <functional>
#include <vector>
//
SHKZ_BEGIN_NAMESPACE
//
class octree3 {
public:
	// Octree cell structure
	typedef struct _leaf3 {
		_leaf3 *children[2][2][2];
		vec3i position;
		bool subdivided;
		vec3i center;
		unsigned depth;
		unsigned dx;
		unsigned corners[2][2][2];	// index reference to the corner indices
		unsigned index;
	} leaf3;
	//
	virtual ~octree3();
	//
	void copy( const octree3 &octree );
	void clear();
	//
	void build_octree( std::function<double(const vec3d &p)> hint, unsigned maxdepth );
	unsigned hit_test( vec3d p, leaf3 *leaf=nullptr ) const;
	void draw_octree( graphics_engine &g ) const;
	//
	leaf3 *m_root {nullptr};
	std::vector<leaf3 *> m_terminals; // Octree terminal list
	std::vector<vec3d> m_nodes;		// Octree corner list
	//
	unsigned m_maxdepth;
	unsigned m_resolution;
	void subdivide( leaf3 *leaf, std::function<double(vec3d p)> hint, unsigned maxdepth );
private:
	void count_num_terminal( leaf3 *leaf, unsigned &index );
	void build_array( leaf3 *leaf, unsigned &index );
	void build_nodes();
	bool release_children( leaf3 *leaf );
	leaf3* alloc_leaf( vec3i center, unsigned depth, vec3i position );
	void draw_octree( graphics_engine &g, const leaf3 *leaf ) const;
	void copy( leaf3 *src, leaf3 *dest );
	uint64_t compute_corner_index( vec3i p ) const;
	bool check_subdivision( vec3i pos, unsigned dx, std::function<double(vec3d p)> hint, int threshold, int depth, unsigned max_nest, unsigned maxdepth ) const;
};
//
SHKZ_END_NAMESPACE
//
#endif
