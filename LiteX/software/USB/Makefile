BUILD_DIR ?= $(BUILD_DIRECTORY)

include ../makefile.inc
INC += -I/media/vsuarez/elocaldata/tools/arduino-1.8.19/libraries/ESP32-USB-Soft-Host/src
INC += -I/media/vsuarez/elocaldata/CONFIDENTIAL/LiteX-consulting/micropython-private/ports/litex
#INC += -I$(BUILD_DIRECTORY)/software/include
CXXFLAGS += $(INC)
CFLAGS += $(INC)

all: usb_main.elf

%.elf: %.o crt0.o sbrk.o usb_main.o usb.o
	$(CC) ./crt0.o $< sbrk.o usb.o $(LDFLAGS) \
		-T linker.ld \
		-N -o $@ \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-L. $(LIBS:lib%=-l%) -lbase 
	chmod -x $@
	$(OBJCOPY) -O binary $@ $@.bin
