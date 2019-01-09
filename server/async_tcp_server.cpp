#include "async_tcp_server.hpp"

using namespace boost::asio;
using boost::system::error_code;

extern luminaire office;

/************************
session class functions
*************************/
session::session(ip::tcp::socket s_) 
	: s(std::move(s_)), lstream_up(LAST_DESK + 1, false), dstream_up(LAST_DESK + 1, false) { }

ip::tcp::socket& session::socket() {return s;}

void session::start() {
	linked = 1;
	auto self(shared_from_this());
	s.async_read_some( buffer(data, max_len), 
		[this, self](const error_code &ec, std::size_t sz){
			if (!ec) 
				interpret_request();
			else{//if any live stream, stop it
				std::fill(lstream_up.begin(), lstream_up.end(), false);
				std::fill(dstream_up.begin(), dstream_up.end(), false);
			}
		}
	);
}

void session::send_reply(std::string& response){
	response += "\n";

	auto self(shared_from_this());
	async_write(s, buffer(response, response.length()), 
		[this, self](const error_code &ec, std::size_t sz){
			if (!ec) 
				start();
			else{//if any live stream, stop it
				std::fill(lstream_up.begin(), lstream_up.end(), false);
				std::fill(dstream_up.begin(), dstream_up.end(), false);
			}
		}
	);
}

//send current value and restart reading requests
void session::begin_stream(int desk, char type, std::chrono::time_point<std::chrono::system_clock> start, float (luminaire::*get_value)(int)){
	float result = (office.*get_value)(desk);
	
	auto current = std::chrono::system_clock::now();
	std::chrono::duration<float> duration = current - start;
	float time_ = duration.count() * 1000;
	
	std::string response = "s ";
	response += type;
	response += ' ';
	response += std::to_string(desk + 1);
	response += ' ';
	response += std::to_string(result);
	response += ' ';
	response += std::to_string(time_);
	
	send_reply(response);
}

//stream cycle
void session::send_stream(int desk, char type, std::vector<bool>& stream_up, std::chrono::time_point<std::chrono::system_clock> start, float (luminaire::*get_value)(int)){
	float result = (office.*get_value)(desk);
	if (result < 0){
		stream_up[desk] = false;
		std::string s_ = "desk down";
		send_reply(s_);
	}
	
	auto current = std::chrono::system_clock::now();
	std::chrono::duration<float> duration = current - start;
	float time_ = duration.count() * 1000;
	
	std::string response = "s ";
	response += type;
	response += ' ';
	response += std::to_string(desk + 1);
	response += ' ';
	response += std::to_string(result);
	response += ' ';
	response += std::to_string(time_);
	response += '\n';

	if (stream_up[desk]){
		auto self(shared_from_this());
		async_write(s, buffer(response, response.length()), 
			[this, self, desk, type, start, get_value, &stream_up]
			(const error_code &ec, std::size_t sz){
				if (! ec)
					send_stream(desk, type, stream_up, start, get_value);
			}
		);
	}
}

int session::get_desk(){
	int desk = std::stoi(&data[4]) - 1;
	if(desk > LAST_DESK || desk < 0) throw std::invalid_argument(" OR desk doesn't exist"); 
		return desk;
}

