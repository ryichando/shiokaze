/*
**	configuration.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 25, 2018.
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
#include <shiokaze/core/configuration.h>
#include <shiokaze/core/console.h>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <sys/ioctl.h>
#include <unistd.h>
#include <mutex>
#include <typeinfo>
#include <boost/date_time/gregorian/gregorian.hpp>
//
#define STACK_DEBUG		0
#define MAX_WIDTH		65
#define SHOW_ID			0
//
SHKZ_USING_NAMESPACE
static std::mutex config_mutex;
//
configuration::configuration() : m_label_index(0) {}
configuration::configuration( std::map<std::string,std::string> dictionary ) : m_dictionary(dictionary) {
	m_label_index = 0;
}
//
void configuration::register_variables ( std::string name, bool is_default, std::string type, std::string value, std::string description ) {
	//
	if( type == "STRING" ) value = std::string("\"") + value + std::string("\"");
	if( m_group_stack.empty()) {
		console::dump( "Error: variable \"%s\" without group.\n", name.c_str());
		throw;
	}
	const Title &group_name = m_group_stack.top();
	auto it = m_groups.find(group_name);
	if( it == m_groups.end()) {
		console::dump( "Error: Group \"%s\" not found.\n", name.c_str());
		throw;
	} else {
		auto &entries = it->second.entries;
		if( entries.find(name) == entries.end()) {
			Entry entry;
			entry.is_default = is_default;
			entry.type = type;
			entry.value = value;
			entry.description = description;
			entries[name] = entry;
		}
	}
}
//
void configuration::print_bar( std::string str ) {
	//
	// Remove color encoding
	std::string print_str = str;
	while(true) {
		auto n0 = str.find('<');
		if( n0 == std::string::npos ) break;
		else {
			for( int n1=n0+1; n1 < str.size(); ++n1 ) {
				if( str[n1] == '>' ) {
					str.erase(n0,n1-n0+1);
					break;
				}
			}
		}
	}
	//
	struct winsize size;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
	int pad = str.empty() ? 0 : 2;
	int max_size = std::min(MAX_WIDTH,(int)size.ws_col)-1;
	int L = std::max(3,max_size-(int)(2*pad+str.size()));
	int n = L / 2, m = L - n;
	for( int i=0; i<n; ++i ) console::dump("-");
	for( int i=0; i<pad; ++i ) console::dump(" ");
	console::dump( "%s", print_str.c_str());
	for( int i=0; i<pad; ++i ) console::dump(" ");
	for( int i=0; i<m; ++i ) console::dump("-");
	for( int i=0; i<pad; ++i ) console::dump(" ");
	console::dump("\n");
}
//
void configuration::print_center( std::string str ) {
	//
	struct winsize size;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
	int max_size = std::min(MAX_WIDTH,(int)size.ws_col)-1;
	int L = std::max(3,max_size-(int)(str.size()));
	int n = L / 2, m = L - n;
	for( int i=0; i<n; ++i ) console::dump(" ");
	console::dump( "%s", str.c_str());
	for( int i=0; i<m; ++i ) console::dump(" ");
	console::dump("\n");
}
//
void configuration::print_credit( const Group &group ) const {
	//
	using namespace boost::gregorian;
	std::string date_str = std::get<0>(group.date) ? 
		to_simple_string(date(std::get<0>(group.date),std::get<1>(group.date),std::get<2>(group.date))) : std::string();
	if( ! group.author.empty()) {
		print_center(
			"<Light_Red>(" + 
			group.author + ")" +
			(date_str.empty() ? "" : (" on "+date_str)) + "<Default>"
		);
		if( ! group.address.empty()) print_center("<"+group.address+">");
	}
}
//
void configuration::print_groupbar( const Title &group_title, const Group &group ) const {
	//
	double version = group.version;
	if( version ) {
		char buf[32];
		if( version == (int)version ) std::snprintf(buf,32,"%.1f",version);
		else std::snprintf(buf,32,"%g",version);
		print_bar(group_title.name+" v"+std::string(buf));
	} else {
#if SHOW_ID
		print_bar(group_title.name+" (id:"+std::to_string(group_title.id)+")");
#else
		if( group_title.argument_name.empty() ) {
			print_bar(group_title.name);
		} else {
			print_bar(group_title.name+" (<Green>"+group_title.argument_name+"<Default>)");
		}
#endif
	}
}
//
void configuration::print_variables () const {
	//
	for( auto git=m_groups.cbegin(); git!=m_groups.cend(); ++git ) {
		const auto &entries = git->second.entries;
		if( ! entries.empty()) {
			print_groupbar(git->first,git->second);
			print_credit(git->second);
			for( auto eit=entries.cbegin(); eit!=entries.cend(); ++eit ) {
				std::string name (eit->first);
				auto pos = name.find(".");
				if( pos != std::string::npos ) {
					name = name.substr(pos+1);
				}
				if( ! eit->second.is_default ) {
					console::dump("<Yellow><BlackCircle>  ");
					console::dump( "%s = %s", name.c_str(), eit->second.value.c_str());
					console::dump( "<Default>\n" );
				} else {
					console::dump("   ");
					console::dump( "%s = <Cyan>%s<Default>\n", name.c_str(), eit->second.value.c_str());
				}
			}
		}
	}
	print_bar();
}
//
void configuration::print_help() const {
	//
	for( auto git=m_groups.cbegin(); git!=m_groups.cend(); ++git ) {
		const auto &entries = git->second.entries;
		if( ! entries.empty()) {
			print_groupbar(git->first,git->second);
			print_credit(git->second);
			for( auto eit=entries.cbegin(); eit!=entries.cend(); ++eit ) {
				console::dump( "   %s (<Green>%s,%s<Default>): <Cyan>%s<Default>\n",
					eit->first.c_str(),
					eit->second.type.c_str(),
					eit->second.value.c_str(),
					eit->second.description.c_str());
			}
		}
	}
	print_bar();
}
//
void configuration::print_splash() const {
	//
	print_bar("Shiokaze Core <Light_Magenta>(MIT)<Default>");
	print_center("A research-oriented fluid solver for computer graphics");
	print_center("Designed and maintained by <Yellow>Ryoichi Ando <Cyan><rand@nii.ac.jp><Default>");
	print_bar();
}
//
void configuration::check_touched () const {
	std::lock_guard<std::mutex> guard(config_mutex);
	for( auto ait=m_dictionary.cbegin(); ait!=m_dictionary.cend(); ++ait ) {
		if( ! check_set(ait->first) ) {
			console::dump("<Light_Red>WARNING: Argument \"%s\" was not touched!<Default>\n", ait->first.c_str());
		}
	}
}
//
bool configuration::is_parameter_set ( std::string name ) const {
	return m_dictionary.find(name) != m_dictionary.end();
}
//
bool configuration::check_set ( std::string name ) const {
	bool found (false);
	for( auto git=m_groups.cbegin(); git!=m_groups.cend(); ++git ) {
		const auto &entries = git->second.entries;
		for( auto eit=entries.cbegin(); eit!=entries.cend(); ++eit ) {
			if( eit->first == name || git->first.argument_name + "." + eit->first == name ) {
				found = true;
			}
		}
	}
	return found;
}
//
std::string configuration::get_current_group_name( bool argument_name ) const {
	std::lock_guard<std::mutex> guard(config_mutex);
	if( m_group_stack.empty()) {
		return std::string();
	} else {
		return argument_name ? m_group_stack.top().argument_name : m_group_stack.top().name;
	}
}
//
void configuration::push_group( const credit &info ) {
	push_group(info.get_name(),info.get_argument_name(),info.get_author(),info.get_email_address(),info.get_date(),info.get_version());
}
//
void configuration::push_group(
					std::string name,
					std::string argument_name,
					std::string author,
					std::string address,
					std::tuple<int,int,int> date,
					double version
					 ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	bool found (false);
	Title target_title;
	for( auto git=m_groups.cbegin(); git!=m_groups.cend(); ++git ) {
		if( git->first.name == name && git->first.argument_name == argument_name ) {
			found = true;
			target_title = git->first;
			break;
		}
	}
	if( ! found ) {
		Group group;
		group.author = author;
		group.address = address;
		group.date = date;
		group.version = version;
		target_title.name = name;
		target_title.argument_name = argument_name;
		target_title.id = ++ m_label_index;
		m_groups[target_title] = group;
	}
	//
	m_group_stack.push(target_title);
#if STACK_DEBUG
	for( int n=0; n<m_group_stack.size(); ++n ) console::dump("  ");
	console::dump( "+ %s\n", m_group_stack.top().c_str());
#endif
}
//
void configuration::pop_group() {
	std::lock_guard<std::mutex> guard(config_mutex);
	if( m_group_stack.size()) {
#if STACK_DEBUG
		for( int n=0; n<m_group_stack.size(); ++n ) console::dump("  ");
		console::dump( "- %s\n", m_group_stack.top().name.c_str());
#endif
		m_group_stack.pop();
	} else {
		console::dump("m_group_stack broken!\n");
		throw;
	}
}
//
std::string configuration::concated_name( std::string name ) const {
	assert(m_group_stack.size());
	return m_group_stack.top().argument_name+"."+name;
}
//
bool configuration::get_integer( std::string name, int &value, std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_integer.find(concated_name(name));
	if( it != default_integer.end() ) {
		value = it->second;
	} else {
		it = default_integer.find(name);
		if( it != default_integer.end()) value = it->second;
	}
	//
	int original_value = value;
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%d",&value);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		register_variables(name,true,"INT",std::to_string(value),description);
		return false;
	} else {
		register_variables(name,original_value==value,"INT",std::to_string(value),description);
		return true;
	}
}
//
void configuration::set_integer( std::string name, int value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	m_dictionary[name] = std::to_string(value);
}
//
void configuration::set_default_integer( std::string name, int value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	default_integer[name] = value;
}
//
bool configuration::get_unsigned( std::string name, unsigned &value, std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_unsigned.find(concated_name(name));
	if( it != default_unsigned.end() ) {
		value = it->second;
	} else {
		it = default_unsigned.find(name);
		if( it != default_unsigned.end()) value = it->second;
	}
	//
	unsigned original_value = value;
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%d",&value);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		register_variables(name,true,"UNSIGNED",std::to_string(value),description);
		return false;
	} else {
		register_variables(name,original_value==value,"UNSIGNED",std::to_string(value),description);
		return true;
	}
}
//
void configuration::set_unsigned( std::string name, unsigned value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	m_dictionary[name] = std::to_string(value);
}
//
void configuration::set_default_unsigned( std::string name, unsigned value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	default_unsigned[name] = value;
}
//
bool configuration::get_bool( std::string name, bool &value, std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_bool.find(concated_name(name));
	if( it != default_bool.end() ) {
		value = it->second;
	} else {
		it = default_bool.find(name);
		if( it != default_bool.end()) value = it->second;
	}
	//
	bool original_value = value;
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::string lower_value = m_dictionary.at(name);
			std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
			bool original_value = value;
			if( lower_value == "yes" || m_dictionary.at(name) == "true" ) {
				value = true;
			} else if( lower_value == "no" || m_dictionary.at(name) == "false" ) {
				value = false;
			} else {
				int int_value;
				std::sscanf(m_dictionary.at(name).c_str(),"%d",&int_value);
				value = (bool) int_value;
			}
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		register_variables(name,true,"BOOL",value ? "Yes" : "No",description);
		return false;
	} else {
		register_variables(name,original_value==value,"BOOL",value ? "Yes" : "No",description);
		return true;
	}
}
//
void configuration::set_bool( std::string name, bool value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	m_dictionary[name] = value ? "Yes" : "No";
}
//
void configuration::set_default_bool( std::string name, bool value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	default_bool[name] = value;
}
//
double configuration::get_double( std::string name, double &value, std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_double.find(concated_name(name));
	if( it != default_double.end() ) {
		value = it->second;
	} else {
		it = default_double.find(name);
		if( it != default_double.end()) value = it->second;
	}
	//
	char buf[32]; auto load_buf = [&]() {
		if( value == (int)value ) std::snprintf(buf,32,"%.1f",value);
		else std::snprintf(buf,32,"%g",value);
	};
	double original_value = value;
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%lf",&value);
			load_buf();
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		load_buf();
		register_variables(name,true,"DOUBLE",buf,description);
		return false;
	} else {
		register_variables(name,original_value==value,"DOUBLE",buf,description);
		return true;
	}
}
//
void configuration::set_double( std::string name, double value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	m_dictionary[name] = std::to_string(value);
}
//
void configuration::set_default_double( std::string name, double value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	default_double[name] = value;
}
//
bool configuration::get_real( std::string name, Real &value, std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_real.find(concated_name(name));
	if( it != default_real.end() ) {
		value = it->second;
	} else {
		it = default_real.find(name);
		if( it != default_real.end()) value = it->second;
	}
	char buf[32]; auto load_buf = [&]() {
		if( value == (int)value ) std::snprintf(buf,32,"%.1f",value);
		else std::snprintf(buf,32,"%g",value);
	};
	//
	Real original_value = value;
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			double _value;
			std::sscanf(m_dictionary.at(name).c_str(),"%lf",&_value);
			value = _value;
			load_buf();
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		load_buf();
		register_variables(name,true,"FLOAT",buf,description);
		return false;
	} else {
		register_variables(name,original_value==value,"FLOAT",buf,description);
		return true;
	}
}
//
void configuration::set_real( std::string name, Real value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	m_dictionary[name] = std::to_string(value);
}
//
void configuration::set_default_real( std::string name, Real value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	default_real[name] = value;
}
//
bool configuration::get_vec2i( std::string name, int value[2], std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_vec2i.find(concated_name(name));
	if( it != default_vec2i.end() ) {
		for( int i=0; i<2; ++i ) value[i] = it->second.v[i];
	} else {
		it = default_vec2i.find(name);
		if( it != default_vec2i.end()) {
			for( int i=0; i<2; ++i ) value[i] = it->second.v[i];
		}
	}
	//
	char buf[64];
	int original_value[2] = { value[0], value[1 ]};
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%d,%d",&value[0],&value[1]);
			std::snprintf(buf,64,"%d,%d",value[0],value[1]);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		std::snprintf(buf,64,"%d,%d",value[0],value[1]);
		register_variables(name,true,"VEC2I",buf,description);
		return false;
	} else {
		register_variables(name,
				original_value[0] == value[0] &&
				original_value[1] == value[1]
				,"VEC2I",buf,description);
		return true;
	}
}
//
void configuration::set_vec2i( std::string name, const int value[2] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	char buf[64];
	std::snprintf(buf,64,"%d,%d",value[0],value[1]);
	m_dictionary[name] = std::string(buf);
}
//
void configuration::set_default_vec2i( std::string name, const int value[2] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	int2 e; for( int i=0; i<2; ++i ) e.v[i] = value[i];
	default_vec2i[name] = e;
}
//
bool configuration::get_vec2d( std::string name, double value[2], std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_vec2d.find(concated_name(name));
	if( it != default_vec2d.end() ) {
		for( int i=0; i<2; ++i ) value[i] = it->second.v[i];
	} else {
		it = default_vec2d.find(name);
		if( it != default_vec2d.end()) {
			for( int i=0; i<2; ++i ) value[i] = it->second.v[i];
		}
	}
	//
	char buf[64];
	double original_value[2] = { value[0], value[1] };
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%lf,%lf",&value[0],&value[1]);
			std::snprintf(buf,64,"%g,%g",value[0],value[1]);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		std::snprintf(buf,64,"%g,%g",value[0],value[1]);
		register_variables(name,true,"VEC2D",buf,description);
		return false;
	} else {
		register_variables(name,
				original_value[0] == value[0] &&
				original_value[1] == value[1]
				,"VEC2D",buf,description);
		return true;
	}
}
//
void configuration::set_vec2d( std::string name, const double value[2] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	char buf[64];
	std::snprintf(buf,64,"%g,%g",value[0],value[1]);
	m_dictionary[name] = std::string(buf);
}
//
void configuration::set_default_vec2d( std::string name, const double value[2] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	double2 e;	for( int i=0; i<2; ++i ) e.v[i] = value[i];
	default_vec2d[name] = e;
}
//
bool configuration::get_vec3i( std::string name, int value[3], std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_vec3i.find(concated_name(name));
	if( it != default_vec3i.end() ) {
		for( int i=0; i<3; ++i ) value[i] = it->second.v[i];
	} else {
		it = default_vec3i.find(name);
		if( it != default_vec3i.end()) {
			for( int i=0; i<3; ++i ) value[i] = it->second.v[i];
		}
	}
	//
	int original_value[3];
	original_value[0] = value[0];
	original_value[1] = value[1];
	original_value[2] = value[2];
	char buf[64];
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%d,%d,%d",&value[0],&value[1],&value[2]);
			std::snprintf(buf,64,"%d,%d,%d",value[0],value[1],value[2]);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		std::snprintf(buf,64,"%d,%d,%d",value[0],value[1],value[2]);
		register_variables(name,true,"VEC3I",buf,description);
		return false;
	} else {
		register_variables(name,
				original_value[0] == value[0] &&
				original_value[1] == value[1] &&
				original_value[2] == value[2]
				,"VEC3I",buf,description);
		return true;
	}
}
//
void configuration::set_vec3i( std::string name, const int value[3] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	char buf[64];
	std::snprintf(buf,64,"%d,%d,%d",value[0],value[1],value[2]);
	m_dictionary[name] = std::string(buf);
}
//
void configuration::set_default_vec3i( std::string name, const int value[3] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	int3 e; for( int i=0; i<3; ++i ) e.v[i] = value[i];
	default_vec3i[name] = e;
}
//
bool configuration::get_vec3d( std::string name, double value[3], std::string description) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_vec3d.find(concated_name(name));
	if( it != default_vec3d.end() ) {
		for( int i=0; i<3; ++i ) value[i] = it->second.v[i];
	} else {
		it = default_vec3d.find(name);
		if( it != default_vec3d.end()) {
			for( int i=0; i<3; ++i ) value[i] = it->second.v[i];
		}
	}
	//
	double original_value[3];
	original_value[0] = value[0];
	original_value[1] = value[1];
	original_value[2] = value[2];
	char buf[64];
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%lf,%lf,%lf",&value[0],&value[1],&value[2]);
			std::snprintf(buf,64,"%g,%g,%g",value[0],value[1],value[2]);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		std::snprintf(buf,64,"%g,%g,%g",value[0],value[1],value[2]);
		register_variables(name,true,"VEC3D",buf,description);
		return false;
	} else {
		register_variables(name,
				original_value[0] == value[0] &&
				original_value[1] == value[1] &&
				original_value[2] == value[2]
				,"VEC3D",buf,description);
		return true;
	}
}
//
void configuration::set_vec3d( std::string name, const double value[3] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	char buf[64];
	std::snprintf(buf,64,"%g,%g,%g",value[0],value[1],value[2]);
	m_dictionary[name] = std::string(buf);
}
//
void configuration::set_default_vec3d( std::string name, const double value[3] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	double3 e; for( int i=0; i<3; ++i ) e.v[i] = value[i];
	default_vec3d[name] = e;
}
//
bool configuration::get_vec4d( std::string name, double value[4], std::string description) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_vec4d.find(concated_name(name));
	if( it != default_vec4d.end() ) {
		for( int i=0; i<4; ++i ) value[i] = it->second.v[i];
	} else {
		it = default_vec4d.find(name);
		if( it != default_vec4d.end()) {
			for( int i=0; i<4; ++i ) value[i] = it->second.v[i];
		}
	}
	//
	double original_value[4];
	original_value[0] = value[0];
	original_value[1] = value[1];
	original_value[2] = value[2];
	original_value[3] = value[3];
	char buf[64];
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			std::sscanf(m_dictionary.at(name).c_str(),"%lf,%lf,%lf,%lf",&value[0],&value[1],&value[2],&value[3]);
			std::snprintf(buf,64,"%g,%g,%g,%g",value[0],value[1],value[2],value[3]);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		std::snprintf(buf,64,"%g,%g,%g,%g",value[0],value[1],value[2],value[3]);
		register_variables(name,true,"VEC4D",buf,description);
		return false;
	} else {
		register_variables(name,
				original_value[0] == value[0] &&
				original_value[1] == value[1] &&
				original_value[2] == value[2] &&
				original_value[3] == value[3]
				,"VEC4D",buf,description);
		return true;
	}
}
//
void configuration::set_vec4d( std::string name, const double value[4] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	char buf[64];
	std::snprintf(buf,64,"%g,%g,%g,%g",value[0],value[1],value[2],value[3]);
	m_dictionary[name] = std::string(buf);
}
//
void configuration::set_default_vec4d( std::string name, const double value[4] ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	double4 e; for( int i=0; i<4; ++i ) e.v[i] = value[i];
	default_vec4d[name] = e;
}
//
bool configuration::get_string( std::string name, std::string &value, std::string description ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	//
	auto it = default_string.find(concated_name(name));
	if( it != default_string.end() ) {
		value = it->second;
	} else {
		it = default_string.find(name);
		if( it != default_string.end()) value = it->second;
	}
	//
	std::string original_value = value;
	auto get = [&]( std::string name ) {
		if( m_dictionary.find(name) != m_dictionary.end()) {
			value = m_dictionary.at(name);
			return true;
		}
		return false;
	};
	if( ! get(concated_name(name)) && ! get(name) ) {
		register_variables(name,true,"STRING",value,description);
		return false;
	} else {
		register_variables(name,original_value==value,"STRING",value,description);
		return true;
	}
}
//
void configuration::set_string( std::string name, std::string value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	m_dictionary[name] = value;
}
//
void configuration::set_default_string( std::string name, std::string value ) {
	std::lock_guard<std::mutex> guard(config_mutex);
	default_string[name] = value;
}
//
bool configuration::exist( std::string name ) const {
	std::lock_guard<std::mutex> guard(config_mutex);
	return (m_dictionary.find(name) != m_dictionary.end()) || (m_dictionary.find(concated_name(name)) != m_dictionary.end());
}