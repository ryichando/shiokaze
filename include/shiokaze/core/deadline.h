/*
**	deadline.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 31, 2017.
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
#ifndef SHKZ_DEADLINE_H
#define SHKZ_DEADLINE_H
//
/************** How to use this code ***************
//
deadline deadlines;
deadlines.add_deadline("SIGGRAPH",1,17);
deadlines.add_deadline("SIGGRAPH Asia",5,17);
deadlines.add_deadline("SIGGRAPH Asia Technical Briefs",8,15);
deadlines.add_deadline("Eurographics",10,14);
//
std::string name; int days;
deadlines.get_next_deadline(name,days);
//
printf( "Next deadline: %s, %d days.\n", name.c_str(), days );
//
****************************************************/
//
#include <vector>
#include <string>
#include <shiokaze/core/common.h>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class that tells you the next deadline.
/// \~japanese @brief 次の〆切を伝えてくれるクラス。
class deadline {
public:
	/**
	 \~english @brief Add a new deadline.
	 @param[in] name Name of the deadline.
	 @param[in] month Month of the deadline.
	 @param[in] day Day of the deadline.
	 \~japanese @brief 新しい〆切を追加する。
	 @param[in] name 〆切の名前。
	 @param[in] month 〆切の月。
	 @param[in] day 〆切の日。
	 */
	void add_deadline( std::string name, int month, int day );
	/**
	 \~english @brief Get days until next closest deadline.
	 @param[out] name Name of the deadline.
	 @param[out] days Days until the deadline.
	 \~japanese @brief 次の〆切までの日数を取得する。
	 @param[out] name 〆切の名前。
	 @param[out] days 〆切までの日数。
	 */
	void get_next_deadline( std::string &name, int &days ) const;
	//
private:
	//
	typedef struct {
		std::string name;
		int month;
		int day;
	} _deadline;
	//
	std::vector<_deadline> m_deadlines;
};
//
SHKZ_END_NAMESPACE
//
#endif