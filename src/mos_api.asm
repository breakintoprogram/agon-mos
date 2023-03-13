;
; Title:	AGON MOS - API code
; Author:	Dean Belfield
; Created:	24/07/2022
; Last Updated:	13/03/2023
;
; Modinfo:
; 03/08/2022:	Added a handful of MOS API calls and stubbed FatFS calls
; 05/08/2022:	Added mos_FEOF, saved affected registers in fopen, fclose, fgetc, fputc and feof
; 09/08/2022:	mos_api_sysvars now returns pointer to _sysvars
; 05/09/2022:	Added mos_REN
; 24/09/2022:	Error codes returned for MOS commands
; 13/10/2022:	Added mos_OSCLI and supporting code
; 20/10/2022:	Tweaked error handling
; 13/03/2023:	Renamed keycode to keyascii, fixed mos_api_getkey, added parameter to mos_api_dir

			.ASSUME	ADL = 1
			
			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XDEF	mos_api		

			XREF	SWITCH_A
			XREF	SET_AHL24
			XREF	SET_ADE24

			XREF	_mos_OSCLI
			XREF	_mos_EDITLINE
			XREF	_mos_LOAD
			XREF	_mos_SAVE
			XREF	_mos_CD
			XREF	_mos_DIR
			XREF	_mos_DEL
			XREF	_mos_REN
			XREF	_mos_FOPEN
			XREF	_mos_FCLOSE
			XREF	_mos_FGETC
			XREF	_mos_FPUTC
			XREF	_mos_FEOF
			XREF	_mos_GETERROR
			XREF	_mos_MKDIR
			
			XREF	_keyascii
			XREF	_keycount
			XREF	_keydown

			XREF	_sysvars
			
; Call a MOS API function
; 00h - 7Fh: Reserved for high level MOS calls
; 80h - FFh: Reserved for low level calls to FatFS
;  A: function to call
;
mos_api:		CP	80h			; Check if it is a FatFS command
			JR	NC, $F			; Yes, so jump to next block
			CALL	SWITCH_A		; Switch on this table
			DW	mos_api_getkey		; 0x00
			DW	mos_api_load		; 0x01
			DW	mos_api_save		; 0x02
			DW	mos_api_cd		; 0x03
			DW	mos_api_dir		; 0x04
			DW	mos_api_del		; 0x05
			DW	mos_api_ren		; 0x06
			DW	mos_api_mkdir		; 0x07
			DW	mos_api_sysvars		; 0x08
			DW	mos_api_editline	; 0x09
			DW	mos_api_fopen		; 0x0A
			DW	mos_api_fclose		; 0x0B
			DW	mos_api_fgetc		; 0x0C
			DW	mos_api_fputc		; 0x0D
			DW	mos_api_feof		; 0x0E
			DW	mos_api_getError	; 0x0F
			DW	mos_api_oscli		; 0x10
;			
$$:			AND	7Fh			; Else remove the top bit
			CALL	SWITCH_A		; And switch on this table
			DW	ffs_api_fopen
			DW	ffs_api_fclose
			DW	ffs_api_fread
			DW	ffs_api_fwrite
			DW	ffs_api_fseek
			DW	ffs_api_ftruncate
			DW	ffs_api_fsync
			DW	ffs_api_fforward
			DW	ffs_api_fexpand
			DW	ffs_api_fgets
			DW	ffs_api_fputc
			DW	ffs_api_fputs
			DW	ffs_api_fprintf
			DW	ffs_api_ftell
			DW	ffs_api_feof
			DW	ffs_api_fsize
			DW	ffs_api_ferror
			DW	ffs_api_dopen
			DW	ffs_api_dclose
			DW	ffs_api_dread
			DW	ffs_api_dfindfirst
			DW	ffs_api_dfindnext
			DW	ffs_api_stat
			DW	ffs_api_unlink
			DW	ffs_api_rename
			DW	ffs_api_chmod
			DW	ffs_api_utime
			DW	ffs_api_mkdir
			DW	ffs_api_chdir
			DW	ffs_api_chdrive
			DW	ffs_api_getcwd
			DW	ffs_api_mount
			DW	ffs_api_mkfs
			DW	ffs_api_fdisk		
			DW	ffs_api_getfree
			DW	ffs_api_getlabel
			DW	ffs_api_setlabel
			DW	ffs_api_setcp

; Get keycode
; Returns:
;  A: ASCII code of key pressed, or 0 if no key pressed
;
mos_api_getkey:		PUSH	HL
			LD	HL, _keycount	
mos_api_getkey_1:	LD	A, (HL)			; Wait for a key to be pressed
$$:			CP	(HL)
			JR	Z, $B
			LD	A, (_keydown)		; Check if key is down
			OR	A 
			JR	Z, mos_api_getkey_1	; No, so loop
			POP	HL 
			LD	A, (_keyascii)		; Get the key code
			RET
			
