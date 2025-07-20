#ifndef AHT30_2_H
#define AHT30_2_H

#include "includes.h"
#include "config.h"
#include "exit.h"

extern pthread_t    aht30_2_thread;
extern volatile int aht30_2_thread_running;

void* aht30_2_handler(void* arg);
int aht30_2_thread_handle(int handle);

#endif