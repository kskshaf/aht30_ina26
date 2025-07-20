#ifndef INA226_H
#define INA226_H

#include "includes.h"
#include "config.h"
#include "exit.h"

extern pthread_t    ina226_thread;
extern volatile int ina226_thread_running;
extern volatile float ina226_voltage;

void* ina226_handler(void* arg);
int ina226_thread_handle(int handle);

#endif