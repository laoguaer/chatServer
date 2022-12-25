#include "groupmodle.h"

// 创建群组
bool GroupModle::createGroup(Group& group) {
	char sql[1024];
	sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
		group.getGroupName().c_str(), group.getGroupDesc().c_str());
	MySQL mysql;
	if (mysql.connect()) {
		if (!mysql.update(sql)) {
			return false;
		}
		group.setGroupId(mysql_insert_id(mysql.getCurConn()));
	}
	return true;
}
// 加入群组
void GroupModle::addIntoGroup(int groupid, int userid, string role) {
	char sql[1024];
	sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());
	MySQL mysql;
	if (mysql.connect()) {
		mysql.update(sql);
	}
}
// 查询用户的群组信息
void GroupModle::queryGroups(int userid, vector<string>& vec) {
	char sql[1024];
	sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a inner join groupuser b on b.groupid = a.id where b.userid = %d", userid);
	MySQL mysql;
	if (mysql.connect()) {
		MYSQL_RES *res = mysql.query(sql);
		MYSQL_ROW row;
		Group group;
		while ((row = mysql_fetch_row(res)) != nullptr) {
			group.setGroupId(atoi(row[0]));
			group.setGroupName(row[1]);
			group.setGroupDesc(row[2]);
			json res;
			res["groupid"] = group.getGroupId();
			res["desc"] = group.getGroupDesc();
			res["groupname"] = group.getGroupName();
			vec.push_back(res.dump());
		}
		mysql_free_result(res);
	}
}
// 查询群组的用户信息
void GroupModle::queryUsers(int groupid, vector<GroupUser>& vec) {
	MySQL mysql;
	if (mysql.connect()) {
		char sql[1024];
		sprintf(sql, "select b.id, b.name, b.state , a.grouprole from groupuser a inner join user b on a.userid = b.id where a.groupid = %d", groupid);
		MYSQL_RES *res = mysql.query(sql);
		MYSQL_ROW row;
		GroupUser user;
		while ((row = mysql_fetch_row(res)) != nullptr) {
			user.setId(atoi(row[0]));
			user.setGroupId(groupid);
			user.setName(row[1]);
			user.setState(row[2]);
			user.setGroupRole(row[3]);
			vec.push_back(user);
		}
		mysql_free_result(res);
	}
}