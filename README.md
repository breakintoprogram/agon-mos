# agon-mos

The Machine Operating System for Agon

### What is the Agon

Agon is a modern, fully open-source, 8-bit microcomputer and microcontroller in one small, low-cost board. As a computer, it is a standalone device that requires no host PC: it puts out its own video (VGA), audio (2 identical mono channels), accepts a PS/2 keyboard and has its own mass-storage in the form of a µSD card.

https://www.thebyteattic.com/p/agon.html

### What is a MOS

The MOS will provide a command line, similar to CP/M or DOS, that will provide a human interface to the Agon file system. It will also provide an API for BBC Basic for Z80 and other applications for file I/O.

### Licenses

The SD filing system is implemented using [FatFS by ChaN](http://elm-chan.org/fsw/ff/00index_e.html). The license for this can be found in [src_fatfs/LICENSE](src_fatfs/LICENSE) along with the accompanying code.