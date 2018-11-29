#ifndef I2COMMUN_H
#define I2COMMUN_H

#include <stdlib.h>
#include <stdio.h>
#include "Arduino.h"
#include <WSWire.h>

#define OCCUPIED 60
#define UNOCCUPIED 20
#define FILTER_SAMPLES 20
#define VCC 5

class I2COMMUN
{
  private:

    int my_adr;
    int pin_led;

    int last_node;
    int first_node;

    int send_myAddress;
    int destination;
    int recalibration;
    int readPerturbation;
    int dest_perturb;

    float K[128];

    int nr_nos;

    float ext_ilum;
    float lux_max;

    int ack_perturb;
    int nr_ledOff;
    int sendAckLedOff;
    int dest_ledOff;
    int start_calib;
    int count_x;
    int ack_K;
    int flag_getK;
    int K_nr;




    //isto é para tirar daqui
    float m;
    float b;
    //R1 em KOhm
    int R1;

  public:


    I2COMMUN();
    I2COMMUN(int _pin_led);
    void write_i2c(byte dest_address, byte action);
    int checkAdress(int _my_adr);
    int findAllNodes();
    void check_flags();
    void performAction(char _action, int _source_adr, String _information);
    int getNextOne();
    void readOwnPerturbation();
    void start_calibration();
    float convert_ADC_to_Lux(float Vi_value);


};


#endif //I2COMMUN_H
