/*
**	dylibloader.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on June 20, 2017.
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
#ifndef SHKZ_DYLIBLOADER_H
#define SHKZ_DYLIBLOADER_H
//
#include <shiokaze/core/configurable.h>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that takes in charge of loading dynamic libraries.
/// \~japanese @brief 動的ライブラリの読み込みを行うクラス。
class dylibloader : public configurable {
public:
	/**
	 \~english @brief Default constructor for dylibloader.
	 \~japanese @brief dylibloader のでデフォルトコンストラクタ。
	 */
	dylibloader();
	/**
	 \~english @brief Default destructor for dylibloader. close_library() will be also called if necessary.
	 \~japanese @brief dylibloader のでデフォルトデストラクタ。必要なら内部で close_library() も呼ばれる。
	 */
	virtual ~dylibloader();
	/**
	 \~english @brief Load dynamic libraries.
	 @param[in] path Path to the dynamic library.
	 \~japanese @brief 動的ライブラリを読み込む。
	 @param[in] path 動的ライブラリへのパス。
	 */
	bool open_library( std::string path );
	/**
	 \~english @brief Unload dynamic library.
	 \~japanese @brief 読み込んだ動的ライブラリをアンロードする。
	 */
	void close_library();
	/**
	 \~english @brief Get the native pointer to the loaded dynamic library.
	 @return Pointer to the loaded library.
	 \~japanese @brief 動的ライブラリへのネイティブなポインターを得る。
	 @return 読み込まれたライブラリへのポインタ。
	 */
	const void *get_handle() const { return m_handle; }
	/**
	 \~english @brief Load function of global variable.
	 @return Pointer to the loaded function or global variable.
	 \~japanese @brief 関数あるいはグローバル変数を読み込む。
	 @return 読み込まれた関数あるいはグローバル変数へのポインタ。
	 */
	void* load_symbol( std::string name ) const;
	/**
	 \~english @brief Call the function symbol "load".
	 @param[in] config Configuration setting.
	 \~japanese @brief 関数シンボル "load" を呼ぶ。
	 @param[in] config 設定。
	 */
	void load( configuration &config ) override;
	/**
	 \~english @brief Call the function symbol "configure".
	 @param[in] config Configuration setting.
	 \~japanese @brief 関数シンボル "configure" を呼ぶ。
	 @param[in] config 設定。
	 */
	void configure( configuration &config ) override;
	/**
	 \~english @brief Call the function symbol "overwrite", expecting that some configuration will be overwritten.
	 @param[in] config Configuration setting.
	 \~japanese @brief 関数シンボル "overwrite" を呼ぶ。その中で、いくつかの設定は上書きされることを想定。
	 @param[in] config 設定。
	 */
	void overwrite( configuration &config ) const;
	//
private:
	void *m_handle;
	std::string m_path;
};
//
SHKZ_END_NAMESPACE
//
#endif