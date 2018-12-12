//Include lybrary's for interrupts
#include <avr/io.h>
#include <avr/interrupt.h>

#include "i2commun.h"
#include <EEPROM.h>

//timer 0 messes with delay(), mills(), etc
//timer 1 is used for the sampling period
//Use in timer 2.
const byte mask = B11111000;
int prescale_pwm = 1;
volatile bool flag_timer = true;

int led_pin = 3;
//aqui devia se o last_node se calhar podemos mudar o retorno da funçao do find_all_nodes
Vector <float> k = Vector <float> (2);

I2COMMUN i2c(led_pin);

//criar o nó
Node n1(0.07, 1, 50, 20, 0);
//Node n2(0.07, 1, 50, 270, 1);

int my_adr;

int iterations = 0;

//------------------------------ INTERRUPT CODE ------------------------------
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
  flag_timer = 1; //notify main loop
}


void setup() {
  Serial.begin(2000000);

  /*
    TCCR2B = (TCCR2B & mask) | prescale_pwm;
    Active_InterruptSample();
  */

  my_adr = EEPROM.read(0);

  Serial.print("my_adr: ");
  Serial.println(my_adr);

  Wire.begin(my_adr); // receive data

  for (int i = 0; i < 2; i++)
    k[i] = -1;

  my_adr = i2c.checkAdress(my_adr, k);

  Wire.onReceive(receiveEvent); //event handler

  //active broadcast
  TWAR = (my_adr << 1) | 1;

  i2c.findAllNodes(k, n1);

  Serial.print("k[0]: ");
  Serial.println(k[0]);
  Serial.print("k[1]: ");
  Serial.println(k[1]);
}


void loop() {

  i2c.check_flags(k, n1);

  if ( ( i2c.deskStatus == CONSENSUS ) && ( i2c.nr_nos > 1 ) )
  {
    Serial.println("SO DEVIA VIR AQUI 50X");

    //acrescentar condiçao de saida com d - d_av
    if ( iterations < 50 ) 
    {
      n1.Primal_solve(k);

      Wire.beginTransmission(0x00);
      Wire.write('k');
      Wire.write((uint8_t) my_adr);
      for (int j = 0; j < i2c.nr_nos; j++)
      {
        Wire.write((uint8_t)round(n1.d[j]));
      }
      Wire.endTransmission();

      Serial.println("VOU ENTRAR NO WAITING???");
      if (i2c.waitingAck(k, n1))
      {
        n1.d[0] = round(n1.d[0]);
        n1.d[1] = round(n1.d[1]);

        Serial.print("n1.d[0]: ");
        Serial.println(n1.d[0]);
        Serial.print("n1.d[1]: ");
        Serial.println(n1.d[1]);

        n1.aux_soma = n1.aux_soma + n1.d;

        Serial.print("n1.aux_soma[0]: ");
        Serial.println(n1.aux_soma[0]);
        Serial.print("n1.aux_soma[1]: ");
        Serial.println(n1.aux_soma[1]);

        n1.d_av = n1.aux_soma * (double)( 1 / (double) i2c.nr_nos );

        Serial.print("n1.d_av[0]: ");
        Serial.println(n1.d_av[0]);
        Serial.print("n1.d_av[1]: ");
        Serial.println(n1.d_av[1]);

        for (int j = 0; j < i2c.nr_nos; j++)
          n1.aux_soma[j] = 0;

        n1.y = n1.y + (n1.d - n1.d_av) * n1.rho;

        Serial.print("n1.y[0]: ");
        Serial.println(n1.y[0]);
        Serial.print("n1.y[1]: ");
        Serial.println(n1.y[1]);

        Serial.print("iterations: ");
        Serial.println(iterations);
        
        iterations ++;
      }
      else
      {
        Serial.println("PAROU A MEIO DO CONSENSUSSSS");
        iterations = 0;
      }
    }
    else
    {
      Serial.println("DEVIA VIR AQUI 1X");
      
      i2c.deskStatus = 0;
      iterations = 0;
    }
  }
}

void receiveEvent(int howMany) {
  char c;
  char action;
  int source_adr;
  int i = 0;
  int flag = 0;

  //check data on BUS
  if (Wire.available() > 0)
  {
    action = Wire.read();
    source_adr = Wire.read();
  }

  while (Wire.available())
  {
    n1.aux_soma[i] = n1.aux_soma[i] + (float) Wire.read();
    i++;
  }

  Serial.print("Action: ");
  Serial.println(action);
  Serial.print("Source_adr: ");
  Serial.println(source_adr);
  Serial.print("n1.aux_soma[0]: ");
  Serial.println(n1.aux_soma[0]);
  Serial.print("n1.aux_soma[1]: ");
  Serial.println(n1.aux_soma[1]);

  i2c.performAction(action, source_adr, k, n1);
}
