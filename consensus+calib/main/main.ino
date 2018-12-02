#include "i2commun.h"
#include <EEPROM.h>

int led_pin = 3;


//criar o nó
Node n1(0.07, 1, 50, 80, 0);

I2COMMUN i2c(n1,led_pin);

//Node n2(0.07, 1, 50, 270, 1);


int my_adr;
int nr_nos = 1;

void setup() {
  Serial.begin(2000000);
  my_adr = EEPROM.read(0);

  Serial.print("my_adr: ");
  Serial.println(my_adr);

  Wire.begin(my_adr); // receive data

  my_adr = i2c.checkAdress(my_adr);

  Wire.onReceive(receiveEvent); //event handler

  //active broadcast
  TWAR = (my_adr << 1) | 1;

  nr_nos = i2c.findAllNodes();

  //
  n1.k=i2c.K;
  n1.n=n1.k.quad_norm();
  n1.m = n1.n - pow( n1.k[1], 2 );

}


void loop() {

  i2c.check_flags();
}

void receiveEvent(int howMany) {
  char c;
  char action;
  int source_adr;
  int i=0;
  int flag=0;

  //check data on BUS
  if (Wire.available() > 0)
  {
    action = Wire.read();
    source_adr = Wire.read();
  }

  
  while (Wire.available())
  {
    n1.aux_soma[i] = n1.aux_soma[i]+Wire.read();
    i++;
   
  }

  Serial.print("Action: ");
  Serial.println(action);

  Serial.print("Source_adr: ");
  Serial.println(source_adr);

  i2c.performAction(action, source_adr);
}
