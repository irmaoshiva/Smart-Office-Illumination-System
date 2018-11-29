#include <stdlib.h>
#include <stdio.h>
//Include lybrary's for interrupts
#include <avr/io.h>
#include <avr/interrupt.h>

#define OCCUPIED 60
#define UNOCCUPIED 20
#define FILTER_SAMPLES 20
#define VCC 5

//timer 0 messes with delay(), mills(), etc
//timer 1 is used for the sampling period
//Use in timer 2.
const byte mask = B11111000;
int prescale_pwm = 1;
volatile bool flag_timer = 0;

//gainWUp is the gain used in AntiWindup (sum the error multiplied by this gain to the integral term in the next sample).
//in order to fit the gainWUp, we made several attempt's and we chose the one that was more fast to change the status.
//One attempt that we did was: put reference to 20Lux and point light (with mobile phone) to LDR. Next, we turn off the mobile phone light,closed the box, and check the response time
float windup_gain = 1;

//to check the system with differents actions
bool deadzone_flag = true;
bool ffw_flag = true;
bool feedback_flag = true;
bool windup_flag = true;
bool filter_flag = true;

float tau;
float ext_perturb;
int x_on;
float pwm_ffw;
float G0;
unsigned long t0;

float vi;
float vf;
float y;

float error;
float old_error = 0;
float i = 0;
float old_i = 0;
float windup_error = 0;

int switch_pin = 7;
int prev_switch_value = LOW;
long switch_debounce = 400;
long prev_switch_time = 0;

int desired_lux = OCCUPIED;

//LDR paramethers
float m = -0.7;
float b = 1.9252;
//R1 em KOhm
int R1 = 10;

//------------------------------ UTILS CODE ------------------------------

//check occupation state through a button switch
int check_switch() {
  int switch_value = digitalRead(switch_pin);

  if (switch_value == HIGH && prev_switch_value == LOW && (switch_debounce + prev_switch_time) < millis()) {
    
    vi = y;
    if (desired_lux == OCCUPIED){
      desired_lux = UNOCCUPIED;
      if (vi > vf) //to avoid flickering due to simulator function
        vi = vf;
    }else{
      desired_lux = OCCUPIED;
      if (vi < vf) //to avoid flickering due to simulator function
        vi = vf;
    }

    environment_init(desired_lux);
    
    prev_switch_time = millis();
    prev_switch_value = switch_value;
    return 1;
  }
  else
    return 0; 
}

//read from the serial desired flags states to (de)activate functionalities
void get_flags(){
  char inputChar;
  String flag = "";

  //read desired iluminance from the serial
  while (Serial.available() > 0){
    inputChar = Serial.read();
    
    if (inputChar != '\n')
      flag += inputChar;
    else
      update_flag(flag);
  }
}

//update desired flags states
void update_flag(String flag){
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
    filter_flag = 0;
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
float convert_ADC_to_Lux(float Vi_value)  {
  float R2_1LUX = pow(10, b);
  float V_volt = Vi_value * VCC / 1023.0;
  float R2 = ( (VCC / V_volt) - 1 ) * R1;

  return pow( R2 / R2_1LUX, 1 / m);
}

//convert Lux value to Volt units
float convert_Lux_to_Volt(float lux) {
  float aux = (float) m * log10(lux) + b;
  float R2 = (float) pow(10, aux);

  return (float) 5 * R1 / (R1 + R2);
}

//Low-Pass filter. Used to decrease noise
//We checked before it is possible to get this number of samples because we have a limited time, because of the interrupt
float LPfilter(int pin){
  y = 0;
  for (int i = 0; i < FILTER_SAMPLES; i++){
    y += analogRead(pin);
    delayMicroseconds(1);
  }
  return y / FILTER_SAMPLES;
}

void print_actual_state(){
  Serial.print(desired_lux);
  Serial.print(", ");
  Serial.print(convert_ADC_to_Lux(simulator() * (1023.0 / 5)));
  Serial.print(" , ");
  Serial.println(convert_ADC_to_Lux(y));
}


//------------------------------ CONTROLLER CODE ------------------------------

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
float ffw_func(int desired_lux){
  return (desired_lux - ext_perturb) / (G0);
}

void environment_init(int desired_lux){
  if (ffw_flag) 
    pwm_ffw = ffw_func(desired_lux);
  else 
    pwm_ffw = 0.0;
  
  //--- It is extremely important to refere that achieving tau, we did an experimental function (time/tau in function of desired lux)
  getTau(desired_lux);
  vf = convert_Lux_to_Volt(desired_lux);
  t0 = micros();
}

//--- It is extremely important to refere that o achieve tau, we did a experimental function (time/tau in function of desired lux)
void getTau(int desired_lux){
  if (desired_lux > x_on)
    desired_lux = x_on;

  tau = ( 1000 * ((float)( (float)0.0018 * pow(desired_lux, 2) - (float)(0.3715 * desired_lux) + 36.6262 )) );
}

//gets reference to be compared with y_out
float simulator(){
  unsigned long dif_time = (micros() - t0);

  //solution of 1st order differetial equation
  return ( (float)( vf - (vf - vi) * pow(2.71828, (signed long)((signed long)(dif_time * -1) / (signed long)tau) ) ) );
}

float PI_func(){
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
    u += pwm_ffw;
  if (feedback_flag)
    u += i + p;

  if (windup_flag)
    antiWindup(u);
    
  return  u;
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

//------------------------------ INTERRUPT CODE ------------------------------
//------Interrupt used to generate 100Hz sample frequency-------
//---- We chosed timer 1 because this have 16 bits!
void Active_InterruptSample(){
  
  cli(); //stop interrupts
  TCCR1A = 0;// clear register
  TCCR1B = 0;// clear register
  TCNT1 = 0;//reset counter

  //when the counter hits the value 20000, it resets and the interrupt is called.
  //We have reached the value through the account: OCR1A= 16E6/(prescale*100) [100 is the desired value to the frequency]
  OCR1A = 20000; //must be <65536
  //=16*10^6/(100*8) , 8= prescale!
  
  TCCR1B |= (1 << WGM12); //CTC On

  //-------------------------
  // Set prescaler for 8. This way, and with OCR1A with 20000 is possible to get 100HZ
  //With prescale=8 we can have the integer counts number [ONLY WITH 16 bits- timer 1]
  //-------------------------
  TCCR1B &= ~(1 << CS12);
  TCCR1B |= (1 << CS11);
  TCCR1B &= ~(1 << CS10);
  //-------------------------
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); //allow interrupts
}

//the interrupt has to be as small as possible
//and we have to ensure that the loop control has to have less than 10 ms
ISR(TIMER1_COMPA_vect) {
  flag_timer = 1; //notify main loop
}

//------------------------------ MAIN CODE ------------------------------

void setup() {
  Serial.begin(2000000);
  
  TCCR2B = (TCCR2B & mask) | prescale_pwm;
  
  feedforward_init();
  environment_init(desired_lux);
  
  Active_InterruptSample();
}

void loop() {
  
  get_flags();
  check_switch();

  //only when the interrupt is finished
  // the control has more or less 2/3 ms (we print the time to ensure this important thing)
  if (flag_timer) {
    
    if (filter_flag)
      y = LPfilter(A0);
    else
      y = analogRead(A0);

    print_actual_state();
    
    y = y * VCC / (1023.0);

    float controller_output = PI_func();
    writeLED(controller_output);

    old_i = i;
    old_error = error;

    flag_timer = 0;
  }
}
