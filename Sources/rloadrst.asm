; BLOAD MODULE FOR 8/16KB ROMS (FOR CASLINK4 PROJECT)
; COPYRIGHT (C) 1999-2023 ALEXEY PODREZOV

	org	#9000

start:	jp	start1

starta:	dw	0
enda:	dw	0
execa:	dw	0
crc:	db	0

caserr:	db	"[CasLink4] Loading failed: CRC ERROR!",0
ramerr:	db	"NO RAM!   "
expand:	db	0
primsl:	db	0
secsl:	db	0

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

crcerr:
	ld	a,(#FFFF)
	cpl
	and	#f0
	ld	(#FFFF),a	; set page for BIOS and Basic
	in	a,(#a8)
	and	#f0
	out	(#a8),a		; set required slot for BIOS and Basic
crcerr1:
	ei
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

NoRam:
	ld	hl,ramerr
	ld	de,caserr+27
	ld	bc,10		; change error message to "NO RAM!"
	ldir
	jp	crcerr

CheckRam:
	ld	a,b
	out	(#a8),a		; set supposed mapper page in slot at 0x4000
	ld	a,c
	ld	(#ffff),a	; set supposed mapper page in slot at 0x4000
	ld	a,#42
	ld	hl,#4000
	ld	(hl),a
	ld	a,(hl)
	cp	#42
	ret

start5:	di
	ld	a,(#fcc1)
	ld	(expand),a	; save expansion flag
	ld	a,(#ffff)
	cpl
	ld	(secsl),a	; save secondary slot configuration
	in	a,(#a8)
	ld	(primsl),a	; save primary slot configuration
	and	#f0
	cp	#10		; machines with 16kb or less RAM with expansion in slot 1
	jr	z,start50
	cp	#20		; machines with 16kb or less RAM with expansion in slot 2
	jr	z,start50
	cp	#30		; machines with 16kb or less RAM with expansion in slot 3
	jr	z,start50
	or	a		; RAM in slot 0?
	jp	nz,start5c

start50:
	ld	a,(primsl)
	set	2,a
	res	3,a
	ld	b,a
	ld	a,(secsl)
	set	2,a
	res	3,a
	ld	c,a
	call	CheckRam	; look for RAM expansion in slot 1
	jr	z,start5a
	ld	a,(primsl)
	res	2,a
	set	3,a
	ld	b,a
	ld	a,(secsl)
	res	2,a
	set	3,a
	ld	c,a
	call	CheckRam	; look for RAM expansion in slot 2
	jr	z,start5a
	ld	a,(primsl)
	set	2,a
	set	3,a
	ld	b,a
	ld	a,(secsl)
	set	2,a
	set	3,a
	ld	c,a
	call	CheckRam	; look for RAM expansion in slot 3
	jp	nz,NoRam
start5a:
	ld	a,b
	out	(#a8),a		; set supposed mapper page in slot at 0x4000
	ld	a,c
	ld	(#ffff),a	; set supposed mapper page in slot at 0x4000
	ld	a,(starta+1)
	or	a		; game starts at 0x0000?
	jr	nz,start5b
	res	0,b
	bit	2,b
	jr	z,SetBit1
	set	0,b
SetBit1:
	res	1,b
	bit	3,b
	jr	z,SetBit2
	set	1,b	
SetBit2:
	res	0,c
	bit	2,c
	jr	z,SetBit3
	set	0,c	
SetBit3:
	res	1,c
	bit	3,c
	jr	z,start5b
	set	1,c		; adjust bits for page 0 if game starts at 0x000

start5b:
	ld	hl,patch1
	ld	(hl),#3e	; ld a,n
	inc	hl
	ld	(hl),c		; number for expansion slot
	inc	hl
	ld	(hl),0		; nop
	ld	hl,patch2
	ld	(hl),#3e	; ld a,n
	inc	hl
	ld	(hl),b		; number for primary slot
	inc	hl
	ld	(hl),0		; nop
	ld	hl,patch3
	ld	(hl),#3e	; ld a,n
	inc	hl
	ld	(hl),b		; number for primary slot

start5c:
	ld	a,(#ffff)
	cpl
	and	#f0
	ld	c,a
	rrca
	rrca
	rrca
	rrca
patch1:	and	15
	or	c
	ld	(#ffff),a	; set required pages
	in	a,(#a8)
	and	#f0
	ld	b,a
	rrca
	rrca
	rrca
	rrca
patch2:	and	15
	or	b
	push	af
	out	(#a8),a		; set required slots

start6:
	ld	hl,(starta)
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
	ldir			; copy external command to BIOS area
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
patch3:	and	#fc
	out	(#a8),a
	jr	Reset

start8:	pop	af
	ei
Reset:	
	ld	a,(#FFFF)
	cpl
	and	#f0
	ld	(#FFFF),a	; set page for BIOS and Basic
	in	a,(#a8)
	and	#f0
	out	(#a8),a		; set required slot for BIOS and Basic

	rst	#30
	db	#80		; slot with expansion flag for Daewoo 400s
	dw	0		; BIOS address
	nop

romcode:

END
