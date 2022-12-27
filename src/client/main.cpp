#include <iostream>
#include "user.h"
#include "group.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <json.hpp>
#include "public.h"
#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
User g_curUser;
vector<User> g_friendList;
vector<Group> g_groupList;
vector<string> g_offlinemsgList;
bool isLogin = false;
mutex resMutex;
condition_variable cv;

void ShowCommonInfo();
void readTaskHandler(int clientfd);
void MainMenu(int clientfd);

int main(int argc, char **argv) {
	if (argc < 3) {
		cerr << "command invalid!  example 127.0.0.1 6000" << endl;
		exit(-1);
	}
	char *ip = argv[1];
	uint16_t port = atoi(argv[2]);

	int clientfd = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in server;
	memset(&server, 0, sizeof server);
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	

	if (-1 == connect(clientfd, (sockaddr*)&server, sizeof server)) {
		cerr << "connect failed!" << endl;
		close(clientfd);
		exit(-1);
	}
	thread readTask(readTaskHandler, clientfd);
	readTask.detach();
	for (;;) {
		cout << "================" << endl;
		cout << "1.login" << endl;
		cout << "2.register" << endl;
		cout << "3.quit" << endl;
		cout << "================" << endl;
		int choice;
		cin >> choice;
		cin.get();
		switch (choice)
		{
		case 1:
		{
			int id = 0;
			char pwd[50];
			cout << "id:";
			cin >> id;
			cin.get();
			cout << "password:";
			cin.getline(pwd, 50);

			json js;
			js["msgid"] = LOGIN_MSG;
			js["id"] = id;
			js["password"] = string(pwd);
			string res = js.dump();
			
			int len = send(clientfd, res.c_str(), res.size() + 1, 0);
			if (len == -1) {
				cerr << "send message erro: " << res << endl;
			}
		
			unique_lock<mutex> lock(resMutex);
			cv.wait(lock);
			if (isLogin) {
				ShowCommonInfo();
				while (isLogin) {
					MainMenu(clientfd);
				}
			}
		}
			break;
		case 2:
		{
			char name[20];
			char pwd[50];
			cout << "name:";
			cin.getline(name, 20);
			cout << "password:";
			cin.getline(pwd, 50);

			json js;
			js["msgid"] = REG_MSG;
			js["name"] = string(name);
			js["password"] = string(pwd);
			string res = js.dump();
			
			int len = send(clientfd, res.c_str(), res.size() + 1, 0);
			if (len != -1) {
				char buf[1024] = {0};
				len = recv(clientfd, buf, 1024, 0);
				if (len != -1) {
					json recvjs = json::parse(buf);
					if (recvjs["msgid"] == REG_MSG_ACK && recvjs["errno"] == 0) {
						int id = recvjs["id"];
						printf("register succeed! remeber your id: %d\n", id);
					} else {
						printf("register falied! account name already exists!\n");
					}
				}
			}
		}
			break;
		case 3:
			close(clientfd);
			exit(0);
			break;
		default:
			break;
		}
	}
}

void ShowCommonInfo() {
	printf("Hello %s ! Welcome to Come Back!\n", g_curUser.getName().c_str());
	printf("************\n");
	printf("friend list :");
	if (g_friendList.size() == 0) {
		printf(" 无");
	}
	printf("\n");
	for (int i = 0; i < g_friendList.size(); ++i) {
		printf("	name: %s   id: %d   stete: %s\n", 
			g_friendList[i].getName().c_str(), g_friendList[i].getId(), g_friendList[i].getState().c_str());
	}
	printf("group list:");
	if (g_groupList.size() == 0) {
		printf("无");
	}
	printf("\n");
	for (int i = 0; i < g_groupList.size(); ++i) {
		printf("	groupname: %s	groupid: %d	  groupbrief: %s\n",
			g_groupList[i].getGroupName().c_str(), g_groupList[i].getGroupId(), g_groupList[i].getGroupDesc().c_str());
	}
	printf("\n************\n");
	printf("离线消息:");
	if (g_offlinemsgList.size() == 0) {
		printf(" 无");
	}
	printf("\n");
	for (int i = 0; i < g_offlinemsgList.size(); i++) {
		json t = json::parse(g_offlinemsgList[i]);
		if (ONE_CHAT_MSG == t["msgid"]) {
			string name = t["name"];
			int fromid = t["from"];
			string msg = t["msg"];
			printf("  from user: %s  id: %d  msg: %s\n",
				name.c_str(), fromid, msg.c_str());
		} else if (GROUP_CHAT_MSG == t["msgid"]) {
			// string grpname = t["groupname"];
			int groupid = t["groupid"];
			string username = t["username"];
			string msg = t["msg"];
			printf("  from groupid: %d  user: %s msg: %s\n", 
				groupid, username.c_str(), msg.c_str());
		}
	}
}

