#include "chatserver.h"
#include <iostream>
#include "signal.h"
#include "chatservice.h"

void resetHandler(int) {
	ChatService::instance()->resetState();
	exit(0);
}

int main(int argc, char **argv) {
	assert(argc == 3);
	signal(SIGINT, resetHandler);
	int port = atoi(argv[2]);
	EventLoop loop;
	InetAddress addr(argv[1], port);
	ChatServer server(&loop, addr, "chatserver");
	server.start();
	loop.loop();
	return 0;
}