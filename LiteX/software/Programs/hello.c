// This file is Copyright (c) 2017-2021 Fupy/LiteX-MicroPython Developers
// This file is Copyright (c) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
// License: BSD-2-Clause

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "irq.h"
#include "uart.h"

//extern uint8_t _start, _fstack; //linker variables

void /*NORETURN*/ hard_reset(void) { ctrl_reset_write(1); for(;;); } //TODO: move to SDK

int upython_main(int argc, char **argv, char *stack_top_arg);

void isr(void);
void (*isrp)(void) = isr; //tests existence

int main(int argc, char **argv) {
    int stack_dummy;
    isrp = NULL;
    //safe way to determine stack top: no other variables in this function //TODO: use alloca()


    irq_setmask(0);
    irq_setie(1);
    uart_init();

#if 0
    printf("Micropython is booting!\n");
    while(upython_main(argc, argv, (char*)&stack_dummy) == 0)
        /*soft_reset()*/;
#else
    printf("Hello!\n");
    for(;;);
#endif
    irq_setie(0);
    irq_setmask(~0);

    hard_reset();
}

