/*
**	hacd_io.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on May 1, 2017.
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
#ifndef SHKZ_HACD_IO_H
#define SHKZ_HACD_IO_H
//
#include <shiokaze/math/vec.h>
#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that enables reading and writing convex hull objects computed by HACD.
/// \~japanese @brief HACD で計算されたコンベックスハルオブジェクトの読み書きを行うクラス。
class hacd_io {
public:
	/// \~english @brief Structure that holds information about a convex hull object.
	/// \~japanese @brief コンベックスハルのオブジェクトを保持する構造体。
	struct convex_object {
		//
		/// \~english @brief Vertices of the polygon.
		/// \~japanese @brief ポリゴンの頂点列。
		std::vector<vec3d> vertices;
		//
		/// \~english @brief Faces of the polygon.
		/// \~japanese @brief ポリゴンの面。
		std::vector<std::vector<size_t> > faces;
	};
	/**
	 \~english @brief Read convex hulls.
	 @param[in] path Path to the file.
	 \~japanese @brief コンベックスハルを読み込む。
	 @param[in] path ファイルへのパス。
	 */
	static std::vector<hacd_io::convex_object> read_hacd ( std::string path ) {
		//
		FILE *fp = fopen(path.c_str(),"rb");
		assert(fp);
		//
		unsigned version;
		assert(fread(&version,1,sizeof(unsigned),fp));
		assert( version == 0x0001 );
		//
		unsigned num_objects;
		assert(fread(&num_objects,1,sizeof(unsigned),fp));
		std::vector<hacd_io::convex_object> objects;
		objects.resize(num_objects);
		//
		for( unsigned n=0; n<num_objects; ++n ) {
		//
			auto &obj = objects[n];
			auto &vertices = obj.vertices;
			auto &faces = obj.faces;
			//
			unsigned num_vertices;
			unsigned num_faces;
			//
			assert(fread(&num_vertices,1,sizeof(unsigned),fp));
			assert(fread(&num_faces,1,sizeof(unsigned),fp));
			//
			vertices.resize(num_vertices);
			faces.resize(num_faces);
			//
			for( size_t k=0; k<num_vertices; k++ ) {
				assert(fread(&vertices[k],1,sizeof(vec3d),fp));
			}
			//
			for( size_t k=0; k<num_faces; k++ ) {
				unsigned size;
				assert(fread(&size,1,sizeof(unsigned),fp));
				faces[k].resize(size);
				for( int f=0; f<size; ++f ) {
					unsigned v;
					assert(fread(&v,1,sizeof(unsigned),fp));
					faces[k][f] = v;
				}
			}
		}
		//
		fclose(fp);
		return objects;
	}
	/**
	 \~english @brief Write convex hulls.
	 @param[in] path Path to the file.
	 \~japanese @brief コンベックスハルを書き出す。
	 @param[in] path ファイルへのパス。
	 */
	static void write_hacd ( std::string path, const std::vector<hacd_io::convex_object> &objects ) {
		//
		FILE *fp = fopen(path.c_str(),"wb");
		assert(fp);
		//
		unsigned version (0x0001);
		fwrite(&version,1,sizeof(unsigned),fp);
		//
		unsigned num_objects (objects.size());
		fwrite(&num_objects,1,sizeof(unsigned),fp);
		//
		for( unsigned n=0; n<objects.size(); ++n ) {
		//
			const auto &obj = objects[n];
			const auto &vertices = obj.vertices;
			const auto &faces = obj.faces;
			//
			unsigned num_vertices = vertices.size();
			unsigned num_faces = faces.size();
			//
			fwrite(&num_vertices,1,sizeof(unsigned),fp);
			fwrite(&num_faces,1,sizeof(unsigned),fp);
			//
			for( size_t k=0; k<vertices.size(); k++ ) {
				fwrite(&vertices[k],1,sizeof(vec3d),fp);
			}
			//
			for( size_t k=0; k<faces.size(); k++ ) {
				unsigned size = faces[k].size();
				fwrite(&size,1,sizeof(unsigned),fp);
				for( int f=0; f<size; ++f ) {
					unsigned v (faces[k][f]);
					fwrite(&v,1,sizeof(unsigned),fp);
				}
			}
		}
		//
		fclose(fp);
	}
};
//
SHKZ_END_NAMESPACE
//
#endif