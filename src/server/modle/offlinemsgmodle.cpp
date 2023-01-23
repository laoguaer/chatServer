#include "offlinemsgmodle.h"

void OfflineMsgModle::insert(int userid, string msg) {
	char sql[1024];
	sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg.c_str());

	// MySQL mysql;
	auto ins = ConnectionPool::instance();
	auto mysql = ins->getConnection();
	if (true) {
		mysql->update(sql);
		printf("--------Offline-------\n");
	}
}
void OfflineMsgModle::remove(int userid) {
	char sql[1024];
	sprintf(sql, "delete from offlinemessage where userid = %d", userid);

	// MySQL mysql;
	auto ins = ConnectionPool::instance();
	auto mysql = ins->getConnection();
	if (true) {
		mysql->update(sql);
	}
}
void OfflineMsgModle::query(int userid, vector<string>& vec) {
	char sql[1024];
	sprintf(sql, "select message from offlinemessage where userid = %d", userid);

	// MySQL mysql;
	auto ins = ConnectionPool::instance();
	auto mysql = ins->getConnection();
	if (true) {
		MYSQL_RES *res = mysql->query(sql);
		MYSQL_ROW row;
		while ((row = mysql_fetch_row(res)) != nullptr) {
			vec.push_back(row[0]);
		}
		mysql_free_result(res);
	}
}
