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


void isr(void);
void (*isrp)(void) = isr; //tests existence

int main(int argc, char **argv) {

    irq_setmask(0);
    irq_setie(1);

    uart_init();
    setup();
    for(;;)
      loop();

    irq_setie(0);
    irq_setmask(~0);

    hard_reset();
}

#include "usb_host.c"
#include "common/tusb_fifo.c" //TinyUSB queues


