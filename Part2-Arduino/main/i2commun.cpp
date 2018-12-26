#include "i2commun.h"

using namespace std;

I2COMMUN::I2COMMUN()
  : myAddr(0), last_node(0), first_node(0), destination(-1),
    nr_nos(0), deskStatus(0), counterAck(0), maxLux(0),
    prev_switch_value(HIGH), switch_debounce(200), prev_switch_time(0)
{

}

//check occupation state through a button switch
void I2COMMUN::check_switch( Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {
  switch_value = digitalRead(switch_pin);

  if (switch_value == HIGH && prev_switch_value == LOW && (switch_debounce + prev_switch_time) < millis()) {
    prev_switch_time = millis();

    _cont.vi = _cont.y;
    if (_cont.desired_lux == OCCUPIED)
    {
      _cont.desired_lux = UNOCCUPIED;
      if (_cont.vi > _cont.vf) //to avoid flickering due to simulator function
        _cont.vi = _cont.vf;
    }
    else
    {
      _cont.desired_lux = OCCUPIED;
      if (_cont.vi < _cont.vf) //to avoid flickering due to simulator function
        _cont.vi = _cont.vf;
    }

    Serial.println("!!!!!!!!!! MUDOU O BOTAO !!!!!!!!!!");

    if (nr_nos > 1)
    {
      if (_cont.desired_lux > maxLux)
        _n1.L = maxLux;
      else
        _n1.L = _cont.desired_lux;

      _n1.consensus_init(nr_nos);

      write_i2c((uint8_t) 0x00, 'd');
      deskStatus = CONSENSUS;
    }
    else
    {
      _cont.environment_init(_cont.desired_lux, _k[myAddr]);
      deskStatus = GENERAL;
    }
  }

  prev_switch_value = switch_value;
}




void I2COMMUN::checkStatus(Vector <float>& _k, Node& _n1, CONTROLLER& _cont) {
  char inputChar;
  String flag = "";

  //read desired iluminance from the serial
  while (Serial.available() > 0) {
    inputChar = Serial.read();

    if (inputChar != '\n')
      flag += inputChar;
    else
      Serial.println("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
      //changeStatus(flag, _k, _n1, _cont);
  }
}

//update desired flags states
void I2COMMUN::changeStatus(String flag, Vector <float>& _k, Node& _n1, CONTROLLER& _cont) {
  char *aux_str = flag.c_str();
  char str[50];

  sscanf(aux_str, "%s", str);

  if (!strcmp(str, "x")) {

    /*

      _cont.vi = _cont.y;
      if (_cont.desired_lux == OCCUPIED)
      {
      _cont.desired_lux = UNOCCUPIED;
      if (_cont.vi > _cont.vf) //to avoid flickering due to simulator function
        _cont.vi = _cont.vf;
      }
      else
      {
      _cont.desired_lux = OCCUPIED;
      if (_cont.vi < _cont.vf) //to avoid flickering due to simulator function
        _cont.vi = _cont.vf;
      }

      Serial.println("!!!!!!!!!! MUDOU O BOTAO !!!!!!!!!!");

      if (nr_nos > 1)
      {
      if (_cont.desired_lux > maxLux)
        _n1.L = maxLux;
      else
        _n1.L = _cont.desired_lux;

      _n1.consensus_init(nr_nos);

      write_i2c((uint8_t) 0x00, 'd');
      deskStatus = CONSENSUS;
      }
      else
      {
      _cont.environment_init(_cont.desired_lux, _k[myAddr]);
      deskStatus = GENERAL;

      if(_cont.desired_lux == UNOCCUPIED)
        send_RPI_button( 254, _cont.desired_lux );
      else
        send_RPI_button( 255, _cont.desired_lux );
      }
    */
    Serial.println("--------------É UM XXXXXXXXXXXXXXXXXXXXXXXX--------------------");
  }

  memset(str, 0, 20);
}

int I2COMMUN::checkAddress( int _myAddr, Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {

  for (int i = 0; i < 2; i++)
    _k[i] = -1;

  //esta so assim pq ainda se vai meter aqui a resolução de conflitos caso já exista este endereço
  myAddr = _myAddr;
  _k[myAddr] = 0;

  _n1.index = myAddr;

  _cont.m = -0.7;

  if (myAddr == 1)
    _cont.b = 2.2472;
  else
    _cont.b = 1.9252;

  return myAddr;
}

void I2COMMUN::write_i2c(uint8_t dest_address, char action) {
  Wire.beginTransmission(dest_address);
  Wire.write(action);
  Wire.write((uint8_t) myAddr);
  Wire.endTransmission();
}

void I2COMMUN::findAllNodes(Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {
  Serial.println("--------------- FIND ALL NODES ---------------");

  nr_nos = 1;

  first_node = myAddr;
  last_node = myAddr;

  write_i2c((uint8_t) 0x00, 'h');

  unsigned long aux;
  aux = millis();

  while (1) {
    if (millis() - aux >= 1000)
      break;
  }

  Serial.print("nr_nos: ");
  Serial.println(nr_nos);

  check_flags(_k, _n1, _cont);

  if (myAddr != first_node)
  {
    write_i2c((uint8_t) first_node, 'x');
    Serial.println("enviei um x");
  }

  if ( nr_nos == 1 || ( (nr_nos > 1) && (myAddr == first_node) ) )
  {
    deskStatus = START_CALIBRATION;
  }
}

void I2COMMUN::readOwnPerturbation( Node& _n1, CONTROLLER& _cont ) {

  _cont.ext_ilum = analogRead(A0);
  _cont.vi = _cont.ext_ilum * VCC / (1023.0);
  _cont.ext_ilum = _cont.convert_ADC_to_Lux(_cont.ext_ilum);

  _n1.o = _cont.ext_ilum;

  Serial.print("_cont.ext_ilum: ");
  Serial.println(_cont.ext_ilum);

  write_i2c((uint8_t) destination, 'k');
}

void I2COMMUN::getK( Vector <float>& _k, CONTROLLER& _cont ) {
  float aux;

  //meter um if k[k_nr]!=-1 e !=0 ??
  //btw com isto tamos smp a recalcular todas as entradas
  aux = analogRead(A0);
  aux = _cont.convert_ADC_to_Lux(aux);

  Serial.print("aux: ");
  Serial.println(aux);

  _k[destination] = (aux - _cont.ext_ilum) / 255;

  Serial.print("destination: ");
  Serial.println(destination);

  Serial.print("_k[destination]: ");
  Serial.println(_k[destination]);

  write_i2c((uint8_t) destination, 'k');
}

int I2COMMUN::getNextOne( Vector <float>& _k ) {

  if (myAddr == last_node )
    return -1;

  int _cur = myAddr + 1;

  while (1)
  {
    if (_k[_cur] != -1)
      return _cur;
    _cur++;
  }

  return -1 ;
}

int I2COMMUN::waitingAck( Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {
  unsigned long startTime = millis();

  while (1) {
    delayMicroseconds(10);

    if (counterAck >= (nr_nos - 1)) {
      counterAck = 0;
      break;
    }

    if ((millis() - startTime) > 1000)
    {
      Serial.println("PORQUE E QUE VIM AQUI? :(");

      deskStatus = 0;
      for (int i = 0; i < nr_nos; i++)
        _k[i] = -1;
      myAddr = checkAddress( myAddr, _k, _n1, _cont );
      findAllNodes(_k, _n1, _cont);

      return 0;
    }
  }

  return 1;
}


void I2COMMUN::recalibration( Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {
  int next_node;

  analogWrite(pin_led, 255);
  delay(200);

  //mandar broadcast para todos lerem com a minha luminosidade maxima
  write_i2c((uint8_t) 0x00, 'm');

  _cont.lux_max = analogRead(A0);
  _cont.lux_max = _cont.convert_ADC_to_Lux(_cont.lux_max);

  Serial.print("_cont.lux_max: ");
  Serial.println(_cont.lux_max);

  _k[myAddr] = (_cont.lux_max - _cont.ext_ilum) / 255;
  //ver o meu k e o k dos outros;
  //meter o codigo aqui

  Serial.print("_k[myAddr]: ");
  Serial.println(_k[myAddr]);

  if (!waitingAck(_k, _n1, _cont))
    return;

  next_node = getNextOne(_k);
  analogWrite(pin_led, 0);
  delay(100);

  Serial.print("next_node: ");
  Serial.println(next_node);

  if (next_node != -1)
  {
    write_i2c((uint8_t) next_node, 's');
    deskStatus = 0;
  }
  else
  {
    Serial.println("--------------- ACABOU A CALIBRACAO ---------------");

    if ( nr_nos > 1 )
    {
      maxLux = 0;

      for (int i = 0; i < nr_nos; i++)
        _k[i] = _k[i] * 2.55;

      _n1.n = _k.quad_norm();
      _n1.m = _n1.n - pow( _k[_n1.index], 2 );

      // to don't allow more lux than it is possible
      for (int i = 0; i < nr_nos; i++)
        maxLux = maxLux + _k[i] * 100;

      maxLux = maxLux + _n1.o;

      Serial.print("max Lux: ");
      Serial.println(maxLux);

      if (_cont.desired_lux > maxLux)
        _n1.L = maxLux;
      else
        _n1.L = _cont.desired_lux;

      //enviar uma flag para começar o consensus
      write_i2c((uint8_t) 0x00, 'c');
      deskStatus = CONSENSUS;
    }
    else
    {
      _cont.environment_init(_cont.desired_lux, _k[myAddr]);
      deskStatus = GENERAL;
    }

  }

  send_RPI_calibration(_cont);
}

void I2COMMUN::start_calibration(Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {

  analogWrite(pin_led, 0);

  //todos desligarem o led
  write_i2c((uint8_t) 0x00, 'v');

  //verificar que todos desligaram o led (ver o que fazer caso nao desliguem)
  //manda para todos a informaçao de que podem ler a perturb. externa!
  if (!waitingAck(_k, _n1, _cont))
    return;

  delay(200);

  write_i2c((uint8_t) 0x00, 'l');

  //para o caso do first node, este lê já aqui o seu
  _cont.ext_ilum = analogRead(A0);
  _cont.vi = _cont.ext_ilum * VCC / (1023.0);
  Serial.print("_cont.vi: ");
  Serial.println(_cont.vi);
  _cont.ext_ilum = _cont.convert_ADC_to_Lux(_cont.ext_ilum);

  _n1.o = _cont.ext_ilum;

  Serial.print("_cont.ext_ilum: ");
  Serial.println(_cont.ext_ilum);

  //verificar que ja todos mediram a sua perturb externa
  //depois o 1º começa a acender, dps o 2º.. etc
  if (!waitingAck(_k, _n1, _cont))
    return;

  //pq ja estamos no node 1, entao ele vai começar a calibrar-se ja
  deskStatus = RECALIB;

  Serial.print("deskStatus: ");
  Serial.println(deskStatus);
}

void I2COMMUN::check_flags( Vector <float>& _k, Node& _n1, CONTROLLER& _cont ) {

  switch (deskStatus) {
    case SEND_MY_ADDRESS:
      write_i2c((uint8_t) destination, 'a');

      deskStatus = 0;
      destination = -1;
      break;

    case START_CALIBRATION:
      start_calibration(_k, _n1, _cont);
      break;

    case LED_OFF:
      write_i2c((uint8_t) destination, 'k');

      deskStatus = 0;
      destination = -1;
      break;

    case PERTURBATION:
      readOwnPerturbation( _n1, _cont );

      deskStatus = 0;
      destination = -1;
      break;

    case RECALIB:
      recalibration( _k, _n1, _cont );
      break;

    case COMPUTE_K:
      getK( _k, _cont );

      deskStatus = 0;
      destination = -1;
      break;
  }
}

void I2COMMUN::performAction( char _action, int _source_adr, Vector <float>& _k, Node& _n1, CONTROLLER& _cont )
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
      deskStatus = RECALIB;
      break;

    case 'c':
      maxLux = 0;

      for (int i = 0; i < nr_nos; i++)
        _k[i] = _k[i] * 2.55;

      _n1.n = _k.quad_norm();
      _n1.m = _n1.n - pow( _k[_n1.index], 2 );

      // to don't allow more lux than it is possible
      for (int i = 0; i < nr_nos; i++)
        maxLux = maxLux + _k[i] * 100;

      maxLux = maxLux + _n1.o;

      Serial.print("max Lux: ");
      Serial.println(maxLux);

      if (_cont.desired_lux > maxLux)
        _n1.L = maxLux;
      else
        _n1.L = _cont.desired_lux;

      //enviar uma flag para começar o consensus
      write_i2c((uint8_t) 0x00, 'c');
      deskStatus = CONSENSUS;
      break;

    case 'd':
      if (_cont.desired_lux > maxLux)
        _n1.L = maxLux;
      else
        _n1.L = _cont.desired_lux;

      _n1.consensus_init(nr_nos);

      deskStatus = CONSENSUS;
      break;
  }
}

void I2COMMUN::send_RPI_button( int _button_status, float _myLux )
{
  double intMyLux, fracMyLux;

  fracMyLux = modf ((double) _myLux , &intMyLux);

  Wire.beginTransmission(0x48);
  Wire.write((uint8_t) _button_status);
  Wire.write((uint8_t) myAddr);
  Wire.write((uint8_t) intMyLux);
  Wire.write((uint8_t) (100 * fracMyLux));
  uint8_t status_Escrita = Wire.endTransmission();

  
}


void I2COMMUN::send_RPI_calibration( CONTROLLER& _cont )
{
  double intLBOff, fracLBOff, intLBOn, fracLBOn, intExtIlum, fracExtIlum;

  fracLBOff = modf ((double) UNOCCUPIED , &intLBOff);
  fracLBOn = modf ((double) OCCUPIED , &intLBOn);
  fracExtIlum = modf ((double) _cont.ext_ilum , &intExtIlum);

  Wire.beginTransmission(0x48);
  Wire.write((uint8_t) (myAddr + 127));
  Wire.write((uint8_t) intLBOff);
  Wire.write((uint8_t) (100 * fracLBOff));
  Wire.write((uint8_t) intLBOn);
  Wire.write((uint8_t) (100 * fracLBOn));
  Wire.write((uint8_t) intExtIlum);
  Wire.write((uint8_t) (100 * fracExtIlum));
  uint8_t status_Escrita = Wire.endTransmission();

  //Serial.print("********************* Status:");
  //Serial.println(status_Escrita);
}

void I2COMMUN::send_RPI_sample( CONTROLLER& _cont )
{
  double intLux, fracLux;

  fracLux = modf ((double) _cont.convert_ADC_to_Lux( _cont.y * (1023.0 / 5) ) , &intLux);

  Wire.beginTransmission(0x48);
  Wire.write((uint8_t) myAddr);
  Wire.write((uint8_t) intLux);
  Wire.write((uint8_t) (100 * fracLux));
  Wire.write((uint8_t) _cont.controller_output);
  uint8_t status_Escrita = Wire.endTransmission();

  //Serial.print("********************* Status:");
  //Serial.println(status_Escrita);
}
