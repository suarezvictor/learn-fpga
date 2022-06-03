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

extern char _fstack[];
extern char _fast_text[];
extern char _efast_text[];
extern const char _fast_text_loadaddr[];
extern char _fast_data[];
extern char _efast_data[];
extern const char _fast_data_loadaddr[];
/*
extern char _fdata[];
extern char _edata[];
extern const char _fdata_rom[];*/
extern int timer0_isr_count, uart_isr_count;
void isr_handler(void);

int main(int argc, char **argv) {
    
    char *_start = _fast_text, *_end = _efast_text, *_src = _fast_text_loadaddr;
    size_t _size = _end - _start;
    memcpy(_start, _src, _size); //void *memcpy(void *dest, const void * src, size_t n)

    char *d_start = _fast_data, *d_end = _efast_data, *d_src = _fast_data_loadaddr;
    size_t d_size = d_end - d_start;
    memcpy(d_start, d_src, d_size); //void *memcpy(void *dest, const void * src, size_t n)

    irq_setmask(0);
    irq_setie(1);

    uart_init();
    //printf("start 0x%p = 0x%08X, src 0x%p = 0x%08X\n", _start, *(int *) _start, _src, *(int *) _src);
    printf("text start 0x%p, end 0x%p (size 0x%X), from 0x%p\n", _start, _end, _size, _src);
    printf("data start 0x%p, end 0x%p (size 0x%X), from 0x%p, stack 0x%p\n", d_start, d_end, d_size, d_src, _fstack);
    //printf("fdata start 0x%p, end 0x%p (size 0x%X), from 0x%p\n", _fdata, _edata, _edata-_fdata, _fdata_rom);
    setup();
    printf("interrupt handler 0x%p, timer0 count:%d, uart count:%d\n", isr_handler, timer0_isr_count, uart_isr_count);

    for(;;)
      loop();

    irq_setie(0);
    irq_setmask(~0);

    hard_reset();
}



