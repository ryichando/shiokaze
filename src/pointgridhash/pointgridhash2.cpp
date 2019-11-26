/*
**	pointgridhash2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 29, 2017.
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
#include <shiokaze/pointgridhash/pointgridhash2_interface.h>
#include <shiokaze/array/array2.h>
#include <shiokaze/parallel/parallel_driver.h>
//
SHKZ_USING_NAMESPACE
//
static const std::vector<size_t> empty;
class pointgridhash2 : public pointgridhash2_interface {
protected:
	//
	virtual void clear() override {
		if( m_num_sorted ) {
			//
			m_hash_cell.clear();
			m_hash_node.clear();
			for( int dim : DIMS2 ) hash_face(dim).clear();
			m_num_sorted = 0;
		}
	}
	virtual void sort_points( const std::vector<vec2r> &points ) override {
		//
		clear();
		//
		std::vector<std::function<void()> > operations;
		if( m_mode & CELL_MODE ) {
			operations.push_back([&](){
				for( size_t n=0; n<points.size(); ++n ) {
					const vec2i &pi = m_shape.find_cell(points[n]/m_dx);
					auto ptr = m_hash_cell.ptr(pi);
					if( ptr ) ptr->push_back(n);
					else m_hash_cell.set(pi,{n});
				}
			});
		}
		//
		if( m_mode & NODAL_MODE ) {
			operations.push_back([&](){
				for( size_t n=0; n<points.size(); ++n ) {
					const vec2i &pi = m_shape.find_node(points[n]/m_dx);
					auto ptr = m_hash_node.ptr(pi);
					if( ptr ) ptr->push_back(n);
					else m_hash_node.set(pi,{n});
				}
			});
		}
		//
		if( m_mode & FACE_MODE ) {
			operations.push_back([&](){
				m_parallel.for_each(DIM2,[&]( size_t dim ) {
					for( size_t n=0; n<points.size(); ++n ) {
						const vec2i &pi = m_shape.find_face(points[n]/m_dx,dim);
						auto ptr = hash_face(dim).ptr(pi);
						if( ptr ) ptr->push_back(n);
						else hash_face(dim).set(pi,{n});
					}
				});
			});
		}
		m_parallel.run(operations);
		m_num_sorted = points.size();
	}
	virtual const std::vector<size_t> & get_points_in_cell( const vec2i &pi ) const override {
		return (m_mode & CELL_MODE) ? m_hash_cell(pi) : empty;
	}
	virtual const std::vector<size_t> & get_points_on_node( const vec2i &pi ) const override {
		return (m_mode & NODAL_MODE) ? m_hash_node(pi) : empty;
	}
	virtual bool exist( const vec2i &pi, hash_type type ) const override {
		if( type == USE_NODAL ) {
			return (m_mode & NODAL_MODE) ? (! m_hash_node(pi).empty()) : false;
		} else if( type == USE_CELL ) {
			return (m_mode & CELL_MODE) ? (! m_hash_cell(pi).empty()) : false;
		} else {
			printf( "\n *** Unknown type! ***\n");
			exit(0);
		}
		return false;
	}
	virtual std::vector<size_t> get_cell_neighbors( const vec2i &pi, hash_type type=USE_CELL, int half_width=1 ) const override {
		std::vector<size_t> neighbors;
		if( type == USE_NODAL ) {
			if (m_mode & NODAL_MODE) {
				for( int ii=pi[0]-half_width+1; ii<=pi[0]+half_width; ++ii ) for( int jj=pi[1]-half_width+1; jj<=pi[1]+half_width; ++jj ) {
					if( ! m_hash_node.shape().out_of_bounds(ii,jj)) {
						const auto &bucket = m_hash_node(ii,jj);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash2::get_cell_neighbors() NODAL_MODE not specified");
				exit(0);
			}
		} else if ( type == USE_CELL ) {
			if (m_mode & CELL_MODE) {
				for( int ii=pi[0]-half_width; ii<=pi[0]+half_width; ++ii ) for( int jj=pi[1]-half_width; jj<=pi[1]+half_width; ++jj ) {
					if( ! m_hash_cell.shape().out_of_bounds(ii,jj)) {
						const auto &bucket = m_hash_cell(ii,jj);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash2::get_cell_neighbors() CELL_MODE not specified");
				exit(0);
			}
		} else {
			printf( "pointgridhash2::get_cell_neighbors: Unsupported type!");
			exit(0);
		}
		return neighbors;
	}
	virtual std::vector<size_t> get_nodal_neighbors( const vec2i &pi, hash_type type=USE_NODAL, int half_width=1 ) const override {
		std::vector<size_t> neighbors;
		if( type == USE_CELL ) {
			if (m_mode & CELL_MODE) {
				for( int ii=pi[0]-half_width; ii<=pi[0]+half_width-1; ++ii ) for( int jj=pi[1]-half_width; jj<=pi[1]+half_width-1; ++jj ) {
					if( ! m_hash_cell.shape().out_of_bounds(ii,jj)) {
						const auto &bucket = m_hash_cell(ii,jj);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash2::get_nodal_neighbors() CELL_MODE not specified");
				exit(0);
			}
		} else if( type == USE_NODAL ) {
			if (m_mode & NODAL_MODE) {
				for( int ii=pi[0]-half_width; ii<=pi[0]+half_width; ++ii ) for( int jj=pi[1]-half_width; jj<=pi[1]+half_width; ++jj ) {
					if( ! m_hash_node.shape().out_of_bounds(ii,jj)) {
						const auto &bucket = m_hash_node(ii,jj);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash2::get_nodal_neighbors() NODAL_MODE not specified");
				exit(0);
			}
		} else {
			printf( "pointgridhash2::get_nodal_neighbors: Unsupported type!");
			exit(0);
		}
		return neighbors;
	}
	virtual std::vector<size_t> get_face_neighbors( const vec2i &pi, unsigned dim, hash_type type=USE_FACE ) const override {
		std::vector<size_t> neighbors;
		if( type == USE_CELL ) {
			if (m_mode & CELL_MODE) {
				for( int dir=-1; dir<=0; ++dir ) {
					int ii = pi[0]+(dim==0)*dir;
					int jj = pi[1]+(dim==1)*dir;
					if( ! m_hash_cell.shape().out_of_bounds(ii,jj)) {
						const auto &bucket = m_hash_cell(ii,jj);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash2::get_face_neighbors() CELL_MODE not specified");
				exit(0);
			}
		} else if( type == USE_FACE ) {
			if (m_mode & FACE_MODE) {
				int i (pi[0]); int j (pi[1]);
				if( dim == 0 ) {
					vec2i neighbor_indices[4] = { vec2i(i,j), vec2i(i-1,j), vec2i(i-1,j+1), vec2i(i,j+1) };
					for( const vec2i& np : neighbor_indices ) {
						if( ! hash_face(1).shape().out_of_bounds(np) ) {
							const auto &bucket = hash_face(1)(np);
							neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
						}
					}
				} else if( dim == 1 ) {
					vec2i neighbor_indices[4] = { vec2i(i,j), vec2i(i+1,j), vec2i(i,j-1), vec2i(i+1,j-1) };
					for( const vec2i& np : neighbor_indices ) {
						if( ! hash_face(0).shape().out_of_bounds(np) ) {
							const auto &bucket = hash_face(0)(np);
							neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
						}
					}
				}
			} else {
				printf( "pointgridhash2::get_face_neighbors() FACE_MODE not specified");
				exit(0);
			}
		} else {
			printf( "pointgridhash2::get_face_neighbors: Unsupported type!");
			exit(0);
		}
		return neighbors;
	}
	virtual void initialize( const shape2 &shape, double dx, int mode=CELL_MODE | NODAL_MODE | FACE_MODE) override {
		//
		clear();
		//
		m_shape = shape;
		m_dx = dx;
		m_mode = mode;
		//
	}
	virtual void post_initialize () override {
		//
		if( m_mode & CELL_MODE ) {
			m_hash_cell.initialize(m_shape.cell());
		}
		if( m_mode & NODAL_MODE ) {
			m_hash_node.initialize(m_shape.nodal());
		}
		if( m_mode & FACE_MODE ) {
			for( int dim : DIMS2 ) {
				hash_face(dim).initialize(m_shape.face(dim));
			}
		}
	}
	//
	shape2 m_shape;
	double m_dx {0.0};
	unsigned m_num_sorted {0};
	int m_mode {0};
	//
	array2<std::vector<size_t> > m_hash_cell{this};
	array2<std::vector<size_t> > m_hash_node{this};
	array2<std::vector<size_t> > m_hash_face0{this}, m_hash_face1{this};
	//
	const array2<std::vector<size_t> > & hash_face(int dim) const { 
		return dim == 0 ? m_hash_face0 : m_hash_face1;
	}
	array2<std::vector<size_t> > & hash_face(int dim) { 
		return dim == 0 ? m_hash_face0 : m_hash_face1;
	}
	//
	parallel_driver m_parallel{this};
	//
};
//
extern "C" module * create_instance() {
	return new pointgridhash2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
