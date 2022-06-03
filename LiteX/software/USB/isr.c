// This file is Copyright (c) 2020 Florent Kermarrec <florent@enjoy-digital.fr>
// License: BSD

#include <generated/csr.h>
#include <generated/soc.h>
#include <irq.h>
#include <libbase/uart.h>
#include "osal/osal.h"

void timer0_isr(void);
int timer0_isr_count = 0;
int uart_isr_count = 0;

#ifdef CONFIG_CPU_HAS_INTERRUPT
void FAST_CODE isr_handler(void) //name change to avoid conflict with default implementation FIXME: find where it is
{
	__attribute__((unused)) unsigned int irqs;

	irqs = irq_pending() & irq_getmask();

#if defined(UART_INTERRUPT) && !defined(UART_POLLING)
	if(irqs & (1 << UART_INTERRUPT))
	{
		uart_isr();
	    ++uart_isr_count;
	}
#endif
#if defined(TIMER0_INTERRUPT) && !defined(TIMER0_POLLING)
	//if(irqs & (1 << TIMER0_INTERRUPT))
	else
	{
		timer0_isr();
	    ++timer0_isr_count;
	}
#endif
}

#else

#error CONFIG_CPU_HAS_INTERRUPT should be enabled
#endif
