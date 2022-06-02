INCLUDE generated/output_format.ld
ENTRY(_start)

__DYNAMIC = 0;

/* 
 * We include our own 'regions.ld', with main_ram that starts 256 KB
 * later (the first 256 KBs are kept fo the OS).
 */
INCLUDE regions.ld

SECTIONS
{
	.text :
	{
		_ftext = .;
		/* Make sure crt0 files come first, and they, and the isr */
		/* don't get disposed of by greedy optimisation */
		*crt0*(.text)
		KEEP(*crt0*(.text))
		KEEP(*(.text.isr))
		KEEP(*(.text.isr_handler))
		*(EXCLUDE_FILE(*ESP*tusb*.o) .text .stub .text.* .gnu.linkonce.t.*)
		_etext = .;
	} > main_ram

	.rodata :
	{
		. = ALIGN(8);
		_frodata = .;
		*(.rodata .rodata.* .gnu.linkonce.r.*)
		*(.rodata1)
		. = ALIGN(8);
		_erodata = .;
	} > main_ram

	.data :
	{
		. = ALIGN(8);
		_fdata = .;
		*(EXCLUDE_FILE(*usb*.o) .data .data.* .gnu.linkonce.d.*)
		*(EXCLUDE_FILE(*usb*.o) .data1)
		_gp = ALIGN(16);
		*(EXCLUDE_FILE(*usb*.o) .sdata .sdata.* .gnu.linkonce.s.*)
		. = ALIGN(8);
		_edata = .;
	} > main_ram

	.bss :
	{
		. = ALIGN(8);
		_fbss = .;
		*(.dynsbss)
		*(.sbss .sbss.* .gnu.linkonce.sb.*)
		*(.scommon)
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(8);
		_ebss = .;
		_end = .;
	} > main_ram
	
	
	.fast_text :
	{
        . = ALIGN (256);
	    _fast_text_loadaddr = LOADADDR(.fast_text);
		_fast_text = .;
		/* *ESP*tusb*(.text) */
	} > sram AT > main_ram
	
	.fast_data : 
	{
	    *usb*.o(.data .data.* .gnu.linkonce.d.*)
	    *usb*.o(.data1)
		*usb*.o(.sdata .sdata.* .gnu.linkonce.s.*)
		*(.fast_data)
		_efast_text = .;
	} > sram AT > main_ram
    	
}

PROVIDE(_fstack = ORIGIN(main_ram) + LENGTH(main_ram) - 4);

PROVIDE(_fdata_rom = LOADADDR(.data));
PROVIDE(_edata_rom = LOADADDR(.data) + SIZEOF(.data));
