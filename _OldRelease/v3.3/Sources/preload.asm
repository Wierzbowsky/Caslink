; PRE-BLOAD MODULE FOR MULTIFILE MODE (FOR CASLINK3 PROJECT)
; COPYRIGHT (C) 1999-2022 ALEXEY PODREZOV

	org	#9000

start:	jp	start1

loadcmd:db	#1e
	db	"bload"
	db	34
	db	"cas:binrom part 1 "
	db	34
	db	",r"
	db	13,0

caslstr:db	"[CasLink3] Loading file, please wait...",0

start1:	ei
	call	#006c		; set screen 0
	ld	a,#0f
	ld	hl,#f3e9
	ld	(hl),a
	ld	a,4
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),a
	call	#0062		; set color 15,4,4
	xor	a
	call	#00c3		; clear screen
	call	#00cc		; hide functional keys
	ld	hl,#0101
	call	#00c6		; set cursor position to 1:1
	ld	de,caslstr
start2:	ld	a,(de)
	or	a
	jr	z,start3
	inc	de
	call	#00a2		; display character
	inc	h
	call	#00c6		; set next position
	jr	start2

start3:	ld	hl,#0103
	call	#00c6		; set cursor position to 1:3
	call	#0156		; clears keyboard buffer
	di
	ld	hl,#fbf0
	ld	(#f3f8),hl
	ld	(#f3fa),hl
	ld	hl,loadcmd
	ld	de,#fbf0
	ld	bc,29
	ldir			; send command to buffer
	ld	hl,#fbf0+29
	ld	(#f3f8),hl
	ei
	ret
	nop
	
END
