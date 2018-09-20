/*
**	console.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on March 16, 2018.
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
#ifndef SHKZ_CONSOLE_H
#define SHKZ_CONSOLE_H
//
#include <shiokaze/core/common.h>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Namespace that helps executing commands, dumping messages, writing log files.
/// \~japanese @brief ログファイルを書き出したり、メッセージを出力したり、コマンドの実行を支援するネームスペース。
namespace console {
	/**
	 \~english @brief Excecute a command and get result.
	 @param[in] foramt Formatted command string.
	 @return Result as string.
	 \~japanese @brief コマンドを実行し、結果を得る。
	 @param[in] format フォーマットされたコマンドの文字列。
	 @return 結果の文字列。
	 */
	std::string run( std::string format, ...);
	/**
	 \~english @brief Excecute a command and get result.
	 @param[in] foramt Formatted command string.
	 @return Result as integer.
	 \~japanese @brief コマンドを実行し、結果を得る。
	 @param[in] format フォーマットされたコマンドの文字列。
	 @return 結果の整数値。
	 */
	int system( std::string format, ...);
	/**
	 \~english @brief Convert time duration to string
	 @param[in] msec Time in milliseconds.
	 @return Converted string.
	 \~japanese @brief 時間を文字に変換する。
	 @param[in] msec ミリセカンド秒。
	 @return 変換された文字。
	*/
	std::string tstr(double msec);
	/**
	 \~english @brief Convert enumeration to string such as 1st, 2nd.
	 @param[in] num Enumeration in integer.
	 @return Converted string.
	 \~japanese @brief 列挙数を 1st, 2nd といった文字に変換する。
	 @param[in] num 列挙の数。
	 @return 変換された文字。
	*/
	std::string nth(int num);
	/**
	 \~english @brief Convert byte size to string such as killo bytes and mega bytes.
	 @param[in] bytes Byte size.
	 @return Converted string.
	 \~japanese @brief バイトサイズをキロバイトやメガバイトといった文字に変換する。
	 @param[in] bytes バイトサイズ。
	 @return 変換された文字。
	*/
	std::string size_str( size_t bytes );
	/**
	 \~english @brief Format string with given arguments.
	 @param[in] format format string.
	 @return Formatted string.
	 \~japanese @brief 様々な入力をもとに文字列をフォーマットする。
	 @param[in] format フォーマットの文字列。
	 @return フォーマットされた文字。
	*/
	std::string format_str( std::string format, ...);
	/**
	 \~english @brief Set path to export log files.
	 @param[in] path Path to export.
	 \~japanese @brief ログファイルを出力するパスを設定する。
	 @param[in] path URL:path 出力先のパス。
	*/
	void set_root_path( std::string path );
	/**
	 \~english @brief Get path to export log files.
	 @return If set, path to export. Empty string otherwise.
	 \~japanese @brief ログファイルを出力するパスを取得する。
	 @return もし設定されていれば、出力先のパス。そうでなければ、空白の文字列を返す。
	*/
	std::string get_root_path();
	/**
	 \~english @brief Print a log message in a single line.
	 @param[in] format Message to print.
	 \~japanese @brief 一行にログメッセージを出力する。
	 @param[in] format 出力するメッセージ。
	*/
	void dump(std::string format, ...);
	/**
	 \~english @brief Set a time for logging.
	 @param[in] time Time to set. Typically in milliseconds.
	 \~japanese @brief シミュレーションの時間をログに記録する。
	 @param[in] time 設定する時間。主に、ミリセカンド秒を想定。
	*/
	void set_time( double time );
	/**
	 \~english @brief Export number associated with the name as log file.
	 @param[in] name Name associated with the number.
	 @param[in] number Number to record.
	 \~japanese @brief ラベル名前に関する数字をログファイルとして出力する。
	 @param[in] name 数字に関連した名前。
	 @param[in] number ログファイルに記録する数字。
	*/
	void write( std::string name, double number );
}
//
SHKZ_END_NAMESPACE
//
#endif