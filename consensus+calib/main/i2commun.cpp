#include "i2commun.h"

using namespace std;

I2COMMUN::I2COMMUN() {
  my_adr = 0;
  pin_led = 0;

  last_node = 0;
  first_node = 0;

  destination = 0;

  nr_nos = 0;
  deskStatus = 0;

  ext_ilum = 0;
  lux_max = 0;

  counterAck = 0;

  //isto é para tirar daqui
  m = 0;
  b = 0;
  //R1 em KOhm
  R1 = 0;
}

I2COMMUN::I2COMMUN( int _pin_led ) {
  my_adr = 0;
  pin_led = _pin_led;

  last_node = 0;
  first_node = 0;

  destination = -1;

  nr_nos = 0;
  deskStatus = 0;

  ext_ilum = 0;
  lux_max = 0;

  counterAck = 0;

  //isto é para tirar daqui
  m = -0.7;
  b = 1.9252;
  //R1 em KOhm
  R1 = 10;
}

float I2COMMUN::convert_ADC_to_Lux(float Vi_value)  {
  float R2_1LUX = pow(10, b);
  float V_volt = Vi_value * VCC / 1023.0;
  float R2 = ( (VCC / V_volt) - 1 ) * R1;

  return pow( R2 / R2_1LUX, 1 / m);
}

int I2COMMUN::checkAdress( int _my_adr, Vector <float>& _k ) {

  //esta so assim pq ainda se vai meter aqui a resolução de conflitos caso já exista este endereço
  my_adr = _my_adr;
  _k[my_adr] = 0;

  return my_adr;
}

void I2COMMUN::write_i2c(uint8_t dest_address, char action) {
  Wire.beginTransmission(dest_address);
  Wire.write(action);
  Wire.write((uint8_t) my_adr);
  Wire.endTransmission();
}

void I2COMMUN::findAllNodes() {

  nr_nos = 1;

  first_node = my_adr;
  last_node = my_adr;

  write_i2c((byte) 0x00, 'h');

  unsigned long aux;
  aux = millis();

  while (1) {
    if (millis() - aux >= 2000)
      break;
  }

  Serial.print("nr_nos: ");
  Serial.println(nr_nos);

  if (my_adr != first_node)
  {
    write_i2c((uint8_t) first_node, 'x');
    Serial.println("enviei um x");
  }

  if (nr_nos == 1)
  {
    deskStatus = START_CALIBRATION;
    Serial.println("Sou o unico vou começar a calibrar");
  }
}

void I2COMMUN::readOwnPerturbation( Node& _n1 ) {

  ext_ilum = analogRead(A0);
  ext_ilum = convert_ADC_to_Lux(ext_ilum);

  //_n1.o = ext_ilum;

  Serial.print("ext_ilum: ");
  Serial.println(ext_ilum);

  write_i2c((uint8_t) destination, 'k');
}

void I2COMMUN::getK( Vector <float>& _k ) {
  float aux;

  //meter um if k[k_nr]!=-1 e !=0 ??
  //btw com isto tamos smp a recalcular todas as entradas
  aux = analogRead(A0);
  aux = convert_ADC_to_Lux(aux);

  Serial.print("aux: ");
  Serial.println(aux);

  _k[destination] = (aux - ext_ilum) / 255;

  Serial.print("destination: ");
  Serial.println(destination);

  Serial.print("_k[destination]: ");
  Serial.println(_k[destination]);

  write_i2c((uint8_t) destination, 'k');
}

int I2COMMUN::getNextOne( Vector <float>& _k ) {

  if (my_adr == last_node )
    return -1;

  int _cur = my_adr + 1;

  while (1)
  {
    if (_k[_cur] != -1)
      return _cur;
    _cur++;
  }

  return -1 ;
}

void I2COMMUN::waitingAck() {
  while (1) {
    if (counterAck >= (nr_nos - 1)) {
      counterAck = 0;
      break;
    }
  }
}


void I2COMMUN::recalibration( Vector <float>& _k, Node& _n1 ) {
  int next_node;

  analogWrite(pin_led, 255);
  delay(200);

  //mandar broadcast para todos lerem com a minha luminosidade maxima
  write_i2c((byte) 0x00, (byte) 'm');

  lux_max = analogRead(A0);
  lux_max = convert_ADC_to_Lux(lux_max);

  Serial.print("lux_max: ");
  Serial.println(lux_max);

  _k[my_adr] = (lux_max - ext_ilum) / 255;
  //ver o meu k e o k dos outros;
  //meter o codigo aqui

  Serial.print("_k[my_adr]: ");
  Serial.println(_k[my_adr]);

  waitingAck();

  next_node = getNextOne(_k);
  analogWrite(pin_led, 0);
  delay(100);

  Serial.print("next_node: ");
  Serial.println(next_node);

  if (next_node != -1)
  {
    write_i2c((uint8_t) next_node, 's');
  }
  else
  {
    Serial.println("acabou a calibração");

    if ( nr_nos > 1 )
    {
      //enviar uma flag para começar o consensus
      write_i2c((uint8_t) 0x00, 'c');
      deskStatus = CONSENSUS;
      _k[0] = 2;
      _k[1] = 1;
      _n1.n = _k.quad_norm();
      _n1.m = _n1.n - pow( _k[_n1.index], 2 );
    }
  }
}

