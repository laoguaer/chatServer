#include "usermodle.h"
#include "db.h"

bool UserModle::insert(User& user) {
	char sql[1024];
	sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",
		user.getName().c_str(), user.getPasswd().c_str(), user.getState().c_str());
	
	MySQL mysql;
	if (mysql.connect()) {
		if (mysql.update(sql)) {
			user.setId(mysql_insert_id(mysql.getCurConn()));
			return true;
		}
	}
	return false;
}

User UserModle::query(int id) {
	char sql[1024];
	
	sprintf(sql, "select * from user where id = %d", id);

	MySQL mysql;
	
	User user;
	if (mysql.connect()) {
		MYSQL_RES *res = mysql.query(sql);
		if (res != nullptr) {
			auto row = mysql_fetch_row(res);
			if (row != nullptr) {
				user.setId(atoi(row[0]));
				user.setName(row[1]);
				user.setPasswd(row[2]);
				user.setState(row[3]);
				mysql_free_result(res);
				return user;
			}
		}
	}
	return user;
}

bool UserModle::updateState(User& user) {
	char sql[1024];
	sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());
	
	MySQL mysql;
	if (mysql.connect()) {
		if (mysql.update(sql)) {	
			
			return true;
		}
	}
	return false; 
}

void UserModle::resetState() {
	char sql[1024] = "update user set state = 'offline'";
	MySQL mysql;
	if (mysql.connect()) {
		mysql.update(sql);
	}
}