# Setup compiler name and flags
CC=i686-elf-gcc
CFLAGS=-Wall -Wextra -Werror -Wno-int-conversion -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdlib -ffreestanding -I./include -std=gnu99 -g -ggdb -c -fno-stack-protector -m32
NASM=nasm
NASMFLAGS=-f elf32 -O0
LDFLAGS=-T linker.ld -ffreestanding -O2 -nostdlib -g -ggdb

OS_NAME=LostOS.bin

# Project directories
ROOT_DIR=./kernel

COMMON_DIR=$(ROOT_DIR)/common
TABLES_DIR=$(ROOT_DIR)/tables
DRIVERS_DIR=$(ROOT_DIR)/drivers
DEBUG_UTILS_DIR=$(ROOT_DIR)/debug_utils
INTERRUPTS_DIR=$(ROOT_DIR)/interrupts

BOOT_DIR=/boot

# Include all make.config
include $(COMMON_DIR)/make.config
include $(TABLES_DIR)/make.config
include $(DRIVERS_DIR)/make.config
include $(DEBUG_UTILS_DIR)/make.config
include $(INTERRUPTS_DIR)/make.config

SOURCES=\
$(ROOT_DIR)/bootloader.o \
$(ROOT_DIR)/kmain.o 	 \
$(COMMON_OBJS) 			 \
$(TABLES_OBJS)			 \
$(DRIVERS_OBJS)			 \
$(DEBUG_UTILS_OBJS)		 \
$(INTERRUPTS_OBJS)


.PHONY: all clean install install-kernel
.SUFFIXES: .o .c .asm

all: $(OS_NAME)

$(OS_NAME): $(SOURCES) linker.ld
	$(CC) $(LDFLAGS) -o $@ $(SOURCES)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.asm.o:
	$(NASM) $(NASMFLAGS) $< -o $@

clean:
	rm -f $(OS_NAME)
	rm -f $(SOURCES) *.o */*.o */*/*.o
	rm -f $(SOURCES:.o=.d) *.d */*.d */*/*.d

install: install-kernel

install-kernel: $(OS_NAME)
	mkdir -p $(BOOT_DIR)
	cp $(OS_NAME) $(BOOT_DIR)