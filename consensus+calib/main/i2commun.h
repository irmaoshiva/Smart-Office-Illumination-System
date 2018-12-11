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


#define SEND_MY_ADDRESS 1
#define START_CALIBRATION 2
#define LED_OFF 3
#define PERTURBATION 4
#define RECALIB 5
#define COMPUTE_K 6
#define CONSENSUS 7

class I2COMMUN
{
  private:

    int my_adr;
    int pin_led;

    int last_node;
    int first_node;

    int destination;

    float ext_ilum;
    float lux_max;

    volatile uint8_t counterAck;

    //isto é para tirar daqui
    float m;
    float b;
    //R1 em KOhm
    int R1;

  public:

    int nr_nos;

    volatile int deskStatus;

    I2COMMUN();
    I2COMMUN( int _pin_led );
    float convert_ADC_to_Lux( float Vi_value );
    int checkAdress( int _my_adr, Vector <float>& _k );
    void write_i2c( uint8_t dest_address, char action );
    void findAllNodes();
    void readOwnPerturbation( Node& _n1 );
    void getK( Vector <float>& _k );
    int getNextOne( Vector <float>& _k );
    void waitingAck();
    void recalibration( Vector <float>& _k, Node& _n1 );
    void start_calibration( Node& _n1 );
    void check_flags( Vector <float>& _k, Node& _n1 );
    void performAction( char _action, int _source_adr, Vector <float>& _k, Node& _n1 );
};


#endif //I2COMMUN_H
