#include <stdlib.h>
#include <stdio.h>

/* -----------Included lybrary's to interrupt---------------- */
#include <avr/io.h>
#include <avr/interrupt.h>
//---------------------------------------------------------------

//----Default defined values
#define OCCUPIED 60
#define UNOCCUPIED 20
#define NR_SAMPLES 20


//to use in timer 2. We can't use timer 1 because we are using to generate 10ms to sampling period
//timer 0 can't also be used because it interferes with delay/millis/micros() functions
const byte mask = B11111000;
int prescale_pwm = 1;
volatile bool flag_timer = 0;

int led_pin = 11;
int pwm_max = 255;
int pwm_min = 0;
float G0 = 0;
int Vcc = 5;
//R1 em KOhm
int R1 = 10;
float ext_perturb = 0;
unsigned long t0 = 0;
float vi;
float pwm_ffw = 0.0;
float v_ref = 0;
float y = 0;
unsigned long tau = 0;
int desire_lux = UNOCCUPIED;
float vf;
float out_PI_FFW;
int x_on = 0;
float last_led = 0;


float i_out_ant = 0;
float i_out;
float e_ant = 0;
float error;
float errorWUp = 0;

//gainWUp is the gain used in AntiWindup (sum the error multiplied by this gain to the integral term in the next sample).
//in order to fit the gainWUp, we made several attempt's and we chose the one that was more fast to change the status.
//One attempt that we did was: put reference to 20Lux and point light (with mobile phone) to LDR. Next, we turn off the mobile phone light,closed the box, and check the response time
float gainWUp = 1;

int switch_pin = 7;
int prev_switch_value = LOW;
long switch_debounce = 400;
long prev_switch_time = 0;

//to check the system with differents actions
bool deadzone_flag = false, ffw_flag = true, feedback_flag = true, windup_flag = true, filter_flag = true;

//------------------------------ BUTTON CODE ------------------------------
void check_switch() {
  int switch_value = digitalRead(switch_pin);

  if (switch_value == HIGH && prev_switch_value == LOW && (switch_debounce + prev_switch_time) < millis()) {
    if (desire_lux == OCCUPIED)
      desire_lux = UNOCCUPIED;
    else
      desire_lux = OCCUPIED;

    t0 = micros();

    vi = y;
    if  (vi > vf)
      vi = vf;

    if (ffw_flag) {
      pwm_ffw = ffw_func();
    }
    else {
      pwm_ffw = 0.0;
    }
    //--- It is extremely important to refere that o achieve tau, we did a experimental function (time/tau in function of desired lux)
    tau = getTau(desire_lux);
    vf = convert_Lux_to_Volt(desire_lux);

    prev_switch_time = millis();
  }

  prev_switch_value = switch_value;
}

//------------------------------ CHANGES CODE ------------------------------

void active_action()
{
  char inputChar;
  String action = "";

  //read desire iluminance from the serial
  while (Serial.available() > 0)
  {
    inputChar = Serial.read();
    if (inputChar != '\n')
      action += inputChar;
    else
    {
      analyse_action(action);
      action = ""; 
    }
  }
}

void analyse_action(String action)
{
  char *aux_str = action.c_str();
  char str[50];

  sscanf(aux_str, "%s", str);

  if (!strcmp(str, "antiwindup_off")) {
    windup_flag = false;
    errorWUp = 0;
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
  } else if (strcmp(!str, "feedback_on")) {
    feedback_flag = true;
  } else if (!strcmp(str, "feedback_off")) {
    feedback_flag = false;
  }
  
  memset(str, 0, 20);
}

//------------------------------ UTILS CODE ------------------------------

//function that convert that value ADC to lux
float convert_ADC_to_Lux(float Vi_value)  {
  //LDR paramethers!
  float m = -0.7;
  float b = 1.9252;

  float R2_1LUX = pow(10, b);
  float V_volt = Vi_value * Vcc / 1023.0;
  float R2 = ( (Vcc / V_volt) - 1 ) * R1;

  return pow( R2 / R2_1LUX, 1 / m);
}

float convert_Lux_to_Volt(float lux) {
  float m = -0.7;
  float b = 1.9252;

  if (lux == 0.00)
    return 0;

  float x = (float) log10(lux);
  float aux = (float) m * x + b;
  float r = (float) pow(10, aux);
  
  return (float) 5 * R1 / (R1 + r);
}

//--- It is extremely important to refere that o achieve tau, we did a experimental function (time/tau in function of desired lux)
unsigned long getTau(int _desire_lux)
{
  if(_desire_lux > x_on)
    _desire_lux = x_on;
  
  return ( 1000 * ((float)( (float)0.0018 * pow( _desire_lux, 2) - (float)(0.3715 * _desire_lux) + 36.6262 )) );
}

void writeLED(float _out_PI_FFW)
{
  //saturation code. We can't have pwm more than 255 and less than 0
  if (_out_PI_FFW > pwm_max)
    _out_PI_FFW = pwm_max;
  else if (_out_PI_FFW < pwm_min)
    _out_PI_FFW = pwm_min;

  if (error < 0 && _out_PI_FFW > last_led)
    _out_PI_FFW = last_led;

  analogWrite(led_pin, _out_PI_FFW);
  last_led = _out_PI_FFW;
}

//------------------------------ FUNCTIONALITIES CODE ------------------------------

