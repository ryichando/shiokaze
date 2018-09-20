/*
**	meshexporter3.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 31, 2017. 
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
#include <shiokaze/utility/utility.h>
#include <shiokaze/core/console.h>
#include <shiokaze/core/global_timer.h>
#include <zlib.h>
#include <cstring>
//
SHKZ_USING_NAMESPACE
//
static void append( const void *data, uint size, std::vector<uint8_t> &buffer ) {
	buffer.insert(buffer.end(),(uint8_t *)data,(uint8_t *)data+size);
}
//
static void compress_memory( void *in_data, size_t in_data_size, std::vector<uint8_t> &out_data ) {
	//
	std::vector<uint8_t> buffer;
	const size_t BUFSIZE = 128 * 1024;
	uint8_t temp_buffer[BUFSIZE];
	z_stream strm;
	strm.zalloc = 0;
	strm.zfree = 0;
	strm.next_in = reinterpret_cast<uint8_t *>(in_data);
	strm.avail_in = in_data_size;
	strm.next_out = temp_buffer;
	strm.avail_out = BUFSIZE;
	deflateInit(&strm, Z_BEST_COMPRESSION);
	while (strm.avail_in != 0) {
		int res = deflate(&strm, Z_NO_FLUSH);
		assert(res == Z_OK);
		if (strm.avail_out == 0) {
			buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
	}
	int deflate_res = Z_OK;
	while (deflate_res == Z_OK) {
		if (strm.avail_out == 0) {
			buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE);
			strm.next_out = temp_buffer;
			strm.avail_out = BUFSIZE;
		}
		deflate_res = deflate(&strm, Z_FINISH);
	}
	assert(deflate_res == Z_STREAM_END);
	buffer.insert(buffer.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
	deflateEnd(&strm);
	out_data.swap(buffer);
}
//
class meshexporter3 : public meshexporter3_interface {
public:
	//
	virtual void set_mesh( const std::vector<vec3d> &vertices, const std::vector<std::vector<size_t> > &faces) override {
		this->vertices = vertices;
		this->faces = faces;
	}
	virtual void set_vertex_colors( const std::vector<vec3d> &vertex_colors ) override {
		this->vertex_colors = vertex_colors;
		assert( vertex_colors.size() == vertices.size());
	}
	virtual void set_texture_coordinates( const std::vector<vec2d> &uv_coordinates ) override {
		this->uv_coordinates = uv_coordinates;
		assert( uv_coordinates.size() == vertices.size());
	}
	//
	virtual bool export_ply( std::string path ) override {
		//
		global_timer::pause();
		//
		std::string tmp_path = console::format_str("%s_tmp",path.c_str());
		FILE *ply_fp = fopen(tmp_path.c_str(),"wb");
		if( ! ply_fp ) {
			printf ( "\nCould not open the path (%s)\n", path.c_str());
			exit(0);
			return false;
		}
		// Write header
		fprintf(ply_fp,"ply\n");
		fprintf(ply_fp,"format binary_little_endian 1.0\n");
		fprintf(ply_fp,"element vertex %d\n", (int)vertices.size());
		fprintf(ply_fp,"property float x\n");
		fprintf(ply_fp,"property float y\n");
		fprintf(ply_fp,"property float z\n");
		if( vertex_colors.size()) {
			fprintf(ply_fp,"property uchar red\n");
			fprintf(ply_fp,"property uchar green\n");
			fprintf(ply_fp,"property uchar blue\n");
		}
		if( uv_coordinates.size()) {
			fprintf(ply_fp,"property float s\n");
			fprintf(ply_fp,"property float t\n");
		}
		fprintf(ply_fp,"element face %d\n", (int)faces.size());
		fprintf(ply_fp,"property list uchar int vertex_indices\n");
		fprintf(ply_fp,"end_header\n");
		fflush(ply_fp);
		fclose(ply_fp);
		//
		// Write vertices
		ply_fp = fopen(tmp_path.c_str(),"ab");
		if( ! ply_fp ) {
			printf ( "Could not open the path (%s)\n", path.c_str());
			return false;
		}
		for( uint n=0; n<vertices.size(); n++ ) {
			float v[3] = { (float)vertices[n][0], (float)vertices[n][1], (float)vertices[n][2] };
			fwrite(v,3,sizeof(float),ply_fp);
			if( vertex_colors.size()) {
				unsigned char colors[3] = {
					(unsigned char)(254.0*std::min(1.0,std::max(0.0,vertex_colors[n][0]))),
					(unsigned char)(254.0*std::min(1.0,std::max(0.0,vertex_colors[n][1]))),
					(unsigned char)(254.0*std::min(1.0,std::max(0.0,vertex_colors[n][2]))) };
				fwrite(colors,3,sizeof(unsigned char),ply_fp);
			}
			if( uv_coordinates.size()) {
				float coord[2] = { (float)uv_coordinates[n][0], (float)uv_coordinates[n][1] };
				fwrite(coord,2,sizeof(float),ply_fp);
			}
		}
		// Write faces
		for( uint n=0; n<faces.size(); n++ ) {
			unsigned char num = faces[n].size();
			fwrite( &num,1,sizeof(unsigned char),ply_fp);
			for( unsigned m=0; m<faces[n].size(); m++ ) {
				fwrite(&faces[n][faces[n].size()-1-m],1,sizeof(int),ply_fp);
			}
		}
		fflush(ply_fp);
		fclose(ply_fp);
		console::system("mv %s %s", tmp_path.c_str(), path.c_str());
		//
		global_timer::resume();
		return true;
	}
	virtual bool export_mitsuba( std::string path ) override {
		//
		global_timer::pause();
		//
		std::string tmp_path = console::format_str("%s_tmp",path.c_str());
		FILE *serialized_fp = fopen(tmp_path.c_str(),"wb");
		if( ! serialized_fp ) {
			printf ("\nCould not open the path (%s)\n",path.c_str());
			exit(0);
			return false;
		}
		//
		uint16_t format = 0x041C;
		uint16_t version = 0x0004;
		fwrite(&format,1,sizeof(uint16_t),serialized_fp);
		fwrite(&version,1,sizeof(uint16_t),serialized_fp);
		//
		std::vector<uint8_t> buffer;
		unsigned bit = 0x2000;
		if( vertex_colors.size()) {
			bit = bit | 0x0008;
		}
		if( uv_coordinates.size()) {
			bit = bit | 0x0002;
		}
		append(&bit,sizeof(unsigned),buffer);
		const char *name = "mesh";
		append(name, strlen(name)+1, buffer);
		uint64_t v_number = vertices.size();
		uint64_t f_number = faces[0].size() == 3 ? faces.size() : 2*faces.size();
		append(&v_number,sizeof(uint64_t),buffer);
		append(&f_number,sizeof(uint64_t),buffer);
		for( unsigned n=0; n<vertices.size(); n++ ) {
			double v[3] = { vertices[n][0], vertices[n][1], vertices[n][2] };
			append(&v[0],sizeof(double),buffer);
			append(&v[1],sizeof(double),buffer);
			append(&v[2],sizeof(double),buffer);
		}
		if( uv_coordinates.size()) {
			for( unsigned n=0; n<uv_coordinates.size(); n++ ) {
				append(&uv_coordinates[n][0],sizeof(double),buffer);
				append(&uv_coordinates[n][1],sizeof(double),buffer);
			}
		}
		if( vertex_colors.size()) {
			for( unsigned n=0; n<vertex_colors.size(); n++ ) {
				double v[3] = { vertex_colors[n][0], vertex_colors[n][1], vertex_colors[n][2] };
				append(&v[0],sizeof(double),buffer);
				append(&v[1],sizeof(double),buffer);
				append(&v[2],sizeof(double),buffer);
			}
		}
		for( unsigned n=0; n<faces.size(); n++ ) {
			const std::vector<size_t> &face = faces[n];
			if( face.size() == 3 ) {
				for( unsigned m=0; m<face.size(); m++ ) {
					unsigned f = (unsigned)face[face.size()-m-1];
					append(&f,sizeof(unsigned),buffer);
				}
			} else if( face.size() == 4 ) {
				// Triangulate
				size_t f[4] = { faces[n][0], faces[n][1], faces[n][2], faces[n][3] };
				unsigned fs[4] = { (unsigned)f[0], (unsigned)f[1], (unsigned)f[2], (unsigned)f[3] };
				append(&fs[2],sizeof(unsigned),buffer);
				append(&fs[1],sizeof(unsigned),buffer);
				append(&fs[0],sizeof(unsigned),buffer);
				append(&fs[3],sizeof(unsigned),buffer);
				append(&fs[2],sizeof(unsigned),buffer);
				append(&fs[0],sizeof(unsigned),buffer);
			}
		}
		std::vector<uint8_t> out_data;
		compress_memory(&buffer[0], buffer.size(), out_data);
		fwrite(&out_data[0],1,out_data.size(),serialized_fp);
		//
		uint64_t first_offset = 0;
		uint64_t second_offset = 0;
		unsigned total_mesh = 1;
		fwrite(&first_offset,1,sizeof(uint64_t),serialized_fp);
		fwrite(&second_offset,1,sizeof(uint64_t),serialized_fp);
		fwrite(&total_mesh,1,sizeof(unsigned),serialized_fp);
		//
		fflush(serialized_fp);
		fclose(serialized_fp);
		console::system("mv %s %s", tmp_path.c_str(), path.c_str());
		//
		global_timer::resume();
		return true;
	}
	//
protected:
	//
	std::vector<vec3d> vertices;
	std::vector<vec3d> vertex_colors;
	std::vector<vec2d> uv_coordinates;
	std::vector<std::vector<size_t> > faces;
};
//
extern "C" module * create_instance() {
	return new meshexporter3();
}
//
extern "C" const char *license() {
	return "MIT";
}
//