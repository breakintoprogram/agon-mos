# agon-mos API

The API can be used by external applications to access MOS functionality

## Usage from Z80 assembler

There are three RST instructions for accessing MOS functionality from Z80.

`RST 00h`: Reset the eZ80
`RST 08h`: Execute a MOS command
`RST 10h`: Output a character to the VDP

## Examples

Include the file mos_api.inc in your project.
As the MOS code is in the eZ80 FLASH ROM from address zero, care must be takn when running code in compatibility mode within a 64K segment.
For example, BBC BASIC for AGON uses the following code for outputting a character to the VDP

```
; OSWRCH: Write a character out to the ESP32 VDU handler via the MOS
; A: Character to write
;
OSWRCH:		RST.LIS	10h			; This calls a RST in the eZ80 address space
			RET
```

The `RST.LIS` ensures the MOS RST instructions are called regardless of the eZ80s current addressing mode

There is a macro in mos_api.inc for calling MOS commands - there is one parameter, the mos command number

```
; OSRDCH: Read a character in from the ESP32 keyboard handler
;
OSRDCH:		MOSCALL	mos_getkey
			OR	A 		
			JR	Z, OSRDCH		; Loop until key is pressed
			RET
```

## MOS commands

MOS commands can be executed from a classic 64K Z80 segment or whilst the eZ80 is running in 24-bit ADL mode. For classic mode, 16 bit registers are passed as pointers to the MOS commands; these are automatically promoted to 24 bit by adding the MB register to bits 16-23 of the register. When running in ADL mode, a 24-bit register will be passed, but MB must be set to 0.

See mos_api.asm for implementation

The following MOS commands are supported

### mos_getkey: Read a keypress from the VDP

Parameters: None

Returns:
- A: The keycode of the character pressed

### mos_load: Load a file from SD card

Parameters: 
- HL(U): Address of filename (zero terminated)
- DE(U): Address at which to load
- BC(U): Maximum allowed size (bytes)

Returns:
- A: File error, or 0 if OK
- F: Carry reset if no room for file, otherwise set

### mos_save: Save a file to SD card

Parameters: 

- HL(U): Address of filename (zero terminated)
- DE(U): Address to save from
- BC(U): Number of bytes to save

Returns:

- A: File error, or 0 if OK
- F: Carry set

### mos_cd: Change current directory on the SD card

Parameters: 

- HL(U): Address of path (zero terminated)

Returns:

- A: File error, or 0 if OK

### mos_dir: List SD card folder contents

Parameters: None

Returns:

- A: File error, or 0 if OK

### mos_del: Delete a file or folder from the SD card

Parameters: 

- HL(U): Address of path (zero terminated)

Returns:

- A: File error, or 0 if OK

### mos_ren: Rename a file on the SD card

Parameters: 

- HL(U): Address of filename1 (zero terminated)
- D(E)U: Address of filename2 (zero terminated)

Returns:

- A: File error, or 0 if OK

### mos_mkdir: Make a folder on the SD card

Parameters: 

- HL(U): Address of path (zero terminated)

Returns:

- A: File error, or 0 if OK

### mos_sysvars: Fetch a pointer to the system variables

Parameters: None

Returns:

- IXU: Pointer to the MOS system variable area (this is always 24 bit)

### mos_editline: Invoke the line editor

Parameters: 

- HL(U): Address of the buffer
- BC(U): Buffer length
- E: 0 to not clear buffer, 1 to clear

Returns:
- A: Key that was used to exit the input loop (CR=13, ESC=27)

### mos_fopen: Get a file handle

Parameters: 

- HL(U): Address of filename (zero terminated)
- C: Mode

Returns:
- A: Filehandle, or 0 if couldn't open

Mode can be one of: fa_read, fa_write, fa_open_existing, fa_create_new, fa_create_always, fa_open_always or fa_open_append

### mos_fclose: Close a file handle

Parameters: 

- C: Filehandle, or 0 to close all open files

Returns:

- A: Number of files still open

### mos_fgetc: Get a character from an open file

Parameters: 

- C: Filehandle

Returns:
- A: Character read
- F: C set if last character in file, otherwise NC

### mos_fputc: Write a character to an open file

Parameters: 

- C: Filehandle
- B: Character to write

Returns: None

### mos_feof: Check for end of file

Parameters: 

- C: Filehandle

Returns:
- A: 1 if at end of file, otherwise 0

### mos_getError: Copy an error string to a buffer

Parameters: 

- E: The error code
- HL(U): Address of buffer to copy message into
- BC(U): Size of buffer

Returns: None

