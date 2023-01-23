#ifndef OFFLINEMSGMODLE_H
#define OFFLINEMSGMODLE_H

#include <vector>
#include "connectionpool.h"

class OfflineMsgModle {
public:
	void insert(int userid, string msg);
	void remove(int userid);
	void query(int userid, vector<string>& vec);
};

#endif