float deadzone(float _error, float err_min, float err_max) {
  if (_error < err_min)
    return _error - err_min;

  else if (_error > err_max)
    return _error - err_max;

  else
    return 0;
}

void antiWindup(float pwmOut ) {

  //it is important to note that pwmOut is u=i+p+pwm_ffw! we might to do this in different way: send to this function only u=i+p and shift de limits to pwm_max- pwm ffw
  //the resulting errorWUp will be sum (multiplied by one gain) to the i_out in next iteration
  if (pwmOut > pwm_max) {
    errorWUp = pwm_max - pwmOut;
  }
  else if (pwmOut < pwm_min) {
    errorWUp = pwm_min - pwmOut;
  }
  else
    errorWUp = 0;
}

//------------------------------ FEEDFOWARD CODE ------------------------------

void feedforward_inicialization() {
  //para chegar ao estado final - o condensador estar descarregado
  analogWrite(led_pin, 0);
  //delay para estabilizar
  delay(1000);
  ext_perturb = analogRead(A0);
  vi = ext_perturb * Vcc / (1023.0);
  ext_perturb = convert_ADC_to_Lux(ext_perturb);

  //ver o x_on maximo agora
  analogWrite(led_pin, pwm_max);
  delay(1000);
  x_on = analogRead(A0);
  x_on = convert_ADC_to_Lux(x_on);
  G0 = (x_on - ext_perturb) / ((float)pwm_max);

  //volta-se a colocar a luminosidade minima
  analogWrite(led_pin, pwm_min);
  delay(1000);
}

float ffw_func()
{
  //the result will be in pwm units. 
  return (desire_lux - ext_perturb) / (G0);
}

//------------------------------ PI CODE ------------------------------

//---- It is the reference that will be compared with y_out-------
float simulator()
{
  unsigned long dif_time = (micros() - t0);

   //solution of 1st order differetial equation 
  return ( (float)( vf - (vf - vi) * pow(2.71828, (signed long)((signed long)(dif_time * -1) / (signed long)tau) ) ) );
}

float PIerror(int _desire_lux)
{
  v_ref = simulator();
  
  return ( v_ref - y );
}

float PI_func(int _desire_lux)
{
  //In order to get the best Kp and Ki, we did Ziegler-Nichols Method to have a reference, and then we adjust empirically.
  //We tried to have a smooth response (without overshoot)and that stabilizes to desired value as fast as possible
  int Kp = 25;
  //b=1, logo k1=kp;
  int Ki = 50;
  float K2 = Kp * Ki / (2 * 100);

  if (deadzone_flag)
    error = deadzone(PIerror(_desire_lux), -0.00005, 0.001);
  else
    error = PIerror(_desire_lux);

  float p_out = Kp * (error);
  //we have o sum gainWUp*errorWup (Antiwindup) to don't allow the integrater charge more light that there is, and take it off light that there isn't
  //we adjust the gainWup that makes the system response more fast
  i_out = i_out_ant + K2 * (error + e_ant) + gainWUp * errorWUp;

  float u = 0;
  if (ffw_flag)
    u += pwm_ffw;
  if (feedback_flag)
    u += i_out + p_out;

  if (windup_flag)
    antiWindup(u);

  return  u;
}

//------------------------------ INTERRUPTION CODE ------------------------------

//------Interrupt used to generate 100Hz sample frequency-------
//---- We chosed timer 1 because this have 16 bits!
void Active_InterruptSample()
{
  cli(); //stop interrupts
  TCCR1A = 0;// clear register
  TCCR1B = 0;// clear register
  TCNT1 = 0;//reset counter

  //whenever the counter has the value 20000, the counter will be reseted and the interrupt is called.
  //We have reached the value through the count: OCR1A= 16E6/(prescale*100) [100 is the desired value to the frequency]
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);
  
  TCCR2B = (TCCR2B & mask) | prescale_pwm;
  
  feedforward_inicialization();
  t0 = micros();
  if (ffw_flag)
    pwm_ffw = ffw_func();

   //--- It is extremely important to refere that o achieve tau, we did a experimental function (time/tau in function of desired lux)
  tau = getTau(desire_lux);
  vf = convert_Lux_to_Volt(desire_lux);

  Active_InterruptSample();
}

void loop() {

  //
  active_action();

  //check if someone have pressed the button
  check_switch();

  //only when the interrupt is finished
  // the control has more or less 2/3 ms (we print the time to ensure this important thing)
  if (flag_timer) {

    //Low-Pass filter. Used to decrease noise
    //We checked before it is possible to get this number of samples because we have a limited time, because of the interrupt
    //---------------------------------------------
      if (filter_flag) {
        y = 0;
        
        for (int i = 0; i < NR_SAMPLES; i++)
        {
          y += analogRead(A0);
          delayMicroseconds(1);
        }
  
        y = y / NR_SAMPLES;
      }
    //..............................................
    else
    {
      y = analogRead(A0);
    }

    Serial.print(desire_lux);
    Serial.print(", ");
    Serial.print(convert_ADC_to_Lux(v_ref * (1023.0 / 5)));
    Serial.print(" , ");
    Serial.println(convert_ADC_to_Lux(y));

    y = y * Vcc / (1023.0);

    out_PI_FFW = PI_func(desire_lux);
    //Serial.println(out_PI_FFW);
    writeLED(out_PI_FFW);

    i_out_ant = i_out;
    e_ant = error;
    flag_timer = 0;
  }
}
