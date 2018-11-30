#include "vector.h"
#include "node.h"

  Node n1(0.07,1,50,80,0);
  Node n2(0.07,1,50,270,1);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
       n1.Primal_solve();
}

void loop() {

  // put your main code here, to run repeatedly:
}
