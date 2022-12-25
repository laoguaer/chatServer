#include "chatservice.h"
#include "public.h"
#include <muduo/base/Logging.h>
using namespace muduo;


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
}

MsgHandler ChatService::getHandler(int msgid) {
	auto it = _msgHandlerMap.find(msgid);
	if (it == _msgHandlerMap.end()) {
		return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
			LOG_ERROR << "msgid: " << msgid << " can't find handler!";
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
		}

		user.setState("online");
		_usermodle.updateState(user);

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
	{
        lock_guard<mutex> lock(_connMutex);
		
        auto it = _userConnMap.find(id);
		if (it != _userConnMap.end()) {
			_userConnMap.erase(it);
		}
    }

	if (user.getId() != -1) {
		user.setState("offline");
		_usermodle.updateState(user);
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

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp) {
	int toid = js["to"];
	{
		// 对方在线 发送在线消息
		lock_guard<mutex> lock(_connMutex);
		auto it = _userConnMap.find(toid);
		if (it != _userConnMap.end()) {
			it->second->send(js.dump());
			return;
		}
	}
		// 对方离线 发送离线消息
	_offlinemsgmodle.insert(toid, js.dump());
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp) {
	int userid = js["userid"];
	int friendid = js["friendid"];
	_friendModle.insert(userid, friendid);
}


void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
	User user;
	{
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
				user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

	if (user.getId() != -1) {
		user.setState("offline");
		_usermodle.updateState(user);
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
			printf("离线消息\n");
			_offlinemsgmodle.insert(user.getId(), js.dump());
		} else {
			// 发送在线消息
			printf("在线消息\n");
			it->second->send(js.dump());
		}
	}
}