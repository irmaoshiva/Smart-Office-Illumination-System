#include <thread>
#include <csignal>

#include "async_tcp_server.hpp"
#include "luminaire.hpp"
#define DEFAULT_PORT 17000


bool server_up = true;
luminaire office (LAST_DESK, SAMPLING_RATE*TIME_HOLDING);

//Ctrl^C signal handler
void terminate(int signalnum){
	office.resume_read();
	server_up = false;
	std::cout << "\n Press enter to properly finish." << std::endl;
}

//read data from the desks
void read_luminaire(){
	office.read_data(server_up);
}

void read_console(){
	std::string action = "";
	while(server_up){
		std::cout << "\nServer actions:\nstart | stop | restart | changeAddr | exit | help\n\n";
		std::cout << ">>";
		std::getline(std::cin, action);
		if (action == "start"){
			office.resume_read();
			continue;
		}else if (action == "stop"){
			office.stop_read();
			continue;
		}else if (action == "restart"){
			office.clear_luminaire();
			continue;
		}else if (action == "changeAddr"){
			std::cout << "\n Insert new address: ";
			std::string addr_ = "";
			getline(std::cin, addr_);
			try{
				int addr = std::stoi(addr_);
				office.change_slave_addr(addr);
			} catch(std::invalid_argument& e){
				std::cerr << "Invalid argument: " << e.what() << "\n";
			}
			continue;
		}else if (action == "help"){
			std::cout << "\nDetailed actions functionalities:\n"
							<< "start - starts reading and storing data from the office;\n"
							<< "stop - stops reading data from the luminaire and ditches acquired data;\n"
							<< "restart - ditches all acquired data from the office;\n"
							<< "changeAddr - change raspberrypi reading address;\n"
							<< "exit - shutdown the program.\n\n";
		}else if (action == "exit" || ! server_up){
			office.resume_read();
			break;
		}else
			std::cout << "\nInvalid action\n\n";
	}
	server_up = false;
}



int main(int argc, char* argv[]) {
	std::signal(SIGINT, terminate);
	std::signal(SIGSEGV, terminate);
	

	int port = DEFAULT_PORT;
	if (argc > 2){
		std::cerr << "Usage: ./server <port> \n"; 
		return 1;
	}else if (argc == 2)
		port = std::atoi(argv[1]);

	std::cout << "\nServer running on port " << port << std::endl;

	std::thread luminaireThread(read_luminaire);

	std::thread consoleThread(read_console);

	//launch tcp server to handle clients
	boost::asio::io_service io;
	server s(io, port);
	while(server_up)
		io.poll_one();

	consoleThread.join();
	luminaireThread.join();
	
	return 0;
}	