/*
 * hbase_client_helper.h
 *
 *  Created on: 2016年11月29日
 *      Author: shuntan
 */

#ifndef HBASE_THRIFT_SHUNTAN_HBASE_CLIENT_HELPER_H_
#define HBASE_THRIFT_SHUNTAN_HBASE_CLIENT_HELPER_H_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include "../../c-hbase-client/src/common.h"
#include "../../c-hbase-client/src/thrift_helper.h"
#include "THBaseService.h"
using namespace apache::hadoop::hbase::thrift2;

typedef struct TCell
{
	std::string  m_Family;
	std::string  m_Qualifier;
	std::string  m_Value;
	uint64_t     m_Time_Stamp;

	TCell(): m_Family(""), m_Qualifier(""), m_Value(""),  m_Time_Stamp(0){}
	TCell(const std::string& family): m_Family(family), m_Qualifier(""), m_Value(""), m_Time_Stamp(0){}
	TCell(const std::string& family, const std::string& qualifier):m_Family(family), m_Qualifier(qualifier), m_Value(""), m_Time_Stamp(0){}
	TCell(const std::string& family, const std::string& qualifier, const std::string& value):m_Family(family), m_Qualifier(qualifier), m_Value(value), m_Time_Stamp(0){}
	TCell(const std::string& family, const std::string& qualifier, const std::string& value, uint64_t timestamp):m_Family(family), m_Qualifier(qualifier), m_Value(value), m_Time_Stamp(timestamp){}
};

typedef struct TRow
{
	typedef std::pair<uint64_t, uint64_t> HBTimeRange;   //时间戳范围
	std::string m_Row_Key;                               //行键值
	std::vector<TCell> m_Cells;                          //若干个列族

	//设置行的键值
	void set_rowkey(const std::string& row)
	{
		m_Row_Key = row;
	}

	//增加一个族名
	void add_famliy(const std::string& family)
	{
		m_Cells.push_back(TCell(family));
	}

	//增加一个族下的列名
	void add_qualifier(const std::string& family, const std::string& qualifier)
	{
		m_Cells.push_back(TCell(family, qualifier));
	}

	/////////////////////////////////////////////服务端生成的时间戳/////////////////////////////////////////

	//一般类型
	void add_value(const std::string& family, const std::string& qualifier, const std::string& value)
	{
		m_Cells.push_back(TCell(family, qualifier, value));
	}

	//increment类型使用
	void add_value(const std::string& family, const std::string& qualifier, int64_t value)
	{
		m_Cells.push_back(TCell(family, qualifier, common::int_tostring(value)));
	}

	/////////////////////////////////////////////以下为自定义时间戳//////////////////////////////////////////

	//一般类型
	void add_value(const std::string& family, const std::string& qualifier, const std::string& value, uint64_t timestamp)
	{
		m_Cells.push_back(TCell(family, qualifier, value, timestamp));
	}

	//increment类型使用
	void add_value(const std::string& family, const std::string& qualifier, int64_t value, uint64_t timestamp)
	{
		m_Cells.push_back(TCell(family, qualifier, common::int_tostring(value), timestamp));
	}

	const std::string& get_rowkey() const
	{
		//assert(!m_Row_Key.empty());
		return m_Row_Key;
	}

	const std::vector<TCell>& get_cells() const
	{
		//assert(!m_row.empty());
		return m_Cells;
	}

	void clear()
	{
		m_Row_Key.clear();
		m_Cells.clear();
	}
};

class CHbaseClientHelper
{
public:
	//  输入IP:PORT 列表 e.g., 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
	CHbaseClientHelper(const std::string& host_list, uint32_t connect_timeout = 2000, uint32_t recive_timeout = 2000, uint32_t send_time_out = 2000);
	~CHbaseClientHelper();

	//	返回单例对象或者指针, CHbaseClientHelper::Get_Singleton() -OR- Get_SingletonPtr()
	static CHbaseClientHelper&  Get_Singleton(const std::string& host_list, uint32_t connect_timeout = 2000, uint32_t recive_timeout = 2000, uint32_t send_time_out = 2000);
	static CHbaseClientHelper*  Get_SingletonPtr(const std::string& host_list, uint32_t connect_timeout = 2000, uint32_t recive_timeout = 2000, uint32_t send_time_out = 2000);
	//  是否开启日志提醒, CHbaseClientHelper::Ignore_Log(); --忽略日志
	static void  Ignore_Log() { m_enable_log = false;}

public:

