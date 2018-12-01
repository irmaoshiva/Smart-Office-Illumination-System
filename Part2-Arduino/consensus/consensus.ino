#include "vector.h"
#include "node.h"
#include "math.h"

Node n1(0.07, 1, 50, 80, 0);
Node n2(0.07, 1, 50, 270, 1);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  for(int i=1; i<50;i++)
  {

      n1.k[0] = 2;
      n1.k[1] = 1;

      n2.k[0] = 1;
      n2.k[1] = 2;

      n2.n = n2.k.quad_norm();
      n2.m = n2.n - pow( n2.k[1], 2 );
      n1.Primal_solve();
      n2.Primal_solve();
      n1.d_av=(n1.d+n2.d)*0.5;
      n2.d_av=(n1.d+n2.d)*0.5;
      
      n1.y=n1.y+(n1.d-n1.d_av)*n1.rho;
      n2.y=n2.y+(n2.d-n2.d_av)*n2.rho;

      
  }
 // n1.d=n1.d_av;
  Serial.println("a solução final é");
  Serial.println(n1.d[0]);
  Serial.println(n1.d[1]);
    Serial.println(n1.c);

  
    Serial.println(n2.d[0]);
  Serial.println(n2.d[1]);
      Serial.println(n1.c);


  

}

void loop() {

  // put your main code here, to run repeatedly:
}
