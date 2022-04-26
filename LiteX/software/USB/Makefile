BUILD_DIR ?= $(BUILD_DIRECTORY)

include ../makefile.inc
INC += -I/media/vsuarez/elocaldata/tools/arduino-1.8.19/libraries/ESP32-USB-Soft-Host/src
INC += -I/media/vsuarez/elocaldata/CONFIDENTIAL/LiteX-consulting/micropython-private/ports/litex
#INC += -I$(BUILD_DIRECTORY)/software/include
CXXFLAGS += $(INC) -O3 -Wno-missing-prototypes -march=rv32im -mabi=ilp32
CFLAGS += $(INC) -O3 -Wno-missing-prototypes -march=rv32im -mabi=ilp32

RVFPLIB=/media/vsuarez/elocaldata/SCRATCH/RVfplib/

all: usb_main.elf

%.elf: %.o crt0.o sbrk.o usb_main.o isr.o usb.o printf.o
	$(CC) ./crt0.o $< sbrk.o isr.o usb.o printf.o $(LDFLAGS) \
		-T linker.ld \
		-N -o $@ \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-L. $(LIBS:lib%=-l%) -L$(RVFPLIB)/build/lib -lrvfp
	chmod -x $@
	$(OBJCOPY) -O binary $@ $@.bin
