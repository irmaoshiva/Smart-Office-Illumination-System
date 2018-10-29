float tau;
float ext_perturb;
int x_on;
float pwm_ffw;
float G0;
unsigned long t0;


float error;
float old_error = 0;
float old_i = 0;
float windup_error = 0;
//gainWUp is the gain used in AntiWindup (sum the error multiplied by this gain to the integral term in the next sample).
//in order to fit the gainWUp, we made several attempt's and we chose the one that was more fast to change the status.
//One attempt that we did was: put reference to 20Lux and point light (with mobile phone) to LDR. Next, we turn off the mobile phone light,closed the box, and check the response time
float windup_gain = 1;

//get the gain for feedforward calculus
int led_pin = 11;
void feedforward_init() {
  analogWrite(led_pin, 0);
  delay(1000);
  ext_perturb = analogRead(A0);
  vi = ext_perturb * VCC / (1023.0);
  ext_perturb = convert_ADC_to_Lux(ext_perturb);

  analogWrite(led_pin, 255);
  delay(1000);
  x_on = analogRead(A0);
  x_on = convert_ADC_to_Lux(x_on);
  G0 = (x_on - ext_perturb) / ((float)255);
  
  analogWrite(led_pin, 0);
  delay(1000);
}



//returns feedforward input in pwm
float ffw_func(int _desired_lux){
  return (_desired_lux - ext_perturb) / (G0);
}



void environment_init(int _desired_lux){
  if (ffw_flag) 
    pwm_ffw = ffw_func(_desired_lux);
  else 
    pwm_ffw = 0.0;
  
  //--- It is extremely important to refere that achieving tau, we did an experimental function (time/tau in function of desired lux)
  getTau(_desired_lux);
  vf = convert_Lux_to_Volt(_desired_lux);
  t0 = micros();
}



//--- It is extremely important to refere that o achieve tau, we did a experimental function (time/tau in function of desired lux)
void getTau(int _desired_lux){
  if (_desired_lux > x_on)
    _desired_lux = x_on;

  tau = ( 1000 * ((float)( (float)0.0018 * pow(_desired_lux, 2) - (float)(0.3715 * _desired_lux) + 36.6262 )) );
}



//gets reference to be compared with y_out
float simulator(){
  unsigned long dif_time = (micros() - t0);

  //solution of 1st order differetial equation
  return ( (float)( vf - (vf - vi) * pow(2.71828, (signed long)((signed long)(dif_time * -1) / (signed long)tau) ) ) );
}



void PI_func(){
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
  float i = old_i + K2 * (error + old_error) + windup_gain * windup_error;

  float u = 0;
  if (ffw_flag)
    u += pwm_ffw;
  if (feedback_flag)
    u += i + p;

  if (windup_flag)
    antiWindup(u);

  writeLED(u);

  old_i = i;
  old_error = error;
}



float PIerror(){
  return simulator() - y;
}



float deadzone(float _error, float err_min, float err_max) {
  if (_error < err_min)
    return _error - err_min;

  else if (_error > err_max)
    return _error - err_max;

  else
    return 0;
}



//it is important to note that pwmOut is u=i+p+pwm_ffw! we might to do this in different way: send to this function only u=i+p and shift de limits to 255- pwm ffw
//the resulting errorWUp will be sum (multiplied by one gain) to the i_out in next iteration
void antiWindup(float pwm_out ) {
  if (pwm_out > 255) {
    windup_error = 255 - pwm_out;
  }
  else if (pwm_out < 0) {
    windup_error = 0 - pwm_out;
  }
  else
    windup_error = 0;
}



float last_led = 0;
void writeLED(float pwm){
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
