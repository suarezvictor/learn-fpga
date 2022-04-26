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

#include "usb_host.h"

int main(int argc, char **argv) {
    uart_init();
    irq_setmask(0);
    irq_setie(1);

    if((size_t)&main < 0x40000000)
      printf("sram\n");
    else
      printf("dram\n");
    hal_gpio_set_direction(8, GPIO_MODE_OUTPUT);
    hal_gpio_set_direction(12, GPIO_MODE_OUTPUT);
    for(;;)
    {
      hal_gpio_set_level(8, 0); hal_gpio_set_level(12, 1);
      hal_gpio_set_level(8, 1); hal_gpio_set_level(12, 0);
    }

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


