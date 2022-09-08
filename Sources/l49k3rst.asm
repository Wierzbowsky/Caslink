; BLOAD MODULE FOR 49KB ROMS PART 3 WITH RESET (FOR CASLINK3 PROJECT)
; COPYRIGHT (C) 1999-2022 ALEXEY PODREZOV

	org	#9000

start:	jp	start1

starta:	dw	0
enda:	dw	0
execa:	dw	0
crc:	db	0

caslstr:db	"[CasLink3] Loading failed: CRC ERROR!",0

start1: di
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
	ld	de,caslstr
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

start5:	di
	ld	a,(#ffff)
	cpl
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

start6: ld	hl,romcode
	ld	de,start8+5
	ld	a,(hl)
	ld	(de),a		; transfer byte from 8000h to patcher
	inc	hl
	ld	de,start8+9
	ld	a,(hl)
	ld	(de),a		; transfer byte from 8001h to patcher
	inc	hl
	ld	de,start8+13
	ld	a,(hl)
	ld	(de),a		; transfer byte from 8002h to patcher
	ld	hl,(execa)
	push	hl
	ld	(start8+16),hl	; transfer start address to patcher
	ld	a,(hl)
	ld	de,start8+20
	ld	(de),a		; transfer 1st byte from epa
	inc	hl
	ld	a,(hl)
	ld	de,start8+24
	ld	(de),a		; transfer 2nd byte from epa
	inc	hl
	ld	a,(hl)
	ld	de,start8+28
	ld	(de),a		; transfer 3rd byte from epa
	pop	hl
	ld	a,#c3
	ld	(hl),a		; place call opcode at epa
	inc	hl
	push	hl
	ld	hl,romcode+4
	ld	de,#8000
	scf
	ccf
	sbc	hl,de		; offset for finding the end of rom+4
	ld	de,(enda)
	scf
	ccf
	adc	hl,de
	ex	de,hl
	push	de
	ld	hl,start8
	ld	bc,33
	ldir			; transfer patcher to the end of rom+4
	ld	hl,(enda)
	ld	de,37
	scf
	ccf
	adc	hl,de
	ld	(enda),hl	; adjust rom's end address with patcher size
	pop	de
	pop	hl
	ld	a,e
	ld	(hl),a		; save the low byte of call address
	inc	hl
	ld	a,d
	ld	(hl),a		; save the high byte of call address

	ld	hl,start7
      	ld	de,start8+1
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

start7:	ld	hl,(starta)
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
	and	#fc
	out	(#a8),a
	rst	#30
	dw	0
	dw	0
	nop

start8:	di
	ld	hl,#8000
	ld	a,0		; +5 bytes
	ld	(hl),a
	inc	hl
	ld	a,0 		; +9 bytes
	ld	(hl),a
	inc	hl
	ld	a,0		; +13 bytes
	ld	(hl),a
	ld	hl,#0000	; +16 bytes
	push	hl
	ld	a,0		; +20 bytes
	ld	(hl),a
	inc	hl
	ld	a,0		; +24 bytes
	ld	(hl),a
	inc	hl
	ld	a,0  		; +28 bytes
	ld	(hl),a
	pop	hl
	jp	(hl)
	nop

romcode:

END
