//Include lybrary's for interrupts
#include <avr/io.h>
#include <avr/interrupt.h>

#include <EEPROM.h>

#include "controller.h"
#include "i2commun.h"

//timer 0 messes with delay(), mills(), etc
//timer 1 is used for the sampling period
//Use in timer 2.
const byte mask = B11111000;
int prescale_pwm = 1;
volatile uint8_t flag_timer = false;
unsigned long startTime = 0;

//aqui devia se o last_node se calhar podemos mudar o retorno da funçao do find_all_nodes
Vector <float> k = Vector <float> (2);

CONTROLLER cont(true, true, true, true, false);

I2COMMUN i2c;

//criar o nó
Node n1(0.07, 1);

int myAddr = 0;
int iterations = 0;
float myLux = 0;

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
  if ( ( i2c.deskStatus == CONSENSUS ) || ( i2c.deskStatus == GENERAL ) )
  {
    cont.general();
    flag_timer = true; //notify main loop
  }
}

void setup() {
  Serial.begin(2000000);

  pinMode(i2c.switch_pin, INPUT);
  digitalWrite(i2c.switch_pin, HIGH);

  myAddr = EEPROM.read(0);

  myAddr = i2c.checkAddress( myAddr, k, n1, cont );

  Serial.print("myAddr: ");
  Serial.println(myAddr);

  Wire.begin(myAddr); // receive data

  Wire.onReceive(receiveEvent); //event handler

  //active broadcast
  TWAR = (myAddr << 1) | 1;

  i2c.findAllNodes(k, n1, cont);

  Serial.print("k[0]: ");
  Serial.println(k[0]);
  Serial.print("k[1]: ");
  Serial.println(k[1]);

  n1.d[myAddr] = -1;

  TCCR2B = (TCCR2B & mask) | prescale_pwm;
  Active_InterruptSample();

  Serial.println("--------------- ACABOU O SETUP ---------------");
}


void loop() {

  //i2c.check_switch(k, n1, cont);

  i2c.check_flags(k, n1, cont);

  if ( ( i2c.deskStatus == CONSENSUS ) && ( i2c.nr_nos > 1 ) )
  {
    //acrescentar condiçao de saida com d - d_av
    if ( iterations < 50 && ( n1.d[0] != n1.d_av[0] || n1.d[1] != n1.d_av[1] ) )
    {
      n1.Primal_solve(k);

      Wire.beginTransmission(0x00);
      Wire.write('k');
      Wire.write((uint8_t) myAddr);
      for (int j = 0; j < i2c.nr_nos; j++)
      {
        Wire.write((uint8_t)round(n1.d[j]));
      }
      Wire.endTransmission();

      if (i2c.waitingAck(k, n1, cont))
      {
        float aux = k * n1.d;
        //compute my Lux to update controller:
        myLux = aux + cont.ext_ilum;

        for (int j = 0; j < i2c.nr_nos; j++)
          n1.d[j] = round(n1.d[j]);

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

        Serial.print("ITERATIONS: ");
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

      cont.pwm_ref = n1.d[myAddr] * 2.55;

      Serial.print("myLux: ");
      Serial.println(myLux);

      cont.simulator_init(myLux);

      if (cont.desired_lux == UNOCCUPIED)
        i2c.send_RPI_button( 254, myLux );
      else
        i2c.send_RPI_button( 255, myLux );

      n1.d[myAddr] = -1;
      i2c.deskStatus = GENERAL;
      iterations = 0;
    }
  }

  if (flag_timer)
  {
    cont.print_actual_state(myLux);
    i2c.send_RPI_sample(cont);

    //Update controller variables
    cont.old_i = cont.i;
    cont.old_error = cont.error;

    flag_timer = false;
  }





/*
  if (millis() - startTime > 10000)
  {
    startTime = millis();
    cont.vi = cont.y;
    if (cont.desired_lux == OCCUPIED)
    {
      cont.desired_lux = UNOCCUPIED;
      if (cont.vi > cont.vf) //to avoid flickering due to simulator function
        cont.vi = cont.vf;
    }
    else
    {
      cont.desired_lux = OCCUPIED;
      if (cont.vi < cont.vf) //to avoid flickering due to simulator function
        cont.vi = cont.vf;
    }

    Serial.println("!!!!!!!!!! MUDOU O BOTAO !!!!!!!!!!");

    if (i2c.nr_nos > 1)
    {
      if (cont.desired_lux > i2c.maxLux)
        n1.L = i2c.maxLux;
      else
        n1.L = cont.desired_lux;

      n1.consensus_init(i2c.nr_nos);

      i2c.write_i2c((uint8_t) 0x00, 'd');
      i2c.deskStatus = CONSENSUS;
    }
    else
    {
      cont.environment_init(cont.desired_lux, k[myAddr]);
      i2c.deskStatus = GENERAL;

      if (cont.desired_lux == UNOCCUPIED)
        i2c.send_RPI_button( 254, cont.desired_lux );
      else
        i2c.send_RPI_button( 255, cont.desired_lux );
    }
  }
  */
}



void receiveEvent(int howMany) {
  char action;
  int source_adr;
  int i = 0;

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

  i2c.performAction(action, source_adr, k, n1, cont);
}
