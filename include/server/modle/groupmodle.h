#ifndef GROUPMODLE_H
#define GROUPMODLE_H

#include <vector>

#include "group.h"
#include "connectionpool.h"
#include "json.hpp"

using json = nlohmann::json;
class GroupModle {
public:
	// 创建群组
	bool createGroup(Group& group);
	// 加入群组
	void addIntoGroup(int groupid, int userid, string role);
	// 查询用户的群组信息
	void queryGroups(int userid, vector<string>& vec);
	// 查询群组的用户信息
	void queryUsers(int groupid, vector<GroupUser>& vec);
private:

};



#endif