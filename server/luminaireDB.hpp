#define POWER 1

class deskDB{
	float lux;	//last measured lux value//VAI SER UM BUFFER
	float duty_cycle; // duty cycle currently imposed in the LED//VAI SER UM BUFFER
	bool occupanccy; // is? occupied
	float ext_lux; // external lux affecting the desk
	float control_ref; // luminance control reference
	float restart_time; // time of last restart event
	float last_sample_time; // time of last acquired sample
	float energy; // accumulated energy consumption
	int samples_nr; // number of samples acquired since last restart
	float comfort; // accumulated confort error (not normalized by samples_nr)
	float comfort_flicker; // acccumulated comfort flicker (not normalized by samples_nr)

}