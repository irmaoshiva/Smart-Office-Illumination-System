#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>

#define OCCUPIED 60
#define UNOCCUPIED 20
#define FILTER_SAMPLES 20
#define VCC 5

class CONTROLLER
{
  private:

    //Analog output pin that the LED is attached to
    int led_pin = 11;

    //LDR paramethers
    float m;
    float b;
    //R1 em KOhm
    int R1 = 10;

    //switch button control variables
    int switch_pin = 7;
    int prev_switch_value;
    long switch_debounce;
    long prev_switch_time;

    bool ffw_flag;
    bool feedback_flag;
    bool windup_flag;
    bool filter_flag;
    bool deadzone_flag;

    int desired_lux;

    float y;

    //Variables of the feedforward
    float ext_perturb;
    int lux_max;
    float G0;
    float pwm_ffw;

    //Variables of the simulator
    unsigned long t0;
    float tau;
    float vi;
    float vf;

    //Variables of the PI
    float error;
    float old_error;
    float i;
    float old_i;
    float windup_error;
    float windup_gain;

    float last_led;

  public:

    CONTROLLER();
    
    void initialize(float _m, float _b, bool _ffw_flag, bool _feedback_flag, bool _windup_flag, bool _filter_flag, bool _deadzone_flag);

    int check_switch();
    void get_flags();
    void update_flag(String flag);
    float convert_ADC_to_Lux(float Vi_value);
    float convert_Lux_to_Volt(float lux);
    float LPfilter(int pin);
    void print_actual_state();

    void feedforward_init();
    float ffw_func(int desired_lux);
    void environment_init(int desired_lux);
    void getTau(int desired_lux);
    float simulator();
    float PI_func();
    float PIerror();
    float deadzone(float _error, float err_min, float err_max);
    void antiWindup(float pwm_out );
    void writeLED(float pwm);
    void general();

};



#endif //CONTROLLER_H 
