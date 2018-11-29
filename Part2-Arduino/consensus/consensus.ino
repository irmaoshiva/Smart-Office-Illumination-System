#include "vector.h"
#include "node.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000);
  
  Node n1;
  Node n2;

  n1.d[0] = 1;
  n1.d[1] = 3;

  n1.d_av[0] = 1;
  n1.d_av[1] = 3;

  int x = n1.d*n1.d_av;

  Serial.println(x);
}

void loop() {
  // put your main code here, to run repeatedly:

}
