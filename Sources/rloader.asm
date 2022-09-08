; BLOAD MODULE FOR 8/16KB ROMS (FOR CASLINK3 PROJECT)
; COPYRIGHT (C) 1999-2022 ALEXEY PODREZOV

	org	#9000

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
	ld	a,(#ffff)
	cpl
	and	#f0
	ld	c,a
	rrca
	rrca
	rrca
	rrca
	and	15
	or	c
	ld	(#ffff),a
	in	a,(#a8)
	and	#f0
	ld	b,a
	rrca
	rrca
	rrca
	rrca
	and	15
	or	b
	push	af
	out	(#a8),a

start6:	ld	hl,(starta)
	ld	a,d
	cp	#80
	jr	c,start7
	ld	hl,start7
      	ld	de,romcode+1
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,start7
	ld	de,#f560
	push	de
	ldir
	ret

start7:	ld	hl,(execa)
	push	hl
	ld	hl,(starta)
      	ld	de,(enda)
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,romcode
	ldir
	pop	hl
	ld	a,h
	cp	#40
	jr	c,start8
	pop	af
	and	#fc
	out	(#a8),a
	ei
	jp	(hl)

start8:	pop	af
	ei
	jp	(hl)
	nop
	
romcode:

END
