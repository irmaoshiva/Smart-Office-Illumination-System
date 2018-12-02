#ifndef I2COMMUN_H
#define I2COMMUN_H

#include <stdlib.h>
#include <stdio.h>
#include "Arduino.h"
#include <WSWire.h>
#include "node.h"
#include "vector.h"

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
    int recalib;
    int readPerturbation;
    int dest_perturb;
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
    int nr_nos;


    Node n1;




    //isto é para tirar daqui
    float m;
    float b;
    //R1 em KOhm
    int R1;

  public:

    int consensus;

    int ack_consensus;

    I2COMMUN();
    I2COMMUN( int _pin_led );
    float convert_ADC_to_Lux( float Vi_value );
    int checkAdress( int _my_adr, Vector <float>& _k );
    void write_i2c( byte dest_address, byte action );
    int findAllNodes();
    void readOwnPerturbation();
    void getK( Vector <float>& _k );
    int getNextOne( Vector <float>& _k );
    void recalibration( Vector <float>& _k );
    void start_calibration();
    void check_flags( Vector <float>& _k );
    void performAction( char _action, int _source_adr, Vector <float>& _k );
};


#endif //I2COMMUN_H