	void* Get_Random_Service();
	bool  Reconnet(void* hbase_client);

	//exists：检查表内是否存在某行或某行内某些列，输入是表名、TGet，输出是bool
	bool  Exist(const std::string& table_name, const TRow& row);

	//对某一行内增加若干列，输入是表名，TPut结构
	bool  Insert(const std::string& table_name, const TRow& row, TDurability::type insert_flag=TDurability::FSYNC_WAL, uint64_t time_stamp = 0);
	//putMultiple：对put的扩展，一次增加若干行内的若个列，输入是表名、TPut数组
	bool  Insert(const std::string& table_name, const std::vector<TRow>& row_list, TDurability::type insert_flag=TDurability::FSYNC_WAL, uint64_t time_stamp = 0);

	//删除某一行内增加若干列，输入是表名，TDelete结构
	bool  Delete(const std::string& table_name, const TRow& row, TDurability::type delete_flag=TDurability::FSYNC_WAL, uint64_t time_stamp = 0);
	//deleteMultiple：对delete的扩展，一次增加若干行内的若个列，输入是表名、TDelete数组
	bool  Delete(const std::string& table_name, const std::vector<TRow> row_list, TDurability::type delete_flag=TDurability::FSYNC_WAL, uint64_t time_stamp = 0);

	//对某一行内的查询，输入是表名、TGet结构，输出是TResult
	bool  Get(const std::string& table_name, TRow& row, TRow::HBTimeRange* time_range = NULL, const std::string& str_filter = "", uint16_t max_version = 0);
	//getMultiple：实际上是对get的扩展，输入是表名、TGet数组，输出是TResult数组
	bool  Get(const std::string& table_name, std::vector<TRow>& row_list, TRow::HBTimeRange* time_range = NULL, const std::string& str_filter = "", uint16_t max_version = 0);
	//查询的条件由TScan封装，在打开时传入。需要注意的是每次取数据的行数要合适，否则有效率问题。
	bool  Get(const std::string& table_name, const std::string& begin_row, const std::string& stop_row, std::vector<TRow>& row_list, uint16_t num_rows, TRow::HBTimeRange* time_range = NULL, const std::string& str_filter = "", uint16_t max_version = 0);

	//对某行中若干列进行追加内容.
	bool  Append(const std::string& table_name, const TRow& row, TDurability::type append_flag=TDurability::FSYNC_WAL);

	//增加一行内某列的值，这个操作比较特别，是专门用于计数的，也保证了“原子”操作特性。
	bool  Increment(const std::string& table_name, const std::string& row_key, const std::string& family_name, const std::string& column_name, int64_t column_value, TDurability::type increment_flag=TDurability::FSYNC_WAL);
	//增加一行内某些列的值，这个操作比较特别，是专门用于计数的，也保证了“原子”操作特性。
	bool  Increment(const std::string& table_name, const TRow& row, TDurability::type increment_flag=TDurability::FSYNC_WAL);

	//当传入的（表名+列族名+列名+新数据+老数据）都存在于数据库时，才做操作
	bool  Check_With_Replace(const std::string& table_name, const std::string& row_key, const std::string& family_name, const std::string& column_name, const std::string& old_column_value, const std::string& new_column_value, TDurability::type check_flag=TDurability::FSYNC_WAL);
	//当传入的（表名+列族名+列名+数据）都存在于数据库时，才做操作
	bool  Check_With_Erase(const std::string& table_name, const std::string& row_key, const std::string& family_name, const std::string& column_name, const std::string& old_column_value, TDurability::type check_flag=TDurability::FSYNC_WAL);

private:
	std::vector<CThriftClientHelper<apache::hadoop::hbase::thrift2::THBaseServiceClient>* > m_hbase_clients;
	static bool m_enable_log;
};

#endif /* HBASE_THRIFT_SHUNTAN_HBASE_CLIENT_HELPER_H_ */
