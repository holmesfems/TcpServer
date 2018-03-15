/*
* tcpServer.cpp
* Created at 2017/06/25
* Copyright (C) 2017 zhai <holmesfems@gmail.com>
*
* Distributed under terms of the MIT license.
*/
#include "tcpServer.h"
#include <sstream>

namespace TcpServer
{
	TcpServer::TcpServer(boost::asio::io_service &io_service, uint16_t port, std::ostream *os)
		: _io_service(io_service),
		_socket(io_service),
		_os(os),
		_port(port),
		_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	{
		if (_os == NULL) //dummy
			_os = new std::ostringstream();
	}

	void TcpServer::start_accept()
	{
		_acceptor.async_accept(
			_socket,
			boost::bind(&TcpServer::_on_accept, this, boost::asio::placeholders::error));
	}

	void TcpServer::send(std::string msg)
	{
		_msgQueue.push(msg + "\n");
		_io_service.post(boost::bind(&TcpServer::_async_write, this));
	}

	void TcpServer::_on_accept(const boost::system::error_code &err)
	{
		if (err)
		{
			*_os << "Failed to accept : " << err.message() << std::endl;
			status = FAIL;
		}
		else
		{
			*_os << "Connection accepted" << std::endl;
			status = ONLINE;
			_async_receive();
		}
	}

	void TcpServer::_async_receive()
	{
		boost::asio::async_read_until(
			_socket,
			_receive_buff,
			"\n",
			boost::bind(&TcpServer::_on_receive, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	void TcpServer::_on_receive(const boost::system::error_code &err, size_t bytes_transferred)
	{
		std::cout << "now status = " << status << std::endl;
		std::cout.flush();
		if (status == EXIT) return;
		if (err && err != boost::asio::error::eof) {
			*_os << "receive failed: " << err.message() << std::endl;
			status = FAIL;
			//start_accept();
		}
		else {
			*_os << "recieve succeed " << "length = " << bytes_transferred << std::endl;
			std::string data = std::string(boost::asio::buffer_cast<const char*>(_receive_buff.data()), bytes_transferred);
			data = data.substr(0, data.length() - 1);
			*_os << "cmd = \"" << data << "\"" << std::endl;
			if (bytes_transferred == 0 && err == boost::asio::error::eof)
			{
				*_os << "connection lost!" << std::endl;
				if (status != EXIT)
				{
					status = LOST;
					//start_accept();
				}
//				_on_offline();
			}
			else
			{
				if (!(data == "exit"))
				{
					_receive_buff.consume(_receive_buff.size());
					_async_receive();
				}
				else
				{
					status = EXIT;
				}
			}
			_os->flush();
		}
	}

	void TcpServer::_async_write()
	{
		if (_msgQueue.empty()) return;
		std::string msg = _msgQueue.front();
		_msgQueue.pop();
		boost::asio::async_write(
			_socket,
			boost::asio::buffer(msg.c_str(), msg.length()),
			boost::bind(&TcpServer::_on_write, this,
				boost::asio::placeholders::error));
	}

	void TcpServer::_on_write(const boost::system::error_code & err)
	{
		if (err)
		{
			*_os << "write failed" << std::endl;
		}
		else
		{
			*_os << "write succeed" << std::endl;
			if (!_msgQueue.empty())
				_async_write();
		}
	}
}