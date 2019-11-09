/*
**	graphplotter.cpp
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 2, 2019.
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
#include <shiokaze/utility/graphplotter_interface.h>
#include <shiokaze/math/vec.h>
#include <shiokaze/image/color.h>
#include <utility>
#include <vector>
#include <map>
#include <cstring>
//
SHKZ_USING_NAMESPACE
//
class graphplotter : public graphplotter_interface {
public:
	graphplotter () {
		//
		m_param.width_per_time = 0.25;
		m_param.padding = 0.1;
		m_param.bottom = 0.7;
		m_param.hue_stride = 65.0;
		m_param.saturation = 0.8;
		m_param.brightness = 1.0;
		m_param.frame_color[0] = m_param.frame_color[1] = m_param.frame_color[2] = 1.0;
		m_param.frame_color[3] = 1.0;
	}
private:
	//
	MODULE_NAME("graphplotter")
	//
	virtual void clear() override {
		//
		m_entries.clear();
		m_id_head = 0;
		m_unit_value = 0.0;
	}
	//
	virtual unsigned create_entry( std::string name ) override {
		//
		const unsigned id = ++ m_id_head;
		const color::hsv hsv_color = { (id-1) * m_param.hue_stride, m_param.saturation, m_param.brightness };
		const color::rgb rgb_color = color::hsv2rgb(hsv_color);
		//
		Entry entry;
		entry.name = name;
		entry.color[0] = rgb_color.r;
		entry.color[1] = rgb_color.g;
		entry.color[2] = rgb_color.b;
		entry.color[3] = 1.0;
		//
		m_entries[id] = entry;
		return id;
	}
	//
	virtual void delete_entry( unsigned id ) override {
		//
		const auto it = m_entries.find(id);
		if( it != m_entries.end()) {
			m_entries.erase(it);
		}
	}
	//
	virtual void set_unit_number( double value ) override {
		m_unit_value = value;
	}
	//
	virtual void add_point( unsigned id, double time, double number ) override {
		//
		assert( id > 0 && id <= m_id_head);
		m_entries[id].data.push_back({time,number});
		if( ! m_unit_value ) m_unit_value = number;
	}
	//
	virtual void set_attribute( unsigned id, std::string name, const void *attribute ) override {
		//
		assert( id > 0 && id <= m_id_head);
		if( name == "color" ) {
			std::memcpy(m_entries[id].color,attribute,4*sizeof(double));
		}
	}
	//
	virtual const void* get_attribute( unsigned id, std::string name ) const override {
		assert( id > 0 && id <= m_id_head);
		if( name == "color" ) {
			return m_entries.at(id).color;
		}
		return nullptr;
	}
	//
	virtual void draw( graphics_engine &g ) const override {
		//
		if( m_entries.empty()) return;
		//
		g.set_2D_coordinate(0.0,1.0,0,1.0);
		g.color4v(m_param.frame_color);
		g.begin(graphics_engine::MODE::LINE_LOOP);
		g.vertex2(m_param.padding,m_param.bottom);
		g.vertex2(m_param.padding,1.0-m_param.padding);
		g.vertex2(1.0-m_param.padding,1.0-m_param.padding);
		g.vertex2(1.0-m_param.padding,m_param.bottom);
		g.end();
		//
		unsigned x, y, window_width, window_height;
		g.get_viewport(x,y,window_width,window_height);
		const double y_stride (30.0 / window_height);
		const double height ((1.0-m_param.padding)-m_param.bottom);
		const double right_edge (1.0-m_param.padding);
		//
		unsigned id (0);
		for( const auto &entry : m_entries ) {
			++ id;
			//
			g.color4v(entry.second.color);
			g.draw_string(vec2d(m_param.padding,m_param.bottom-id*y_stride).v,entry.second.name.c_str());
			//
			g.begin(graphics_engine::MODE::LINES);
			vec2d prev_pos;
			for( const auto &p : entry.second.data ) {
				const double x (m_param.padding + p.first*m_param.width_per_time);
				const double y (m_param.bottom + 0.5*height*p.second/m_unit_value);
				if( x < right_edge ) {
					if( ! prev_pos.norm2()) prev_pos = vec2d(x,y);
					g.vertex2v(prev_pos.v);
					g.vertex2(x,y);
					prev_pos = vec2d(x,y);
				}
			}
			g.end();
		}
	}
	//
	struct Entry {
		std::vector<std::pair<double,double> > data;
		std::string name;
		double color[4];
	};
	//
	struct Parameters {
		double padding;
		double bottom;
		double width_per_time;
		double hue_stride;
		double saturation;
		double brightness;
		double frame_color[4];
	};
	//
	Parameters m_param;
	std::map<unsigned,Entry> m_entries;
	unsigned m_id_head;
	double m_unit_value;
};
//
extern "C" module * create_instance() {
	return new graphplotter();
}
//
extern "C" const char *license() {
	return "MIT";
}