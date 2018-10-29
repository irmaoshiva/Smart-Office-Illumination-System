#include <stdlib.h>
#include <stdio.h>

#define OCCUPIED 60
#define UNOCCUPIED 20
#define FILTER_SAMPLES 20
#define VCC 5

int switch_pin = 7;
int prev_switch_value = LOW;
long switch_debounce = 400;
long prev_switch_time = 0;

extern bool deadzone_flag;
extern bool ffw_flag;
extern bool feedback_flag;
extern bool windup_flag;

extern float vi;
extern float vf;
extern float y;

int desired_lux = OCCUPIED;

//LDR paramethers
float m = -0.7;
float b = 1.9252;
//R1 em KOhm
int R1 = 10;



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


extern float windup_error;
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
  } else if (strcmp(!str, "feedback_on")) {
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
  if (lux == 0.00)
    return 0;

  float x = (float) log10(lux);
  float aux = (float) m * x + b;
  float r = (float) pow(10, aux);

  return (float) 5 * R1 / (R1 + r);
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
