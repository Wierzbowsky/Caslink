; BLOAD MODULE FOR 32KB ROMS PART 1 (FOR CASLINK3 PROJECT)
; COPYRIGHT (C) 1999-2022 ALEXEY PODREZOV

	org	#9000

start:	jp	start1

starta:	dw	0
enda:	dw	0
execa:	dw	0
crc:	db	0

loadcmd:db	#1e
	db	"bload"
	db	34
	db	"cas:binrom part 2 "
	db	34
	db	",r"
	db	13,0

caslstr:db	"[CasLink3] Loading file, please wait...",0

caserr:	db	"[CasLink3] Loading failed: CRC ERROR!",0

start1:	di
	ld	hl,(starta)
      	ld	de,(enda)
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	push	hl
	pop	bc
	ld	hl,romcode
        xor	a
	push	af
start2:	pop	af
	add	a,(hl)
	inc	hl
	dec	bc
	push	af
	ld	a,c
	or	a
	jr	nz,start2
	ld	a,b
	or	a
	jr	nz,start2
	pop	af
	ld	b,a
	ld	hl,crc
	ld	a,(hl)
	cp	b
	jp	z,start5	

crcerr:	ei
	call	#006c		; set screen 0
	ld	a,#0f
	ld	hl,#f3e9
	ld	(hl),a
	ld	a,8
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),a
	call	#0062		; set color 15,8,8
	xor	a
	call	#00c3		; clear screen
	call	#00cf		; unhide functional keys
	ld	hl,#0101
	call	#00c6		; set cursor position to 1:1
	ld	de,caserr
start3:	ld	a,(de)
	or	a
	jr	z,start4
	inc	de
	call	#00a2		; display character
	inc	h
	call	#00c6		; set next position
	jr	start3

start4:	ld	hl,#0103
	call	#00c6		; set cursor position to 1:3
	call	#00c0		; beep
	call	#0156		; clears keyboard buffer
	ret

start5:	ei
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
start6:	ld	a,(de)
	or	a
	jr	z,start7
	inc	de
	call	#00a2		; display character
	inc	h
	call	#00c6		; set next position
	jr	start6

start7:	ld	hl,#0103
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

loadrom:di
	ld	a,(#ffff)
	cpl
	push	af
	ld	c,a
	and	#f0
	ld	b,a
	ld	a,c
	rrca
	rrca
	rrca
	rrca
	and	15
	or	b
	ld	(#ffff),a
	in	a,(#a8)
	push	af
	and	#f0
	ld	b,a
	rrca
	rrca
	rrca
	rrca
	and	15
	or	b
	out	(#a8),a

	ld	hl,(starta)
      	ld	de,(enda)
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,romcode
	ld	de,(starta)
	ldir
	pop	af
	out	(#a8),a
	pop	af
	ld	(#ffff),a
	ei
	ret
	nop

romcode:

END
