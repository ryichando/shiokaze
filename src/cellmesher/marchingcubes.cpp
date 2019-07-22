/*
**	marchingcubes.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 30, 2018.
**
**	The code originates from the Marching Cubes Example Program by Cory Bloyd.
**	http://paulbourke.net/geometry/polygonise/marchingsource.cpp
**
**	This code is public domain.
*/
//
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <unordered_map>
#include <shiokaze/utility/utility.h>
#include <shiokaze/array/shared_array3.h>
#include "mc_table.h"
//
SHKZ_USING_NAMESPACE
//
class marchingcubes : public cellmesher3_interface {
protected:
	//
	LONG_NAME("Marching Cubes Mesh Generator 3D")
	MODULE_NAME("marchingcubes")
	AUTHOR_NAME("Cory Bloyd")
	//
	virtual void generate_mesh( const array3<float> &levelset, std::vector<vec3d> &vertices, std::vector<std::vector<size_t> > &faces ) const override {
		//
		assert(m_dx);
		vec3d global_origin = levelset.shape()==m_shape.nodal() ? vec3d() : m_dx * vec3d(0.5,0.5,0.5);
		//
		std::vector<std::unordered_map<size_t,vec3d> > global_edge_vertices(levelset.get_thread_num());
		std::vector<std::vector<std::vector<size_t> > > global_faces(levelset.get_thread_num());
		//
		const shape3 s = levelset.shape();
		levelset.const_parallel_actives([&]( int i, int j, int k, const auto &it, int tn ) {
			vec3d local_origin = global_origin + m_dx*vec3d(i,j,k);
			double value[8];
			int flag (0);
			//
			for(int n=0; n<8; ++n ) {
				vec3i pi ( i+a2fVertexOffset[n][0],
						   j+a2fVertexOffset[n][1],
						   k+a2fVertexOffset[n][2] );
				if( s.out_of_bounds(pi) || ! levelset.active(pi)) return;
				value[n] = levelset(pi);
				if(value[n] < 0.0) flag |= 1 << n;
			}
			int edge_flag = aiCubeEdgeFlags[flag];
			if( edge_flag ) {
				size_t global_ids[12];
				for( int n=0; n<12; ++n ) {
					if(edge_flag & (1<<n)) {
						//
						size_t global_id = global_edge_id(n,i,j,k,s.w,s.h,s.d);
						size_t edge0 = a2iEdgeConnection[n][0];
						size_t edge1 = a2iEdgeConnection[n][1];
						//
						vec3d p1 = local_origin+m_dx*vec3d(
							a2fVertexOffset[edge0][0],
							a2fVertexOffset[edge0][1],
							a2fVertexOffset[edge0][2]);
						//
						vec3d p2 = local_origin+m_dx*vec3d(
							a2fVertexOffset[edge1][0],
							a2fVertexOffset[edge1][1],
							a2fVertexOffset[edge1][2]);
						//
						double v1 = value[edge0];
						double v2 = value[edge1];
						double fraction = utility::fraction(v1,v2);
						//
						vec3d p;
						if( v1 < 0.0 ) {
							p = fraction*p2+(1.0-fraction)*p1;
						} else {
							p = fraction*p1+(1.0-fraction)*p2;
						}
						global_edge_vertices[tn][global_id] = p;
						global_ids[n] = global_id;
					}
				}
				for( int n=0; n<=5; ++n) {
					if(a2iTriangleConnectionTable[flag][3*n] < 0) break;
					std::vector<size_t> face(3);
					for( int m=0; m<3; ++m) {
						face[m] = global_ids[a2iTriangleConnectionTable[flag][3*n+m]];
					}
					global_faces[tn].push_back(face);
				}
			}
		});
		//
		std::unordered_map<size_t,vec3d> merged_vertices;
		std::vector<std::vector<size_t> > merged_faces;
		//
		for( auto it : global_edge_vertices ) merged_vertices.insert(it.begin(),it.end());
		for( auto it : global_faces ) merged_faces.insert(merged_faces.end(),it.begin(),it.end());
		//
		std::unordered_map<size_t,size_t> remap;
		size_t index (0);
		for( auto it=merged_vertices.cbegin(); it!=merged_vertices.cend(); it++ ) {
			remap[it->first] = index ++;
		}
		//
		vertices.resize(index);
		for( auto it=merged_vertices.cbegin(); it!=merged_vertices.cend(); it++ ) {
			vertices[remap[it->first]] = it->second;
		}
		//
		faces.resize(merged_faces.size());
		for( size_t n=0; n<merged_faces.size(); ++n ) {
			faces[n].resize(3);
			for( int m=0; m<3; ++m ) faces[n][m] = remap[merged_faces[n][m]];
		}
	}
	//
	virtual void initialize ( const shape3 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	shape3 m_shape;
	double m_dx {0.0};
	//
};
//
extern "C" module * create_instance() {
	return new marchingcubes;
}
//
extern "C" const char *license() {
	return "Public domain";
}
//