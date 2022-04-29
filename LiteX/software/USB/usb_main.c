// This file is Copyright (c) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
// License: BSD-2-Clause

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <liblitesdk/litesdk_timer.h>
#include "irq.h"
#include "uart.h"


void loop(void);
void setup(void);

void /*NORETURN*/ hard_reset(void) { ctrl_reset_write(1); for(;;); } //TODO: move to SDK

void _putchar(char c) { uart_write(c); }

int timer_handler = 0;
void timer0_isr(void)
{
  ++timer_handler;
}
void timer_init()
{
   litetimer_t *tim = litetimer_instance(0);
   litetimer_set_periodic_cycles(tim, LITETIMER_BASE_FREQUENCY*8);
   
   printf("before %04X\n", irq_getmask());
   irq_setmask(irq_getmask() | (1 << TIMER0_INTERRUPT));
   printf("interrupt mask after %04X\n", irq_getmask());

   //timer0_ev_enable_write(1);
   
   litetimer_start(tim);
   for(;;)
   {
     static int sa  = 0;
     int s = litetimer_get_value_ms(tim)/1000;
     if(s != sa)
     {
       sa = s;
       printf("timer %d, %d ints\n", s, timer_handler);
     }
   }
   
}

int main(int argc, char **argv) {

    irq_setmask(0);
    irq_setie(1);

    uart_init();
    timer_init();
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


