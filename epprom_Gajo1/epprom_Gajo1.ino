#include <EEPROM.h>
float m = -0.7;
float b = 1.9252;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);
EEPROM.write(0,0);
EEPROM.write(1,m);
EEPROM.write(2,b);
}

void loop() {
  // put your main code here, to run repeatedly:
Serial.println(EEPROM.read(0));
}
