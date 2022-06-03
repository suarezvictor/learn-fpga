BUILD_DIR ?= $(BUILD_DIRECTORY)

LEARNFPGALIB_DIR = /media/vsuarez/elocaldata/SCRATCH/learn-fpga/LiteX/software/Libs

include ../makefile.inc
USBHOST_DIR=/media/vsuarez/elocaldata/tools/arduino-1.8.19/libraries/ESP32-USB-Soft-Host/src
INC += -I$(USBHOST_DIR)
INC += -I/media/vsuarez/elocaldata/CONFIDENTIAL/LiteX-consulting/micropython-private/ports/litex
INC += -I$(LEARNFPGALIB_DIR)
#INC += -I$(BUILD_DIRECTORY)/software/include
CXXFLAGS += $(INC) -O2 -march=rv32im -mabi=ilp32
CFLAGS += $(INC) -O2 -Wno-missing-prototypes -march=rv32im -mabi=ilp32

RVFPLIB=/media/vsuarez/elocaldata/SCRATCH/RVfplib/
IMGUI_OBJECTS=imgui.o imgui_demo.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui_sw.o
USB_OBJECTS=$(USBHOST_DIR)/usb_host.o $(USBHOST_DIR)/common/tusb_fifo.o


all: usb_main.elf

#lto flag added to allow sources with mixed -flto or -fno-lto (link time) optimizations settings
%.elf: %.o crt0.o sbrk.o usb_main.o isr.o usb.o printf.o $(LEARNFPGALIB_DIR)/lite_fb.o $(IMGUI_OBJECTS) $(USB_OBJECTS) linker.ld
	$(CC)  $< ./crt0.o sbrk.o $(USB_OBJECTS) isr.o usb.o printf.o \
	     $(LEARNFPGALIB_DIR)/lite_fb.o $(IMGUI_OBJECTS) $(LDFLAGS) \
		-T linker.ld \
		-Xlinker -Map=$@.map \
		-N -o $@ \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-flto \
		-L. $(LIBS:lib%=-l%) -L$(RVFPLIB)/build/lib -lrvfp
	chmod -x $@
	$(OBJCOPY) -O binary $@ $@.bin
