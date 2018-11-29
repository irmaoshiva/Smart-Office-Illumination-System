#include "controller.h"

//Include lybrary's for interrupts
#include <avr/io.h>
#include <avr/interrupt.h>

//timer 0 messes with delay(), mills(), etc
//timer 1 is used for the sampling period
//Use in timer 2.
const byte mask = B11111000;
int prescale_pwm = 1;
volatile bool flag_timer = false;

//CONTROLLER cont(-0.7, 1.9252, true, true, true, true, true);

CONTROLLER cont;

//------Interrupt used to generate 100Hz sample frequency-------
//---- We chosed timer 1 because this have 16 bits!
void Active_InterruptSample() {

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
  flag_timer = true; //notify main loop
}


void setup() {
  Serial.begin(2000000);

  cont.initialize(-0.7, 1.9252, true, true, true, true, true);

  TCCR2B = (TCCR2B & mask) | prescale_pwm;

  cont.feedforward_init();

  cont.environment_init(UNOCCUPIED);

  Active_InterruptSample();
}

void loop() {

  cont.get_flags();
  cont.check_switch();

  //Serial.println("EU VIM AQUI");

  //only when the interrupt is finished
  // the control has more or less 2/3 ms (we print the time to ensure this important thing)
  if (flag_timer) {

    cont.general();

    flag_timer = false;
  }
}
