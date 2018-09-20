/*
**	sysstats_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 21, 2017. 
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
#ifndef SHKZ_SYSSTATS_INTERFACE_H
#define SHKZ_SYSSTATS_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for handling system status.
/// \~japanese @brief システムの状態について管理するインターフェース。
class sysstats_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(sysstats_interface,"System Stats Analyzer","SysStats","System Stats Analyzer")
	/**
	 \~english @brief Report system status.
	 \~japanese @brief システムの状態を報告する。
	 */
	virtual void report_stats() const = 0;
	/**
	 \~english @brief Plot the data of log files as graph.
	 \~japanese @brief ログファイルのデータをグラフに書き出す。
	 */
	virtual void plot_graph() const = 0;
};
//
using sysstats_ptr = std::unique_ptr<sysstats_interface>;
//
SHKZ_END_NAMESPACE
//
#endif