/*
**	vecfluid2.cpp
**
**	This example is provided as part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Dec 15, 2017, originally written for the book CG Gems 2012.
**
**	For more detail and licensing, please visit:
**	https://www.borndigital.co.jp/book/94.html (Japanese)
**
*/
#include <cmath>
#include <list>
#include <shiokaze/ui/drawable.h>
//
typedef struct {
	double x, y;
} point2;
std::list<point2> contour;
//
SHKZ_USING_NAMESPACE
//
class vecfluid2 : public drawable {
private:
	//
	LONG_NAME("Vecfluid")
	ARGUMENT_NAME("Vecfluid")
	//
	// ********** Variables **********
	unsigned Nx, Ny;
	double dx, ds, dt;
	double **u, **v, **pressure;
	bool running;
	unsigned max_verties;
	// *******************************
	//
	static double ** copy2( double **in, double **out, unsigned nx, unsigned ny ) {
		for( unsigned i=0; i<nx; i++ ) for( unsigned j=0; j<ny; j++ ) {
			out[i][j] = in[i][j];
		}
		return out;
	}
	//
	static double ** memset2( double **q, double value, unsigned nx, unsigned ny ) {
		for( unsigned i=0; i<nx; i++ ) for( unsigned j=0; j<ny; j++ ) {
			q[i][j] = value;
		}
		return q;
	}
	//
	static double ** alloc2( unsigned nx, unsigned ny ) {
		double **q = new double *[nx];
		for( unsigned i=0; i<nx; i++ ) q[i] = new double [ny];
		return memset2(q,0.0,nx,ny);
	}
	//
	static double ** duplicate2( double **q, unsigned nx, unsigned ny ) {
		double **ptr = alloc2(nx,ny);
		return copy2(q,ptr,nx,ny);
	}
	//
	static void free2( double **q, unsigned nx, unsigned ny ) {
		for( unsigned i=0; i<nx; i++ ) delete [] q[i];
		delete [] q;
	}
	//
	static double interp(double x, double y, double **q, unsigned Nx, unsigned Ny, double dx ) {
		//
		x = fmax(0.0,fmin(Nx-1-1e-6,x/dx));
		y = fmax(0.0,fmin(Ny-1-1e-6,y/dx));
		unsigned i = x;
		unsigned j = y;
		double f[4] = { q[i][j], q[i][j+1], q[i+1][j], q[i+1][j+1] };
		x = x-i; y = y-j;
		double c[4] = { (1.-x)*(1.-y), (1.-x)*y, x*(1.-y), x*y };
		return c[0]*f[0]+c[1]*f[1]+c[2]*f[2]+c[3]*f[3];
	}
	//
	virtual void setup_window( std::string &name, int &width, int &height ) const override {
		//
		name = "Vecfluid";
		height = width * Ny / (double) Nx;
	}
	//
	virtual void load ( configuration &config ) override {
		//
		Nx = 32;
		Ny = 24;
		running = true;
		max_verties = 2e4;
	}
	//
	virtual void configure( configuration &config ) override {
		config.get_unsigned("Nx",Nx,"Resolution X");
		config.get_unsigned("Ny",Ny,"Resolution Y");
	}
	//
	virtual void initialize( const environment_map &environment ) override {
		//
		u = alloc2(Nx+1,Ny);
		v = alloc2(Nx,Ny+1);
		pressure = alloc2(Nx,Ny);
		dx = 1.0/fmax(Nx,Ny);
		dt = 0.1*dx;
		ds = 0.1*dx;
		reset();
		//
		printf( "r: reset\n" );
		printf( "p: pause\n" );
		printf( "w: export SVG\n" );
	}
	//
	void reset() {
		//
		contour.clear();
		addMarble(contour,0.5*Nx*dx,0.5*Ny*dx,0.2*fmin(Nx*dx,Ny*dx));
		memset2(u,0.0,Nx+1,Ny);
		memset2(v,0.0,Nx,Ny+1);
		running = true;
	}
	//
	void addMarble( std::list<point2> &contour, double x, double y, double r ) {
		//
		double phi;
		double maxv = 3.1415*2.0;
		int steps = 30;
		contour.clear();
		for( phi = 0.0f; phi <= maxv; phi += maxv/steps ) {
			point2 p = {x+r*cos(phi), y+r*sin(phi) };
			contour.push_back(p);
		}
	}
	//
	virtual bool keyboard( char key ) override {
		//
		switch( key ) {
			case '\e':
				exit(0);
				return true;
			case 'W':
				exportSVG(contour,"output.svg");
				return true;
			case 'R':
				reset();
				return true;
			case 'P':
				running = ! running;
				return true;
		}
		return false;
	}
	//
	virtual void drag( int width, int height, double x, double y, double _u, double _v ) override {
		//
		double mouse_pos[2] = {x,y};
		double force[2] = {_u,_v};
		//
		int i = std::max(0.0,std::min(Nx-1-1e-6,mouse_pos[0]/dx));
		int j = std::max(0.0,std::min(Ny-1-1e-6,mouse_pos[1]/dx));
		//
		if( i>0 && j>0 && i<Nx-1 && j<Ny-1 ) {
			double wx = mouse_pos[0]/dx-i;
			double wy = mouse_pos[1]/dx-j;
			u[i][j] += (1.0-wx)*force[0];
			u[i+1][j] += wx*force[0];
			v[i][j] += (1.0-wy)*force[1];
			v[i][j+1] += wy*force[1];
		}
	}
	//
	virtual void idle() override {
		if(running) advance(dt);
	}
	//
	void advance( double dt ) {
		//
		advect(Nx,Ny,dx,dt,u,v);
		project(Nx,Ny,dx,dt,1.0,u,v,pressure);
		deform(Nx,Ny,dx,dt,u,v,contour);
		resample(contour,0.5*ds,ds);
	}
	//
	void advect(unsigned Nx, unsigned Ny, double dx, double dt,	double **u, double **v) {
		//
		double **u_old = duplicate2(u,Nx+1,Ny);
		double **v_old = duplicate2(v,Nx,Ny+1);
		for(unsigned i=1; i<Nx; i++) for(unsigned j=0; j<Ny; j++) {
			double x = i*dx;
			double y = (j+0.5)*dx;
			x = x - dt*interp(x,y-0.5*dx,u_old,Nx+1,Ny,dx);
			y = y - dt*interp(x-0.5*dx,y,v_old,Nx,Ny+1,dx);
			u[i][j] = interp(x,y-0.5*dx,u_old,Nx+1,Ny,dx);
		}
		for(unsigned i=0; i<Nx; i++) for(unsigned j=1; j<Ny; j++) {
			double x = (i+0.5)*dx;
			double y = j*dx;
			x = x - dt*interp(x,y-0.5*dx,u_old,Nx+1,Ny,dx);
			y = y - dt*interp(x-0.5*dx,y,v_old,Nx,Ny+1,dx);
			v[i][j] = interp(x-0.5*dx,y,v_old,Nx,Ny+1,dx);
		}
		free2(u_old,Nx+1,Ny);
		free2(v_old,Nx,Ny+1);
	}
	//
	void project(unsigned Nx, unsigned Ny, double dx, double dt, double rho,
				 double **u, double **v, double **pressure ) {
		//
		double scale = dt/(rho*dx*dx);
		double eps = 1.0e-4;
		double err;
		do {
			err = 0.0;
			for(unsigned j=0; j<Ny; j++) for(unsigned i=0; i<Nx; i++) {
				double D[4] = { 1.0, 1.0, -1.0, -1.0 };
				bool   F[4] = { i<Nx-1, j<Ny-1, i>0, j>0 };
				double P[4] = { F[0] ? pressure[i+1][j] : 0.0, 
								F[1] ? pressure[i][j+1] : 0.0, 
								F[2] ? pressure[i-1][j] : 0.0,
								F[3] ? pressure[i][j-1] : 0.0 };
				double U[4] = { u[i+1][j], v[i][j+1], u[i][j], v[i][j] };
				double det = 0.0;
				double sum_L = 0.0;
				double sum_R = 0.0;
				for(unsigned n=0; n<4; ++n) {
					det += F[n]*scale;
					sum_L += F[n]*P[n]*scale;
					sum_R += F[n]*D[n]*U[n]/dx;
				}
				err = fmax(err,fabs(det*pressure[i][j]-sum_L+sum_R));
				pressure[i][j] = (sum_L-sum_R)/det;
			}
		} while( eps < err );
		//
		for(unsigned i=1; i<Nx; i++ ) for(unsigned j=0; j<Ny; j++ ) {
			u[i][j] = u[i][j] - dt/rho*(pressure[i][j]-pressure[i-1][j])/dx;
		}
		for(unsigned i=0; i<Nx; i++ ) for(unsigned j=1; j<Ny; j++ ) {
			v[i][j] = v[i][j] - dt/rho*(pressure[i][j]-pressure[i][j-1])/dx;
		}
	}
	//
	void deform(unsigned Nx, unsigned Ny, double dx, double dt, double **u, double **v,	std::list<point2> &contour ) {
		//
		std::list<point2>::iterator it;
		for( it=contour.begin(); it!=contour.end(); it++ ) {
			point2 &p = *it;
			double k1_u = interp(p.x,p.y-0.5*dx,u,Nx+1,Ny,dx);
			double k1_v = interp(p.x-0.5*dx,p.y,v,Nx,Ny+1,dx);
			double k2_u = interp(p.x+0.5*dt*k1_u,p.y-0.5*dx+0.5*dt*k1_v,u,Nx+1,Ny,dx);
			double k2_v = interp(p.x-0.5*dx+0.5*dt*k1_u,p.y+0.5*dt*k1_v,v,Nx,Ny+1,dx);
			double k3_u = interp(p.x+0.5*dt*k2_u,p.y-0.5*dx+0.5*dt*k2_v,u,Nx+1,Ny,dx);
			double k3_v = interp(p.x-0.5*dx+0.5*dt*k2_u,p.y+0.5*dt*k2_v,v,Nx,Ny+1,dx);
			double k4_u = interp(p.x+dt*k3_u,p.y-0.5*dx+dt*k3_v,u,Nx+1,Ny,dx);
			double k4_v = interp(p.x-0.5*dx+dt*k3_u,p.y+dt*k3_v,v,Nx,Ny+1,dx);
			p.x = p.x + dt*(k1_u+2.0*k2_u+2.0*k3_u+k4_u)/6.0;
			p.y = p.y + dt*(k1_v+2.0*k2_v+2.0*k3_v+k4_v)/6.0;
		}
		*(--contour.end()) = *contour.begin();
	}
	//
	void resample( std::list<point2> &contour, double min_ds, double max_ds ) {
		//
		std::list<point2>::iterator forward = contour.begin();
		std::list<point2>::iterator backward = forward;
		for( forward++; forward!=contour.end(); ) {
			point2 &p0 = *backward;
			point2 &p1 = *forward;
			double d = hypot(p0.x-p1.x,p0.y-p1.y);
			if( d < min_ds ) {
				p0.x = 0.5*(p0.x+p1.x);
				p0.y = 0.5*(p0.y+p1.y);
				forward = contour.erase(forward);
			} else if( d > max_ds ) {
				point2 p = {0.5*(p0.x+p1.x), 0.5*(p0.y+p1.y)};
				forward = contour.insert(forward,p);
			} else {
				backward = forward;
				forward ++;
			}
		}
		if( contour.size() > max_verties ) {
			printf( "maxinum vertex number reached.\n" );
			running = false;
		}
	}
	//
	virtual void draw( const graphics_engine &g, int width, int height ) const override {
		//
		draw_contour(g,contour);
		g.color4(1.0,1.0,1.0,0.5);
		draw_grid(g);
		g.color4(1.0,1.0,0.0,0.5);
		draw_velocity(g);
	};
	//
	void draw_contour( const graphics_engine &g, const std::list<point2> &contour ) const {
		//
		std::list<point2>::const_iterator forward = contour.begin();
		std::list<point2>::const_iterator backward = forward;
		g.enable(graphics_engine::CAPABILITY::COLOR_LOGIC_OP);
		g.logic_op(graphics_engine::OPERATION::INVERT);
		g.begin(graphics_engine::MODE::TRIANGLES);
		for(forward++; forward!=contour.end(); forward++) {
			const point2 &p0 = *backward;
			const point2 &p1 = *forward;
			g.vertex2(0.0,0.0);
			g.vertex2(p0.x,p0.y);
			g.vertex2(p1.x,p1.y);
			backward = forward;
		}
		g.end();
		g.disable(graphics_engine::CAPABILITY::COLOR_LOGIC_OP);
	}
	//
	void exportSVG( const std::list<point2> &contour, const char *path ) const {
		//
		FILE *fp = fopen(path,"w");
		if( fp ) {
			std::list<point2>::const_iterator it = contour.begin();
			fprintf(fp,"<svg viewBox=\"0 0 1 1\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n" );
			fprintf(fp,"<path fill=\"rgb(128,200,256)\" d=\"M %f %f\n", (*it).x, (*it).y );
			for(it++; it!=contour.end(); it++) {
				fprintf(fp,"L %f %f\n", (*it).x, (*it).y );
			}
			fprintf(fp,"Z\" />\n");
			fprintf(fp,"</svg>");
			fclose(fp);
		}
	}
	//
	void draw_velocity( const graphics_engine &g ) const {
		//
		g.begin(graphics_engine::MODE::LINES);
		for( unsigned i=0; i<Nx; i++ ) for( unsigned j=0; j<Ny; j++ ) {
			point2 p = {(i+0.5)*dx,(j+0.5)*dx};
			point2 vel = {0.5*(u[i][j]+u[i+1][j]),0.5*(v[i][j]+v[i][j+1])};
			double ks = 1.0;
			g.vertex2(p.x,p.y);
			g.vertex2(p.x+ks*dx*vel.x,p.y+ks*dx*vel.y);
		}
		g.end();
	}
	//
	void draw_grid( const graphics_engine &g ) const {
		//
		g.begin(graphics_engine::MODE::LINES);
		for( unsigned i=0; i<Nx+1; i++ ) {
			g.vertex2(dx*i,0.0);
			g.vertex2(dx*i,dx*Ny);
		}
		for( unsigned j=0; j<Ny+1; j++ ) {
			g.vertex2(0.0,dx*j);
			g.vertex2(dx*Nx,dx*j);
		}
		g.end();
	}
	//
	virtual void resize( const graphics_engine &g, int width, int height ) override {
		//
		double r = Ny/(double)Nx;
		double h2 = 0.5*r*width;
		double margin = 0.02;
		g.viewport(0,height/2-h2,width,2*h2);
		g.load_identity();
		g.ortho(-margin,Nx*dx+margin,-margin,Ny*dx+margin,-1.0,1.0);
	}
};
//
extern "C" module * create_instance() {
	return new vecfluid2;
}