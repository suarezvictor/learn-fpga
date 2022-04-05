// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2014-2019 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2015 Yann Sionneau <ys@m-labs.hk>
// This file is Copyright (c) 2015 whitequark <whitequark@whitequark.org>
// This file is Copyright (c) 2019 Ambroz Bizjak <ambrop7@gmail.com>
// This file is Copyright (c) 2019 Caleb Jamison <cbjamo@gmail.com>
// This file is Copyright (c) 2018 Dolu1990 <charles.papon.90@gmail.com>
// This file is Copyright (c) 2018 Felix Held <felix-github@felixheld.de>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2018 Jean-François Nguyen <jf@lambdaconcept.fr>
// This file is Copyright (c) 2018 Sergiusz Bazanski <q3k@q3k.org>
// This file is Copyright (c) 2016 Tim 'mithro' Ansell <mithro@mithis.com>
// This file is Copyright (c) 2020 Franck Jullien <franck.jullien@gmail.com>
// This file is Copyright (c) 2020 Antmicro <www.antmicro.com>

// License: BSD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <irq.h>

#include "boot.h"
#include "readline.h"
#include "helpers.h"
#include "command.h"

#undef PROMPT
#define PROMPT "\e[92;1mliteOS\e[0m> "

#include <generated/csr.h>
#include <generated/soc.h>
#include <generated/mem.h>
#include <generated/git.h>

#include <libbase/console.h>
#include <libbase/crc.h>
#include <libbase/uart.h>

#include <liblitedram/sdram.h>

#include "lite_fb.h"

#define I 0xffffffff
#define O 0x00000000

static uint32_t pattern[4][4] = {
   {I,O,O,O},
   {O,I,O,O},
   {O,O,O,I},
   {O,O,I,O}
};


/* X11 memories :-) */
#ifdef CSR_VIDEO_FRAMEBUFFER_BASE
static void init_framebuffer(void) {
   uint32_t* ptr = fb_base;
   fb_off();
   for(int y=0; y<FB_HEIGHT; ++y) {   
      for(int x=0; x<FB_WIDTH; ++x) {
	 *ptr++ = pattern[x&3][y&3];
      }
   }
   fb_on();   
}
#endif


int upython_main(int argc, char **argv, char *stack_top_arg);
void start_micropython(int argc, char **argv)
{
    int stack_dummy;
    upython_main(argc, argv, (char*)&stack_dummy);
}


int main(int argc, char **argv)
{
	char buffer[CMD_LINE_BUFFER_SIZE];
	char *params[MAX_PARAM];
	char *command;
	struct command_struct *cmd;
	int nb_params;

#ifdef CONFIG_CPU_HAS_INTERRUPT
	irq_setmask(0);
	irq_setie(1);
#endif
#ifdef CSR_UART_BASE
	uart_init();
#endif


	printf("\n");
	printf("\e[1m        __   _ __      _  __\e[0m\n");
	printf("\e[1m       / /  (_) /____ | |/_/\e[0m\n");
	printf("\e[1m      / /__/ / __/ -_)>  <\e[0m\n");
	printf("\e[1m     /____/_/\\__/\\__/_/|_|\e[0m\n");
	printf("\e[1m   Build your hardware, easily!\e[0m\n");
	printf("\n");
	printf(" (c) Copyright 2012-2021 Enjoy-Digital\n");
	printf(" (c) Copyright 2007-2015 M-Labs\n");
	printf("\n");

#ifdef CONFIG_WITH_BUILD_TIME
	printf(" BIOS built on "__DATE__" "__TIME__"\n");
#endif
	printf("\n");
	printf(" Migen git sha1: "MIGEN_GIT_SHA1"\n");
	printf(" LiteX git sha1: "LITEX_GIT_SHA1"\n");
	printf("\n");
	printf("--=============== \e[1mSoC\e[0m ==================--\n");
	printf("\e[1mCPU\e[0m:\t\t%s @ %dMHz\n",
		CONFIG_CPU_HUMAN_NAME,
		CONFIG_CLOCK_FREQUENCY/1000000);
	printf("\e[1mBUS\e[0m:\t\t%s %d-bit @ %dGiB\n",
		CONFIG_BUS_STANDARD,
		CONFIG_BUS_DATA_WIDTH,
		(1 << (CONFIG_BUS_ADDRESS_WIDTH - 30)));
	printf("\e[1mCSR\e[0m:\t\t%d-bit data\n",
		CONFIG_CSR_DATA_WIDTH);
	printf("\e[1mROM\e[0m:\t\t%dKiB\n", ROM_SIZE/1024);
	printf("\e[1mSRAM\e[0m:\t\t%dKiB\n", SRAM_SIZE/1024);
#ifdef CONFIG_L2_SIZE
	printf("\e[1mL2\e[0m:\t\t%dKiB\n", CONFIG_L2_SIZE/1024);
#endif
#ifdef CSR_SPIFLASH_CORE_BASE
	printf("\e[1mFLASH\e[0m:\t\t%dKiB\n", SPIFLASH_MODULE_TOTAL_SIZE/1024);
#endif
#ifdef MAIN_RAM_SIZE
#ifdef CSR_SDRAM_BASE
	printf("\e[1mSDRAM\e[0m:\t\t%dKiB %d-bit @ %dMT/s ",
		MAIN_RAM_SIZE/1024,
		sdram_get_databits(),
		sdram_get_freq()/1000000);
	printf("(CL-%d",
		sdram_get_cl());
	if (sdram_get_cwl() != -1)
		printf(" CWL-%d", sdram_get_cwl());
	printf(")\n");
#else
	printf("\e[1mMAIN-RAM\e[0m:\t%dKiB \n", MAIN_RAM_SIZE/1024);
#endif
#endif
	printf("\n");


#ifdef CSR_VIDEO_FRAMEBUFFER_BASE
        init_framebuffer();
#endif   
   
	init_dispatcher();

    start_micropython(argc, argv);



	printf("--============= \e[1mConsole\e[0m ================--\n");
#if !defined(TERM_MINI) && !defined(TERM_NO_HIST)
	hist_init();
#endif
	printf("\n%s", PROMPT);
	while(1) {
		readline(buffer, CMD_LINE_BUFFER_SIZE);
		if (buffer[0] != 0) {
			printf("\n");
			nb_params = get_param(buffer, &command, params);
			cmd = command_dispatcher(command, nb_params, params);
			if (!cmd)
				printf("Command not found");
		}
		printf("\n%s", PROMPT);
	}
	return 0;
}
