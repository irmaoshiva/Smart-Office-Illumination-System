#ifndef TCP_SERVER
#define TCP_SERVER

#include <iostream>
#include <boost/asio.hpp>
#define LAST_DESK 126

using namespace boost::asio;
using boost::system::error_code;

class session : public std::enable_shared_from_this <session>{
	ip::tcp::socket s;
	enum { max_len = 1024 };
	char data[max_len];
	int linked = 0; //is? connected
	
public:
	session(ip::tcp::socket s_);
	ip::tcp::socket& socket();
	void start();
	~session(){ if(linked) printf("\n\nA client left\n\n");
				else 	   printf("\n\nA client failed to connect\n\n");};

private:
	void write_back(const std::string& response, const std::size_t& sz);
	void interpret_request();
	int get_desk();
};

class server {
	ip::tcp::acceptor acc;
	ip::tcp::socket s;

public:
	server(io_service& io, short port);

private:
	void start_accept();
};

#endif //TCP_SERVER