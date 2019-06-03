# LostOS

This is an operating system written from scratch to help me better understand what's "under the hood".

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
The virtual machine should start, print some initialization messages to the screen and write "INIT COM1" in the com1.out file.

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

## Todos

 - Currently working on a PIT implementation

### Problems
When in /src/kernel/interrupts/timer.c (function *tickHandler()*) the machine execute *printf("Tick: %d\n", tick);* the screen prints 
```
Tick: Exception: Page fault (err code 2)
```
and obviously the system halts as it is set.

When it only execute printf("Tick\n"); it works printing an infinite list of "Tick:", though. 
Also if I don't print anything, the page fault doesn't occur.
A printf (/src/kernel/debug_utils/printf.c) error perhaps?

## Dreams
 - Good project organization (an */arch* directory, for example)
 - Semantic versioning 
 - Keyboard driver
 - Paging
 - Multithreading
 - Filesystem
 - Graphical interface
 - Network
 - System calls

## Authors

* **Thomas Perticaroli** - *Starting project* - [thpertic](https://github.com/thpertic)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details