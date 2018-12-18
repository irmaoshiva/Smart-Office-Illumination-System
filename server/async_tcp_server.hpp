#ifndef TCP_SERVER
#define TCP_SERVER

#include "luminaire.hpp"
#include <atomic>//POR NO LUMINAIRE/DB SE FOR PRECISO
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "defs.h"

using namespace boost::asio;
using boost::system::error_code;

class session : public std::enable_shared_from_this<session> {
	ip::tcp::socket s;
	enum { max_len = 1024 };
	char data[max_len];
	int linked = 0; //is? connected

	std::atomic<bool> lstream_up{false};
	std::atomic<bool> dstream_up{false};
	
public:
	session(ip::tcp::socket s_);
	ip::tcp::socket& socket();
	void start();
	~session(){ if(linked) printf("\n\nA client left\n\n");
				else 	   printf("\n\nA client failed to connect\n\n");};

private:
	int get_desk();
	void interpret_request();
	void send_reply(std::string& response);
	//void begin_stream(int desk, char type, std::function<float(int)> get_value);
	//void send_stream(int desk, char type, std::atomic<bool> &stream_up, std::function<float(int)> get_value);
	void begin_stream(int desk, char type, std::chrono::time_point<std::chrono::system_clock> start, float (luminaire::*get_value)(int));
	void send_stream(int desk, char type, std::atomic<bool> &stream_up, std::chrono::time_point<std::chrono::system_clock> start, float (luminaire::*get_value)(int));
};

class server {
	ip::tcp::socket s;
	ip::tcp::acceptor acc;

public:
	server(io_service& io, short port);

private:
	void start_accept();
};

#endif //TCP_SERVER