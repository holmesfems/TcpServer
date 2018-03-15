#include <iostream>
#include <thread>
#include "tcpServer.h"

uint16_t defaultPort = 5025;

int makeServer()
{
	std::string msg;
	boost::asio::io_service io_service;
	TcpServer::TcpServer server(io_service,defaultPort,&std::cout);
	server.start_accept();
	std::thread thread
	(
		[&io_service]()
	{
		io_service.run();
	});
	while (true)
	{
		std::string msg;
		std::getline(std::cin, msg);
		server.send(msg);
		if (msg == "exit") break;
	}
	std::cout << "server exit" << std::endl;
	thread.join();
	return 0;
}

int initialize()
{
	return 0;
}

int main(int argc, char*argv[])
{
	initialize();
	makeServer();
    return 0;
}