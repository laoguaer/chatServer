#ifndef CHATSERVER_H
#define CHATSERVER_H

// #include <muduo/net/EventLoop.h>
// #include <muduo/net/TcpServer.h>
// using namespace muduo;
// using namespace muduo::net;
#include <mymuduo/TcpServer.h>
#include <string>
using namespace std;

class ChatServer {
public:
	ChatServer(EventLoop* loop,
		const InetAddress& listenAddr,
		const string& nameArg);

	void start();
private:
	void processConn(const TcpConnectionPtr &);
	void processMesg(const TcpConnectionPtr &, Buffer *, Timestamp);
	TcpServer _server;
	EventLoop *_loop;
};

#endif