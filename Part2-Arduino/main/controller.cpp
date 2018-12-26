#include "controller.h"

using namespace std;

CONTROLLER::CONTROLLER() {
  //LDR paramethers
  m = 0;
  b = 0;

  desired_lux = 0;

  //to check the system with differents actions
  ffw_flag = 0;
  feedback_flag = 0;
  windup_flag = 0;
  filter_flag = 0;
  deadzone_flag = 0;

  y = 0;

  //Variables of the feedforward
  ext_ilum = 0;
  lux_max = 0;
  G0 = 0;
  pwm_ref = 0;

  //Variables of the simulator
  t0 = 0;
  tau = 0;
  vi = 0;
  vf = 0;

  //Variables of the PI
  error = 0;
  old_error = 0;
  i = 0;
  old_i = 0;
  windup_error = 0;
  windup_gain = 0;

  last_led = 0;
  controller_output=0;
}

CONTROLLER::CONTROLLER(bool _ffw_flag, bool _feedback_flag, bool _windup_flag, bool _filter_flag, bool _deadzone_flag) {
  //LDR paramethers
  m = 0;
  b = 0;

  desired_lux = UNOCCUPIED;

  //to check the system with differents actions
  ffw_flag = _ffw_flag;
  feedback_flag = _feedback_flag;
  windup_flag = _windup_flag;
  filter_flag = _filter_flag;
  deadzone_flag = _deadzone_flag;

  y = 0;

  //Variables of the feedforward
  ext_ilum = 0;
  lux_max = 0;
  G0 = 0;
  pwm_ref = 0;

  //Variables of the simulator
  t0 = 0;
  tau = 0;
  vi = 0;
  vf = 0;

  //Variables of the PI
  error = 0;
  old_error = 0;
  i = 0;
  old_i = 0;
  windup_error = 0;
  windup_gain = 1;

  last_led = 0;
  controller_output=0;
}

//read from the serial desired flags states to (de)activate functionalities
void CONTROLLER::get_flags() {
  char inputChar;
  String flag = "";

  //read desired iluminance from the serial
  while (Serial.available() > 0) {
    inputChar = Serial.read();

    if (inputChar != '\n')
      flag += inputChar;
    else
      update_flag(flag);
  }
}

//update desired flags states
void CONTROLLER::update_flag(String flag) {
  char *aux_str = flag.c_str();
  char str[50];

  sscanf(aux_str, "%s", str);

  if (!strcmp(str, "antiwindup_off")) {
    windup_flag = false;
    windup_error = 0;
  } else if (!strcmp(str, "antiwindup_on")) {
    windup_flag = true;
  } else if (!strcmp(str, "ffw_on")) {
    ffw_flag = true;
  } else if (!strcmp(str, "ffw_off")) {
    ffw_flag = false;
  } else if (!strcmp(str, "deadzone_off")) {
    deadzone_flag = false;
  } else if (!strcmp(str, "deadzone_on")) {
    deadzone_flag = true;
  } else if (!strcmp(str, "filter_on")) {
    filter_flag = true;
  } else if (!strcmp(str, "filter_off")) {
    filter_flag = false;
  } else if (!strcmp(str, "feedback_on")) {
    feedback_flag = true;
  } else if (!strcmp(str, "feedback_off")) {
    feedback_flag = false;
  }

  memset(str, 0, 20);
}

//convert ADC value to Lux units
float CONTROLLER::convert_ADC_to_Lux(float Vi_value)  {
  float R2_1LUX = pow(10, b);
  float V_volt = Vi_value * VCC / 1023.0;
  float R2 = ( (VCC / V_volt) - 1 ) * R1;

  return pow( R2 / R2_1LUX, 1 / m);
}

//convert Lux value to Volt units
float CONTROLLER::convert_Lux_to_Volt(float lux) {
  float aux = (float) m * log10(lux) + b;
  float R2 = (float) pow(10, aux);

  return (float) 5 * R1 / (R1 + R2);
}

//Low-Pass filter. Used to decrease noise
//We checked before it is possible to get this number of samples because we have a limited time, because of the interrupt
float CONTROLLER::LPfilter(int pin) {
  y = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    y += analogRead(pin);
    delayMicroseconds(1);
  }
  return y / FILTER_SAMPLES;
}

