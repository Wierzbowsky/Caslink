; SINGLE BIN FILE LOADER (FOR CASLINK3 PROJECT)
; COPYRIGHT (C) 1999-2022 ALEXEY PODREZOV

	org	#8200

start:	jp	start1

starta:	dw	0
enda:	dw	0
execa:	dw	0
crc:	db	0


caserr:	db	"[CasLink3] Loading failed: CRC ERROR!",0

start1:	di
	ld	hl,(starta)
      	ld	de,(enda)
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	inc	hl
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
	ei
	ret

start5:	di
	ld	hl,(execa)
	push	hl
	ld	hl,romcode
	ld	de,(starta)
	ld	a,h
	cp	d
	jr	c,start7	; target area lower than destination address?
	jr	nz,start6
	ld	a,l
	cp	e
	jr	c,start7	; target area lower than destination address?

start6:	ld	hl,starti	; incremental transfer routine
      	ld	de,startd+1
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,starti
	ld	de,#f560
	push	de
	ldir
	ret

start7:	ld	hl,startd	; decremental transfer routine
      	ld	de,romcode+1
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,startd
	ld	de,#f560
	push	de
	ldir
	ret

starti:	ld	hl,(starta)
      	ld	de,(enda)
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,romcode
	ld	de,(starta)
	inc	bc
	ldir
	ei
	ret
	nop

startd:	ld	hl,(starta)
      	ld	de,(enda)
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,(starta)
	scf
	ccf
	adc	hl,bc		; end of data at target address
	ex	de,hl
	ld	hl,romcode
	scf
	ccf
	adc	hl,bc		; end of data at source address
	inc	bc
	lddr
	ei
	ret
	nop

romcode:

END
