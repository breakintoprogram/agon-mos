# agon-mos

The Machine Operating System for Agon

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.

https://www.thebyteattic.com/p/agon.html

### What is a MOS

The MOS is a command line machine operating system, similar to CP/M or DOS, that provides a human interface to the Agon file system.

It also provides an API for file I/O and other common operations for BBC Basic for Z80 and other third-party applications.

### MOS Commands

* `CAT`: Directory listing of the current directory. Aliases include `DIR` and `.`
* `LOAD filename addr`: Load a file from the SD card to the specified address
* `SAVE filename addr size`: Save a block of memory to the SD card
* `DEL filename`: Delete a file
* `JMP addr`: Jump to the specified address in memory
* `RUN addr`: Call an address in memory (switching to Z80 mode - ADL=0)

NB:

1. Commands are case sensitive and all parameters are space delimited.
2. Numbers are in decimal and can be prefixed by '&' for hexadecimal.
3. Addresses are 24-bit:
	- &000000 to &01FFFF: ROM
	- &020000 to &09FFFF: RAM
4. The RUN command is incomplete - it will eventually call code in ADL mode as well. Use RET.LIS to return back to MOS 

### Etiquette

Please do not issue pull requests or issues for this project; it is very much a work-in-progress.
I will review this policy once the code is approaching live status and I have time to collaborate more.

### Build

The eZ80 is programmed via the ZDI connector on the left-hand side of the board. This requires a Zilog USB Smart Cable (part number ZUSBSC00100ZACG) that can be purchased from online stockists such as Mouser or RS Components. Note that at time of writing (July 2022) there are lead times for this cable.

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