#include <thread>
#include <csignal>

#include "async_tcp_server.hpp"
#define DEFAULT_PORT 17000
#include "luminaire.hpp"

/*
//Ctrl^C signal handler
void terminate(int signalnum){
	printf("FFEEEEEEZZZ\n");
	gpioTerminate();
	exit(signalnum);
}
*/


//-------------------------------------------------------------------
/*
void read_luminaire(luminaire& master){
	master.read_data();
	std::cout << "\nbscXfer() returned negative status.\n";
	exit(2);
}
*/
//------------------------------------------------------------------


//ESTA SUBUSTITUI A DE BAIXO -->> void read_console(luminaire& master){
void read_console(){
	std::string action = "";
	while(action != "exit"){
		std::cout << "\nServer actions:\nstart | stop | restart | changeAddr | exit | help\n\n";
		std::cout << ">>";
		getline(std::cin, action);
		if (action == "start"){
			//master.resume_read();
			continue;
		}
		if (action == "stop"){
			//master.stop_read();
			continue;
		}
		if (action == "restart"){
			//wipe all data read from the luminaire
			continue;
		}
		if (action != "exit" && action == "changeAddr"){
			std::cout << "\n Insert new address: ";
			std::string addr_ = "";
			getline(std::cin, addr_);
			try{
				int addr = std::stoi(addr_);
				//master.change_slave_addr(addr);
			} catch(std::invalid_argument& e){
				std::cerr << "Invalid argument: " << e.what() << "\n";
			}
			continue;
		}
		if (action == "help")
			std::cout << "\nDetailed actions functionalities:\n"
							<< "start - starts reading and storing data from the luminaire;\n"
							<< "stop - stops reading data from the luminaire and ditches acquired data;\n"
							<< "restart - ditches all acquired data from the luminaire;\n"
							<< "changeAddr - change raspberrypi reading address;\n"
							<< "exit - shutdown the program.\n\n";
		else
			std::cout << "\nInvalid action\n\n";
	}
	exit(0);
}



int main(int argc, char* argv[]) {

	/*
	std::signal(SIGINT, terminate_);
	std::signal(SIGSEGV, terminate);
	*/

	int port = DEFAULT_PORT;
	if (argc > 2){
		std::cerr << "Usage: server <port> \n"; 
		return 1;
	}else if (argc == 2)
		port = std::atoi(argv[1]);

	std::cout << "\nServer running on port " << port << std::endl;

//----------------------------------------------------------------------------
	//luminaire master;

	//std::thread luminaireThread(read_luminaire, std::ref(master));
	//std::thread consoleThread(read_console, std::ref(master));
//----------------------------------------------------------------------------	
	std::thread consoleThread(read_console);

	//launch tcp server to handle clients
	boost::asio::io_service io;
	server s(io, port);
	io.run();
	std::cout << "\ntasks queue emptied; io_service terminated run().\n" << std::endl;
	return -1;
}	

/*
int luminaire::close_slave(bsc_xfer_t &xfer){
xfer.control=0;
bscXfer(&xfer);
}
*/