void I2COMMUN::start_calibration() {

  analogWrite(pin_led, 0);

  //todos desligarem o led
  write_i2c((uint8_t) 0x00, 'v');

  //verificar que todos desligaram o led (ver o que fazer caso nao desliguem)
  //manda para todos a informaçao de que podem ler a perturb. externa!
  waitingAck();

  delay(200);

  write_i2c((uint8_t) 0x00, 'l');

  //para o caso do first node, este lê já aqui o seu
  ext_ilum = analogRead(A0);
  ext_ilum = convert_ADC_to_Lux(ext_ilum);

  Serial.print("ext_ilum: ");
  Serial.println(ext_ilum);

  //verificar que ja todos mediram a sua perturb externa
  //depois o 1º começa a acender, dps o 2º.. etc
  waitingAck();

  //pq ja estamos no node 1, entao ele vai começar a calibrar-se ja
  deskStatus = RECALIBRATION;
}

void I2COMMUN::check_flags( Vector <float>& _k, Node& _n1 ) {
  if (deskStatus == SEND_MY_ADDRESS)
  {
    write_i2c((uint8_t) destination, 'a');

    deskStatus = 0;
    destination = -1;
  }

  if (deskStatus == START_CALIBRATION)
  {
    start_calibration();
    deskStatus = 0;
  }

  if (deskStatus == LED_OFF)
  {
    write_i2c((uint8_t) destination, 'k');

    deskStatus = 0;
    destination = -1;
  }

  if (deskStatus == PERTURBATION)
  {
    readOwnPerturbation( _n1 );

    deskStatus = 0;
    destination = -1;
  }

  if (deskStatus == RECALIBRATION)
  {
    recalibration(_k, _n1);

    deskStatus = 0;
  }

  if (deskStatus == COMPUTE_K)
  {
    getK(_k);
    deskStatus = 0;
    destination = -1;
  }

}

void I2COMMUN::performAction( char _action, int _source_adr, Vector <float>& _k, Node& _n1 )
{

  switch (_action) {

    case 'h':
      deskStatus = SEND_MY_ADDRESS;
      destination = _source_adr;

      //qd entra um novo a meio, tenho de recalcular a rede toda
      if (_k[_source_adr] == -1)
      {
        if (_source_adr < first_node)
          first_node = _source_adr;
        if (_source_adr > last_node)
          last_node = _source_adr;

        nr_nos++;
        _k[_source_adr] = 0;

        Serial.print("nr_nos: ");
        Serial.println(nr_nos);

        Serial.print("first_node: ");
        Serial.println(first_node);

        Serial.print("last_node: ");
        Serial.println(last_node);
      }
      break;

    case 'a':
      if (_k[_source_adr] == -1)
      {
        if (_source_adr < first_node)
          first_node = _source_adr;
        if (_source_adr > last_node)
          last_node = _source_adr;

        nr_nos++;
        _k[_source_adr] = 0;

        Serial.print("nr_nos: ");
        Serial.println(nr_nos);

        Serial.print("first_node: ");
        Serial.println(first_node);

        Serial.print("last_node: ");
        Serial.println(last_node);
      }
      break;

    case 'x':
      counterAck++;
      if (counterAck = (nr_nos - 1))
      {
        deskStatus = START_CALIBRATION;
        counterAck = 0;
      }
      break;

    case 'v':
      analogWrite(pin_led, 0);

      deskStatus = LED_OFF;
      destination = _source_adr;
      break;

    case 'k':
      counterAck++;
      break;

    case 'l':
      deskStatus = PERTURBATION;
      destination = _source_adr;
      break;

    case 'm':
      deskStatus = COMPUTE_K;
      destination = _source_adr;
      break;

    case 's':
      deskStatus = RECALIBRATION;
      break;

    case 'c':
      deskStatus = CONSENSUS;
      _k[0] = 2;
      _k[1] = 1;
      _n1.n = _k.quad_norm();
      _n1.m = _n1.n - pow( _k[0], 2 );
      break;
  }
}
