#include <iostream>
#include <thread>
using namespace std;
#include "connectionpool.h"

int main()
{
	clock_t begin;


// #if 0
{
	begin = clock();
	for (int i = 0; i < 1000; ++i)
	{
		MySQL conn;
		conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
	}
	std::cout << "1000 1thread without pool: " << (double)(clock() - begin) / CLOCKS_PER_SEC << endl;
}
// #endif 


// #if 0
{
	begin = clock();


	thread t[16];
	for (int i = 0; i < 5; ++i) {
		MySQL conn;
		conn.connect();
		t[i] = thread([]{
			int cnt = 0;
			for (int j = 0; j < 200; j++) {
				MySQL conn;
				if (conn.connect()) {
					++cnt;
				}
			}
			printf("%d\n", cnt);
		});
	}
	for (int i = 0; i < 16; ++i) {
		t[i].join();
	}

	clock_t end = clock();
	cout << "1000 4thread without pool: " << (double)(end - begin) / CLOCKS_PER_SEC << endl;
}
// #endif

// #if 0
	begin = clock();
	ConnectionPool *ins = ConnectionPool::instance();
	for (int i = 0; i < 1000; ++i) {
		auto conn = ins->getConnection();
	}
	cout << "1000 1thread with pool: " << (double)(clock() - begin) / CLOCKS_PER_SEC << endl;	
// #endif

// #if 0
{
	begin = clock();
	auto ins = ConnectionPool::instance();
	thread t[4];
	for (int i = 0; i < 4; ++i) {
		t[i] = thread([&]{
			for (int j = 0; j < 250; ++j) {
				auto conn = ins->getConnection();
			}
		});
	}
	for (int i = 0; i < 4; ++i) {
		t[i].join();
	}

	clock_t end = clock();
	std::cout << "1000 4thread with pool: " << (double)(end - begin) / CLOCKS_PER_SEC << endl;
}
// #endif
	return 0;
}



