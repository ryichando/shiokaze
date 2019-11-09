/*
**	marchingsquare.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on October 29, 2019.
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
#include <shiokaze/cellmesher/cellmesher2_interface.h>
#include <shiokaze/utility/meshutility2_interface.h>
//
SHKZ_USING_NAMESPACE
//
class marchingsquare : public cellmesher2_interface {
protected:
	//
	LONG_NAME("Marching Square")
	//
	virtual void generate_contour( const array2<float> &levelset, std::vector<vec2d> &vertices, std::vector<std::vector<size_t> > &faces ) const override {
		//
		assert(m_dx);
		vec2d global_origin = levelset.shape()==m_shape.nodal() ? vec2d() : m_dx * vec2d(0.5,0.5);
		//
		vertices.clear();
		faces.clear();
		//
		const shape2 s = levelset.shape();
		size_t index (0);
		levelset.const_serial_actives([&]( int i, int j, const auto &it ) {
			if( i < s[0]-1 && j < s[1]-1 ) {
				//
				vec2d p[8];	int pnum; double v[2][2]; vec2d _vertices[2][2];
				for( int ni=0; ni<2; ni++ ) for( int nj=0; nj<2; nj++ ) {
					v[ni][nj] = levelset(i+ni,j+nj);
					_vertices[ni][nj] = m_dx*vec2d(i+ni,j+nj);
				}
				m_meshutility->march_points(v,_vertices,p,pnum,false);
				std::vector<size_t> face;
				for( int m=0; m<pnum; m++ ) {
					vertices.push_back(p[m]+global_origin);
					face.push_back(index++);
					if( face.size() == 2 ) {
						faces.push_back(face);
						face.clear();
						face.push_back(index);
					}
				}
			}
		});
	}
	//
	virtual void initialize ( const shape2 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	meshutility2_driver m_meshutility{this,"meshutility2"};
	//
	shape2 m_shape;
	double m_dx {0.0};
};
//
extern "C" module * create_instance() {
	return new marchingsquare;
}
//
extern "C" const char *license() {
	return "MIT";
}
//