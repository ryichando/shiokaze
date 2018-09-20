/*
**	cmdparser.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Feb 1, 2017. 
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
#ifndef SHKZ_CMDPARSER_H
#define SHKZ_CMDPARSER_H
//
#include <shiokaze/core/configuration.h>
#include <algorithm>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that parses command line arguments.
/// \~japanese @brief コマンドラインの変数の読み込みと解析を行うクラス。
class cmdparser : public configuration {
public:
	/**
	 \~english @brief Default constructor.
	 \~japanese @brief デフォルトコンストラクタ。
	 */
	cmdparser() {}
	/**
	 \~english @brief Constructor for cmdparser.
	 @param[in] argc Number of arguments.
	 @param[in] argv List of arguments.
	 \~japanese @brief cmdparser のコンストラクタ。
	 @param[in] argc 変数の数。
	 @param[in] argv 変数リスト。
	 */
	cmdparser( int argc, const char * argv[] ) {
		parse(argc,argv);
	}
	/**
	 \~english @brief Copy constructor for cmdparser.
	 @param[in] parser A reference to an instance of cmdparser to copy.
	 \~japanese @brief cmdparser のコピーコンストラクタ。
	 @param[in] parser コピーする cmdparser のインスタンスへの参照。
	 */
	cmdparser( const cmdparser &parser ) {
		parse(parser.get_dictionary());
	}
	/**
	 \~english @brief Constructor for cmdparser.
	 @param[in] dictionary A set of arguments.
	 \~japanese @brief cmdparser のコンストラクタ。
	 @param[in] dictionary 変数のセット。
	 */
	cmdparser( std::map<std::string,std::string> dictionary ) {
		parse(dictionary);
	}
	/**
	 \~english @brief Parse dictionary and set arguments.
	 @param[in] dictionary A set of arguments.
	 \~japanese @brief 辞書を解析し、変数を設定する。
	 @param[in] dictionary 変数のセット。
	 */
	void parse( std::map<std::string,std::string> dictionary ) {
		for( auto it=dictionary.begin(); it!=dictionary.end(); ++it ) {
			set_string(it->first,it->second);
		}
		rebuild_arg_str();
	}
	/**
	 \~english @brief Analyze arguments and set from the arguments called from main function.
	 @param[in] argc Number of arguments.
	 @param[in] argv List of arguments.
	 \~japanese @brief main 関数から呼ばれる引数を元に変数を解析し、設定する。
	 @param[in] argc 変数の数。
	 @param[in] argv 変数リスト。
	 */
	void parse ( int argc, const char * argv[] ) {
		for( unsigned i=0; i<argc; i++ ) {
			std::string str(argv[i]);
			for( unsigned k=0; k<str.size(); k++ ) {
				if( str.at(k) == '=' ) {
					std::string name(str.begin(),str.begin()+k);
					std::string value(str.begin()+k+1,str.end());
					value.erase(std::remove(value.begin(), value.end(), '\\'), value.end());
					set_string(name,value);
				}
			}
			m_arg_str += str;
			if( i != argc-1 ) m_arg_str += " ";
		}
		rebuild_arg_str();
	}
	/**
	 \~english @brief Parse dictionary and set arguments.
	 \~japanese @brief 辞書を解析し、変数を設定する。
	 */
	void rebuild_arg_str() {
		m_arg_str = std::string();
		const auto &dictionary = get_dictionary();
		for( auto it=dictionary.crbegin(); it!=dictionary.crend(); ++it ) {
			m_arg_str += it->first + "=" + it->second + " ";
		}
	}
	/**
	 \~english @brief Get the organized string of arguments.
	 @return Organized string of arguments.
	 \~japanese @brief 変数をまとめた文字列を取得する。
	 @return 変数をまとめた文字列。
	 */
	std::string get_arg_string() const {
		return m_arg_str;
	}
	//
private:
	std::string m_arg_str;
};
//
SHKZ_END_NAMESPACE
//
#endif