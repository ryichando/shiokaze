/*
**	SDFGen.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on February 5, 2017.
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
#include <shiokaze/meshlevelset/meshlevelset_interface.h>
#include "makelevelset3.h"
#include <cassert>
#include <shiokaze/array/array_interpolator3.h>
#include <shiokaze/utility/utility.h>
//
SHKZ_USING_NAMESPACE
//
class SDFGen : public meshlevelset_interface {
protected:
	//
	LONG_NAME("SDF Distance Field Converter")
	AUTHOR_NAME("Christopher Batty and Robert Bridson")
	ARGUMENT_NAME("SDFGen")
	//
	virtual void set_mesh( const std::vector<vec3d> &vertices, const std::vector<std::vector<size_t> > &faces) override {
		m_vertices = vertices;
		m_faces = faces;
		//
		assert( m_dx );
		assert( m_vertices.size());
		//
		// Compute bounding box
		m_corner0 = m_corner1 = m_vertices[0];
		for( unsigned n=1; n<m_vertices.size(); ++n ) {
			for( int dim : DIMS3 ) {
				m_corner0[dim] = std::min(m_vertices[n][dim],m_corner0[dim]);
				m_corner1[dim] = std::max(m_vertices[n][dim],m_corner1[dim]);
			}
		}
		m_scaling = 0.0;
		for( int dim : DIMS3 ) {
			m_scaling = std::max(m_scaling,m_corner1[dim]-m_corner0[dim]);
		}
		for( int dim : DIMS3 ) {
			m_corner0[dim] -= m_dx * m_padding;
			m_corner1[dim] += m_dx * m_padding;
		}
		for( int dim : DIMS3 ) {
			m_scaling = std::max(m_scaling,m_corner1[dim]-m_corner0[dim]);
		}
		for( int dim : DIMS3 ) {
			m_shape[dim] = std::ceil((m_corner1[dim]-m_corner0[dim]) / m_dx);
		}
	}
	virtual void generate_levelset() override {
		//
		m_levelset_array.initialize(m_shape);
		//
		std::vector<Vec3ui> tri(m_faces.size());
		for( unsigned n=0; n<m_faces.size(); ++n ) {
			assert(m_faces[n].size()==3);
			for( int i=0; i<3; ++i ) tri[n][i] = m_faces[n][i];
		}
		//
		std::vector<Vec3f> x(m_vertices.size());
		for( unsigned n=0; n<m_vertices.size(); ++n ) {
			for( int dim : DIMS3 ) x[n][dim] = m_vertices[n][dim];
		}
		//
		int nx (m_shape[0]), ny (m_shape[1]), nz (m_shape[2]);
		Vec3f corner0 (m_corner0[0],m_corner0[1],m_corner0[2]);
		Array3f phi(nx,ny,nz);
		//
		make_level_set3(tri,x,corner0,m_dx,nx,ny,nz,phi,m_halfwidth_band);
		//
		for( int i=0; i<nx; ++i ) {
			for( int j=0; j<ny; ++j ) {
				for( int k=0; k<nz; ++k ) {
					m_levelset_array.set(i,j,k,phi(i,j,k));
				}
			}
		}
	}
	virtual double get_levelset( const vec3d &p ) const override {
		double box_levelset = utility::box(p,m_corner0,m_corner1);
		const vec3d converted_p = p - m_corner0;
		return std::max(0.0,box_levelset)+array_interpolator3::interpolate(m_levelset_array,converted_p/m_dx-vec3d(0.5,0.5,0.5));
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_unsigned("ExactBand",m_halfwidth_band,"Exact halfwidth band");
		config.get_unsigned("MeshPadding",m_padding,"m_Padding for mesh to levelset");
	}
	virtual void initialize( double dx ) override {
		m_dx = dx;
	}
	//
	shape3 m_shape;
	double m_dx {0.0};
	unsigned m_padding{3};
	unsigned m_halfwidth_band{1};
	//
	array3<double> m_levelset_array{this};
	std::vector<vec3d> m_vertices;
	std::vector<std::vector<size_t> > m_faces;
	//
	vec3d m_corner0;
	vec3d m_corner1;
	double m_scaling {0.0};
};
//
extern "C" module * create_instance() {
	return new SDFGen();
}
//
extern "C" const char *license() {
	return "MIT";
}
//
