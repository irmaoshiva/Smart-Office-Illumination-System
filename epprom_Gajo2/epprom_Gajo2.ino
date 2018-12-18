#include <EEPROM.h>
float m = -0.7;
float b = 2.2472;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);
EEPROM.write(0,1);
EEPROM.write(1,m);
EEPROM.write(2,b);
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println(EEPROM.read(0));
}
