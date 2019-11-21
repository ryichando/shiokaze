/*
**	configurable.h
**
**	This is part of Shiokaze, a research-oriented fluid solver for computer graphics.
**	Created by Ryoichi Ando <rand@nii.ac.jp> on Sep 8, 2017. 
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
#ifndef SHKZ_CONFIGURABLE_H
#define SHKZ_CONFIGURABLE_H
//
#include <shiokaze/core/configuration.h>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
//
SHKZ_BEGIN_NAMESPACE
//
/** @file */
/// \~english @brief Class for managing the workflow of load - configure - initialize.
/// \~japanese @brief load - configure - initialize ワークフローを管理するための基本クラス。
class configurable {
public:
	/**
	 \~english @brief Type for environment_map.
	 \~japanese @brief environment_map のタイプ。
	 */
	using environment_map = std::map<std::string,const void *>;
	/**
	 \~english @brief Load the program. Used to load files and libraries into memory.
	 @param[in] config Configuration setting.
	 \~japanese @brief プログラムを読み込む。ファイルやライブラリをメモリに読み込むことに使われる。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	virtual void load ( configuration &config ) {}
	/**
	 \~english @brief Configure the program. Used to load and set parameters.
	 @param[in] config Configuration setting.
	 \~japanese @brief プログラムの設定を実際に行う。パラメータの読み込みと設定に使われる。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	virtual void configure ( configuration &config ) {}
	/**
	 \~english @brief Initialize the program. Used to get things ready for actual use.
	 @param[in] config Configuration setting.
	 \~japanese @brief プログラムの初期化を行う。実際にプログラムを実行可能な状態を作るために使われる。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	virtual void initialize( const environment_map &environment ) {}
	/**
	 \~english @brief Assign the global settings of the program.
	 @param[in] config Configuration setting.
	 \~japanese @brief グローバルの設定を与える。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	static configuration& set_global_configuration( const configuration& config );
	/**
	 \~english @brief Get the global settings of the program.
	 @return Configuration setting.
	 \~japanese @brief グローバルの設定を取得する。
	 @return config 設定を管理する configuration のインスタンスへの参照。
	 */
	static configuration& get_global_configuration();
	/**
	 \~english @brief Extract an specified type of pointer from the input environment.
	 @return Extracted pointer.
	 \~japanese @brief 入力の環境のインスタンスから、指定された種類のポインターを取得する。
	 @return config 設定を管理する configuration のインスタンスへの参照。
	 */
	template <class T > static const T& get_env( const environment_map &environment, std::string key ) {
		return *reinterpret_cast<const T *>(environment.at(key));
	}
	/**
	 \~english @brief Run load - configure - initialize processes.
	 \~japanese @brief load - configure - initialize のプロセスを呼ぶ。
	 */
	virtual void setup_now( configuration& config=get_global_configuration() ) {
		assert(not_recursive());
		load(config);
		configure(config);
		initialize(environment_map());
	}
	/**
	 \~english @brief Check if this instance is not derived from recursive_configurable.
	 @return \c true if the class is not inherited from recursive_configurable. \c false if inherited.
	 \~japanese @brief recursive_configurable から継承されたインスタンスでないか確認する。
	 @return recursive_configurable から継承されていなければ \c true を返し、継承されていれば \c false を返す。
	 */
	virtual bool not_recursive() { return true; } // for safety check
protected:
	/**
	 \~english @brief Check if the values for a variable keys exist.
	 @return \c true if all the values for the keys exist. \c false otherwise.
	 \~japanese @brief 変数のキーの配列に対応する値が存在するか取得する。
	 @return もし全ての変数のキーに対応する値が存在すれば \c true を、そうでなければ \c false を返す。
	 */
	bool check_set( const environment_map &environment, std::vector<std::string> names ) {
		bool result (true);
		for( const auto &key : names ) {
			result = result && environment.find(key) != environment.end();
		}
		return result;
	}
};
//
/// \~english @brief Extended configurable class that holds multiple children of configurable.
/// \~japanese @brief 複数の子供 configurable を保持する configurable クラスの拡張。
class recursive_configurable : public configurable {
public:
	/**
	 \~english @brief Default constructor.
	 \~japanese @brief デフォルトコンストラクタ。
	 */
	recursive_configurable() = default;
	/**
	 \~english @brief Load the program itself and relay the same to its children. Used to load files and libraries into memory.
	 @param[in] config Configuration setting.
	 \~japanese @brief 自身のプログラムを読み込み、子供インスタンスにも同様の処理を行う。ファイルやライブラリをメモリに読み込むことに使われる。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	virtual void recursive_load ( configuration &config ) {
		load(config);
		for( auto it=m_children.rbegin(); it!=m_children.rend(); ++it ) (*it)->load(config);
		for( auto it=m_recursive_children.rbegin(); it!=m_recursive_children.rend(); ++it ) (*it)->recursive_load(config);
		post_load();
		m_load_done = true;
	}
	/**
	 \~english @brief Configure the program itself and relay the same to its children. Used to load and set parameters.
	 @param[in] config Configuration setting.
	 \~japanese @brief プログラムの設定を実際に行い、子供インスタンスにも同様の処理を行う。パラメータの読み込みと設定に使われる。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	virtual void recursive_configure ( configuration &config ) {
		assert(m_load_done);
		configure(config);
		for( auto it=m_children.rbegin(); it!=m_children.rend(); ++it ) (*it)->configure(config);
		for( auto it=m_recursive_children.rbegin(); it!=m_recursive_children.rend(); ++it ) (*it)->recursive_configure(config);
		post_configure();
		m_configure_done = true;
	}
	/**
	 \~english @brief Initialize the program itself and relay the same to its children. Used to get things ready for actual use.
	 @param[in] config Configuration setting.
	 \~japanese @brief プログラムの初期化を行い、子供インスタンスにも同様の処理を行う。実際にプログラムを実行可能な状態を作るために使われる。
	 @param[in] config 設定を管理する configuration のインスタンスへの参照。
	 */
	virtual void recursive_initialize( const configurable::environment_map &environment=configurable::environment_map() ) {
		assert(m_configure_done);
		configurable::environment_map merged_environment(m_environment);
		merged_environment.insert(environment.begin(),environment.end());
		initialize(merged_environment);
		for( auto it=m_children.rbegin(); it!=m_children.rend(); ++it ) (*it)->initialize(merged_environment);
		for( auto it=m_recursive_children.rbegin(); it!=m_recursive_children.rend(); ++it ) (*it)->recursive_initialize(merged_environment);
		post_initialize();
		m_initialize_done = true;
	}
	/**
	 \~english @brief Get if the instance is initialized.
	 @return \c true if the instance is initialized \c false otherwise.
	 \~japanese @brief インスタンスが初期化されているか取得する。
	 @return \c もしインスタンスが初期化されていれば \c true を返し、そうでなければ \false を返す。
	 */
	virtual bool is_ready() const {
		return m_initialize_done;
	}
	/**
	 \~english @brief Add a child instance.
	 @param[in] child Pointer to an instance to add.
	 \~japanese @brief 子供インスタンスを追加する。
	 @param[in] child 追加する子供インスタンスへのポインター。
	 */
	virtual void add_child( configurable *child ) {
		assert(child->not_recursive());
		m_children.push_back(child);
	}
	/**
	 \~english @brief Add a child instance.
	 @param[in] child Pointer to an instance to add.
	 \~japanese @brief 子供インスタンスを追加する。
	 @param[in] child 追加する子供インスタンスへのポインター。
	 */
	virtual void add_child( recursive_configurable *child ) {
		m_recursive_children.push_back(child);
	}
	/**
	 \~english @brief Remove a child instance.
	 @param[in] child Pointer to an instance to remove.
	 \~japanese @brief 子供インスタンスを削除する。
	 @param[in] child 削除する子供インスタンスへのポインター。
	 */
	virtual void remove_child( configurable *child ) {
		assert(child->not_recursive());
		m_children.erase(find(m_children.begin(),m_children.end(),child));
	}
	/**
	 \~english @brief Remove a child instance.
	 @param[in] child Pointer to an instance to remove.
	 \~japanese @brief 子供インスタンスを削除する。
	 @param[in] child 削除する子供インスタンスへのポインター。
	 */
	virtual void remove_child( recursive_configurable *child ) {
		m_recursive_children.erase(find(m_recursive_children.begin(),m_recursive_children.end(),child));
	}
	/**
	 \~english @brief Run recursive_load - recursive_configure - recursive_initialize processes.
	 \~japanese @brief recursive_load - recursive_configure - recursive_initialize のプロセスを呼ぶ。
	 */
	virtual void setup_now( configuration& config=get_global_configuration() ) override {
		recursive_load(config);
		recursive_configure(config);
		recursive_initialize();
	}
	/**
	 \~english @brief Check if this instance is not derived from recursive_configurable.
	 @return \c Always returns true.
	 \~japanese @brief recursive_configurable から継承されたインスタンスでないか確認する。
	 @return 常に true を返す。
	 */
	virtual bool not_recursive() override { return false; } // for safety check
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
	//
protected:
	//
	/// \~english @brief Class for setting environemt.
	/// \~japanese @brief 環境を設定するクラス。
	class environment_setter {
	public:
		/**
		 \~english @brief Constructor for environment_setter.
		 @param[in] instance Pointer to an instance of recursive_configurable
		 \~japanese @brief environment_setter のコンストラクタ。
		 @param[in] instane recursive_configurable のインスタンスへのポインター。
		 */
		environment_setter ( recursive_configurable *instance ) {
			instance->m_environment.clear();
		}
		/**
		 \~english @brief Constructor for environment_setter.
		 @param[in] instance Pointer to an instance of recursive_configurable
		 @param[in] name Name of an argument.
		 @param[in] value Pointer to an argument value.
		 \~japanese @brief environment_setter のコンストラクタ。
		 @param[in] instane recursive_configurable のインスタンスへのポインター。
		 @param[in] name 変数の名前。
		 @param[in] value 変数のポインター。
		 */
		environment_setter ( recursive_configurable *instance, std::string name, const void *value ) {
			instance->m_environment[name] = value;
		}
		/**
		 \~english @brief Constructor for environment_setter.
		 @param[in] instance Pointer to an instance of recursive_configurable
		 @param[in] env Environmental map.
		 \~japanese @brief environment_setter のコンストラクタ。
		 @param[in] instane recursive_configurable のインスタンスへのポインター。
		 @param[in] env 環境マップ。
		 */
		environment_setter ( recursive_configurable *instance, const configurable::environment_map &env ) {
			instance->m_environment = env;
		}
	};
	//
private:
	//
	using configurable::load;
	using configurable::configure;
	//
	virtual void initialize( const environment_map &environment ) override { configurable::initialize(environment); }
	virtual void post_load () {}
	virtual void post_configure () {}
	virtual void post_initialize () {}
	//
	std::vector<configurable *> m_children;
	std::vector<recursive_configurable *> m_recursive_children;
	configurable::environment_map m_environment;
	//
	bool m_load_done{false}, m_configure_done{false}, m_initialize_done{false};
	//
	recursive_configurable( const recursive_configurable & ) = delete;
	recursive_configurable& operator=( const recursive_configurable & ) = delete;
};
//
SHKZ_END_NAMESPACE
//
#endif
