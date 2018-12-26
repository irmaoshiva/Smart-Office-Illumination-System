#ifndef I2COMMUN_H
#define I2COMMUN_H

#include <stdlib.h>
#include <stdio.h>
#include <WSWire.h>
#include "Arduino.h"

#include "controller.h"
#include "node.h"
#include "vector.h"

#define SEND_MY_ADDRESS 1
#define START_CALIBRATION 2
#define LED_OFF 3
#define PERTURBATION 4
#define RECALIB 5
#define COMPUTE_K 6
#define CONSENSUS 7
#define GENERAL 8

class I2COMMUN
{
  private:

    int myAddr;
    const int pin_led = 3;

    //switch button control variables
    volatile uint8_t switch_value;
    volatile uint8_t prev_switch_value;
    long switch_debounce;
    long prev_switch_time;

    int last_node;
    int first_node;

    int destination;

    volatile uint8_t counterAck;

  public:

    int nr_nos;

    volatile int deskStatus;

    const int switch_pin = 10;

    float maxLux;

    I2COMMUN();
    void check_switch( Vector <float>& _k, Node& _n1, CONTROLLER& _cont );

    void checkStatus(Vector <float>& _k, Node& _n1, CONTROLLER& _cont);
    void changeStatus(String flag, Vector <float>& _k, Node& _n1, CONTROLLER& _cont);
    
    int checkAddress( int _myAddr, Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void write_i2c( uint8_t dest_address, char action );
    void findAllNodes( Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void readOwnPerturbation( Node& _n1, CONTROLLER& _cont );
    void getK( Vector <float>& _k, CONTROLLER& _cont );
    int getNextOne( Vector <float>& _k );
    int waitingAck( Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void recalibration( Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void start_calibration( Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void check_flags( Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void performAction( char _action, int _source_adr, Vector <float>& _k, Node& _n1, CONTROLLER& _cont );
    void send_RPI_button( int _button_status, float _myLux );
    void send_RPI_calibration( CONTROLLER& _cont );
    void send_RPI_sample( CONTROLLER& _cont );
};


#endif //I2COMMUN_H
