#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <ctime>
using namespace std;

class MySQL
{
public:
// 初始化数据库连接
	MySQL();
	~MySQL();
// 连接数据库
	bool connect(string ip, unsigned short port, 
	string username, string password, string dbname);
	bool connect();

// 更新操作
	bool update(string sql);

// 查询操作
	MYSQL_RES* query(string sql);

// 获取当前连接
	MYSQL* getCurConn();

// 刷新一下连接的起始的空闲时间点
	void refreshAliveTime() { _alivetime = clock(); }
	// 返回存活的时间
	clock_t getAliveeTime()const { return clock() - _alivetime; }

private:
	MYSQL *_conn;
	clock_t _alivetime;
};	

#endif