void readTaskHandler(int clientfd) {
	for (;;) {
		char buf[1024] = {0};
		int len = recv(clientfd, buf, 1024, 0);
		if (len == 0 || len == -1) {
			close(clientfd);
			exit(-1);
		}
		json recvjs = json::parse(buf);
		if (ONE_CHAT_MSG == recvjs["msgid"]) {
			cout << "new user msg -- from user" << recvjs["name"] << " said: "<< recvjs["msg"] << endl;
		} else if (GROUP_CHAT_MSG == recvjs["msgid"]) {
			cout << "new group msg -- from group " << recvjs["groupname"] << " user : " << recvjs["username"] << recvjs["msg"] << endl;
		} else if (CREATE_GROUP_ACK == recvjs["msgid"]) {
			if (recvjs["errno"] != 0) {
				cout << "create group failed! reason : " << recvjs["errmsg"] << endl;
			} else {
				cout << "create group succeed! The GroupId is " << recvjs["groupid"] << endl;
			}
		} else if (LOGIN_MSG_ACK == recvjs["msgid"]) {
			
			if (recvjs["errno"] != 0) {
				cout << "login failed : " << recvjs["errmsg"] << endl;
				cv.notify_all();
				continue;
			}
			string name = recvjs["name"];
						
			g_curUser.setId(recvjs["id"]);
			g_curUser.setName(recvjs["name"]);
			g_curUser.setState("online");

			g_friendList.clear();
			g_groupList.clear();
			g_offlinemsgList.clear();
			
			vector<string> grouplist = recvjs["group"];
			vector<string> friendlist = recvjs["friend"];
			g_offlinemsgList = (vector<string>)recvjs["offlinemsg"];

			for (string& grp : grouplist) {
				json t = json::parse(grp);
				int id = t["groupid"];
				string name = t["groupname"];
				string desc = t["desc"];
				g_groupList.emplace_back(id, name, desc);
			}

			for (string &fri : friendlist) {
				json t = json::parse(fri);
				int id = t["id"];
				string name = t["name"];
				string state = t["state"];
				g_friendList.emplace_back(id, name, "", state);
			}
			isLogin = true;
			cv.notify_one();
		}
	}
}

void help(int fd = -1, string str = "");
void chat(int, string);
void addfriend(int, string);
void logout(int, string);
void groupmsg(int, string);
void creategroup(int, string);
void joingroup(int, string);

unordered_map<string, string> commandMap = {
	{"help", 		"显示所有命令 		格式 help"},
	{"addfriend", 	"通过id添加好友 	格式 addfriend:friendid"},
	{"chat", 		"与好友进行聊天		格式 chat:friendid:msg"},
	{"logout", 		"注销当前用户		格式 logout"},
	{"groupmsg",	"发布群组消息		格式 groupmsg:groupid:msg"},
	{"creategroup", "创建一个群聊	格式 creategroup:groupname:groupbreif"},
	{"joingroup", 	"加入一个群组		格式 joingroup:groupid"}
};

unordered_map<string, function<void(int, string)>> commandHandlerMap = {
	{"help", help},
	{"chat", chat},
	{"addfriend", addfriend},
	{"logout", logout},
	{"groupmsg", groupmsg},
	{"creategroup", creategroup},
	{"joingroup", joingroup}
};

