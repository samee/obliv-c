AMBER_CROSSTOOL ?= arm-linux-gnueabi

AS       = $(AMBER_CROSSTOOL)-as
CC       = $(AMBER_CROSSTOOL)-gcc
CXX      = $(AMBER_CROSSTOOL)-g++
AR       = $(AMBER_CROSSTOOL)-ar
LD       = $(AMBER_CROSSTOOL)-ld
DS       = $(AMBER_CROSSTOOL)-objdump
OC       = $(AMBER_CROSSTOOL)-objcopy


all : start.o

start.o : start.S
	$(AS) -c start.S -o start.o

clean:
	@rm -rfv *.o

