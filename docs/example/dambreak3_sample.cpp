//! [code]
#include <shiokaze/math/vec.h>
#include <shiokaze/core/configuration.h>
#include <string>
#include <cmath>
//
SHKZ_USING_NAMESPACE
//
static double width (0.232), height (0.432), level (0.095), depth (0.2532);
//
extern "C" void configure( configuration &config ) {
	configuration::auto_group group(config,"Dambreak Scene 3D","Dambreak");
	config.get_double("Width",width,"Width of the dam");
	config.get_double("Height",height,"Height of the dam");
	config.get_double("Level",level,"Height of the pool");
}
//
extern "C" double fluid( const vec3d &p ) {
	double value = -1e9;
	value = std::max(value,p[0]-width);
	value = std::max(value,p[1]-height);
	value = std::min(value,p[1]-level);
	return value;
}
//
extern "C" const char *license() {
	return "MIT";
}
//! [code]
//! [wscript]
bld.shlib(source = 'dambreak3.cpp',
			target = 'dambreak3'+bld.env.suffix,
			cxxflags = ['-Wno-return-type-c-linkage'],
			use = bld.get_target_name(bld,'core'))
//! [wscript]