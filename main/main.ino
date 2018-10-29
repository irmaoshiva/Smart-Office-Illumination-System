#include <stdlib.h>
#include <stdio.h>
//Include lybrary's for interrupts
#include <avr/io.h>
#include <avr/interrupt.h>

#define VCC 5

extern bool filter_flag;
extern int desired_lux;
float y;

//timer 0 messes with delay(), mills(), etc
//timer 1 is used for the sampling period
//Use in timer 2.
const byte mask = B11111000;
int prescale_pwm = 1;
volatile bool flag_timer = 0;


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

    PI_func();

    flag_timer = 0;
  }
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