; Load an area of memory from a file.
; HLU: Address of filename (zero terminated)
; DEU: Address at which to load
; BCU: Maximum allowed size (bytes)
; Returns:
; - A: File error, or 0 if OK
; - F: Carry reset indicates no room for file.
;
mos_api_load:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
			JR	Z, $F		; If it is, we can assume HL and DE are 24 bit
;
; Now we need to mod HLU and DEU to include the MBASE in the U byte
;
			CALL	SET_AHL24
			CALL	SET_ADE24
;
; Finally, we can do the load
;
$$:			PUSH	BC		; UINT24   size
			PUSH	DE		; UNIT24   address
			PUSH	HL		; char   * filename
			CALL	_mos_LOAD	; Call the C function mos_LOAD
			LD	A, L		; Return value in HLU, put in A
			POP	HL
			POP	DE
			POP	BC
			SCF			; Flag as successful
			RET

; Save a file to the SD card from RAM
; HLU: Address of filename (zero terminated)
; DEU: Address to save from
; BCU: Number of bytes to save
; Returns:
; - A: File error, or 0 if OK
; - F: Carry reset indicates no room for file
;
mos_api_save:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
			JR	Z, $F		; If it is, we can assume HL and DE are 24 bit
;
; Now we need to mod HLU and DEU to include the MBASE in the U byte
;
			CALL	SET_AHL24
			CALL	SET_ADE24
;
; Finally, we can do the save
;
$$:			PUSH	BC		; UINT24   size
			PUSH	DE		; UNIT24   address
			PUSH	HL		; char   * filename
			CALL	_mos_SAVE	; Call the C function mos_LOAD
			LD	A, L		; Return vaue in HLU, put in A
			POP	HL
			POP	DE
			POP	BC
			SCF			; Flag as successful
			RET
			
