#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
using namespace std;
class MySQL
{
public:
// 初始化数据库连接
	MySQL();
	~MySQL();
// 连接数据库
	bool connect();

// 更新操作
	bool update(string sql);

// 查询操作
	MYSQL_RES* query(string sql);

// 获取当前连接
	MYSQL* getCurConn();

private:
	MYSQL *_conn;
};	

#endif