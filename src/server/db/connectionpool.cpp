#include <fstream>
#include <thread>
#include <functional>
#include <iostream>

#include <mymuduo/logger.h>
#include "connectionpool.h"
#include "json.hpp"

using namespace nlohmann;
using namespace std;


ConnectionPool* ConnectionPool::instance()
{
	static ConnectionPool ins;
	return &ins;
}

ConnectionPool::ConnectionPool()
{
	std::ifstream ifs("../src/server/db/my.ini");
	if (!ifs.is_open()) {
		LOG_FATAL("open my.ini file filed!");
	}
	json js = json::parse(ifs);
	
	_ip = js["ip"]; // mysql的ip地址
	_port = js["port"]; // mysql的端口号 3306
	_username = js["username"]; // mysql登录用户名
	_password = js["password"]; // mysql登录密码
	_dbname = js["dbname"]; // 连接的数据库名称
	_initSize = js["initSize"]; // 连接池的初始连接量
	_maxSize = js["maxSize"]; // 连接池的最大连接量
	_maxIdleTime = js["maxIdleTime"]; // 连接池最大空闲时间
	_connectionTimeout = js["connectionTimeOut"]; // 连接池获取连接的超时时间

		// 创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		MySQL *p = new MySQL();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}

	_isAlive = true;
	thread produce(&ConnectionPool::produceConnectionTask, this);
	thread scanner(&ConnectionPool::scannerConnectionTask, this);
	produce.detach();
	scanner.detach();
}

ConnectionPool::~ConnectionPool() 
{
	exit(0);
}

// 运行在独立的线程中，专门负责生产新连接
void ConnectionPool::produceConnectionTask()
{
	while (_isAlive)
	{
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty())
		{
			cv.wait(lock); // 队列不空，此处生产线程进入等待状态
		}

		// 连接数量没有到达上限，继续创建新的连接
		if (_connectionCnt < _maxSize)
		{
			MySQL *p = new MySQL();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;
		}

		// 通知消费者线程，可以消费连接了
		cv.notify_all();
	}
}

// 给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<MySQL> ConnectionPool::getConnection()
{
	unique_lock<mutex> lock(_queueMutex);
	while (_connectionQue.empty())
	{
		// sleep
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty())
			{
				LOG_ERROR("获取空闲连接超时了...获取连接失败!");
					return nullptr;
			}
		}
	}

	/*
	shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于
	调用connection的析构函数，connection就被close掉了。
	这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue当中
	*/
	shared_ptr<MySQL> sp(_connectionQue.front(), 
		[&](MySQL *pcon) {
		// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
		unique_lock<mutex> lock(_queueMutex);
		pcon->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connectionQue.push(pcon);
	});

	_connectionQue.pop();
	cv.notify_all();  // 消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接
	
	return sp;
}

// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
void ConnectionPool::scannerConnectionTask()
{
	while (_isAlive)
	{
		// 通过sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));

		// 扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize)
		{
			MySQL *p = _connectionQue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p; // 调用~Connection()释放连接
			}
			else
			{
				break; // 队头的连接没有超过_maxIdleTime，其它连接肯定没有
			}
		}
	}
}

