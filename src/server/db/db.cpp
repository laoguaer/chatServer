#include "db.h"
#include <string>
// #include <muduo/base/Logging.h>
#include <mymuduo/logger.h>
using namespace std;

// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";

// 初始化数据库连接
MySQL::MySQL()
{
	_conn = mysql_init(nullptr);
}
MySQL::~MySQL()
{
	if (_conn != nullptr)
	mysql_close(_conn);
}

// 连接数据库
bool MySQL::connect(string ip, unsigned short port, 
	string username, string password, string dbname)
{
	// 连接数据库
	MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	if (p != nullptr)
	{
		// mysql_query(_conn, "set names gbk");
		// LOG_INFO << "connect mysql" ;
		//LOG_INFO("connect mysql");
	}
	return p;
}

// 连接数据库
bool MySQL::connect()
{
	// 连接数据库
	MYSQL *p = mysql_real_connect(_conn, "127.0.0.1", user.c_str(),
		password.c_str(), dbname.c_str(), 3306, nullptr, 0);
	if (p != nullptr)
	{
		// mysql_query(_conn, "set names gbk");
		// LOG_INFO << "connect mysql" ;
		// LOG_INFO("connect mysql");
	}
	return p != nullptr;
}

// 更新操作
bool MySQL::update(string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		// LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
		// << sql << "更新失败!";
		LOG_INFO("%s:%d:sql更新失败", __FILE__, __LINE__);
		return false;
	}
	return true;
}

// 查询操作
MYSQL_RES* MySQL::query(string sql)
{
	if (mysql_query(_conn, sql.c_str()))
	{
		// LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
		// << sql << "查询失败!";
		LOG_INFO("%s:%d:sql查询失败",__FILE__, __LINE__);
		return nullptr;
	}
	return mysql_use_result(_conn);
}

MYSQL* MySQL::getCurConn() 
{
	return _conn;
}