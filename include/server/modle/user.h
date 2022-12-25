#ifndef USER_H
#define USER_H
#include <string>
using namespace std;

class User {
public:

	User(int id = -1, string name = "", string passwd = "", string state = "offline") 
		: _id(id), _name(name), _passwd(passwd), _state(state) {}

	int getId() {
		return _id;
	}
	string getName() {
		return _name;
	}
	string getPasswd() {
		return _passwd;
	}
	string getState() {
		return _state;
	}

	void setId(int id) {
		_id = id;
	}
	void setName(string name) {
		_name = name;
	}
	void setPasswd(string passwd) {
		_passwd = passwd;
	}
	void setState(string state) {
		_state = state;
	}
	
protected:
	int _id;
	string _name;
	string _passwd;
	string _state;
};
#endif