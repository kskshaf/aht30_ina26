#ifndef EXIT_H
#define EXIT_H

#include "driver_aht30.h"
#define KEEP_FILE "/tmp/aht30_keep"

extern volatile sig_atomic_t keep_running;

void signal_handler(int signum);
int check_keep_file();

#endif