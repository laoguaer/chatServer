#ifndef GROUP_H
#define GROUP_H
#include "user.h"

class GroupUser;

class Group
{
public:
	Group(int id = -1, string name = "", string desc = "") : _groupId(id), _groupDesc(desc), _groupName(name) {}

	int getGroupId() {
		return _groupId;
	}
	string getGroupName() {
		return _groupName;
	}
	string getGroupDesc() {
		return _groupDesc;
	}
	
	void setGroupId(int id) {
		_groupId = id;
	}
	void setGroupName(string name) {
		_groupName = name;
	}
	void setGroupDesc(string desc) {
		_groupDesc = desc;
	}
private:
	int _groupId;
	string _groupName;
	string _groupDesc;
};

class GroupUser : public User {
public:
	int getGroupId() {
		return _groupId;
	}
	string getGroupRole() {
		return _groupRole;
	}
	void setGroupId(int id) {
		_groupId = id;
	}
	void setGroupRole(string role) {
		_groupRole = role;
	}
private:
	int _groupId;
	string _groupRole;
};



#endif
