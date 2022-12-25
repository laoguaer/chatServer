#include "friendmodle.h"
#include "user.h"
#include "json.hpp"
using namespace nlohmann;
void FriendModle::insert(int userid, int friendid) {
	char sql[1024];
	sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);
	
	MySQL mysql;
	if (mysql.connect()) {
		if (mysql.update(sql)) {

		}
	}
}

void  FriendModle::query(int userid, vector<string>& vec) {
	char sql[1024];
	sprintf(sql, "select a.id, a.name, a.state from user a inner join friend b on a.id = b.friendid where b.userid = %d", userid);
	
	MySQL mysql;
	if (mysql.connect()) {
		MYSQL_RES *res = mysql.query(sql);
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res)) != nullptr) {
			json js;
			js["id"] = atoi(row[0]);
			js["name"] = string(row[1]);
			js["state"] = string(row[2]);
			vec.push_back(js.dump());
		}
	}
}