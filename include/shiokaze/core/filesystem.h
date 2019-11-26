/*
**	filesystem.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 6, 2017.
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
#ifndef SHKZ_FILESYSTEM_H
#define SHKZ_FILESYSTEM_H
//
#include <shiokaze/core/common.h>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that perform file system related tasks.
/// \~japanese @brief ファイルシステム関連の処理を行うクラス。
class filesystem {
public:
	/**
	 \~english @brief Check if the file to a path exists
	 @param[in] path Path to check.
	 @return \c true if exists \c false otherwise.
	 \~japanese @brief パスで指定されたファイルが存在するか取得する。
	 @param[in] path 確認するパス。
	 @return 存在すれば \c true 存在していなければ \c false が返る。
	 */
	static bool is_exist( std::string path );
	/**
	 \~english @brief Create directory to a path.
	 @return \c true if successful \c false otherwise.
	 \~japanese @brief ディレクトリを指定されたパスに作成する。
	 @return 成功すれば \c true 失敗すれば \c false が返る。
	 */
	static bool create_directory( std::string path );
	/**
	 \~english @brief Delete the file to a path.
	 @param[in] path Path to the file to delete.
	 \~japanese @brief パスで指定されたファイルを削除する。
	 @param[in] path 削除するファイルへのパス。
	 */
	static void remove_file( std::string path );
	/**
	 \~english @brief Delete all the files in a directory.
	 @param[in] path Path to the directory.
	 \~japanese @brief パスで指定されたディレクトリの中身を全て削除する。
	 @param[in] path ディレクトリのパス。
	 */
	static void remove_dir_contents( std::string path );
	/**
	 \~english @brief Get a file path in the resource directory.
	 @param[in] directory Name of the directory
	 @param[in] name Name of the file.
	 \~japanese @brief リソースディレクトリからファイルのパスを取得する。
	 @param[in] directory ディレクトリの名前。
	 @param[in] name ファイルの名前。
	 */
	static std::string find_resource_path( std::string directory, std::string name );
	/**
	 \~english @brief Get a file path in the dynamic library directory.
	 @param[in] name Name of the dynamic library without "lib" prefix and ".so" suffix.
	 \~japanese @brief ライブラリのパスを取得する。
	 @param[in] name "lib" プレフィックスと ".so" サフィックスを除いたライブラリの名前。
	 */
	static std::string resolve_libname( std::string name );
};
//
SHKZ_END_NAMESPACE
//
#endif