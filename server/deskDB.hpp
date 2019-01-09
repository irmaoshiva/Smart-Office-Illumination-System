#ifndef DESK_DATABASE
#define DESK_DATABASE

#include <cmath>
#include <vector>
#include <iostream>
#include <boost/circular_buffer.hpp>
#include <mutex>
#include <chrono>

#define POWER 1

class deskDB{
	boost::circular_buffer<float> lux; // measured lux values
	boost::circular_buffer<float> duty_cycle; // imposed duty cycles
	unsigned int buff_size; // lux and duty cycles memory
	bool occupancy; // is? occupied
	float lower_bound[2] = {0}; // minimal lux desired (0 - unoccupied, 1 - occupied)
	float ext_lux = 0; // external lux affecting the desk
	float control_ref; // luminance control reference
	std::chrono::time_point<std::chrono::system_clock> restart_time; // time of last restart event
	std::chrono::time_point<std::chrono::system_clock> last_sample_time; // time of last acquired sample
	float energy = 0; // accumulated energy consumption
	unsigned long long int samples_nr = 0; // number of samples acquired since last restart
	float comfort = 0; // accumulated confort error (not normalized by samples_nr)
	float comfort_flicker = 0; // acccumulated comfort flicker (not normalized by samples_nr)
	std::mutex mtx[3]; //restric multiple access to values in memory

public:
	deskDB(int samples_holder);
	float get_lux();
	float get_duty_cycle();
	bool get_occupancy();
	float get_lower_bound();
	float get_ext_lux();
	float get_control_ref();
	float get_power();
	float get_restart_time();
	float get_energy();
	float get_comfort();
	float get_comfort_flicker();
	void get_lux_holder(std::vector<float>& holder);
	void get_duty_cycle_holder(std::vector<float>& holder);
	void insert_sample(float lux_, float duty_cycle_);
	void set_occupancy(bool occupancy_, float control_ref_);
	void set_parameters(float lower_bound_off, float lower_bound_on, float ext_lux_);
	void clearDB();

private:
	int count_samples_held(); // CAN ONLY BE INVOKED INSIDE A LOCK
	void weak_lock();
	void weak_unlock();
	void strong_lock();
	void strong_unlock();

};

#endif //DESK_DATABASE