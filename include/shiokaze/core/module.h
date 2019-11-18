/*
**	module.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 3, 2017. 
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
/** @file */
//
#ifndef SHKZ_MODULE_H
#define SHKZ_MODULE_H
//
#include <shiokaze/core/credit.h>
#include <shiokaze/core/configuration.h>
#include <iostream>
#include <memory>
#include <cassert>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Macro that simplifies the use of quick_alloc_module.
/// \~japanese @brief quick_alloc_module を簡単に使うための定義。
#define DEFINE_QUICK_ALLOC_MODULE(CLASS_T,ARG_NAME,DESCRIPTION) \
	ARGUMENT_NAME(ARG_NAME) \
	static std::unique_ptr<CLASS_T> quick_alloc_module( configuration &config, std::string name ) { \
		return unique_alloc_module<CLASS_T>(config,ARG_NAME,name,DESCRIPTION); \
	}
//
/// \~english @brief Module class.
/// \~japanese @brief モジュールクラス。
class module : public credit {
public:
	/**
	 \~english @brief Default constructor for module.
	 \~japanese @brief module のデフォルトコンストラクタ。
	 */
	module ();
	/**
	 \~english @brief Default destructor for module.
	 \~japanese @brief モジュールのデフォルトデストラクタ。
	 */
	virtual ~module();
	/**
	 \~english @brief Send a message to the core module.
	 @param[in] message Message
	 @param[in] ptr Pointer to some value.
	 \~japanese @brief コアモジュールにメッセージを送る
	 @param[in] message メッセージ
	 @param[in] ptr あるポインターの値
	 */
	virtual void send_message( unsigned message, void *ptr ) {}
	/**
	 \~english @brief Send a message to the core module.
	 @param[in] message Message
	 @param[in] ptr Pointer to some value.
	 \~japanese @brief コアモジュールにメッセージを送る
	 @param[in] message メッセージ
	 @param[in] ptr あるポインターの値
	 */
	virtual void send_message( unsigned message, void *ptr ) const {}
	/**
	 \~english @brief Get the module name
	 @return module name
	 \~japanese @brief モジュールの名前を得る
	 @return モジュールの名前
	 */
	virtual std::string get_module_name() const { return std::string(); }
	/**
	 \~english @brief Get the path to the dynamic library. e.g., "mylib" -> "symlink-public/lib/libshiokaze_mylib.dylib"
	 @param[in] module_name Name of module.
	 \~japanese @brief 動的ライブラリへのパスを取得する (例: "mylib" -> "symlink-public/lib/libshiokaze_mylib.dylib")
	 @param[in] module_name モジュールの名前。
	 */
	static std::string module_libpath( std::string module_name );
	/**
	 \~english @brief Automatically reads the parameter "arg_name" to fetch the name for the library, and allocate the library.
	 @param[in] config Configutation setting.
	 @param[in] arg_name Argument corresponding argument key for the library.
	 @param[in] default_module_name Default module name in the case that module associated with arg_name key is not found.
	 @param[om] description Description of parameter arg_name.
	 @return Pointer to the loaded dynamic library.
	 \~japanese @brief パラメータ "arg_name" を読み込んで、その名前のライブラリを探し読み込む。
	 @param[in] config 設定ファイル。
	 @param[in] arg_name ライブラリの名前に対応する変数。
	 @param[in] default_module_name arg_name のライブラリが見つからなかった時のデフォルトモジュール名。
	 @param[om] description パラメータ arg_name に関する説明。
	 @return 読み込んだ動的ライブラリへのポインター。
	 */
	static module * alloc_module( configuration &config, std::string arg_name, std::string default_module_name, std::string description );
	/**
	 \~english @brief Reads the dynamic library of a class module, allocate, and return the pointer to it
	 @param[in] path Path to the dynamic library
	 @return Pointer to the loaded dynamic library.
	 \~japanese @brief module クラスの動的ライブラリを読み込み、そのポインターを返す関数。
	 @param[in] path 動的ライブラリへのパス。
	 @return 読み込んだ動的ライブラリへのポインター。
	 */
	static module * alloc_module( std::string path );
	/**
	 \~english @brief Close all the handles that are still unloaded.
	 @return Number of closed handles by this call.
	 \~japanese @brief まだアンロードされていない全ての動的ライブラリへのハンドルを閉じる。
	 @return このルーチンの呼び出しで閉じられたハンドルの数。
	 */
	static unsigned close_all_handles ();
	/**
	 \~english @brief Allocate the module and cast to the a specified class T.
	 @param[in] config Configutation setting.
	 @param[in] arg_name Argument corresponding argument key for the library.
	 @param[in] default_module_name Default module name in the case that module associated with arg_name key is not found.
	 @param[om] description Description of parameter arg_name.
	 @return Pointer to the loaded dynamic library.
	 \~japanese @brief モジュールを読み込み、特定されたクラス T にキャストする。
	 @param[in] config 設定ファイル。
	 @param[in] arg_name ライブラリの名前に対応する変数。
	 @param[in] default_module_name arg_name のライブラリが見つからなかった時のデフォルトモジュール名。
	 @param[om] description パラメータ arg_name に関する説明。
	 @return 読み込んだ動的ライブラリへのポインター。
	 */
	template <class T> static std::unique_ptr<T> unique_alloc_module( configuration &config, std::string arg_name, std::string default_module_name, std::string description ) {
		auto ptr = dynamic_cast<T*>(alloc_module(config,arg_name,default_module_name,description));
		assert(ptr);
		return std::unique_ptr<T>(ptr);
	}
	/**
	 \~english @brief Count all the open modules.
	 @return Number of load modules.
	 \~japanese @brief 読み込まれた全てのモジュールの数を得る。
	 @return 読み込まれたモジュールの数。
	 */
	static unsigned count_open_modules();
	//
private:
	std::string m_path;
	module( const module & ) = delete;
	module& operator=( const module & ) = delete;
};
//
/******** Your binary should include the following function **********
//
extern "C" module * create_instance();
//
**********************************************************************/
//
SHKZ_END_NAMESPACE
//
#endif