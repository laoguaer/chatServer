#ifndef USERMODLE_H
#define USERMODLE_H
#include "user.h"

class UserModle {
public:
	bool insert(User& user);
	User query(int id);
	bool updateState(User& user);
	void resetState();
};

#endif