#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodle.h"
#include "offlinemsgmodle.h"
#include "friendmodle.h"
#include "groupmodle.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
using namespace muduo;
using namespace muduo::net;

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

class ChatService {
public:
	static ChatService* instance();

	// 登录业务	
	void login(const TcpConnectionPtr &conn, json &js, Timestamp);
	// 注册业务
	void reg(const TcpConnectionPtr &conn, json &js, Timestamp);
	// 注销业务
	void logout(const TcpConnectionPtr &conn, json &js, Timestamp);
	// 点对点消息业务
	void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp);
	// 添加好友业务
	void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp);

	// 客服端异常退出
	void clientCloseException(const TcpConnectionPtr& conn);
	// 服务端异常退出
	void resetState();

	// 创建群组并自己是creator
	void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp);
	// 加入群组
	void addintoGroup(const TcpConnectionPtr &conn, json &js, Timestamp);
	// 发送群消息
	void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp);


	MsgHandler getHandler(int msgid);
private:
	ChatService();
	unordered_map<int, MsgHandler> _msgHandlerMap;
	unordered_map<int, TcpConnectionPtr> _userConnMap;
	mutex _connMutex;

	UserModle _usermodle;
	OfflineMsgModle _offlinemsgmodle;
	FriendModle _friendModle;
	GroupModle _groupmodle;
};

#endif