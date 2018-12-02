#include "i2commun.h"
#include <EEPROM.h>

int led_pin = 3;

I2COMMUN i2c(led_pin);

//aqui devia se o last_node se calhar podemos mudar o retorno da funçao do find_all_nodes
Vector <float> k = Vector <float> (128);

//criar o nó
Node n1(0.07, 1, 50, 80, 0, k);
//Node n2(0.07, 1, 50, 270, 1);

int my_adr;
int nr_nos = 1;

int iterations = 0;

int flag_timer = true;

void setup() {
  Serial.begin(2000000);
  my_adr = EEPROM.read(0);

  Serial.print("my_adr: ");
  Serial.println(my_adr);

  Wire.begin(my_adr); // receive data

  for (int i = 0; i < 128; i++)
    k[i] = -1;

  my_adr = i2c.checkAdress(my_adr, k);

  Wire.onReceive(receiveEvent); //event handler

  //active broadcast
  TWAR = (my_adr << 1) | 1;

  nr_nos = i2c.findAllNodes();
}


void loop() {

  i2c.check_flags(k);

  if (flag_timer) {

    if (i2c.consensus)
    {

      //acrescentar condiçao de saida com d - d_av
      if ( iterations < 50 ) {
        n1.Primal_solve(k);
        i2c.write_i2c((byte) 0x00, (byte)'z');
        for (int j = 0; j < nr_nos; j++)
        {
          Wire.beginTransmission(0x00);
          Wire.write((uint8_t)round(n1.d[j]));
          Wire.endTransmission();
        }

        while (1) {
          if (i2c.ack_consensus >= nr_nos)
          {
            i2c.ack_consensus = 0;
            break;
          }
        }

        n1.aux_soma = n1.aux_soma + n1.d;
        n1.d_av = n1.aux_soma * (1 / nr_nos);
        for (int j = 0; j < nr_nos; j++)
          n1.aux_soma[j] = 0;
        n1.y = n1.y + (n1.d - n1.d_av) * n1.rho;

        iterations ++;
      }
    }

    //cont.general();

    flag_timer = false;

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
    n1.aux_soma[i] = n1.aux_soma[i] + Wire.read();
    i++;
  }

  Serial.print("Action: ");
  Serial.println(action);

  Serial.print("Source_adr: ");
  Serial.println(source_adr);

  i2c.performAction(action, source_adr, k);
}
