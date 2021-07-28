#!/bin/bash

# Build a bootable cdrom image
mkdir -p ./isodir/boot/grub
cp ./src/LostOS.bin ./isodir/boot/
echo 'menuentry "LostOS" {
        multiboot /boot/LostOS.bin
    }' > isodir/boot/grub/grub.cfg
grub-mkrescue -o LostOS.iso isodir