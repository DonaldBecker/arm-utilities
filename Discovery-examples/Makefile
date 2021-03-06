#!/usr/bin/make
# ARM build environment for STM32 example code
# Written 2010,2011,2015 by WWC and DB
# Released under the GPL v2

# Commonly modified settings

# Chip support environment and ST-Link tools are available from
#  http://code.google.com/p/arm-utilties
STLINK-DOWNLOAD=/usr/bin/stlink-download
ARM-UTILS=../stm32-env

ifneq "$(wildcard /usr/bin/arm-none-eabi-gcc)" ""
ARM=arm-none-eabi
else
ARM=arm-linux-gnueabi
endif

CC=$(ARM)-gcc
LD=$(ARM)-ld
ARM-CC=$(ARM)-gcc
ARM-LD=$(ARM)-ld
ARM-OBJCOPY=$(ARM)-objcopy
ARM-OBJDUMP=$(ARM)-objdump

ASFLAGS= -g
LDFLAGS= -T$(ARM-UTILS)/stm32.ld -nostartfiles
#LDFLAGS=-Tstm32f100.ld -L../lib  $(ARM-UTILS)/printf.o
# Super sleazy way to add the printf library.
LDEXTRAS= $(ARM-UTILS)/printf.o
CFLAGS = -fno-common -fconserve-stack -fno-stack-protector -fcaller-saves -mcpu=cortex-m3 -mthumb -I.
CFLAGS += -g -O1 -Wall -Wstrict-prototypes
CFLAGS += -I$(ARM-UTILS)/ -I../common/ -DSTM32=1

%.lst: %.c
	$(ARM-CC) $(CFLAGS) -c $< -Wa,-adhlns=$(<:.c=.lst)
%.elf: %.o
	$(ARM-LD) $(LDFLAGS) $< -o $@
%.bin: %.elf
	$(ARM-OBJCOPY) $< -O binary $@

# Other interesting options:     -ffreestanding -nostdlib -nostdinc

# Disassemble
ARM-DISASM=$(ARM-OBJDUMP)  -marm -Mthumb -EL -b binary -D

VPATH=../common/

all: stm32L.bin

download: stm32L.bin
	$(STLINK-DOWNLOAD) /dev/stlink erase flash:w:$<
erase: 
	$(STLINK-DOWNLOAD) /dev/stlink debug reset erase=all


vvvv-main.o stm32-devs.o stm32-adc.o obd2.o pi-loop.o tach.o: vvvvroom.h
can-control.o motor-commands.o command-plus.o: command.h
can-control.o obd2.o mcp2515.o bxCAN.o: can.h

clean:
	rm -f *.o *.elf *.bin
distclean: clean
	rm -f *~