void session::interpret_request(){
	std::string invalid = "invalid request";
	if (data[0] == 'r' && data[1] == '\n'){
		//restartsystem...------......
		std::string s_ = "System restart not implemented :c";
		send_reply(s_);
		return;
	}
	if (data[1] != ' ' || data[3] != ' '){
		send_reply(invalid);
		return;
	}

	int desk, end_desk;
	std::string response = "";
	response += data[2];
	if (data[4] != 'T'){
		try{desk = get_desk();}
		catch(std::invalid_argument& e){
			std::string invalid_ = invalid + e.what();
			send_reply(invalid_);
			return;
		}
		end_desk = desk;
		response += ' ';
		response += std::to_string(desk + 1);
		response += ' ';
	}else{
		desk = 0;
		end_desk = LAST_DESK;
		response += " T ";
	}
	
	switch(data[0]){
		//-------------------STANDART PARAMETERS---------------------
		case 'g':{
			switch(data[2]){
				case 'l':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					float lux = office.get_lux(desk);
					response = response + std::to_string(lux) + '\n';
					send_reply(response);
				break;
				}
				case 'd':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					float duty_cycle = office.get_duty_cycle(desk);
					response = response + std::to_string(duty_cycle) + '\n';
					send_reply(response);
				break;
				}
				case 's':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					bool state = office.get_occupancy(desk);
					response = response + std::to_string(state) + '\n';
					send_reply(response);
				break;
				}
				case 'L':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					float lower_bound = office.get_lower_bound(desk);
					response = response + std::to_string(lower_bound) + '\n';
					send_reply(response);
				break;
				}
				case 'o':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					float ext_lux = office.get_ext_lux(desk);
					response = response + std::to_string(ext_lux) + '\n';
					send_reply(response);
				break;
				}
				case 'r':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					float control_ref = office.get_control_ref(desk);
					response = response + std::to_string(control_ref) + '\n';
					send_reply(response);
				break;
				}
				case 'p':{
					float power = 0;
					for(; desk <= end_desk; desk++)
						power += office.get_power(desk);

					response = response + std::to_string(power) + '\n';
					send_reply(response);
				break;
				}
				case 't':{
					if (data[4] == 'T'){
						send_reply(invalid);
						return;
					}
					float elapsed_time = office.get_restart_time(desk);
					response = response + std::to_string(elapsed_time) + '\n';
					send_reply(response);
				break;
				}
				case 'e':{
					float energy = 0;
					for(; desk <= end_desk; desk++)
						energy += office.get_energy(desk);
					
					response = response + std::to_string(energy) + '\n';
					send_reply(response);
				break;
				}
				case 'c':{
					float comfort = 0;
					for(; desk <= end_desk; desk++)
						comfort += office.get_comfort(desk);
					
					response = response + std::to_string(comfort) + '\n';
					send_reply(response);
				break;
				}
				case 'v':{
					float comfort_flicker = 0;
					for(; desk <= end_desk; desk++)
						comfort_flicker += office.get_comfort_flicker(desk);
					
					response = response + std::to_string(comfort_flicker) + '\n';
					send_reply(response);
				break;
				}
				default:{
					send_reply(invalid);
				break;
				}
			}
		break;
		}
		//-------------------LAST MINUTE BUFFERS---------------------
		case 'b':{
			std::string b_response = "b ";
			response += data[2];
			response += ' ';
			switch(data[2]){
				case 'l':{
					std::vector<float> l_holder;
					office.get_lux_holder(desk, l_holder);
					for (unsigned int i = 0; i < l_holder.size(); i ++)
						b_response = b_response + ',' + '\n' + std::to_string(l_holder[i]);
					send_reply(b_response);
				break;
				}
				case 'd':{
					std::vector<float> d_holder;
					office.get_duty_cycle_holder(desk, d_holder);
					for (unsigned int i = 0; i < d_holder.size(); i ++)
						b_response = b_response + ',' + '\n' + std::to_string(d_holder[i]);
					send_reply(b_response);
				break;
				}
				default:{
					send_reply(invalid);
				break;
				}
			}
		break;
		}
		//-----------------------STREAMS--------------------------------
		case 's':{
			auto start = std::chrono::system_clock::now();
			switch(data[2]){
				case 'l':{
					if (! lstream_up[desk]){
						lstream_up[desk] = true;
						begin_stream(desk, data[2], start, &luminaire::get_lux);
						send_stream(desk, data[2], lstream_up, start, &luminaire::get_lux_on_change);
					}else{
						lstream_up[desk] = false;
						std::string ack = "ack";
						send_reply(ack);
					}
				break;
				}
				case 'd':{
					if (! dstream_up[desk]){
						dstream_up[desk] = true;
						begin_stream(desk, data[2], start, &luminaire::get_duty_cycle);
						send_stream(desk, data[2], dstream_up, start, &luminaire::get_duty_cycle_on_change);
					}else{
						dstream_up[desk] = false;
						std::string ack = "ack";
						send_reply(ack);
					}
				break;
				}
				default:{
					send_reply(invalid);
				break;
				}
			}	
		break;
		}
		//--------------------------NO MORE OPTIOS-------------------------
		default :{
			send_reply(invalid);
		break;
		}
	}
return;
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