; Change directory
; HLU: Address of path (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;			
mos_api_cd:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
; Finally, we can do the load
;
			PUSH	HL		; char   * filename	
			CALL	_mos_CD
			LD	A, L		; Return vaue in HLU, put in A
			POP	HL
			RET

; Directory listing
; HLU: Address of path (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;	
mos_api_dir:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
; Finally, we can run the command
;
			PUSH	HL		; char * path
			CALL	_mos_DIR
			LD	A, L		; Return value in HLU, put in A
			POP	HL
			RET
			
; Delete a file from the SD card
; HLU: Address of filename (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_del:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
; Finally, we can do the delete
;
			PUSH	HL		; char   * filename
			CALL	_mos_DEL	; Call the C function mos_DEL
			LD	A, L		; Return vaue in HLU, put in A
			POP	HL
			RET

; Rename a file on the SD card
; HLU: Address of filename1 (zero terminated)
; DEU: Address of filename2 (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_ren:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
			JR	Z, $F		; If it is, we can assume HL and DE are 24 bit
;
; Now we need to mod HLU and DEu to include the MBASE in the U byte
; 
			CALL	SET_AHL24
			CALL	SET_ADE24
;
; Finally we can do the rename
; 
$$:			PUSH	DE		; char * filename2
			PUSH	HL		; char * filename1
			CALL	_mos_REN	; Call the C function mos_REN
			LD	A, L		; Return vaue in HLU, put in A
			POP	HL
			POP	DE
			RET

; Make a folder on the SD card
; HLU: Address of filename (zero terminated)
; Returns:
; - A: File error, or 0 if OK
;
mos_api_mkdir:		LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
; Finally, we can do the load
;
			PUSH	HL		; char   * filename
			CALL	_mos_MKDIR	; Call the C function mos_DEL
			LD	A, L		; Return vaue in HLU, put in A
			POP	HL
			RET

; Get a pointer to a system variable
; Returns:
; IXU: Pointer to system variables (see mos_api.asm for more details)
;
mos_api_sysvars:	LD	IX, _sysvars
			RET
			
; Invoke the line editor
; HLU: Address of the buffer
; BCU: Buffer length
;   E: 0 to not clear buffer, 1 to clear
; Returns:
;   A: Key that was used to exit the input loop (CR=13, ESC=27)
;
mos_api_editline:	LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
			PUSH	DE		; UINT8	  clear
			PUSH	BC		; int 	  bufferLength
			PUSH	HL		; char	* buffer
			CALL	_mos_EDITLINE
			LD	A, L		; return value, only interested in lowest byte
			POP	HL
			POP	BC
			POP	DE
			RET

; Open a file
; HLU: Filename
;   C: Mode
; Returns:
;   A: Filehandle, or 0 if couldn't open
;
mos_api_fopen:		PUSH	BC
			PUSH	DE
			PUSH	HL
			PUSH	IX
			PUSH	IY
;
			LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU and DEU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
			LD	A, C	
			LD	BC, 0
			LD	C, A
			PUSH	BC		; byte	  mode
			PUSH	HL		; char	* buffer
			CALL	_mos_FOPEN
			LD	A, L		; Return fh
			POP	HL
			POP	BC
;
			POP	IY
			POP	IX
			POP	HL
			POP	DE
			POP	BC
			RET

; Close a file
;   C: Filehandle
; Returns
;   A: Number of files still open
;
mos_api_fclose:		PUSH	BC
			PUSH	DE
			PUSH	HL
			PUSH	IX
			PUSH	IY
;
			LD	A, C
			LD	BC, 0
			LD	C, A
			PUSH	BC		; byte 	  fh
			CALL	_mos_FCLOSE
			LD	A, L		; Return # files still open
			POP	BC
;
			POP	IY
			POP	IX
			POP	HL
			POP	DE
			POP	BC
			RET
			
; Get a character from a file
;   C: Filehandle
; Returns:
;   A: Character read
;   F: C set if last character in file, otherwise NC
;
mos_api_fgetc:		PUSH	BC
			PUSH	DE
			PUSH	HL
			PUSH	IX
			PUSH	IY
;			
			LD	DE, 0
			LD	E, C
			PUSH	DE		; byte	  fh
			CALL	_mos_FGETC
			POP	BC
			OR	A		; TODO: Need to set C if EOF
;
			POP	IY
			POP	IX
			POP	HL
			POP	DE
			POP	BC
			RET
	
; Write a character to a file
;   C: Filehandle
;   B: Character to write
;
mos_api_fputc:		PUSH	AF
			PUSH	BC
			PUSH	DE
			PUSH	HL
			PUSH	IX
			PUSH	IY
;		
			LD	DE, 0
			LD	E, B		
			PUSH	DE		; byte	  char
			LD	E, C
			PUSH	DE		; byte	  fh
			CALL	_mos_FPUTC
			POP	DE
			POP	DE
;			
			POP	IY
			POP	IX
			POP	HL
			POP	DE
			POP	BC
			POP	AF
			RET
			
; Check whether we're at the end of the file
;   C: Filehandle
; Returns:
;   A: 1 if at end of file, otherwise 0
;     
mos_api_feof:		PUSH	BC
			PUSH	DE
			PUSH	HL
			PUSH	IX
			PUSH	IY
;			
			LD	DE, 0
			LD	E, C
			PUSH	DE		; byte	  fh
			CALL	_mos_FEOF
			POP	DE
;			
			POP	IY
			POP	IX
			POP	HL
			POP	DE
			POP	BC
			RET
			
; Copy an error message
;   E: The error code
; HLU: Address of buffer to copy message into
; BCU: Size of buffer
;
mos_api_getError:	LD	A, MB		; Check if MBASE is 0
			OR	A, A
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
; Now copy the error message
;
			PUSH	BC		; UINT24 size
			PUSH	HL		; UINT24 address
			PUSH	DE		; byte   errno
			CALL	_mos_GETERROR
			POP	DE
			POP	HL
			POP	BC			
			RET

; Execute a MOS command
; HLU: Pointer the the MOS command string
; DEU: Pointer to additional command structure
; BCU: Number of additional commands
; Returns:
;   A: MOS error code
;
mos_api_oscli:		LD	A, MB		; Check if MBASE is 0
			OR	A, A				
;
; Now we need to mod HLU to include the MBASE in the U byte
;
			CALL	NZ, SET_AHL24	; If it is running in classic Z80 mode, set U to MB
;
; Now execute the MOS command
;
$$:			PUSH	HL		; char * buffer
			CALL	_mos_OSCLI
			LD	A, L		; Return vaue in HLU, put in A			
			POP	HL
			RET			
			
			
; Commands that have not been implemented yet
;
ffs_api_fopen:
ffs_api_fclose:
ffs_api_fread:		
ffs_api_fwrite:		
ffs_api_fseek:		
ffs_api_ftruncate:	
ffs_api_fsync:		
ffs_api_fforward:	
ffs_api_fexpand:	
ffs_api_fgets:		
ffs_api_fputc:		
ffs_api_fputs:		
ffs_api_fprintf:	
ffs_api_ftell:		
ffs_api_feof:		
ffs_api_fsize:		
ffs_api_ferror:		
ffs_api_dopen:		
ffs_api_dclose:		
ffs_api_dread:		
ffs_api_dfindfirst:	
ffs_api_dfindnext:	
ffs_api_stat:		
ffs_api_unlink:		
ffs_api_rename:		
ffs_api_chmod:		
ffs_api_utime:		
ffs_api_mkdir:		
ffs_api_chdir:		
ffs_api_chdrive:	
ffs_api_getcwd:		
ffs_api_mount:		
ffs_api_mkfs:		
ffs_api_fdisk		
ffs_api_getfree:	
ffs_api_getlabel:	
ffs_api_setlabel:	
ffs_api_setcp:		
			RET