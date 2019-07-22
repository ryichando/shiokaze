/*
**	convexhullrasterizer2.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 27, 2017. 
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
#include <shiokaze/particlerasterizer/particlerasterizer2_interface.h>
//
SHKZ_USING_NAMESPACE
//
class convexhullrasterizer2 : public particlerasterizer2_interface {
protected:
	//
	LONG_NAME("Convex Hull Rasterizer 2D")
	MODULE_NAME("convexhullrasterizer2")
	ARGUMENT_NAME("ConvexHullRasterizer")
	//
	virtual void build_levelset( array2<float> &fluid, const bitarray2 &mask, const std::vector<Particle2> &particles ) const override {
		//
		auto sqr = [](double x) { return x*x; };
		auto getParticleConvextHullLevelsetSphere = [&]( const vec2d p, const Particle2 &p0 ) {
			return (p0.p-p).len()-(p0.r+m_param.surface_margin*m_dx);
		};
		auto getParticleConvextHullLevelsetCylinder = [&]( vec2d p, const Particle2 &p0, const Particle2 &p1 ) {
			// Line
			double max_phi = -1e9;
			bool out_of_bound = false;
			vec2d offset = (p0.p-p1.p).rotate90();
			double x1 = (p0.p+offset)[0];
			double y1 = (p0.p+offset)[1];
			double x2 = (p1.p+offset)[0];
			double y2 = (p1.p+offset)[1];
			p += offset;
			double r1 = p0.r+m_param.surface_margin*m_dx;
			double r2 = p1.r+m_param.surface_margin*m_dx;
			double DET = x2*y1-x1*y2;
			if( ! (p0.p-p1.p).norm2() ) return 1e9;
			if( ! DET ) {
				return 1e9;
			} else {
				double A1 = (y2-y1)/DET;
				double B1 = (r2*y1-r1*y2)/DET;
				double A2 = (x1-x2)/DET;
				double B2 = (r1*x2-r2*x1)/DET;
				double det = sqr(A1)*(1.0-sqr(B2))+sqr(A2)*(1.0-sqr(B1))+2.0*A1*A2*B1*B2;
				if( det > 0.0 ) {
					// Solve it !
					for( unsigned k=0; k<2; k++ ) {
						double d = ((k==0?1:-1)*sqrtf(det)-A1*B1-A2*B2)/(sqr(A1)+sqr(A2));
						vec2d normal(A1*d+B1,A2*d+B2);
						vec2d head0 = (p0.p+offset)-normal*r1;
						vec2d head1 = (p1.p+offset)-normal*r2;
						double dist = -(normal*p+d);
						vec2d out = p+normal*dist;
						if( (out-head0)*(out-head1) < 0.0 ) {
							if( dist > max_phi ) max_phi = dist;
						} else {
							max_phi = -1e9;
							break;
						}
					}
				}
			}
			p -= offset;
			if( max_phi < -1.0 ) out_of_bound = true;
			// Sphere
			if( out_of_bound ) {
				out_of_bound = false;
				max_phi = 1e9;
				const Particle2 *pairs[] = { &p0, &p1 };
				for( uint n=0; n<2; n++ ) {
					double phi = getParticleConvextHullLevelsetSphere(p,*pairs[n]);
					if( phi < max_phi ) {
						max_phi = phi;
						out_of_bound = false;
					}
				}
			}
			return out_of_bound ? 1.0 : max_phi;
		};
		//
		auto getConvexhullLevelset = [&](const vec2d &p, const std::vector<size_t> &neighbors) {
			double min_phi = 1.0;
			double wall_offset = 0.25*m_dx;
			vec2d fp(p);
			for( unsigned dim : DIMS2 ) fp[dim] = std::min(m_dx*m_shape[dim]-wall_offset,std::max(wall_offset,fp[dim]));
			for( size_t n0=0; n0<neighbors.size(); n0++ ) for( size_t n1=n0+1; n1<neighbors.size(); n1++ ) {
				const Particle2 &p0 = particles[neighbors[n0]];
				const Particle2 &p1 = particles[neighbors[n1]];
				double r_limit = 2.0;
				if( (p0.p-p1.p).norm2() < sqr(r_limit*(p0.r+p1.r)) ) {
					min_phi = fmin(min_phi,getParticleConvextHullLevelsetCylinder(fp,p0,p1));
				}
			}
			return min_phi;
		};
		//
		std::vector<vec2f> points(particles.size());
		for( size_t n=0; n<points.size(); ++n ) {
			points[n] = particles[n].p;
		}
		const_cast<pointgridhash2_driver &>(m_pointgridhash)->sort_points(points);
		//
		fluid.clear();
		fluid.activate_as(mask);
		fluid.parallel_actives( [&](int i, int j, auto &it ) {
			std::vector<size_t> neighbors = m_pointgridhash->get_cell_neighbors(vec2i(i,j),pointgridhash2_interface::USE_NODAL);
			it.set(getConvexhullLevelset(m_dx*vec2i(i,j).cell(),neighbors));
		});
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_double("SurfaceMargin",m_param.surface_margin,"Margin for surface sphere");
	}
	virtual void initialize( const shape2 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	typedef struct {
		double surface_margin {0.125};
	} Parameters;
	Parameters m_param;
	//
	pointgridhash2_driver m_pointgridhash{this,"pointgridhash2"};
	shape2 m_shape;
	double m_dx;
};
//
extern "C" module * create_instance() {
	return new convexhullrasterizer2();
}
//
extern "C" const char *license() {
	return "MIT";
}
//