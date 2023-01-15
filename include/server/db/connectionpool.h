#ifndef __CONNECTIONPOOL_H__
#define __CONNECTIONPOOL_H__

#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include "db.h"

class ConnectionPool {
public:
	static ConnectionPool* instance();
	std::shared_ptr<MySQL> getConnection();
	~ConnectionPool();
private:
	ConnectionPool();



	// 运行在独立的线程中，专门负责生产新连接
	void produceConnectionTask();

	// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
	void scannerConnectionTask();

	string _ip; // mysql的ip地址
	unsigned short _port; // mysql的端口号 3306
	string _username; // mysql登录用户名
	string _password; // mysql登录密码
	string _dbname; // 连接的数据库名称
	int _initSize; // 连接池的初始连接量
	int _maxSize; // 连接池的最大连接量
	int _maxIdleTime; // 连接池最大空闲时间
	int _connectionTimeout; // 连接池获取连接的超时时间

	queue<MySQL*> _connectionQue; // 存储mysql连接的队列
	mutex _queueMutex; // 维护连接队列的线程安全互斥锁
	atomic_int _connectionCnt; // 记录连接所创建的connection连接的总数量 
	condition_variable cv; // 设置条件变量，用于连接生产线程和连接消费线程的通信

	bool _isAlive;
};

#endif // __CONNECTIONPOOL_H__