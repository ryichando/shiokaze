/*
**	meshexporter_demo3.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Aug 29, 2017. 
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
#include <shiokaze/meshexporter/meshexporter3_interface.h>
#include <shiokaze/cellmesher/cellmesher3_interface.h>
#include <shiokaze/array/shared_array3.h>
#include <shiokaze/core/timer.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/runnable.h>
//
SHKZ_USING_NAMESPACE
//
class meshexporter_demo3 : public runnable {
public:
	LONG_NAME("Mesh Exporter Demo")
	//
	meshexporter_demo3() {
		m_export_path = "sphere.ply";
	}
	//
protected:
	//
	virtual void load( configuration &config ) override {
		if( console::get_root_path().size()) {
			m_export_path = console::get_root_path() + "/" + m_export_path;
		}
	}
	//
	virtual void configure( configuration &config ) override {
		//
		config.get_unsigned("Resolution",m_shape[0], "Grid resolution");
		m_shape[1] = m_shape[2] = m_shape[0];
		config.get_unsigned("ResolutionX",m_shape[0],"Resolution towards X axis");
		config.get_unsigned("ResolutionY",m_shape[1],"Resolution towards Y axis");
		config.get_unsigned("ResolutionZ",m_shape[2],"Resolution towards Z axis");
		double scale (1.0);
		config.get_double("ResolutionScale",scale,"Resolution doubling scale");
		//
		config.get_string("Path",m_export_path,"PLY export path");
		m_shape *= scale;
		m_dx = m_shape.dx();
		//
		set_environment("shape",&m_shape);
		set_environment("dx",&m_dx);
	}
	//
	virtual void post_initialize() override {
		//
		scoped_timer timer(this);
		//
		timer.tick(); console::dump("Generating spherical levelset (%dx%dx%d)...", m_shape[0],m_shape[1],m_shape[2]);
		shared_array3<Real> sphere(m_shape);
		sphere->parallel_all([&](int i, int j, int k, auto &it ) {
			vec3d p = vec3d(i-m_shape.w/2.0,j-m_shape.h/2.0,k-m_shape.d/2.0) * m_dx;
			it.set(p.len() - 0.4);
		});
		console::dump( "Done. Took %s.\n", timer.stock().c_str());
		//
		std::vector<vec3d> vertices;
		std::vector<std::vector<size_t> > faces;
		//
		timer.tick(); console::dump("Generating mesh...");
		m_mesher->generate_mesh(sphere(),vertices,faces);
		console::dump( "Done. Generated %d vertices and %d faces. Took %s.\n", vertices.size(),faces.size(),timer.stock().c_str());
		//
		timer.tick(); console::dump("Exporting mesh \"%s\"...",m_export_path.c_str());
		m_exporter->set_mesh(vertices,faces);
		m_exporter->export_ply(m_export_path.c_str());
		console::dump( "Done. Took %s.\n", timer.stock().c_str());
		//
	};
	//
private:
	//
	shape3 m_shape{64,64,64};
	double m_dx;
	std::string m_export_path;
	cellmesher3_driver m_mesher{this,"marchingcubes"};
	meshexporter3_driver m_exporter{"meshexporter3"};
};
//
extern "C" module * create_instance() {
	return new meshexporter_demo3;
}
//
extern "C" const char *license() {
	return "MIT";
}
