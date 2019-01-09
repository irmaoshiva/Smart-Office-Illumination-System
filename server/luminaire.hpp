#ifndef LUMINAIRE
#define LUMINAIRE

#include "deskDB.hpp"

#include <string>
#include <unistd.h>
#include <memory.h>
#include <chrono>
#include <condition_variable>
#include <stdio.h>
#include <iostream>
#include <pigpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#define CONV_VAR_TIMEOUT 30
#define SLAVE_ADDR_DEFAULT 0x48


class luminaire{
	bsc_xfer_t xfer;
	int status;
	bool read_state = 1;
	std::mutex mtx;

	int last_desk;
	std::vector<std::shared_ptr<deskDB>> desksDB;
	//to notify streaming
	std::vector<std::condition_variable> cv;
	std::vector<std::mutex> stream_mtx;

public:
	luminaire(int last_desk_, int samples_holder);
	~luminaire();
	//read from i2C functions
	void make_read(int addr1);
	void read_sample(int addr1, int desk);
	void read_occupancy(int addr1, bool state);
	void read_reaclib(int addr1, int desk);
	void read_data(bool& server_up);
	void stop_read();
	void resume_read();
	void change_slave_addr(int addr);
	//desksDB access
	float get_lux(int desk);
	float get_lux_on_change(int desk);
	float get_duty_cycle(int desk);
	float get_duty_cycle_on_change(int desk);
	bool get_occupancy(int desk);
	float get_lower_bound(int desk);
	float get_ext_lux(int desk);
	float get_control_ref(int desk);
	float get_power(int desk);
	float get_restart_time(int desk);
	float get_energy(int desk);
	float get_comfort(int desk);
	float get_comfort_flicker(int desk);
	void get_lux_holder(int desk, std::vector<float>& holder);
	void get_duty_cycle_holder(int desk, std::vector<float>& holder);
	void insert_sample(int desk, float lux, float duty_cycle);
	void set_occupancy(int desk, bool state, float control_ref);
	void set_parameters(int desk, float lower_bound_off, float lower_bound_on, float ext_lux);
	void clear_desk(int desk);
	void clear_luminaire();

private:
	//pgpio related
	int init_slave(bsc_xfer_t &xfer, int addr);
	void close_slave(bsc_xfer_t &xfer);

};

#endif //LUMINAIRE
