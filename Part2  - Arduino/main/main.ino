#include "i2commun.h"
#include <EEPROM.h>

int led_pin = 3;

I2COMMUN i2c(led_pin);

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
}


void loop() {

  i2c.check_flags();

  kklkljljljllkjlk
}

void receiveEvent(int howMany) {
  char c;
  char action;
  int source_adr;
  String information = "";

  //check data on BUS
  if (Wire.available() > 0)
  {
    action = Wire.read();
    source_adr = Wire.read();
  }

  while (Wire.available())
  {
    c = Wire.read();
    information += c;
  }

  Serial.print("Action: ");
  Serial.println(action);

  Serial.print("Source_adr: ");
  Serial.println(source_adr);

  i2c.performAction(action, source_adr, information);
}
