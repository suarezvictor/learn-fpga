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

typedef void (*timer_isr_t)(void); //FIXME: redefinition
timer_isr_t timer_handler = NULL;
void timer0_isr(void)
{
  timer0_ev_pending_write(timer0_ev_pending_read()); //FIXME: add to SDK
  if(timer_handler)
  {
    timer_handler();
  }
}

void litex_timer_setup(uint32_t usec, timer_isr_t handler)
{
   timer_handler = handler;
   litetimer_t *tim = litetimer_instance(0);
     
   litetimer_set_periodic_cycles(tim, litetimer_us_to_cycles(tim, usec));

   
   //printf("before %04X\n", irq_getmask());
   irq_setmask(irq_getmask() | (1 << TIMER0_INTERRUPT));
   //printf("interrupt mask after %04X\n", irq_getmask());

   timer0_ev_pending_write(timer0_ev_pending_read()); //FIXME: move to SDK
   timer0_ev_enable_write(1); //FIXME: move to SDK
   litetimer_start(tim);
   
   /*
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
   */
}

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