void CONTROLLER::print_actual_state( float _mylux ) {
  Serial.print(desired_lux);
  Serial.print(", ");
  Serial.print(_mylux);
  Serial.print(", ");
  Serial.print(convert_ADC_to_Lux(simulator() * (1023.0 / 5)));
  Serial.print(" , ");
  Serial.println(convert_ADC_to_Lux( y * (1023.0 / 5) ));
}

void CONTROLLER::simulator_init( float _myLux ) {
  //--- It is extremely important to refere that achieving tau, we did an experimental function (time/tau in function of desired lux)
  getTau(_myLux);
  vf = convert_Lux_to_Volt(_myLux);
  t0 = micros();

  Serial.print("tau: ");
  Serial.println(tau);
  Serial.print("vf: ");
  Serial.println(vf);
  Serial.print("t0: ");
  Serial.println(t0);
}

void CONTROLLER::environment_init( float _myLux, float G0 ) {
  if (ffw_flag)
    pwm_ref = (_myLux - ext_ilum) / (G0);
  else
    pwm_ref = 0.0;

  Serial.print("pwm_ref: ");
  Serial.println(pwm_ref);

  simulator_init( _myLux );
}

//--- It is extremely important to refere that o achieve tau, we did a experimental function (time/tau in function of desired lux)
void CONTROLLER::getTau(int desired_lux) {
  if (desired_lux > lux_max)
    desired_lux = lux_max;

  tau = ( 1000 * ((float)( (float)0.0018 * pow(desired_lux, 2) - (float)(0.3715 * desired_lux) + 36.6262 )) );
}

//gets reference to be compared with y_out
float CONTROLLER::simulator() {
  unsigned long dif_time = (micros() - t0);

  //solution of 1st order differetial equation
  return ( (float)( vf - (vf - vi) * pow(2.71828, (signed long)((signed long)(dif_time * -1) / (signed long)tau) ) ) );
}

float CONTROLLER::PI_func() {
  //In order to get the best Kp and Ki, we did Ziegler-Nichols Method to have a reference, and then we adjusted empirically.
  //We tried to have a smooth response (without overshoot)and that stabilizes to desired value as fast as possible
  int Kp = 25;
  int Ki = 50;
  float K2 = Kp * Ki / (2 * 100);

  if (deadzone_flag)
    error = deadzone(PIerror(), -0.00005, 0.001);
  else
    error = PIerror();

  float p = Kp * (error);
  i = old_i + K2 * (error + old_error) + windup_gain * windup_error;

  float u = 0;
  if (ffw_flag)
    u += pwm_ref;
  if (feedback_flag)
    u += i + p;

  if (windup_flag)
    antiWindup(u);

  return  u;
}

float CONTROLLER::PIerror() {
  return simulator() - y;
}

float CONTROLLER::deadzone(float _error, float err_min, float err_max) {
  if (_error < err_min)
    return _error - err_min;

  else if (_error > err_max)
    return _error - err_max;

  else
    return 0;
}

//it is important to note that pwmOut is u=i+p+pwm_ref! we might to do this in different way: send to this function only u=i+p and shift de limits to 255- pwm ffw
//the resulting errorWUp will be sum (multiplied by one gain) to the i_out in next iteration
void CONTROLLER::antiWindup(float pwm_out ) {
  if (pwm_out > 255) {
    windup_error = 255 - pwm_out;
  }
  else if (pwm_out < 0) {
    windup_error = 0 - pwm_out;
  }
  else
    windup_error = 0;
}

void CONTROLLER::writeLED(float pwm) {
  //avoid flickering due to known spontaneous flaws from the controller
  if (error < 0 && pwm > last_led || error > 0 && pwm < last_led)
    pwm = last_led;

  //prevent saturation
  if (pwm > 255)
    pwm = 255;
  else if (pwm < 0)
    pwm = 0;

  analogWrite(led_pin, pwm);
  last_led = pwm;
}

void CONTROLLER::general() {
  if (filter_flag)
    y = LPfilter(A0);
  else
    y = analogRead(A0);

  y = y * VCC / (1023.0);

  controller_output = PI_func();
  writeLED(controller_output);
}
