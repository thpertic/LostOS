# LostOS

This is a 32-bit operating system written from scratch to help me better understand what's "under the hood".

## Getting Started

### Prerequisites

You will need a cross-compiler and Bochs/Qemu. I advice to stay on Linux.
I'm using the GCC cross-compiler built from source following the guide on https://wiki.osdev.org/GCC_Cross-Compiler.

### Executing

Once you download the project, execute
```
./start.sh
```
It should be enough if everything is right.

I recommend execute the scripts singularly, though, as you can see if one of them crashes.
The order is:
```
./compile.sh
./build.sh
./bochs.sh  or ./qemu.sh
```
The emulator should start, print some initialization messages to the screen and write "INIT COM1" in the com1.out file.

### Clear
If you want to rebuild everything you can just execute
```
./clean.sh
```
which will remove every .o file and the .iso image. 
You can then follow the **Executing** part to re-build the OS.

# Features

 - GRUB
 - Video support (printf-like function)
 - Serial port (COM1) support
 - Custom GDT and IDT installed
 - PIC remapped 
 - IRQs and ISRs set
 - PIT (Channel 0)
 - Physical Memory Manager
 - Virtual Memory Manager

## Todos
 
 - Docs 
 - Kernel Heap

### Problems


## Dreams
 - Good project organization (an */arch* directory, for example)
 - ERRNO integration
 - panic function integration
 - Semantic versioning (as soon I get to the alpha release)
 - Keyboard driver
 - Multithreading
 - Filesystem
 - Graphical interface
 - Network
 - System calls

## Authors

* **Thomas Perticaroli** - *Starting project* - [thpertic](https://github.com/thpertic)

See also the list of [contributors](https://github.com/thpertic/LostOS/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
