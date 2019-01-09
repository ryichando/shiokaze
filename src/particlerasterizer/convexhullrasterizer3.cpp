/*
**	convexhullrasterizer3.cpp
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
#include <shiokaze/pointgridhash/pointgridhash3_interface.h>
#include <shiokaze/particlerasterizer/particlerasterizer3_interface.h>
//
SHKZ_USING_NAMESPACE
//
class convexhullrasterizer3 : public particlerasterizer3_interface {
public:
	//
	LONG_NAME("Convex Hull Rasterizer 3D")
	ARGUMENT_NAME("ConvexHullRasterizer")
	//
	virtual void build_levelset( array3<double> &fluid, const bitarray3 &mask, const std::vector<Particle3> &particles ) const override {
		//
		auto sqr = [](double x) { return x*x; };
		auto getParticleConvextHullLevelsetSphere = [&]( const vec3d p, const Particle3 &p0 ) {
			return (p0.p-p).len()-(p0.r+m_param.surface_margin*m_dx);
		};
		auto getParticleConvexHullLevelsetCylinder = [&](vec3d p, const Particle3 &p0, const Particle3 &p1 ) {
			vec3d original_pos = p;
			vec3d positions[3] = { p0.p, p1.p, p };
			vec3d nm = ((positions[2]-positions[0])^(positions[1]-positions[0])).normal();
			vec3d e[2];
			e[0] = (positions[1]-positions[0]).normal();
			e[1] = nm ^ e[0];
			vec3d old_points[3];
			for( unsigned i=0; i<3; i++ ) old_points[i] = positions[i];
			for( unsigned i=0; i<3; i++ ) {
				positions[i][0] = e[0] * (old_points[i]-old_points[0]);
				positions[i][1] = e[1] * (old_points[i]-old_points[0]);
				positions[i][2] = 0.0;
			}	
			double max_phi = -1e9;
			bool out_of_bound = false;
			vec3d offset = vec3d((positions[0]-positions[1])[1],-(positions[0]-positions[1])[0],0.0);
			double x1 = (positions[0]+offset)[0];
			double y1 = (positions[0]+offset)[1];
			double x2 = (positions[1]+offset)[0];
			double y2 = (positions[1]+offset)[1];
			p += offset;
			double r1 = p0.r+m_param.surface_margin*m_dx;
			double r2 = p1.r+m_param.surface_margin*m_dx;
			double DET = x2*y1-x1*y2;
			if( ! (p0.p-p1.p).norm2() ) return 1e9;
			if( ! DET ) {
				out_of_bound = true;
			} else {
				// Build a quadric equation
				double A1 = (y2-y1)/DET;
				double B1 = (r2*y1-r1*y2)/DET;
				double A2 = (x1-x2)/DET;
				double B2 = (r1*x2-r2*x1)/DET;
				double det = sqr(A1)*(1.0-sqr(B2))+sqr(A2)*(1.0-sqr(B1))+2.0*A1*A2*B1*B2;
				if( det > 0.0 ) {
					// Solve it !
					for( unsigned k=0; k<2; k++ ) {
						double detn = sqr(A1)+sqr(A2);
						if( detn ) {
							double d = ((k==0?1:-1)*sqrtf(det)-A1*B1-A2*B2)/detn;
							vec3d normal(A1*d+B1,A2*d+B2,0.0);
							vec3d head0 = (positions[0]+offset)-normal*r1;
							vec3d head1 = (positions[1]+offset)-normal*r2;
							double dist = normal*positions[2]+d;
							vec3d out = positions[2]+normal*dist;
							// Check if out is inside the line
							if( (out-head0)*(out-head1) <= 0.0 ) {
								if( dist > max_phi ) {
									max_phi = dist;
								}
							} else {
								max_phi = -1e9;
								break;
							}
						}
					}
				}
			}
			p = original_pos;
			if( max_phi < -1.0 ) out_of_bound = true;
			if( out_of_bound ) {
				// Sphere
				max_phi = 1e9;
				const Particle3 *pair[] = { &p0, &p1 };
				for( unsigned n=0; n<2; n++ ) {
					vec3d fpos = original_pos;
					double phi = getParticleConvextHullLevelsetSphere(fpos,*pair[n]);
					if( phi < max_phi ) {
						max_phi = phi;
						out_of_bound = false;
					}
				}
			}
			return out_of_bound ? 1e9 : max_phi;
		};
		auto getParticleConvexHullLevelsetPlane = [&](vec3d p, const Particle3 &p0, const Particle3 &p1, const Particle3 &p2 ) {
			// Triangle
			double max_phi = -1e9;
			bool out_of_bound = false;
			vec3d original_pos = p;
			vec3d offset = (p1.p-p0.p) ^ (p2.p-p0.p);
			double x1 = (p0.p+offset)[0];
			double y1 = (p0.p+offset)[1];
			double z1 = (p0.p+offset)[2];
			double x2 = (p1.p+offset)[0];
			double y2 = (p1.p+offset)[1];
			double z2 = (p1.p+offset)[2];
			double x3 = (p2.p+offset)[0];
			double y3 = (p2.p+offset)[1];
			double z3 = (p2.p+offset)[2];
			p += offset;
			double r1 = p0.r+m_param.surface_margin*m_dx;
			double r2 = p1.r+m_param.surface_margin*m_dx;
			double r3 = p2.r+m_param.surface_margin*m_dx;
			double DET = -x3*y2*z1+x2*y3*z1+x3*y1*z2-x1*y3*z2-x2*y1*z3+x1*y2*z3;
			if( ! (p0.p-p1.p).norm2() || ! (p1.p-p2.p).norm2() || ! (p2.p-p0.p).norm2() ) return 1e9;
			if( DET ) {
				double A1 = (-y3*z1-y1*z2+y3*z2+y2*(z1-z3)+y1*z3)/DET;
				double B1 = (-r3*y2*z1+r2*y3*z1+r3*y1*z2-r1*y3*z2-r2*y1*z3+r1*y2*z3)/DET;
				double A2 = (x3*z1+x1*z2-x3*z2-x1*z3+x2*(-z1+z3))/DET;
				double B2 = (r3*x2*z1-r2*x3*z1-r3*x1*z2+r1*x3*z2+r2*x1*z3-r1*x2*z3)/DET;
				double A3 = (-x3*y1-x1*y2+x3*y2+x2*(y1-y3)+x1*y3)/DET;
				double B3 = (-r3*x2*y1+r2*x3*y1+r3*x1*y2-r1*x3*y2-r2*x1*y3+r1*x2*y3)/DET;
				double detn = sqr(A1)+sqr(A2)+sqr(A3);
				double det = 4.0*sqr(A1*B1+A2*B2+A3*B3)-4.0*detn*(sqr(B1)+sqr(B2)+sqr(B3)-1.0);
				if( det > 0.0 && detn ) {
					// Solve it !
					for( unsigned k=0; k<2; k++ ) {
						double d = ((k==0?1:-1)*0.5*sqrtf(det)-A1*B1-A2*B2-A3*B3)/detn;
						vec3d normal(A1*d+B1,A2*d+B2,A3*d+B3);
						vec3d head0 = (p0.p+offset)-normal*r1;
						vec3d head1 = (p1.p+offset)-normal*r2;
						vec3d head2 = (p2.p+offset)-normal*r3;
						double dist = -(normal*p+d);
						vec3d out = p+normal*dist;
						double cross1 = ((head1-head0)^(out-head0)) * normal;
						double cross2 = ((head2-head1)^(out-head1)) * normal;
						double cross3 = ((head0-head2)^(out-head2)) * normal;
						if( (cross1 >= 0.0 && cross2 >= 0.0 && cross3 >= 0.0) || (cross1 <= 0.0 && cross2 <= 0.0 && cross3 <= 0.0) ) {
							if( dist > max_phi ) max_phi = dist;
						} else {
							max_phi = -1e9;
							break;
						}
					}
				}
			}
			p = original_pos;
			if( max_phi < -1.0 ) out_of_bound = true;
			if( out_of_bound ) {
				// Cylineder
				max_phi = 1e9;
				const Particle3 *triple[] = { &p0, &p1, &p2 };
				for( unsigned n0=0; n0<3; n0++ ) for( unsigned n1=n0+1; n1<3; n1++ ) {
					double phi = getParticleConvexHullLevelsetCylinder(p,*triple[n0],*triple[n1]);
					if( phi < max_phi ) {
						max_phi = phi;
						out_of_bound = false;
					}
				}
			}
			return out_of_bound ? 1e9 : max_phi;
		};
		//
		auto getConvexhullLevelset = [&](const vec3d &p, const std::vector<size_t> &neighbors ) {
			double min_phi = 1.0;
			double wall_offset = 0.25*m_dx;
			vec3d fp(p);
			for( unsigned dim : DIMS3 ) fp[dim] = std::min(m_dx*m_shape[dim]-wall_offset,std::max(wall_offset,fp[dim]));
			for( unsigned n0=0; n0<neighbors.size(); n0++ ) for( unsigned n1=n0+1; n1<neighbors.size(); n1++ ) for( unsigned n2=n1+1; n2<neighbors.size(); n2++ ) {
				const Particle3 &p0 = particles.at(neighbors[n0]);
				const Particle3 &p1 = particles.at(neighbors[n1]);
				const Particle3 &p2 = particles.at(neighbors[n2]);
				double r_limit = 2.0;
				if( (p0.p-p1.p).norm2() < sqr(r_limit*(p0.r+p1.r)) && (p1.p-p2.p).norm2() < sqr(r_limit*(p1.r+p2.r)) && (p2.p-p0.p).norm2() < sqr(r_limit*(p2.r+p0.r)) ) {
					min_phi = fmin(min_phi,getParticleConvexHullLevelsetPlane(fp,p0,p1,p2));
				}
			}
			return min_phi;
		};
		//
		std::vector<vec3d> points(particles.size());
		for( unsigned n=0; n<points.size(); ++n ) {
			points[n] = particles[n].p;
		}
		const_cast<pointgridhash3_driver &>(m_pointgridhash)->sort_points(points);
		//
		fluid.clear();
		fluid.activate_as(mask);
		fluid.parallel_actives( [&](int i, int j, int k, auto &it ) {
			std::vector<size_t> neighbors = m_pointgridhash->get_cell_neighbors(vec3i(i,j,k),pointgridhash3_interface::USE_NODAL);
			it.set(getConvexhullLevelset(m_dx*vec3i(i,j,k).cell(),neighbors));
		});
	}
	//
protected:
	//
	virtual void configure( configuration &config ) override {
		config.get_double("SurfaceMargin",m_param.surface_margin,"Margin for surface sphere");
	}
	virtual void initialize( const shape3 &shape, double dx ) override {
		m_shape = shape;
		m_dx = dx;
	}
	//
	typedef struct {
		double surface_margin {0.125};
	} Parameters;
	Parameters m_param;
	//
	pointgridhash3_driver m_pointgridhash{this,"pointgridhash3"};
	shape3 m_shape;
	double m_dx;
};
//
//
extern "C" module * create_instance() {
	return new convexhullrasterizer3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//