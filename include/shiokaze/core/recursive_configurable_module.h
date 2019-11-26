/*
**	recursive_configurable_module.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Jan 12, 2018.
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
#ifndef SHKZ_RECURSIVE_CONFIGURABLE_MODULE_H
#define SHKZ_RECURSIVE_CONFIGURABLE_MODULE_H
//
#include <shiokaze/core/module.h>
#include <shiokaze/core/configurable.h>
#include <vector>
#include <cassert>
//
SHKZ_BEGIN_NAMESPACE
/**
 \~english @brief Definition that simplifies the loading module.
 \~japanese @brief モジュールの読み込みを簡単にする定義。
 */
#define DEFINE_MODULE(CLASS_T,LNG_NAME,ARG_NAME,DESCRIPTION) \
	DEFINE_QUICK_ALLOC_MODULE(CLASS_T,ARG_NAME,DESCRIPTION) \
	LONG_NAME(LNG_NAME) \
	static std::unique_ptr<CLASS_T> quick_load_module( configuration &config, std::string name ) { \
		auto result = unique_alloc_module<CLASS_T>(config,ARG_NAME,name,DESCRIPTION); \
		result->recursive_load(config); \
		return result; \
	}
/// \~english @brief recursive_configurable class that also inherits module.
/// \~japanese @brief module を継承する recursive_configurable クラス。
class recursive_configurable_module : public recursive_configurable, public module {
public:
	/**
	 \~english @brief Start recursively load configurables.
	 @param[in] config Configuration setting.
	 \~japanese @brief 再帰的に load を呼ぶ。
	 @param[in] config 設定。
	 */
	virtual void recursive_load ( configuration &config ) override {
		configuration::auto_group group(config,*this);
		recursive_configurable::recursive_load(config);
	}
	/**
	 \~english @brief Recursively call configure.
	 @param[in] config Configuration setting.
	 \~japanese @brief 再帰的に configure を呼ぶ。
	 @param[in] config 設定。
	 */
	virtual void recursive_configure ( configuration &config ) override {
		configuration::auto_group group(config,*this);
		recursive_configurable::recursive_configure(config);
	}
};
/**
 \~english @brief Class that encapsulates recursive_configurable class.
 \~japanese @brief recursive_configurable のクラスを抱擁するクラス。
 */
template <class T> class recursive_configurable_driver : public configurable, public messageable {
public:
	/**
	 \~english @brief Constructor for recursive_configurable_driver.
	 @param[in] name Module name.
	 @param[in] argname Argument name for this instance.
	 \~japanese @brief recursive_configurable_driver のコンストラクタ。
	 @param[in] name モジュール名。
	 @param[in] argname このインスタンスの変数名.
	 */
	recursive_configurable_driver ( std::string name, std::string argname="" ) : m_name(name), m_argname(argname) {}
	/**
	 \~english @brief Constructor for recursive_configurable_driver.
	 @param[in] parent Full mame for this instance.
	 @param[in] name Module name.
	 \~japanese @brief recursive_configurable_driver のコンストラクタ。
	 @param[in] name モジュール名。
	 @param[in] m_argname このインスタンスの変数名.
	 */
	recursive_configurable_driver ( recursive_configurable *parent, std::string name ) : m_name(name) {
		assert(parent);
		parent->add_child(this);
	}
	/**
	 \~english @brief Set name.
	 @param[in] long_name Module name.
	 @param[in] argname Argument name for this instance.
	 \~japanese @brief 名前を設定する。
	 @param[in] long_name モジュール名。
	 @param[in] argname このインスタンスの変数名。
	 */
	void set_name( std::string long_name, std::string argname="" ) {
		m_long_name = long_name;
		if( argname.size()) m_argname = argname;
	}
	/**
	 \~english @brief Set argument name.
	 @param[in] argname Argument name for this instance.
	 \~japanese @brief 変数名前を設定する。
	 @param[in] argname このインスタンスの変数名。
	 */
	void set_argument_name( std::string argname ) {
		m_argname = argname;
	}
	/**
	 \~english @brief Set the pointer to an environmental value associated with an input name.
	 @param[in] name Name for the environmental value to set.
	 @param[in] value Pointer to the value to set.
	 \~japanese @brief 入力名に関連した環境変数の値へのポインターを設定する。
	 @param[in] name 設定する環境変数への名前。
	 @param[in] value 設定する変数の値へのポインター。
	 */
	void set_environment( std::string name, const void *value ) {
		m_environment[name] = value;
	}
	/**
	 \~english @brief Do loading.
	 @param[in] config Configuration setting.
	 \~japanese @brief 読み込みを行う。
	 @param[in] config 設定。
	 */
	virtual void load ( configuration &config ) override {
		m_object = T::quick_alloc_module(config,m_name);
		if( m_long_name.size()) m_object->set_name(m_long_name);
		if( m_argname.size()) m_object->set_argument_name(m_argname);
		m_object->recursive_load(config);
	}
	/**
	 \~english @brief Do configure.
	 @param[in] config Configuration setting.
	 \~japanese @brief configure 処理を行う。
	 @param[in] config 設定。
	 */
	virtual void configure ( configuration &config ) override {
		m_object->recursive_configure(config);
	}
	/**
	 \~english @brief Initialize with an environmental map.
	 @param[in] environment Enivironmental map.
	 \~japanese @brief 環境変数を元に初期化を行う。
	 @param[in] environment 環境変数。
	 */
	virtual void initialize( const configurable::environment_map &environment ) override {
		configurable::environment_map merged_environment(m_environment);
		merged_environment.insert(environment.begin(),environment.end());
		m_object->recursive_initialize(merged_environment);
	}
	/**
	 \~english @brief Get the raw pointer.
	 @return Raw poiter.
	 \~japanese @brief 生ポインターを取得する
	 @return 生ポインター。
	 */
	T* operator->() { return get(); }
	/**
	 \~english @brief Get the const raw pointer.
	 @return Const raw poiter.
	 \~japanese @brief const な生ポインターを取得する
	 @return const な生ポインター。
	 */
	const T* operator->() const { return get(); }
	/**
	 \~english @brief Get the raw pointer.
	 @return Raw poiter.
	 \~japanese @brief 生ポインターを取得する
	 @return 生ポインター。
	 */
	T* get() { return m_object.get(); }
	/**
	 \~english @brief Get the raw pointer.
	 @return Raw poiter.
	 \~japanese @brief 生ポインターを取得する
	 @return 生ポインター。
	 */
	const T* get() const { return m_object.get(); }
	//
private:
	//
	std::string m_name, m_long_name, m_argname;
	std::unique_ptr<T> m_object;
	configurable::environment_map m_environment;
	//
	recursive_configurable_driver( const recursive_configurable_driver & ) = delete;
	recursive_configurable_driver& operator=( const recursive_configurable_driver & ) = delete;
};
//
SHKZ_END_NAMESPACE
//
#endif
