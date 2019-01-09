#include "deskDB.hpp"

deskDB::deskDB(int samples_holder)
	: lux(samples_holder), duty_cycle(samples_holder), buff_size(samples_holder) { }


int deskDB::count_samples_held(){
	if (samples_nr > buff_size) 
		return buff_size;
	else 		
		return samples_nr;
}

void deskDB::weak_lock(){
	mtx[2].lock();
	mtx[1].lock();
	mtx[0].lock();
	mtx[1].unlock();
}

void deskDB::weak_unlock(){
	mtx[0].unlock();
	mtx[2].unlock();
}

void deskDB::strong_lock(){
	mtx[1].lock();
	mtx[0].lock();
	mtx[1].unlock();
}

void deskDB::strong_unlock(){
	mtx[0].unlock();
}

float deskDB::get_lux(){
	float l = -1;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr)
			l = lux.back();
	weak_unlock();//________________PROTECTED REGION________________
	
	return l;
}

float deskDB::get_duty_cycle(){
	float d = -1;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr)
			d = duty_cycle.back();
	weak_unlock();//________________PROTECTED REGION________________
	
	return d;
}

bool deskDB::get_occupancy(){
	bool o = false;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr)
			o = occupancy;
	weak_unlock();//________________PROTECTED REGION________________
	
	return o;
}

float deskDB::get_lower_bound(){
	float lb = -1;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr){
			if (occupancy) 
				lb = lower_bound[1];
			else 
				lb = lower_bound[0];
		}
	weak_unlock();//________________PROTECTED REGION________________
	
	return lb;
}

float deskDB::get_ext_lux(){
	float ext = -1;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr)
			ext = ext_lux;
	weak_unlock();//________________PROTECTED REGION________________
	
	return ext;
}

float deskDB::get_control_ref(){
	float c = -1;
	
	weak_unlock();//________________PROTECTED REGION________________
		if (samples_nr)
			c = control_ref;
	weak_unlock();//________________PROTECTED REGION________________
	
	return c;
}

float deskDB::get_power(){
	if (samples_nr)
		return POWER*get_duty_cycle();
	else
		return 0;
}

float deskDB::get_restart_time(){
	std::chrono::duration<float> elapsed;
	auto current = std::chrono::system_clock::now();

	weak_lock();//________________PROTECTED REGION________________
		if (! samples_nr){
			weak_unlock();
			return -1;
		}
		elapsed = current - restart_time;
	weak_unlock();//________________PROTECTED REGION________________
	
	return elapsed.count();
}

float deskDB::get_energy(){
	float e = 0;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr)
			e = energy;
	weak_unlock();//________________PROTECTED REGION________________
	
	return e;
}

float deskDB::get_comfort(){
	float c = 0, s = 0;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr){
			c = comfort;
			s = samples_nr;
		}
	weak_unlock();//________________PROTECTED REGION________________
	
	if (c == 0)	
		return 0;
	else 		
		return c/s;
}

float deskDB::get_comfort_flicker(){
	float c = 0, s = 0;
	
	weak_lock();//________________PROTECTED REGION________________
		if (samples_nr){
			c = comfort_flicker;
			s = samples_nr;
		}
	weak_unlock();//________________PROTECTED REGION________________
	
	if (c == 0)	
		return 0;
	else 		
		return c/s;
}

void deskDB::get_lux_holder(std::vector<float>& holder){

	weak_lock();//________________PROTECTED REGION________________
		int samples_held = count_samples_held();
		//insert buff values into vector to return
		for (int i = 0; i < samples_held; i++) 
			holder.push_back(lux[i]);
	weak_unlock();//________________PROTECTED REGION________________
}

void deskDB::get_duty_cycle_holder(std::vector<float>& holder){

	weak_lock();//________________PROTECTED REGION________________
		int samples_held = count_samples_held();
		//insert buff values into vector to return
		for (int i = 0; i < samples_held; i++) 
			holder.push_back(duty_cycle[i]);
	weak_unlock();//________________PROTECTED REGION________________
}

void deskDB::insert_sample(float lux_, float duty_cycle_){
	auto new_sample_time = std::chrono::system_clock::now();

	strong_lock();//________________PROTECTED REGION________________
		if (! samples_nr){ //first sample after restart
			restart_time = new_sample_time;
			last_sample_time = restart_time;
		}
		//gap between last sample and the current
		std::chrono::duration<float> duration = new_sample_time - last_sample_time;
		float sampling_time = duration.count();
		//write new parameters
		lux.push_back(lux_);
		duty_cycle.push_back(duty_cycle_);
		samples_nr ++;
		last_sample_time = new_sample_time;
		//accumulate energy
		energy += POWER * duty_cycle_ * sampling_time;
		//accumulate comfort error
		float comfort_ = control_ref - lux_;
		if (comfort_ > 0) 
			comfort += comfort_;
		//accumulate comfort flickering error
		if (samples_nr >= 3){
			int sh = count_samples_held() - 1;
			float s1 = lux[sh] - lux[sh-1], s2 = lux[sh-1] - lux[sh-2];
			if( s1 * s2 < 0)
				comfort_flicker += (std::fabs(s1) + std::fabs(s2)) / (2*sampling_time);
		}
	strong_unlock();//________________PROTECTED REGION________________
}

void deskDB::set_occupancy(bool occupancy_, float control_ref_){
	strong_lock();//________________PROTECTED REGION________________
		occupancy = occupancy_;
		control_ref = control_ref_;
	strong_unlock();//________________PROTECTED REGION________________
}

void deskDB::set_parameters(float lower_bound_off, float lower_bound_on, float ext_lux_){
	strong_lock();//________________PROTECTED REGION________________
		lower_bound[0] = lower_bound_off;
		lower_bound[1] = lower_bound_on;
		ext_lux = ext_lux_;
	strong_unlock();//________________PROTECTED REGION________________
}

void deskDB::clearDB(){
	weak_lock();//________________PROTECTED REGION________________
		lux.clear();
		duty_cycle.clear();
		samples_nr = 0;
	weak_unlock();//________________PROTECTED REGION________________
}