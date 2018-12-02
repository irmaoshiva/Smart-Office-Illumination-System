#include "i2commun.h"

using namespace std;

I2COMMUN::I2COMMUN() {
  my_adr = 0;
  pin_led = 0;

  last_node = 0;
  first_node = 0;

  send_myAddress = 0;
  destination = 0;

  recalibration = 0;
  readPerturbation = 0;
  dest_perturb = 0;

  int i;
  for (i = 0; i < 128; i++)
    K[i] = -1;

  nr_nos = 0;

  ext_ilum = 0;
  lux_max = 0;

  ack_perturb = 0;
  nr_ledOff = 0;
  sendAckLedOff = 0;
  dest_ledOff = 0;
  start_calib = 0;
  ack_K = 0;
  flag_getK = 0;
  K_nr = -1;
  count_x = 0;

  consensus=0;
  ack_consensus=0;
  

  //isto é para tirar daqui
  m = 0;
  b = 0;
  //R1 em KOhm
  R1 = 0;
}

I2COMMUN::I2COMMUN(Node &_n1,int _pin_led) {
  n1=_n1;
  my_adr = 0;
  pin_led = _pin_led;

  last_node = 0;
  first_node = 0;

  send_myAddress = 0;
  destination = -1;

  recalibration = 0;
  readPerturbation = 0;
  dest_perturb = -1;

  int i;
  for (i = 0; i < 128; i++)
    K[i] = -1;

  nr_nos = 0;

  ext_ilum = 0;
  lux_max = 0;

  ack_perturb = 0;
  nr_ledOff = 0;
  sendAckLedOff = 0;
  dest_ledOff = -1;
  start_calib = 0;
  ack_K = 0;
  flag_getK = 0;
  K_nr = -1;
  count_x = 0;

  consensus=0;
  ack_consensus=0;

  //isto é para tirar daqui
  m = -0.7;
  b = 1.9252;
  //R1 em KOhm
  R1 = 10;
}

int I2COMMUN::checkAdress(int _my_adr) {

  //esta so assim pq ainda se vai meter aqui a resolução de conflitos caso já exista este endereço
  my_adr = _my_adr;
  K[my_adr] = 0;
  return my_adr;
}

void I2COMMUN::write_i2c(byte dest_address, byte action) {
  Wire.beginTransmission(dest_address);
  Wire.write(action);
  Wire.write((byte) my_adr);
  Wire.endTransmission();
}

int I2COMMUN::findAllNodes() {

  nr_nos = 1;
  int i;

  first_node = my_adr;
  last_node = my_adr;

  K[my_adr] = 0;

  write_i2c((byte) 0x00, (byte) 'h');

  unsigned long aux;
  aux = millis();

  while(1) {
    if(millis() - aux >= 2000)
      break;
  }

  Serial.print("nr_nos: ");
  Serial.println(nr_nos);

  if (my_adr != first_node)
  {
    write_i2c((byte) first_node, (byte) 'x');
    Serial.println("enviei um x");
  }

  if(nr_nos == 1)
  {
    start_calib=1;
    Serial.println("sou o unico vou começar a calibrar");
  }

  return nr_nos;
}

int I2COMMUN::getNextOne() {

  if (my_adr == last_node )
      return -1;
      
  int _cur = my_adr + 1;

  while(1)
  {
    if (K[_cur] != -1)
      return _cur;    
    _cur++;
  }

  return -1 ;
}

void I2COMMUN::readOwnPerturbation() {
  ext_ilum = analogRead(A0);
  ext_ilum = convert_ADC_to_Lux(ext_ilum);

  Serial.print("ext_ilum: ");
  Serial.println(ext_ilum);

  write_i2c((byte) dest_perturb, (byte) 'p');
}

