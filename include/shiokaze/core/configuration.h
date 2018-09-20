/*
**	configuration.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on July 19, 2017. 
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
#ifndef SHKZ_CONFIGURATION_H
#define SHKZ_CONFIGURATION_H
//
#include <string>
#include <map>
#include <stack>
#include <shiokaze/core/common.h>
#include <shiokaze/core/credit.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief  Class that controls the settings of the program.
/// \~japanese @brief プログラムの設定などを管理するクラス。
class configuration {
public:
	/**
	 \~english @brief Default constructor for configuration.
	 \~japanese @brief configuration のデフォルトコンストラクタ。
	 */
	configuration();
	/**
	 \~english @brief Constructor for configuration.
	 @param[in] dictionary List of parameters.
	 \~japanese @brief configuration のコンストラクタ。
	 @param[in] dictionary パラメータのリスト。
	 */
	configuration( std::map<std::string,std::string> dictionary );
	/**
	 \~english @brief Print the currently set parameters.
	 \~japanese @brief 現在設定されているパラメータを出力する。
	 */
	void print_variables () const;
	/**
	 \~english @brief Print a help manual of the currently set parameters.
	 \~japanese @brief 現在設定されているパラメータのヘルプマニュアルを出力する。
	 */
	void print_help() const;
	/**
	 \~english @brief Print splash greeting messsages.
	 \~japanese @brief スプラッシュ挨拶のメッセージを出力する。
	 */
	void print_splash() const;
	/**
	 \~english @brief Check if all the parameters are loaded by the program.
	 @return \c true if all the parameters are loaded. \c false otherwise.
	 \~japanese @brief 設定されたパラメータのリストが全て読み込まれたか確認する。
	 @return もし設定されたパラメータが全て読み込まれていれば \c true を返し、そうでなけば \c false を返す。
	 */
	void check_touched () const;
	/**
	 \~english @brief Print the input message surrounded by -----.
	 \~japanese @brief 入力のメッセージを ----- で囲んで出力する。
	 */
	static void print_bar( std::string str="" );
	/**
	 \~english @brief Print the input message at the center of the line.
	 \~japanese @brief 入力のメッセージを行の中心に出力する。
	 */
	static void print_center( std::string str );
	/**
	 \~english @brief Class that automates the push and pop groups.
	 \~japanese @brief グループの push と pop を自動化するクラス。
	 */
	struct auto_group {
		/**
		 \~english @brief Constructor for auto_group.
		 @param[in] config An instance to configuration class.
		 @param[in] name Name of the group.
		 @param[in] argument_name Argument name of the group.
		 @param[in] author Author of the corresponding group.
		 @param[in] address Email address of the corresponding group.
		 @param[in] date Date of the corresponding group.
		 @param[in] version Version of the group.
		 \~japanese @brief auto_group のコンストラクタ。
		 @param[in] config configuration のインスタンスへの参照。
		 @param[in] name グループの名前。
		 @param[in] argument_name グループの引数の名前。
		 @param[in] author グループの該当する著者名。
		 @param[in] address 該当する著者のメールアドレス。
		 @param[in] date 該当する著者のメールアドレス。
		 @param[in] version グループのバージョン。
		 */
		auto_group( configuration &config,
					std::string name,
					std::string argument_name="",
					std::string author="",
					std::string address="",
					std::tuple<int,int,int> date=std::tuple<int,int,int>(0,0,0),
					double version=0.0
		 ) : config(config) {
			config.push_group(name,argument_name,author,address,date,version);
		}
		/**
		 \~english @brief Constructor for auto_group.
		 @param[in] config An instance to configuration class.
		 @param[in] info Credit information of the group.
		 \~japanese @brief auto_group のコンストラクタ。
		 @param[in] config configuration のインスタンスへの参照。
		 @param[in] info グループの情報。
		 */
		auto_group( configuration &config, const credit &info ) : config(config) {
			config.push_group(info);
		}
		/**
		 \~english @brief Destructor for auto_group.
		 \~japanese @brief auto_group のデストラクタ。
		 */
		~auto_group() { config.pop_group(); }
	private:
		configuration &config;
	};
	/**
	 \~english @brief Get the name of the currently focused group (or argument name).
	 @param[in] argument_name Request argument name.
	 @return The name of the current group (or argument name).
	 \~japanese @brief 現在フォーカスされているグループの名前(あるいは引数名)を得る。
	 @param[in] argument_name 引数名を要求するか。
	 @return 現在のグループ名(あるいは引数名)。
	 */
	std::string get_current_group_name( bool argument_name=false ) const;
	/**
	 \~english @brief Get if the group stack is empty.
	 @return \c true if the stack is empty. \c false otherwise.
	 \~japanese @brief グループのスタックが空白かどうか取得する。
	 @return もし空白であれば \c true を、そうでなければ \false を返す。
	 */
	bool stack_empty() const { return m_group_stack.empty(); }
	/**
	 \~english @brief Push the group to the top of the group stack.
	 @param[in] info An instance of credit from which group information is extracted.
	 \~japanese @brief グループをグループスタックの先頭に追加する。
	 @param[in] info グループの情報を抽出するための credit のインスタンス。
	 */
	void push_group( const credit &info );
	/**
	 \~english @brief Push the group to the top of the group stack.
	 @param[in] name Name of the group.
	 @param[in] argument_name Argument name of the group.
	 @param[in] author Author of the corresponding group.
	 @param[in] address Email address of the corresponding group.
	 @param[in] date Date of the corresponding group.
	 @param[in] version Version of the group.
	 \~japanese @brief グループをグループスタックの先頭に追加する。
	 @param[in] config configuration のインスタンスへの参照。
	 @param[in] name グループの名前。
	 @param[in] argument_name グループの引数の名前。
	 @param[in] author グループの該当する著者名。
	 @param[in] address 該当する著者のメールアドレス。
	 @param[in] date 該当する著者のメールアドレス。
	 @param[in] version グループのバージョン。
	 */
	void push_group( std::string name,
					 std::string argument_name="",
					 std::string author="",
					 std::string address="",
					 std::tuple<int,int,int> date=std::tuple<int,int,int>(0,0,0),
					 double version=0.0 );
	/**
	 \~english @brief Remove the current group from the gruop stack.
	 \~japanese @brief グループスタックから現在のグループを削除する。
	 */
	void pop_group();
	/**
	 \~english @brief Get the integer parameter.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief 整数のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_integer( std::string name, int &value, std::string description=std::string() );
	/**
	 \~english @brief Set the integer parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 整数のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_integer( std::string name, int value );
	/**
	 \~english @brief Set default integer parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 整数の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_integer( std::string name, int value );
	/**
	 \~english @brief Get the unsigned integer parameter.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief 符号なし整数のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_unsigned( std::string name, unsigned &value, std::string description=std::string() );
	/**
	 \~english @brief Set the unsigned integer parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 符号なし整数のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_unsigned( std::string name, unsigned value );
	/**
	 \~english @brief Set default unsgined integer parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 符号なし整数の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_unsigned( std::string name, unsigned value );
	/**
	 \~english @brief Get the boolean parameter.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief ブーリアンのパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_bool( std::string name, bool &value, std::string description=std::string() );
	/**
	 \~english @brief Set the boolean parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief ブーリアンのパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_bool( std::string name, bool value );
	/**
	 \~english @brief Set default boolean parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 符号なしブーリアンの初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_bool( std::string name, bool value );
	/**
	 \~english @brief Get the parameter of double type.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief double 型のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	double get_double( std::string name, double &value, std::string description=std::string() );
	/**
	 \~english @brief Set the parameter of double type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief double 型のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_double( std::string name, double value );
	/**
	 \~english @brief Set default parameter of double type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief double 型の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_double( std::string name, double value );
	/**
	 \~english @brief Get the parameter of float type.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief float 型のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_float( std::string name, float &value, std::string description=std::string() );
	/**
	 \~english @brief Set the parameter of float type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief float 型のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_float( std::string name, float value );
	/**
	 \~english @brief Set default parameter of float type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief float 型の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_float( std::string name, float value );
	/**
	 \~english @brief Get the parameter of vec2i type.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief vec2i 型のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_vec2i( std::string name, int value[2], std::string description=std::string() );
	/**
	 \~english @brief Set the parameter of vec2i type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec2i 型のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_vec2i( std::string name, const int value[2] );
	/**
	 \~english @brief Set default parameter of vec2i type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec2i 型の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_vec2i( std::string name, const int value[2] );
	/**
	 \~english @brief Get the parameter of vec2d type.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief vec2d 型のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_vec2d( std::string name, double value[2], std::string description=std::string() );
	/**
	 \~english @brief Set the parameter of vec2d type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec2d 型のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_vec2d( std::string name, const double value[2] );
	/**
	 \~english @brief Set default parameter of vec2d type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec2d 型の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_vec2d( std::string name, const double value[2] );
	/**
	 \~english @brief Get the parameter of vec3i type.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief vec3i 型のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_vec3i( std::string name, int value[3], std::string description=std::string() );
	/**
	 \~english @brief Set the parameter of vec3i type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec3i 型のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_vec3i( std::string name, const int value[3] );
	/**
	 \~english @brief Set default parameter of vec3i type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec3i 型の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_vec3i( std::string name, const int value[3] );
	/**
	 \~english @brief Get the parameter of vec3d type.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief vec3d 型のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_vec3d( std::string name, double value[3], std::string description=std::string());
	/**
	 \~english @brief Set the parameter of vec3d type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec3d 型のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_vec3d( std::string name, const double value[3] );
	/**
	 \~english @brief Set default parameter of vec3d type.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief vec3d 型の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_vec3d( std::string name, const double value[3] );
	/**
	 \~english @brief Get the string parameter.
	 @param[in] name Name of the parameter.
	 @param[out] value Output value.
	 @param[in] description Description of the parameter.
	 \~japanese @brief 文字のパラメータを得る。
	 @param[in] name パラメータの名前。
	 @param[out] value 出力値。
	 @param[in] description パラメータの説明。
	 */
	bool get_string( std::string name, std::string &value, std::string description=std::string() );
	/**
	 \~english @brief Set string parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 文字のパラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_string( std::string name, std::string value );
	/**
	 \~english @brief Set default string parameter.
	 @param[in] name Name of the parameter.
	 @param[in] value Value to set.
	 \~japanese @brief 文字の初期パラメータを設定する。
	 @param[in] name パラメータの名前。
	 @param[in] value 設定する値。
	 */
	void set_default_string( std::string name, std::string value );
	/**
	 \~english @brief Get if the parameter of the input name exists.
	 @param[in] name Name of the parameter.
	 @return \c true if the parameter exists \c false otherwise.
	 \~japanese @brief 入力の名前のパラメータが存在するか取得する。
	 @param[in] name パラメータの名前。
	 @return パラメータが存在すれば \c true なければ \false を返す。
	 */
	bool exist( std::string name ) const;
	/**
	 \~english @brief Get the dictionary of parameter.
	 @return The list of parameters.
	 \~japanese @brief パラメータの辞書を得る。
	 @return パラメータのリスト。
	 */
	const std::map<std::string,std::string> & get_dictionary() const { return m_dictionary; }
	//
