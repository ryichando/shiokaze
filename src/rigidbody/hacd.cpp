/*
**	hacd.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 23, 2019.
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
#include <shiokaze/ui/drawable.h>
#include <shiokaze/graphics/graphics_utility.h>
#include <shiokaze/core/console.h>
#include <shiokaze/polygon/polygon3_interface.h>
#include <shiokaze/image/color.h>
#include <shiokaze/core/filesystem.h>
#include <hacd/hacdHACD.h>
#include <hacd/hacdMicroAllocator.h>
#include <shiokaze/rigidbody/hacd_io.h>
//
SHKZ_USING_NAMESPACE
//
class hacd : public drawable {
private:
	//
	LONG_NAME("HACD Dispatcher")
	ARGUMENT_NAME("HACD")
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_string("FileName",filename,"File name in the resource directory to process");
		config.get_bool("DoExport",m_param.export_hacd,"Whether to export HACD polygons");
		config.get_double("CompacityWeight",m_param.compacity_weight,"Compacity weigtht");
		config.get_double("ConnectDist",m_param.connect_dist,"Max distance to connect CCs");
		config.get_unsigned("MinClusters",m_param.min_clusters,"Minimal number of clusters");
		config.get_unsigned("VerticesPerConvexHull",m_param.vertices_per_convex_hull,"Number of vertices per convex-hull");
		config.get_double("Concavity",m_param.concavity,"Max concavity");
		config.get_double("SmallClusterThreshold",m_param.small_cluster_threshold,"Threshold for small clusters");
		config.get_unsigned("NumberTargetTrianglesDecimatedmesh",m_param.number_target_triangles_decimatedmesh,"Scale");
		config.get_bool("AddExtraDistpoints",m_param.add_extra_distpoints,"Whether to add extra distpoints");
		config.get_bool("AddFacesPoints",m_param.add_faces_points,"Whether to add faces points");
		//
		if( ! filesystem::is_exist(filename.c_str()) ) {
			filename = filesystem::find_resource_path("objects",filename.c_str());
			if( ! filesystem::is_exist(filename.c_str())) {
				console::dump( "Error: FileName variable is not valid.\n" );
				exit(0);
			}
		}
		//
		if( m_param.export_hacd && m_param.export_path.empty()) {
			m_param.export_path = filename + ".hacd";
		}
	}
	//
	static void call_back(const char * msg, double progress, double concavity, size_t nVertices) {
		console::dump( "%s\n", msg );
	}
	//
	virtual bool keyboard ( char key ) override {
		if( key == 'N' ) {
			m_focus_cluster = (m_focus_cluster+1) % m_num_clusters;
			return true;
		}
		return false;
	};
	//
	virtual void post_initialize() override {
		//
		console::dump( ">>> Running HACD on %s...\n", filename.c_str());
		//
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		m_objects.clear();
		m_num_clusters = 0;
		m_focus_cluster = 0;
		//
		polygon->load_mesh(filename.c_str());
		polygon->get_mesh(vertices,faces);
		//
		std::vector< HACD::Vec3<HACD::Real> > points(vertices.size());
		std::vector< HACD::Vec3<long> > triangles(faces.size());
		//
		for( size_t n=0; n<vertices.size(); ++n ) {
			for( int i=0; i<3; ++i ) points[n][i] = vertices[n][i];
		}
		for( size_t n=0; n<faces.size(); ++n ) {
			int size = faces[n].size();
			HACD::Vec3<long> tri(size);
			for( int i=0; i<size; ++i ) tri[i] = faces[n][size-i-1];
			triangles[n] = tri;
		}
		//
		HACD::HeapManager * heapManager = HACD::createHeapManager(65536*(1000));
		HACD::HACD * const myHACD = HACD::CreateHACD(heapManager);
		//
		myHACD->SetPoints(&points[0]);
		myHACD->SetNPoints(points.size());
		myHACD->SetTriangles(&triangles[0]);
		myHACD->SetNTriangles(triangles.size());
		myHACD->SetCompacityWeight(m_param.compacity_weight);
		myHACD->SetVolumeWeight(m_param.volume_weight);
		myHACD->SetConnectDist(m_param.connect_dist);
		myHACD->SetNClusters(m_param.min_clusters);
		myHACD->SetNVerticesPerCH(m_param.vertices_per_convex_hull);
		myHACD->SetConcavity(m_param.concavity);
		myHACD->SetSmallClusterThreshold(m_param.small_cluster_threshold);
		myHACD->SetNTargetTrianglesDecimatedMesh(m_param.number_target_triangles_decimatedmesh);
		myHACD->SetCallBack(&call_back);
		myHACD->SetAddExtraDistPoints(m_param.add_extra_distpoints);
		myHACD->SetAddFacesPoints(m_param.add_faces_points);
		//
		myHACD->Compute();
		//
		m_num_clusters = myHACD->GetNClusters();
		for( size_t n=0; n<m_num_clusters; ++n ) {
			//
			size_t num_points = myHACD->GetNPointsCH(n);
			size_t num_triangles = myHACD->GetNTrianglesCH(n);
			//
			hacd_io::convex_object obj;
			obj.vertices.resize(num_points);
			obj.faces.resize(num_triangles);
			//
			std::vector<HACD::Vec3<HACD::Real> > pointsCH(num_points);
			std::vector<HACD::Vec3<long> > trianglesCH(num_triangles);
			//
			myHACD->GetCH(n,pointsCH.data(),trianglesCH.data());
			for( size_t k=0; k<num_points; ++k ) {
				for( int i=0; i<3; ++i ) obj.vertices[k][i] = pointsCH[k][i];
			}
			for( size_t k=0; k<num_triangles; ++k ) {
				obj.faces[k].resize(3);
				for( int i=0; i<3; ++i ) obj.faces[k][i] = trianglesCH[k][i];
			}
			//
			m_objects.push_back(obj);
		}
		//
		HACD::DestroyHACD(myHACD);
		HACD::releaseHeapManager(heapManager);
		//
		if( m_param.export_path.size()) {
			//
			console::dump( "Saving HACD to %s...\n", m_param.export_path.c_str());
			hacd_io::write_hacd(m_param.export_path,m_objects);
			console::dump( "Done.\n" );
		}
		//
		vec3d min_v(1e18,1e18,1e18);
		vec3d max_v = -1.0 * min_v;
		//
		for( int n=0; n<m_objects.size(); ++n ) {
			//
			const auto &vertices = m_objects[n].vertices;
			for( size_t k=0; k<vertices.size(); k++ ) {
				for( int dim : DIMS3 ) {
					min_v[dim] = std::min(vertices[k][dim],min_v[dim]);
					max_v[dim] = std::max(vertices[k][dim],max_v[dim]);
				}
			}
		}
		//
		double scale (0.8);
		//
		for( int n=0; n<m_objects.size(); ++n ) {
			double norm_s = max_v[0]-min_v[0];
			if( norm_s ) {
				auto &vertices = m_objects[n].vertices;
				for( size_t k=0; k<vertices.size(); k++ ) {
					vertices[k] = scale * (vertices[k]-min_v) / norm_s + vec3d(0.5,0.0,0.5) - 0.5 * scale * vec3d(1.0,0.0,1.0);
					console::dump( "%f,%f,%f\n", vertices[k][0],vertices[k][1],vertices[k][2]);
				}
			}
		}
		//
		console::dump( "<<< Done.\n");
	}
	//
	virtual void draw( graphics_engine &g, int width, int height ) const override {
		//
		g.color4(1.0,1.0,1.0,0.5);
		graphics_utility::draw_wired_box(g);
		//
		unsigned size = m_objects.size();
		for( int n=0; n<size; ++n ) {
			//
			color::rgb rgb_color = color::hsv2rgb({ n * 360.0 / size, 0.5, 1.0 });
			const auto &vertices = m_objects[n].vertices;
			const auto &faces = m_objects[n].faces;
			if( n == m_focus_cluster ) g.color4(rgb_color.r,rgb_color.g,rgb_color.b,0.8);
			else g.color4(rgb_color.r,rgb_color.g,rgb_color.b,0.1);
			//
			for( size_t i=0; i<faces.size(); i++ ) {
				g.begin(graphics_engine::MODE::LINE_LOOP);
				for( unsigned j=0; j<faces[i].size(); j++ ) g.vertex3v(vertices[faces[i][j]].v);
				g.end();
			}
		}
		//
		g.color4(1.0,1.0,1.0,1.0);
		g.draw_string(vec3d(0.05,0.025,0.0).v,console::format_str("# Clusters = %u, Focus = %u", m_num_clusters, m_focus_cluster+1 ).c_str());
		g.draw_string(vec3d(0.05,0.1,0.0).v,"Type \"N\" to iterate clusters.");
	}
	//
	std::string filename {"bunny_watertight_low.ply"};
	polygon3_driver polygon{this,"polygon3"};
	//
	size_t m_num_clusters {0};
	size_t m_focus_cluster {0};
	//
	struct Parameters {
		bool export_hacd {false};
		std::string export_path;
		double compacity_weight {0.0001};
		double volume_weight {0.0};
		double connect_dist {30.0};
		unsigned min_clusters {2};
		unsigned vertices_per_convex_hull {100};
		double concavity {80.0};
		double small_cluster_threshold {0.25};
		unsigned number_target_triangles_decimatedmesh {2000};
		bool add_extra_distpoints {false};
		bool add_faces_points {false};
	};
	//
	Parameters m_param;
	std::vector<hacd_io::convex_object> m_objects;
};
//
extern "C" module * create_instance() {
	return new hacd;
}
//
extern "C" const char *license() {
	return "BSD";
}
