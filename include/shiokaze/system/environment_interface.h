/*
**	environment_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 17, 2018.
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
#ifndef SHKZ_ENVIRONMENT_INTERFACE_H
#define SHKZ_ENVIRONMENT_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for retriving system environment.
/// \~japanese @brief システム環境の情報を取得するインターフェース。
class environment_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(environment_interface,"Environment Tools","EnvTool","System Information Interface")
	/**
	 \~english @brief Get the UTC time.
	 @return UTC time string.
	 \~japanese @brief UTC 時間を取得する。
	 @return UTC 時間の文字列。
	 */
	virtual std::string today_UTC() const = 0;
	/**
	 \~english @brief Get the CPU name.
	 @return CPU name string.
	 \~japanese @brief CPU の名前を取得する。
	 @return CPU の名前。
	 */
	virtual std::string cpu_name() const = 0;
	/**
	 \~english @brief Get the version of GCC.
	 @return GCC version string.
	 \~japanese @brief GCC のバージョンを取得する。
	 @return GCC のバージョンの文字列。
	 */
	virtual std::string get_gcc_version() const = 0;
	/**
	 \~english @brief Get the revision of git repository.
	 @return Git revision number.
	 \~japanese @brief Git のリポジトリのリビジョン番号を取得する。
	 @return Git のリビジョン番号。
	 */
	virtual std::string get_git_revnumber() const = 0;
	/**
	 \~english @brief Get the number of available threads.
	 @return Number of available threads.
	 \~japanese @brief 使用可能なスレッドの数を取得する。
	 @return 使用可能なスレッドの数。
	 */
	virtual unsigned get_num_threads() const = 0;
	//
};
//
using environment_ptr = std::unique_ptr<environment_interface>;
//
SHKZ_END_NAMESPACE
//
#endif