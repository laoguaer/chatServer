#ifndef FRIENDMODLE_H
#define FRIENDMODLE_H
#include <vector>
#include <string>
#include "db.h"
using namespace std;

class FriendModle {
public:
	void insert(int userid, int friendid);
	void query(int userid, vector<string>& vec);
};


#endif