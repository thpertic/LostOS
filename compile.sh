#!/bin/bash

# Some configurations
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

export PATH="$HOME/opt/cross/bin:$PATH"

# Actual script
cd src
make all

if grub-file --is-x86-multiboot LostOS.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi