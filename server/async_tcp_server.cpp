#include "async_tcp_server.hpp"

using namespace boost::asio;
using boost::system::error_code;


/************************
session class functions
*************************/
session::session(ip::tcp::socket s_) 
	: s(std::move(s_)) { }

ip::tcp::socket& session::socket() {return s;}

void session::start() {
	linked = 1;
	auto self(shared_from_this());
	s.async_read_some( buffer(data, max_len), 
		[this, self](const error_code &ec, std::size_t sz){
			if (!ec) 
				interpret_request();});
}

void session::write_back(const std::string& response, const std::size_t& sz){
	auto self(shared_from_this());
	async_write(s, buffer(response, sz), 
		[this, self](const error_code &ec, std::size_t sz){
			if (!ec) 
				start();});
}

int session::get_desk(){
	int desk = std::stoi(&data[4]); 
	if(desk > LAST_DESK) throw; 
	return desk;
}

void session::interpret_request(){
	std::string invalid = "invalid request\n";
	if (data[1] != ' ' || data[3] != ' '){
		write_back(invalid, invalid.length());
		return;
	}
	
	switch(data[0]){
		case 'g':
			switch(data[2]){
				case 'l':
					try{get_desk();}
					catch(std::invalid_argument& e){write_back(invalid, invalid.length());}
					return;
				case 'd':
					return;
				case 's':
					return;
				case 'L':
					return;
				case 'o':
					return;
				case 'r':
					return;
				case 'p':
					return;
				case 't':
					return;
				case 'e':
					return;
				case 'c':
					return;
				case 'v':
					return;
				default:
					write_back(invalid, invalid.length());
					return;
			}
			return;
		case 'r':
			return;
		case 'b':
			return;
		case 's':
			return;
		default :
			write_back(invalid, invalid.length());
			return;
	}
}


/************************
server class functions
*************************/
server::server(io_service& io, short port)
	: s(io), acc(io, ip::tcp::endpoint(ip::tcp::v4(), port)) 
{
	start_accept();
}

void server::start_accept() {
	acc.async_accept(s,
		[this](const error_code &ec){
			if (!ec) 
				std::make_shared<session>(std::move(s))->start();
			start_accept();});
}