void MainMenu(int clientfd) {
	help();
	char buf[1024] = {0};
	
	while (isLogin) {
		memset(buf, 0, 1024);
		cin.getline(buf, 1024);
		string cmdbuf(buf);
		string cmd = cmdbuf;
		int idx = cmdbuf.find(':');
		if (idx != -1) {
			cmd = cmdbuf.substr(0, idx);
		}
		auto it = commandHandlerMap.find(cmd);
		if (it == commandHandlerMap.end()) {
			cout << "command not found, try again!" << endl;
			continue;
		}
		it->second(clientfd, cmdbuf.substr(idx + 1));
	}
}



void help(int, string) {
	for (auto &item : commandMap) {
		printf(" %s:%s\n", item.first.c_str(), item.second.c_str());
	}
}

void addfriend(int clientfd, string cmd) {
	int id = stoi(cmd);
	json js;
	js["msgid"] = ADD_FRIEND_MSG;
	js["userid"] = g_curUser.getId();
	js["friendid"] = id;
	string buf = js.dump();
	int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
	if (len == -1) {
		cerr << "send msg erro : " << buf << endl;
	}
}
void chat(int clientfd, string cmd) {
	int pos = cmd.find(':');
	if (pos == -1) {
		printf("incerrect input. pls follow the format!\n");
		return;
	}
	int friendid = stoi(cmd.substr(0, pos));
	string msg = cmd.substr(pos + 1);
	json js;
	js["msgid"] = ONE_CHAT_MSG;
	js["from"] = g_curUser.getId();
	js["name"] = g_curUser.getName();
	js["to"] = friendid;
	js["msg"] = msg;
	string buf = js.dump();
	int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
	if (len == -1) {
		cerr << "send msg erro : " << buf << endl;
	}
}
void logout(int clientfd, string cmd) {
	json js;
	js["msgid"] = LOGOUT_MSG;
	js["id"] = g_curUser.getId();
	string buf = js.dump();
	int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
	if (len == -1) {
		cerr << "send msg erro : " << buf << endl;
		return;
	}
	isLogin = false;
}

void groupmsg(int clientfd, string cmd) {
	int idx = cmd.find(':');
	if (idx == -1) {
		printf("incerrect input. pls follow the format!\n");
		return;
	}
	int groupid = stoi(cmd.substr(0, idx));
	string msg = cmd.substr(idx + 1);
	json js;
	js["msgid"] = GROUP_CHAT_MSG;
	js["userid"] = g_curUser.getId();
	js["username"] = g_curUser.getName();
	js["groupid"] = groupid;
	js["msg"] = msg;
	string buf = js.dump();
	int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
	if (len == -1) {
		cerr << "send msg erro : " << buf << endl;
	}
}

void creategroup(int clientfd, string cmd) {
	json js;

	int idx = cmd.find(':');
	if (idx == -1) {
		printf("incerrect input. pls follow the format!\n");
		return;
	}
	string groupname = cmd.substr(0, idx);
	string breif = cmd.substr(idx + 1);
	
	js["msgid"] = CREATE_GROUP_MSG;
	js["userid"] = g_curUser.getId();
	js["username"] = g_curUser.getName();
	js["groupname"] = groupname;
	js["desc"] = breif;

	string buf = js.dump();
	int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
	if (len == -1) {
		cerr << "send msg erro : " << buf << endl;
	}
}
void joingroup(int clientfd, string cmd) {
	json js;

	int groupid = stoi(cmd);

	js["msgid"] = ADDINTO_GROUP_MSG;
	js["userid"] = g_curUser.getId();
	js["username"] = g_curUser.getName();
	js["groupid"] = groupid;

	string buf = js.dump();
	int len = send(clientfd, buf.c_str(), buf.size() + 1, 0);
	if (len == -1) {
		cerr << "send msg erro : " << buf << endl;
	}
}