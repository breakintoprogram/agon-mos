# agon-mos

Part of the official Quark firmware for the Agon series of microcomputers

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.

https://www.thebyteattic.com/p/agon.html

### What is a MOS

The MOS is a command line machine operating system, similar to CP/M or DOS, that provides a human interface to the Agon file system.

It also provides an API for file I/O and other common operations for BBC Basic for Z80 and other third-party applications.

### System Requirements

* A 32GB or less micro-SD card formatted FAT32

### The MOS folder

From version 1.02 of MOS, a `mos` folder needs to be created in the root of the SD card. This is for MOS extensions that run off SD card

### MOS Commands

* `CAT`: Directory listing of the current directory. Aliases include `DIR` and `.`
* `CD path`: Change current directory
* `LOAD filename <addr>`: Load a file from the SD card to the specified address
* `MKDIR filename`: Make a folder on the SD card
* `SAVE filename addr size`: Save a block of memory to the SD card
* `RUN <addr>`: Call an executable binary loaded in memory
* `DELETE filename`: Delete a file or folder (must be empty). Aliases include `ERASE`
* `RENAME filename1 filename2`: Rename a file
* `JMP addr`: Jump to the specified address in memory
* `SET option value`: Set a system option

NB:

1. Commands can be abbreviated with a dot, so `DELETE myfile` and `DEL. myfile` are equivalent.
2. Commands are case-insensitive and parameters are space delimited.
3. Optional parameters are written as `<param>`
4. Default LOAD and RUN address is set to 0x040000
5. Numbers are in decimal and can be prefixed by '&' for hexadecimal.
6. Addresses are 24-bit:
	- `&000000 - &01FFFF`: MOS (Flash ROM)
	- `&040000 - &0BDFFF`: User RAM
	- `&0B0000 - &0B7FFF`: Storage for loading MOS star command executables off SD card 
	- `&0BC000 - 0BFFFFF`: Global heap and stack
7. The RUN command checks a header embedded from byte 64 of the executable and can run either Z80 or ADL mode executables 
8. MOS will also search the `mos` folder on the SD card for any executables, and will run those like built-in MOS commands

### System options

`SET KEYBOARD n`: Set the keyboard layout (0: UK, 1: US)

### The autoexec.txt file

If the MOS detects an autoexec.txt file on the root of the SD card during cold-boot, it will read the file in, and execute the MOS commands in the file sequentially from top to bottom.

For example, to set keyboard to US, load BBC BASIC from the root folder, change to the test folder, then run BASIC

```
SET KEYBOARD 1
LOAD bbcbasic
CD test
RUN
```

### Loadig BBC Basic for Z80

1. Download bbcbasic.bin from [agon-bbc-basic releases](https://github.com/breakintoprogram/agon-bbc-basic/releases)
2. Copy it to the root directory of the Agon SD card
3. Insert the SD card into the AGON and reset/boot it
4. Check the file is on the SD card with a `CAT` or `.` command
5. Type the following commands into MOS:
	- `LOAD bbcbasic.bin`
	- `RUN`
6. You should then be greeted with the BBC Basic for Z80 prompt

### Etiquette

Please do not issue pull requests or issues for this project; it is very much a work-in-progress.
I will review this policy once the code is approaching live status and I have time to collaborate more.

### Build

The eZ80 is programmed via the ZDI connector on the left-hand side of the board. This requires a Zilog USB Smart Cable (part number ZUSBSC00100ZACG) that can be purchased from online stockists such as Mouser or RS Components. Note that at time of writing (July 2022) there are lead times for this cable.

Important! Make sure you get that exact model of cable; there are variants for the Zilog Encore CPU that look similar, but are not compatible with the eZ80 CPU.

In addition to the cable, you will need to download the free ZDS II tools ([product ID SD00063](https://zilog.com/index.php?option=com_zcm&task=view&soft_id=38&Itemid=74)). Note that this is only available for Windows.

Any custom settings for Agon development is contained within the project files, so no further configuration will need to be done.

### Licenses

This code is released under an MIT license, with the following exceptions:

* FatFS: The license for the [FAT filing system by ChaN](http://elm-chan.org/fsw/ff/00index_e.html) can be found here [src_fatfs/LICENSE](src_fatfs/LICENSE) along with the accompanying code.

### Links

- [Zilog eZ80 User Manual](http://www.zilog.com/docs/um0077.pdf)
- [ZiLOG Developer Studio II User Manual](http://www.zilog.com/docs/devtools/um0144.pdf)
- [FatFS - Generic File System](http://elm-chan.org/fsw/ff/00index_e.html)
- [AVRC Tutorials - Initialising an SD Card](http://www.rjhcoding.com/avrc-sd-interface-1.php)