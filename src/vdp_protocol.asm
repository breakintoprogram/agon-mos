;
; Title:	AGON MOS - VDP serial protocol
; Author:	Dean Belfield
; Created:	03/08/2022
; Last Updated:	18/08/2022
;
; Modinfo:
; 09/08/2022:	Added vdp_protocol_CURSOR
; 18/08/2022:	Added vpd_protocol_SCRCHAR, vpd_protocol_POINT, vdp_protocol_AUDIO, bounds checking for protocol

			INCLUDE	"macros.inc"
			INCLUDE	"equs.inc"

			.ASSUME	ADL = 1

			DEFINE .STARTUP, SPACE = ROM
			SEGMENT .STARTUP
			
			XDEF	vdp_protocol
			
			XREF	_keycode
			XREF	_keymods
			XREF	_cursorX
			XREF	_cursorY
			XREF	_scrchar
			XREF	_scrpixel
			XREF	_audioChannel
			XREF	_audioSuccess
			XREF	_vpd_protocol_flags
			XREF	_vdp_protocol_state
			XREF	_vdp_protocol_cmd
			XREF	_vdp_protocol_len
			XREF	_vdp_protocol_ptr
			XREF	_vdp_protocol_data

			XREF	serial_TX
			XREF	serial_RX
							
; The UART protocol handler state machine
;
vdp_protocol:		LD	A, (_vdp_protocol_state)
			OR	A
			JR	Z, vdp_protocol_state0
			DEC	A
			JR	Z, vdp_protocol_state1
			DEC	A
			JR	Z, vdp_protocol_state2
			XOR	A
			LD	(_vdp_protocol_state), A
			RET
;
; Wait for control byte (>=80h)
;
vdp_protocol_state0:	LD	A, C			; Wait for a header byte (bit 7 set)
			SUB	80h
			RET	C
			CP	vdp_protocol_vesize	; Check whether the command is in bounds
			RET	NC			; Out of bounds, so just ignore
			LD	(_vdp_protocol_cmd), A	; Store the cmd (discard the top bit)
			LD	(_vdp_protocol_ptr), HL	; Store the buffer pointer
			LD	A, 1			; Switch to next state
			LD	(_vdp_protocol_state), A
			RET
			
;
; Read the packet length in
;
vdp_protocol_state1:	LD	A, C			; Fetch the length byte
			CP	VDPP_BUFFERLEN + 1	; Check if it exceeds buffer length (16)
			JR	C, $F			;
			XOR	A			; If it does exceed buffer length, reset state machine
			LD	(_vdp_protocol_state), A
			RET
;
$$:			LD	(_vdp_protocol_len), A	; Store the length
			OR	A			; If it is zero
			JR	Z, vdp_protocol_exec	; Then we can skip fetching bytes, otherwise
			LD	A, 2			; Switch to next state
			LD	(_vdp_protocol_state), A
			RET
			
; Read the packet body in
;
vdp_protocol_state2:	LD	HL, (_vdp_protocol_ptr)	; Get the buffer pointer
			LD	(HL), C			; Store the byte in it
			INC	HL			; Increment the buffer pointer
			LD	(_vdp_protocol_ptr), HL
			LD	A, (_vdp_protocol_len)	; Decrement the length
			DEC	A
			LD	(_vdp_protocol_len), A
			RET	NZ			; Stay in this state if there are still bytes to read
;
; When len is 0, we can action the packet
;

vdp_protocol_exec:	XOR	A			; Reset the state
			LD	(_vdp_protocol_state), A	
			LD	DE, vdp_protocol_vector
			LD	HL, 0			; Index into the jump table
			LD	A, (_vdp_protocol_cmd)	; Get the command byte...
			LD	L, A			; ...in HLU
			ADD	HL, HL			; Multiply by four, as each entry is 4 bytes
			ADD	HL, HL			; And add the address of the vector table
			ADD	HL, DE
			JP	(HL)			; And jump to the entry in the jump table
;
; Jump table for UART commands
;
vdp_protocol_vector:	JP	vdp_protocol_GP
			JP	vdp_protocol_KEY
			JP	vdp_protocol_CURSOR
			JP	vpd_protocol_SCRCHAR
			JP	vdp_protocol_POINT
			JP	vdp_protocol_AUDIO
;
vdp_protocol_vesize:	EQU	($-vdp_protocol_vector)/4
	
; General Poll
;
vdp_protocol_GP:	RET

; Keyboard Data
; Received after a keypress event in the VPD
;
vdp_protocol_KEY:	LD		A, (_vdp_protocol_data + 1)
			LD		(_keymods), A
			LD		A, (_vdp_protocol_data + 0)	
			LD		(_keycode), A			
			CP		1Bh			; Check for ESC + CTRL + SHIFT combo
			RET		NZ			
			LD		A, (_keymods)		; ESC is pressed, so check CTRL + SHIFT
			AND		03h			; Bottom two bits
			CP		03h
			RET		NZ	
			RST		00h			; And reset	

; Cursor data
; Received after the cursor position is updated in the VPD
;
vdp_protocol_CURSOR:	LD		A, (_vdp_protocol_data+0)
			LD		(_cursorX), A
			LD		A, (_vdp_protocol_data+1)
			LD		(_cursorY), A
			LD		A, (_vpd_protocol_flags)
			OR		VDPP_FLAG_CURSOR
			LD		(_vpd_protocol_flags), A
			RET
			
; Screen character data
; Received after VDU 23,0,0,x;y;
;
vpd_protocol_SCRCHAR:	LD		A, (_vdp_protocol_data+0)
			LD		(_scrchar), A
			LD		A, (_vpd_protocol_flags)
			OR		VDPP_FLAG_SCRCHAR
			LD		(_vpd_protocol_flags), A
			RET
			
; Pixel value data (RGB)
; Received after VDU 23,0,1,x;y;
;
vdp_protocol_POINT:	LD		HL, (_vdp_protocol_data+0)
			LD		(_scrpixel), HL
			LD		A, (_vpd_protocol_flags)
			OR		VDPP_FLAG_POINT
			LD		(_vpd_protocol_flags), A
			RET
			
; Audio acknowledgement
; Received after VDU 23,0,5,channel,volume,frequency,duration
;
vdp_protocol_AUDIO:	LD		A, (_vdp_protocol_data+0)
			LD		(_audioChannel), A
			LD		A, (_vdp_protocol_data+1)
			LD		(_audioSuccess), A
			LD		A, (_vpd_protocol_flags)
			OR		VDPP_FLAG_AUDIO
			LD		(_vpd_protocol_flags), A
			RET
