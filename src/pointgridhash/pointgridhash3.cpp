/*
**	pointgridhash3.cpp
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
#include <shiokaze/pointgridhash/pointgridhash3_interface.h>
#include <shiokaze/array/array3.h>
#include <shiokaze/parallel/parallel_driver.h>
//
SHKZ_USING_NAMESPACE
//
static const std::vector<size_t> empty;
class pointgridhash3 : public pointgridhash3_interface {
private:
	//
	virtual void clear() override {
		if( m_num_sorted ) {
			//
			m_hash_cell.clear();
			m_hash_node.clear();
			for( int dim : DIMS3 ) hash_edge(dim).clear();
			m_num_sorted = 0;
		}
	}
	virtual void sort_points( const std::vector<vec3d> &points ) override {
		//
		clear();
		//
		std::vector<std::function<void()> > operations;
		if( m_mode & CELL_MODE ) {
			operations.push_back([&](){
				for( size_t n=0; n<points.size(); ++n ) {
					const vec3i pi = m_shape.find_cell(points[n]/m_dx);
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
					const vec3i pi = m_shape.find_node(points[n]/m_dx);
					auto ptr = m_hash_node.ptr(pi);
					if( ptr ) ptr->push_back(n);
					else m_hash_node.set(pi,{n});
				}
			});
		}
		//
		if( m_mode & EDGE_MODE ) {
			operations.push_back([&](){
				m_parallel.for_each(DIM3,[&]( size_t dim ) {
					for( size_t n=0; n<points.size(); ++n ) {
						const vec3i pi = m_shape.find_edge(points[n]/m_dx,dim);
						auto ptr = hash_edge(dim).ptr(pi);
						if( ptr ) ptr->push_back(n);
						else hash_edge(dim).set(pi,{n});
					}
				});
			});
		}
		m_parallel.run(operations);
		m_num_sorted = points.size();
	}
	//
	virtual const std::vector<size_t> & get_points_in_cell( const vec3i &pi ) const override {
		return (m_mode & CELL_MODE) ? m_hash_cell(pi) : empty;
	}
	virtual const std::vector<size_t> & get_points_on_node( const vec3i &pi ) const override {
		return (m_mode & NODAL_MODE) ? m_hash_node(pi) : empty;
	}
	//
	virtual bool exist( const vec3i &pi, hash_type type ) const override {
		if( type == USE_NODAL ) {
			return (m_mode & NODAL_MODE) ? (! m_hash_node(pi[0],pi[1],pi[2]).empty()) : false;
		} else if( type == USE_CELL ) {
			return (m_mode & CELL_MODE) ? (! m_hash_cell(pi[0],pi[1],pi[2]).empty()) : false;
		} else {
			printf( "\n *** Unknown type! ***\n");
			exit(0);
		}
		return false;
	}
	virtual std::vector<size_t> get_cell_neighbors( const vec3i &pi, hash_type type=USE_CELL, int half_width=1 ) const override {
		std::vector<size_t> neighbors;
		if( type == USE_NODAL ) {
			if (m_mode & NODAL_MODE) {
				for( int ii=pi[0]-half_width+1; ii<=pi[0]+half_width; ++ii ) for( int jj=pi[1]-half_width+1; jj<=pi[1]+half_width; ++jj ) for( int kk=pi[2]-half_width+1; kk<=pi[2]+half_width; ++kk ) {
					if( ! m_hash_node.shape().out_of_bounds(ii,jj,kk) ) {
						const auto &bucket = m_hash_node(ii,jj,kk);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash3::get_cell_neighbors() NODAL_MODE not specified");
				exit(0);
			}
		} else if( type == USE_CELL ) {
			if (m_mode & CELL_MODE) {
				for( int ii=pi[0]-half_width; ii<=pi[0]+half_width; ++ii ) for( int jj=pi[1]-half_width; jj<=pi[1]+half_width; ++jj ) for( int kk=pi[2]-half_width; kk<=pi[2]+half_width; ++kk ) {
					if( ! m_hash_cell.shape().out_of_bounds(ii,jj,kk) ) {
						const auto &bucket = m_hash_cell(ii,jj,kk);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash3::get_cell_neighbors() CELL_MODE not specified");
				exit(0);
			}
		} else {
			printf( "pointgridhash3::get_cell_neighbors: Unsupported type!");
			exit(0);
		}
		//
		return std::move(neighbors);
	}
	virtual std::vector<size_t> get_nodal_neighbors( const vec3i &pi, hash_type type=USE_NODAL, int half_width=1 ) const override {
		std::vector<size_t> neighbors;
		if( type == USE_CELL ) {
			if (m_mode & CELL_MODE) {
				for( int ii=pi[0]-half_width; ii<=pi[0]+half_width-1; ++ii ) for( int jj=pi[1]-half_width; jj<=pi[1]+half_width-1; ++jj ) for( int kk=pi[2]-half_width; kk<=pi[2]+half_width-1; ++kk ) {
					if( ! m_hash_cell.shape().out_of_bounds(ii,jj,kk) ) {
						const auto &bucket = m_hash_cell(ii,jj,kk);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash3::get_nodal_neighbors() CELL_MODE not specified");
				exit(0);
			}
		} else if( type == USE_NODAL ) {
			if (m_mode & NODAL_MODE) {
				for( int ii=pi[0]-half_width; ii<=pi[0]+half_width; ++ii ) for( int jj=pi[1]-half_width; jj<=pi[1]+half_width; ++jj ) for( int kk=pi[2]-half_width; kk<=pi[2]+half_width; ++kk ) {
					if( ! m_hash_node.shape().out_of_bounds(ii,jj,kk) ) {
						const auto &bucket = m_hash_node(ii,jj,kk);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash3::get_nodal_neighbors() NODAL_MODE not specified");
				exit(0);
			}
		} else {
			printf( "pointgridhash3::get_nodal_neighbors: Unsupported type!");
			exit(0);
		}
		//
		return std::move(neighbors);
	}
	virtual std::vector<size_t> get_face_neighbors( const vec3i &pi, unsigned dim, hash_type type=USE_EDGE ) const override {
		std::vector<size_t> neighbors;
		if( type == USE_CELL ) {
			if (m_mode & CELL_MODE) {
				for( int dir=-1; dir<=0; ++dir ) {
					int ii = pi[0]+(dim==0)*dir;
					int jj = pi[1]+(dim==1)*dir;
					int kk = pi[2]+(dim==2)*dir;
					if( ! m_hash_cell.shape().out_of_bounds(ii,jj,kk) ) {
						const auto &bucket = m_hash_cell(ii,jj,kk);
						neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
					}
				}
			} else {
				printf( "pointgridhash3::get_face_neighbors() CELL_MODE not specified");
				exit(0);
			}
		} else if( type == USE_EDGE ) {
			if (m_mode & EDGE_MODE) {
				for( int dir=-1; dir<=0; ++dir ) {
					for( int ri=0; ri<=1; ++ri ) for( int rj=0; rj<=1; ++rj ) for( int rk=0; rk<=1; ++rk ) {
						int ii = pi[0]+(dim!=0)*ri+(dim==0)*dir;
						int jj = pi[1]+(dim!=1)*rj+(dim==1)*dir;
						int kk = pi[2]+(dim!=2)*rk+(dim==2)*dir;
						if( ! hash_edge(dim).shape().out_of_bounds(ii,jj,kk) ) {
							const auto &bucket = hash_edge(dim)(ii,jj,kk);
							neighbors.insert(neighbors.end(),bucket.begin(),bucket.end());
						}
					}
				}
			} else {
				printf( "pointgridhash3::get_face_neighbors() EDGE_MODE not specified");
				exit(0);
			}
		} else {
			printf( "pointgridhash3::get_face_neighbors: Unsupported type!");
			exit(0);
		}
		return std::move(neighbors);
	}
	virtual void initialize( const shape3 &shape, double dx, int mode=CELL_MODE | NODAL_MODE | EDGE_MODE ) override {
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
		if( m_mode & EDGE_MODE ) {
			for( int dim : DIMS3 ) {
				hash_edge(dim).initialize(m_shape.edge(dim));
			}
		}
	}
	//
	shape3 m_shape;
	double m_dx {0.0};
	unsigned m_num_sorted {0};
	int m_mode {0};
	//
	array3<std::vector<size_t> > m_hash_cell{this};
	array3<std::vector<size_t> > m_hash_node{this};
	array3<std::vector<size_t> > m_hash_edge0{this}, m_hash_edge1{this}, m_hash_edge2{this};
	//
	const array3<std::vector<size_t> > & hash_edge(int dim) const { 
		return dim == 0 ? m_hash_edge0 : (dim==1 ? m_hash_edge1 : m_hash_edge2);
	}
	array3<std::vector<size_t> > & hash_edge(int dim) { 
		return dim == 0 ? m_hash_edge0 : (dim==1 ? m_hash_edge1 : m_hash_edge2);
	}
	//
	parallel_driver m_parallel{this};
	//
};
//
extern "C" module * create_instance() {
	return new pointgridhash3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//