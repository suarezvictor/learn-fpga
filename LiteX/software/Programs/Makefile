include ../makefile.inc

all: imgui_test.elf # hello.elf  tinyraytracer.elf spirograph.elf ST_NICCC.elf  

%.elf: %.o libliteos.a crt0.o sbrk.o
	$(CC) ./crt0.o $< sbrk.o $(LDFLAGS) \
		-T linker.ld \
		-N -o $@ \
		$(PACKAGES:%=-L$(BUILD_DIR)/software/%) \
		-L. -lliteos $(LIBS:lib%=-l%) -lbase 
	chmod -x $@
	$(OBJCOPY) -O binary $@ $@.bin
