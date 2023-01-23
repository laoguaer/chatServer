#include <iostream>
#include <thread>
using namespace std;
#include "connectionpool.h"


int main()
{
	clock_t begin;
	mysql_library_init(0, NULL, NULL);


// #if 0
{
	begin = clock();
	int cnt = 0;
	for (int i = 0; i < 1000; ++i)
	{
		MySQL conn;
		if (!conn.connect("127.0.0.1", 3306, "root", "123456", "chat")) {
			cnt++;
		}
	}
	printf("%d\n", cnt);
	std::cout << "1000 1thread without pool: " << (double)(clock() - begin) / CLOCKS_PER_SEC << endl;
}
// #endif 


// #if 0
{
	begin = clock();


	thread t[5];
	for (int i = 0; i < 5; ++i) {
		t[i] = thread([]() {
			for (int i = 0; i < 200; ++i) {
				MySQL conn;
				conn.connect();
			}
		});
	}
	for (int i = 0; i < 5; ++i) {
		if (t[i].joinable()) {
			t[i].join();
		}
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
		if (t[i].joinable()) {
			t[i].join();
		}
	}

	clock_t end = clock();
	std::cout << "1000 4thread with pool: " << (double)(end - begin) / CLOCKS_PER_SEC << endl;
}
// #endif
	mysql_library_end();
	return 0;
}



