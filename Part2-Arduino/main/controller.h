#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>

#define OCCUPIED 90
#define UNOCCUPIED 20
#define FILTER_SAMPLES 20
#define VCC 5

class CONTROLLER
{
  private:

    //Analog output pin that the LED is attached to
    const int led_pin = 3;

    //R1 em KOhm
    const int R1 = 10;

    bool ffw_flag;
    bool feedback_flag;
    bool windup_flag;
    bool filter_flag;
    bool deadzone_flag;

    //Variables of the feedforward
    float G0;

    //Variables of the simulator
    float tau;

    //Variables of the PI
    float windup_error;
    float windup_gain;

    float last_led;

  public:

    //LDR paramethers
    float m;
    float b;

    //Variables
    float desired_lux;

    float y;

    //Variables of the feedforward
    float ext_ilum;
    float lux_max;
    float pwm_ref;

    //Variables of the simulator
    unsigned long t0;
    float vi;
    float vf;

    //Variables of the PI
    float error;
    float old_error;
    float i;
    float old_i;

    float controller_output;

    CONTROLLER();
    CONTROLLER(bool _ffw_flag, bool _feedback_flag, bool _windup_flag, bool _filter_flag, bool _deadzone_flag);

    void get_flags();
    void update_flag(String flag);
    float convert_ADC_to_Lux(float Vi_value);
    float convert_Lux_to_Volt(float lux);
    float LPfilter(int pin);
    void print_actual_state( float _mylux );

    void simulator_init( float _myLux );
    void environment_init( float myLux, float G0 );    void getTau(int desired_lux);
    float simulator();
    float PI_func();
    float PIerror();
    float deadzone(float _error, float err_min, float err_max);
    void antiWindup(float pwm_out );
    void writeLED(float pwm);
    void general();

};



#endif //CONTROLLER_H 
