// This file is Copyright (c) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
// License: BSD-2-Clause

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "irq.h"
#include "uart.h"


void loop(void);
void setup(void);

void /*NORETURN*/ hard_reset(void) { ctrl_reset_write(1); for(;;); } //TODO: move to SDK

void _putchar(char c) { uart_write(c); }



int main(int argc, char **argv) {

    irq_setmask(0);
    irq_setie(1);

    uart_init();
/*
    printf("myfloat %d %d %d %f %d %d %g %d\n", 1, 4, 5, 1.1f, -10, -11, 2.3, -12);
    printf("myfloat %d %d %d %f %d %g %d %d\n", 1, 4, 5, 1.1f, -10, 2.3, -11, -12);
    printf("myfloat %d %d %f %d %d %g %d\n", 4, 5, 1.1f, -10, -11, 2.3, -12);
    printf("myfloat %d %d %f %d %g %d %d\n", 4, 5, 1.1f, -10, 2.3, -11, -12);
  */  
    setup();
    for(;;)
      loop();

    irq_setie(0);
    irq_setmask(~0);

    hard_reset();
}

#include "usb_host.c"
#include "common/tusb_fifo.c" //TinyUSB queues


