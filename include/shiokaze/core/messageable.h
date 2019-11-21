/*
**	messageable.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 19, 2019.
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
#ifndef SHKZ_MESSAGEABLE_H
#define SHKZ_MESSAGEABLE_H
//
#include <shiokaze/core/common.h>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/// \~english @brief Message class.
/// \~japanese @brief メッセージクラス。
class messageable {
public:
	/**
	 \~english @brief Send a message.
	 @param[in] message Message
	 @param[in] ptr Pointer to some value.
	 @return \c true if handled \c false otherwise.
	 \~japanese @brief メッセージを送る
	 @param[in] message メッセージ
	 @param[in] ptr 何らかのポインター
	 @return もし処理されたら \c true を、処理されなかったら \c false
	 */
	virtual bool send_message( std::string message, void *ptr=nullptr ) { return false; }
	/**
	 \~english @brief Send a message.
	 @param[in] message Message
	 @param[in] ptr Pointer to some value.
	 @return \c true if handled \c false otherwise.
	 \~japanese @brief メッセージを送る
	 @param[in] message メッセージ
	 @param[in] ptr 何らかのポインター
	 @return もし処理されたら \c true を、処理されなかったら \c false
	 */
	virtual bool const_send_message( std::string message, void *ptr=nullptr ) const { return false; }
};
//
SHKZ_END_NAMESPACE
//
#endif