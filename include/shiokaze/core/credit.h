/*
**	credit.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Aug 29, 2017.
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
#ifndef SHKZ_CREDIT_H
#define SHKZ_CREDIT_H
//
#include <shiokaze/core/common.h>
#include <string>
#include <tuple>
//
SHKZ_BEGIN_NAMESPACE
//
/// \~english @brief Macro that defines the full name.
/// \~japanese @brief 完全な名前を定義するマクロ。
#define LONG_NAME(long_name)			virtual std::string get_name() const override { return credit::m_name.empty() ? long_name : credit::m_name; }
/// \~english @brief Macro that defines the argument name.
/// \~japanese @brief 引数の名前を定義するマクロ。
#define ARGUMENT_NAME(argument_name)	virtual std::string get_argument_name() const override { return credit::m_argument_name.empty() ? argument_name : credit::m_argument_name; }
/// \~english @brief Macro that defines the author's name.
/// \~japanese @brief 作者の名前を定義するマクロ。
#define AUTHOR_NAME(author_name)	virtual std::string get_author() const override { return author_name; }
//
/// \~english @brief Class that defines the name, argument name, author's name, email address, date and the version of the code.
/// \~japanese @brief 名前、引数名、作者の名前、メールアドレス、日付、コードのバージョンなどを定義するクラス。
class credit {
public:
	/**
	 \~english @brief Default constructor.
	 \~japanese @brief デフォルトコンストラクタ。
	 */
	credit() = default;
	/**
	 \~english @brief Constructor for credit.
	 @param[in] name Name.
	 @param[in] argument_name Name for the argument (alternatively, space-less short name).
	 \~japanese @brief credit のコンストラクタ。
	 @param[in] name 名前。
	 @param[in] argument_name 引数の名前。(あるいは、スペースの無い短い名前)
	 */
	credit( std::string name, std::string argument_name ) : m_name(name), m_argument_name(argument_name) {}
	/**
	 \~english @brief Set name (and perhaps, argument name together)
	 @param[in] name Name to set.
	 @param[in] argument_name Name for the argument to set (can be omitted).
	 \~japanese @brief 引数の名前を設定する。
	 @param[in] name 設定する名前。
	 @param[in] argument_name 設定する引数の名前 (省略可)。
	 */
	virtual void set_name( std::string name, std::string argument_name="" ) { 
		m_name = name;
		if( ! argument_name.empty()) m_argument_name = argument_name;
	}
	/**
	 \~english @brief Get the name.
	 \~japanese @brief 名前を得る。
	 */
	virtual std::string get_name() const { return m_name.empty() ? "Unknown" : m_name; }
	/**
	 \~english @brief Set an argument name.
	 @param[in] argument_name Name for the argument to set.
	 \~japanese @brief 引数の名前を設定する。
	 @param[in] argument_name 設定する引数の名前。
	 */
	virtual void set_argument_name( std::string argument_name ) { m_argument_name = argument_name; }
	/**
	 \~english @brief Get an argument name.
	 @return Currently set argument name.
	 \~japanese @brief 引数の名前を得る。
	 @return 現在設定されている引数の名前。
	 */
	virtual std::string get_argument_name() const { return m_argument_name; }
	/**
	 \~english @brief Get version.
	 @return Currently set version.
	 \~japanese @brief バージョンを取得する。
	 @return 現在設定されているバージョン。
	 */
	virtual double get_version() const { return 0.0; }
	/**
	 \~english @brief Get author's name.
	 @return Currently set author's name.
	 \~japanese @brief 作者の名前を取得する。
	 @return 現在設定されている作者の名前。
	 */
	virtual std::string get_author() const { return ""; }
	/**
	 \~english @brief Get currently set date.
	 @return Currently set date.
	 \~japanese @brief 日付を取得する。
	 @return 現在設定されている日付。
	 */
	virtual std::tuple<int,int,int> get_date() const { return std::make_tuple(0,0,0); }
	/**
	 \~english @brief Get email address.
	 @return Currently set email address.
	 \~japanese @brief メールアドレスを取得する。
	 @return 現在設定されているメールアドレス。
	 */
	virtual std::string get_email_address() const { return ""; }
	//
protected:
	/**
	 \~english @brief Name of credit.
	 \~japanese @brief credit の名前。
	 */
	std::string m_name;
	/**
	 \~english @brief Argument name.
	 \~japanese @brief 変数名。
	 */
	std::string m_argument_name;
};
//
SHKZ_END_NAMESPACE
//
#endif