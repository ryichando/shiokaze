/*
**	graphplotter_interface.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on November 2, 2019.
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
#ifndef SHKZ_GRAPHPLOTTER2_INTERFACE_H
#define SHKZ_GRAPHPLOTTER2_INTERFACE_H
//
#include <shiokaze/core/recursive_configurable_module.h>
#include <shiokaze/graphics/graphics_engine.h>
#include <vector>
#include <string>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Interface for plotting graph. "graphplotter" is provided as implementation.
/// \~japanese @brief グラフの描画を処理するインターフェース。"graphplotter" が実装として提供される。
class graphplotter_interface : public recursive_configurable_module {
public:
	//
	DEFINE_MODULE(graphplotter_interface,"Graph Plotter","Graph","Graph plotter module")
	/**
	 \~english @brief Clear the graph.
	 \~japanese @brief グラフをクリアする。
	 */
	virtual void clear() = 0;
	/**
	 \~english @brief Create an entry.
	 @param[in] name Name of the entry.
	 @return Entry id.
	 \~japanese @brief エントリーを作成する。
	 @param[in] name エントリーの名前。
	 @return エントリーのID。
	 */
	virtual unsigned create_entry( std::string name ) = 0;
	/**
	 \~english @brief Delete an entry.
	 @param[in] id Entry id.
	 \~japanese @brief エントリーを作成する。
	 @param[in] id エントリーのID。
	 */
	virtual void delete_entry( unsigned id ) = 0;
	/**
	 \~english @brief Set a unit number.
	 @param[in] value Unit number.
	 \~japanese @brief 単位数を設定する。
	 @param[in] value 単位数。
	 */
	virtual void set_unit_number( double value ) = 0;
	/**
	 \~english @brief Add an point.
	 @param[in] id Entry id.
	 @param[in] time Time of the point.
	 @param[in] number Value of the point.
	 \~japanese @brief ポイントを追加する。
	 @param[in] id エントリーID。
	 @param[in] time ポイントの時間。
	 @param[in] number 値。
	 */
	virtual void add_point( unsigned id, double time, double number ) = 0;
	/**
	 \~english @brief Set an attribute.
	 @param[in] id Entry id.
	 @param[in] name Name of the atrribute to set.
	 @param[in] attribute Pointer to an attribute value.
	 \~japanese @brief 属性を設定する。
	 @param[in] id エントリーID。
	 @param[in] name 属性の名前。
	 @param[in] attribute 属性の値へのポインター。
	 */
	virtual void set_attribute( unsigned id, std::string name, const void *attribute ) = 0;
	/**
	 \~english @brief Get an attribute.
	 @param[in] id Entry id.
	 @param[in] name Name of the atrribute to set.
	 @return pointer to an atribute value.
	 \~japanese @brief 属性を得る。
	 @param[in] id エントリーID。
	 @param[in] name 属性の名前。
	 @return 属性の値へのポインタ。
	 */
	virtual const void * get_attribute( unsigned id, std::string name ) const = 0;
	/**
	 \~english @brief Draw the graph.
	 @param[in] g Graphics engine.
	 \~japanese @brief グラフを描く。
	 @param[in] g グラフィックスのエンジン。
	 */
	virtual void draw( graphics_engine &g ) const = 0;
};
//
using graphplotter_ptr = std::unique_ptr<graphplotter_interface>;
using graphplotter_driver = recursive_configurable_driver<graphplotter_interface>;
//
SHKZ_END_NAMESPACE
//
#endif