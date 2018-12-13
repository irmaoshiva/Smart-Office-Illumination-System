#ifndef LUMINAIRE
#define LUMINAIRE

#include <string>
#include <unistd.h>
#include <memory.h>

#include <stdio.h>
#include <iostream>
#include <pigpio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <mutex>
#define SLAVE_ADDR_DEFAULT 0x00

class luminaire{
	bsc_xfer_t xfer;
	int status;
	int read_state = 1;
	std::mutex mtx;

public:
	luminaire();
	~luminaire();
	void read_data();
	void stop_read();
	void resume_read();
	void change_slave_addr(int addr);

private:
	int init_slave(bsc_xfer_t &xfer, int addr);
	//int close_slave(bsc_xfer_t &xfer);

};

#endif //LUMINAIRE