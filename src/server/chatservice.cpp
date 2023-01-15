#include "chatservice.h"
#include "public.h"


// #include <muduo/base/Logging.h>
#include <mymuduo/logger.h>
#include <string>
// using namespace muduo;
using namespace std::placeholders;

ChatService* ChatService::instance() {
	static ChatService service;
	return &service;
}

ChatService::ChatService() {
	_msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
	_msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
	_msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
	_msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
	_msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
	_msgHandlerMap.insert({ADDINTO_GROUP_MSG, std::bind(&ChatService::addintoGroup, this, _1, _2, _3)});
	_msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
	_msgHandlerMap.insert({LOGOUT_MSG, std::bind(&ChatService::logout, this, _1, _2, _3)});

	if (_redis.connect()) {
		_redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
	}
}

MsgHandler ChatService::getHandler(int msgid) {
	auto it = _msgHandlerMap.find(msgid);
	if (it == _msgHandlerMap.end()) {
		return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
			// LOG_ERROR << "msgid: " << msgid << " can't find handler!";
			LOG_ERROR("msgid:%d can't find handler!", msgid);
		};
	} else {
		return _msgHandlerMap[msgid];
	}
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp) {
	int id = js["id"];
	string pwd = js["password"];

	User user = _usermodle.query(id);
	
	if (user.getId() == id && user.getPasswd() == pwd) {

		//用户已上线
		if (user.getState() == "online") {
			json res;
			res["msgid"] = LOGIN_MSG_ACK;
			res["errno"] = 2;
			res["errmsg"] = "该用户已经登录, 请重新输入账号";
			conn->send(res.dump());
			return;
		}

		//登录成功
		{
			lock_guard<mutex> lock(_connMutex);
			_userConnMap.insert({id, conn});
			_TcpConnMap.insert({conn, id});
		}

		user.setState("online");
		_usermodle.updateState(user);

		_redis.subscribe(user.getId());

		json res;
		vector<string> offlineMsgvec;
		vector<string> friendvec;
		vector<string> groupvec;
		_offlinemsgmodle.query(id, offlineMsgvec);
		_friendModle.query(id, friendvec);
		_groupmodle.queryGroups(id, groupvec);
		res["msgid"] = LOGIN_MSG_ACK;
		res["errno"] = 0;
		res["id"] = id;
		res["name"] = user.getName();
		res["offlinemsg"] = offlineMsgvec;
		res["friend"] = friendvec;
		res["group"] = groupvec;
		conn->send(res.dump());
		_offlinemsgmodle.remove(id);
	} else {
		// 登录失败
		json res;
		res["msgid"] = LOGIN_MSG_ACK;
		res["errno"] = 1;
		res["errmsg"] = "用户名或者密码错误";
		conn->send(res.dump());
	}
}

void ChatService::logout(const TcpConnectionPtr &conn, json &js, Timestamp) {
	User user;
	int id = js["id"];
	user.setId(id);
	// 查找 _userConnMap 中有无id
	{
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(id);
		if (it != _userConnMap.end()) {
			_userConnMap.erase(it);
			_TcpConnMap.erase(it->second);
		}
    }

	if (user.getId() != -1) {
		user.setState("offline");
		_usermodle.updateState(user);

		_redis.unsubscribe(user.getId());
	}
}
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp) {
	string name = js["name"];
	string pwd = js["password"];

	User user;
	user.setName(name);
	user.setPasswd(pwd);
	bool state = _usermodle.insert(user);
	if (state) {
		// 注册成功
		json res;
		res["msgid"] = REG_MSG_ACK;
		res["errno"] = 0;
		res["id"] = user.getId();
		conn->send(res.dump());
	} else {
		// 注册失败
		json res;
		res["msgid"] = REG_MSG_ACK;
		res["errno"] = 1;
		conn->send(res.dump());
	}
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp now) {
	int toid = js["to"];
	{
		// 对方在线 发送在线消息
		unique_lock<mutex> lock(_connMutex);
		auto it = _userConnMap.find(toid);
		if (it != _userConnMap.end()) {
			json temp = js;
			printf("oneChat:%s\n",temp.dump().c_str());
			// it->second->send(temp.dump());
			auto ptr = it->second;
			(*ptr).send(js.dump());
			return;
		}
	}
	User user = _usermodle.query(toid);
	if (user.getState() == "online") {
		_redis.publish(user.getId(), js.dump());
		return;
	}
		// 对方离线 发送离线消息
	_offlinemsgmodle.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp) {
	int userid = js["userid"];
	int friendid = js["friendid"];
	_friendModle.insert(userid, friendid);
}

//  客户端退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
	// LOG_INFO << "clientCLostException";
	LOG_INFO(" ");
	
	User user;
	{
        lock_guard<mutex> lock(_connMutex);
        auto it = _TcpConnMap.find(conn);
		if (it != _TcpConnMap.end()) {
			_TcpConnMap.erase(it);
			user.setId(it->second);
			_userConnMap.erase(it->second);
		}
    }

	if (user.getId() != -1) {
		user.setState("offline");
		_usermodle.updateState(user);

		_redis.unsubscribe(user.getId());
	}
}
void ChatService::resetState() {
	_usermodle.resetState();
}


// 创建群组并自己是creator 
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp) {
	int userid = js["userid"];
	string username = js["username"];
	string groupname = js["groupname"];
	string desc = js["desc"];
	Group group;
	group.setGroupDesc(desc);
	group.setGroupName(groupname);
	json res;
	if (_groupmodle.createGroup(group)) {
		res["errno"] = 0;
	} else {
		res["errno"] = 1;
		res["errmsg"] = "Group Name already exits!";
	}
	_groupmodle.addIntoGroup(group.getGroupId(), userid, "creator");
	res["msgid"] = CREATE_GROUP_ACK;
	res["groupid"] = group.getGroupId();
	conn->send(res.dump());
}
// 加入群组
void ChatService::addintoGroup(const TcpConnectionPtr &conn, json &js, Timestamp) {
	int groupid = js["groupid"];
	int userid = js["userid"];
	_groupmodle.addIntoGroup(groupid, userid, "normal");
}
// 发送群消息
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp) {
	vector<GroupUser> userVec;
	int groupid = js["groupid"];
	_groupmodle.queryUsers(groupid, userVec);

	lock_guard<mutex> lock(_connMutex);
	for (auto &user : userVec) {
		auto it = _userConnMap.find(user.getId());
		if (it == _userConnMap.end()) {

			// 发送离线消息
			if (user.getState() == "online") {
				_redis.publish(user.getId(), js.dump());
				return;
			}
			_offlinemsgmodle.insert(user.getId(), js.dump());
		} else {
			// 发送在线消息
			it->second->send(js.dump());
		}
	}
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlinemsgmodle.insert(userid, msg);
}