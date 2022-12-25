#include "chatserver.h"
#include "chatservice.h"
#include <string>
using namespace std;
using namespace placeholders;
#include "json.hpp"
using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop, 
		const InetAddress &listenAddr, 
		const string &nameArg) :
		_server(loop, listenAddr, nameArg),
		_loop(loop) 
{
	_server.setConnectionCallback(bind(&ChatServer::processConn, this, _1));
	_server.setMessageCallback(bind(&ChatServer::processMesg, this, _1, _2, _3));
	_server.setThreadNum(4);
}

void ChatServer::start() {
	_server.start();
}
void ChatServer::processConn(const TcpConnectionPtr &conn) {
	if (!conn->connected()) {
		ChatService::instance()->clientCloseException(conn);
		conn->shutdown();
	}
}
void ChatServer::processMesg(const TcpConnectionPtr &conn, 
							Buffer *buffer, 
							Timestamp time) 
{
	string buf = buffer->retrieveAllAsString();
	json js = json::parse(buf);
	
	auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
	msgHandler(conn, js, time);
}