private:
	//
	struct double2 { double v[2]; };
	struct double3 { double v[3]; };
	struct int2 { int v[2]; };
	struct int3 { int v[3]; };
	//
	std::map<std::string,int> default_integer;
	std::map<std::string,unsigned> default_unsigned;
	std::map<std::string,bool> default_bool;
	std::map<std::string,double> default_double;
	std::map<std::string,float> default_float;
	std::map<std::string,double2> default_vec2d;
	std::map<std::string,double3> default_vec3d;
	std::map<std::string,int2> default_vec2i;
	std::map<std::string,int3> default_vec3i;
	std::map<std::string,std::string> default_string;
	//
	struct Entry {
		bool is_default;
		std::string type;
		std::string value;
		std::string description;
	};
	//
	struct Group {
		std::string author;
		std::string address;
		std::tuple<int,int,int> date;
		double version;
		std::map<std::string,Entry> entries;
	};
	//
	struct Title {
		std::string name;
		std::string argument_name;
		unsigned id;
		bool operator<( const Title &a ) const { return id < a.id; }
	};
	//
	void register_variables ( std::string name, bool is_default, std::string type, std::string value, std::string description );
	//
	void print_credit( const Group &group ) const;
	void print_groupbar( const Title &group_title, const Group &group ) const;
	std::string concated_name( std::string name ) const;
	//
	std::map<std::string,std::string> m_dictionary;
	std::map<Title,Group> m_groups;
	std::stack<Title> m_group_stack;
	unsigned m_label_index;
};
//
SHKZ_END_NAMESPACE
//
#endif
//