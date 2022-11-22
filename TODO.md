# TODO
### Next Jobs
- Add upgrade functionality so that the MOS can be updated from SD card
### Fixed Bugs / New Features
#### Version 1.02
- The RUN command: modify to load file in and run either in Z80 or ADL mode
- Add code to run external * commands from SD card
- MOS commands are now case-insensitive and can be abbreviated with a '.'
#### Version 1.01
- Minor bug fixes and improvements
#### Version 0.08
- Improved the line editor
- Added MOS API command `REN`: Rename a file on the SD card
- Completed enough of the MOS API so that BBC BASIC can run
- Added default address for LOAD and RUN commands
- Added function level comments
#### Version 0.07
- Added support for audio
#### Version 0.06
- Enabled hardware flow control between eZ80 and VDP UARTs
- Cursor position now fetched from MOS
- System variables organised into a continous chunk
#### Version 0.05
- Tweaked memory map
	- `&000000 - &01FFFF`: MOS (Flash ROM)
	- `&040000 - &0BDFFF`: User RAM
	- `&0BE000 - 0BFFFFF`: Global heap and stack
- Moved some source code into new files to make the code easier to navigate
- Added autoexec.txt feature to run MOS commands on cold boot
- Added UART0 interrupt handler
- Packet data now sent from VPD -> MOS over UART
- Keyboard data from VPD -> MOS now includes keycode and modifiers (SHIFT, ALT, etc)
- CTRL + SHIFT + ESC now does a warm reboot
- Added MOS API commands
	- `SYSVARS`: Get pointer to system variables
	- `EDITLINE`: Invoke the MOS line editor
	- `LOAD`: Load a file into memory
	- `SAVE`: Save a block of memory to the SD card
	- `CD`: Change current directory
	- `DIR`: List contents of current directory
	- `DEL`: Delete file
	- `FOPEN`: Open a file and return a file handle
	- `FCLOSE`: Close a file
	- `FGETC`: Read a character from a file stream
	- `FPUTC`: Write a character to a file stream
#### Version 0.04
- Now runs in STMIX mode for mixed mode interrupt support
- Add a RST 08h API handler
- Fixed keyboard input issues
#### Version 0.03
- Added VBLANK interrupt
- Added GPIO functions
- Added RUN command
- Moved keyboard read to VBLANK
#### Version 0.02
- Added JMP, SAVE and DEL commands
- Improved error reporting
#### Version 0.01
- Initial Version