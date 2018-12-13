#include "luminaire.hpp"

luminaire::luminaire(){
    if(gpioInitialise()<0)
        printf("\nFailed to initialize gpio");
    else
        status = init_slave(xfer, SLAVE_ADDR_DEFAULT);
}

luminaire::~luminaire(){
    //close_slave(xfer);
    gpioTerminate();
    printf("luminaire destructed\n");
}

int luminaire::init_slave(bsc_xfer_t &xfer, int addr) {
    gpioSetMode(18, PI_ALT3);
    gpioSetMode(19, PI_ALT3);
    xfer.control = (addr<<16) |/* Slave address */
    (0x00<<13) | /* invert transmit status flags */
    (0x00<<12) | /* enable host control */
    (0x00<<11) | /* enable test fifo */
    (0x00<<10) | /* invert receive status flags */
    (0x01<<9) | /* enable receive */
    (0x01<<8) | /* enable transmit */
    (0x00<<7) | /* abort and clear FIFOs */
    (0x00<<6) | /* send control reg as 1st I2C byte */
    (0x00<<5) | /* send status regr as 1st I2C byte */
    (0x00<<4) | /* set SPI polarity high */
    (0x00<<3) | /* set SPI phase high */
    (0x01<<2) | /* enable I2C mode */
    (0x00<<1) | /* enable SPI mode */
    0x01 ; /* enable BSC peripheral */
    return bscXfer(&xfer);
}

void luminaire::read_data(){
    while (1){
        mtx.lock();
        xfer.txCnt = 0;
        if (status = bscXfer(&xfer) < 0)
            break;
        if (xfer.rxCnt > 0){
            printf("\nReceived %d bytes\n", xfer.rxCnt);

            for(int j=0;j<xfer.rxCnt;j++)
            printf("%c",xfer.rxBuf[j]);
        }
        mtx.unlock();
    }
}

void luminaire::stop_read(){
    if (read_state){
        mtx.lock();
        read_state--;
    }
}

void luminaire::resume_read(){
    if(!read_state){
        mtx.unlock();
        read_state++;
    }

}

void luminaire::change_slave_addr(int addr){
    mtx.lock();
    status = init_slave(xfer, addr);
    if (status < 0){
        printf("\nAddress change failed, default address reset.\n");
        status = init_slave(xfer, addr);
    }
    mtx.unlock();
}

/*
int luminaire::close_slave(bsc_xfer_t &xfer){
xfer.control=0;
bscXfer(&xfer);
}
*/