void I2COMMUN::check_flags() {
  int error;
  int next_node;

  if (send_myAddress == 1)
  {

   Serial.println("Vou mandar um aaaaaaa");

    write_i2c((byte) destination, (byte) 'a');

    send_myAddress = 0;
    destination = -1;
  }

  if (sendAckLedOff)
  {
    write_i2c((byte) dest_ledOff, (byte) 'o');

    dest_ledOff = -1;
    sendAckLedOff = 0;
  }

  if (readPerturbation)
  {
    readOwnPerturbation();
    dest_perturb = -1;
    readPerturbation = 0;
  }

  if (flag_getK)
  {
    float aux;

    //meter um if k[k_nr]!=-1 e !=0 ??
    //btw com isto tamos smp a recalcular todas as entradas
    aux = analogRead(A0);
    aux = convert_ADC_to_Lux(aux);

    Serial.print("aux: ");
    Serial.println(aux);
    
    K[K_nr] = (aux - ext_ilum) / 255;

    Serial.print("K_nr: ");
    Serial.println(K_nr);

    Serial.print("K[K_nr]: ");
    Serial.println(K[K_nr]);

    write_i2c((byte) K_nr, (byte) 'y');

    flag_getK = 0;
  }

  if (recalibration)
  {

    Serial.println("La La LA");
    
    analogWrite(pin_led, 255);
    unsigned prev_time = millis();

    while (1)
    {
      if ((millis() - prev_time) > 150)
        break;
    }

    lux_max = analogRead(A0);
    lux_max = convert_ADC_to_Lux(lux_max);

    Serial.print("lux_max: ");
    Serial.println(lux_max);
    
    K[my_adr] = (lux_max - ext_ilum) / 255;
    //ver o meu k e o k dos outros;
    //meter o codigo aqui

    Serial.print("K[my_adr]: ");
    Serial.println(K[my_adr]);

    //mandar broadcast para todos lerem com a minha luminosidade maxima
    write_i2c((byte) 0x00, (byte) 'm');

    while(1) {
      if(ack_K >= (nr_nos - 1))
        break;

      Serial.print("ack_K: ");
      Serial.println(ack_K);
    }

    next_node = getNextOne();
    analogWrite(pin_led, 0);
    delay(100);

    Serial.print("next_node: ");
    Serial.println(next_node);
    
    if (next_node != -1)
    {
      write_i2c((byte) next_node, (byte) 's');
    }
    else
    {
      Serial.println("acabou a calibração");
      //enviar uma flag para começar o consensus
       write_i2c((byte) 0x00, (byte) 'c');
       consensus=1;

     
    }

    recalibration = 0;
  }

  if (start_calib)
  {
    start_calibration();
    start_calib = 0;
  }

  if(consensus)
  {

    //se calhar é melhor meter i=1:50 para começar a testar com o do stor a ver se dá bem
    
    /*while(1)
    {
      if((n1.d-n1.d_av).quad_norm()<0.5)
        break; */
    for (int i=1; i<50;i++)
    {
        n1.Primal_solve();
         write_i2c((byte) 0x00, (byte)'z');
        for (int j=0; j<nr_nos; j++)
        { 
          Wire.beginTransmission(0x00);
          Wire.write((uint8_t)round(n1.d[j]));
          Wire.endTransmission();
        }
        
        while(1){
           if(ack_consensus>=nr_nos)
           {
            ack_consensus=0;
            break;
           }
        }
         
         n1.aux_soma=n1.aux_soma+n1.d;
        n1.d_av=n1.d_av*(1/nr_nos);
        for (int j=0; j<nr_nos;j++)
          n1.aux_soma[j]=0;
       n1.y=n1.y+(n1.d-n1.d_av)*n1.rho;   

    }
  }
}




void I2COMMUN::performAction(char _action, int _source_adr)
{

  switch (_action) {

    case 'h':
      send_myAddress = 1;
      destination = _source_adr;
      //qd entra um novo a meio, tenho de recalcular a rede toda
      if (K[_source_adr] == -1)
      {
        if (_source_adr < first_node)
          first_node = _source_adr;
        if (_source_adr > last_node)
          last_node = _source_adr;
        nr_nos++;
        K[_source_adr] = 0;

        Serial.print("nr_nos: ");
        Serial.println(nr_nos);
        
        Serial.print("first_node: ");
        Serial.println(first_node);

        Serial.print("last_node: ");
        Serial.println(last_node);
      }
      break;

    case 'a':
      if (K[_source_adr] == -1)
      {
        if (_source_adr < first_node)
          first_node = _source_adr;
        if (_source_adr > last_node)
          last_node = _source_adr;
        nr_nos++;
        K[_source_adr] = 0;

        Serial.print("nr_nos: ");
        Serial.println(nr_nos);
        
        Serial.print("first_node: ");
        Serial.println(first_node);

        Serial.print("last_node: ");
        Serial.println(last_node);
      }
      break;

    case 's':
      recalibration = 1;
      break;

    case 'v':
      analogWrite(pin_led, 0);
      sendAckLedOff = 1;
      dest_ledOff = _source_adr;
      break;

    case 'o':
      nr_ledOff++;
      break;

    case 'l':
      readPerturbation = 1;
      dest_perturb = _source_adr;
      break;

    case 'p':
      ack_perturb++;
      break;

    case 'x':
      count_x++;
      if (count_x = (nr_nos - 1))
        start_calib = 1;
      break;

    case 'm':
      flag_getK = 1;
      K_nr = _source_adr;
      break;

    case 'y':
      ack_K++;
      break;

    case 'c':
      consensus=1;
      break;

     case 'z':
      ack_consensus++;      
      break;

  }
}



void I2COMMUN::start_calibration() {

  analogWrite(pin_led, 0);
  delay(200);

  //todos desligarem o led
  write_i2c((byte) 0x00, (byte) 'v');

  //verificar que todos desligaram o led (ver o que fazer caso nao desliguem)
  //manda para todos a informaçao de que podem ler a perturb. externa!
  while (1) {
    if(nr_ledOff >= (nr_nos - 1))
      break;

    Serial.print("nr_ledOff: ");
    Serial.println(nr_ledOff);
  }
    
  write_i2c((byte) 0x00, (byte) 'l');

  //para o caso do first node, este lê já aqui o seu
  delay(100);
  ext_ilum = analogRead(A0);
  ext_ilum = convert_ADC_to_Lux(ext_ilum);

  Serial.print("ext_ilum: ");
  Serial.println(ext_ilum);

  //verificar que ja todos mediram a sua perturb externa
  //depois o 1º começa a acender, dps o 2º.. etc
  while(1){
    if(ack_perturb >= (nr_nos - 1))
      break;
  }

  //pq ja estamos no node 1, entao ele vai começar a calibrar-se ja
  recalibration = 1;
}



float I2COMMUN::convert_ADC_to_Lux(float Vi_value)  {
  float R2_1LUX = pow(10, b);
  float V_volt = Vi_value * VCC / 1023.0;
  float R2 = ( (VCC / V_volt) - 1 ) * R1;

  return pow( R2 / R2_1LUX, 1 / m);
}
