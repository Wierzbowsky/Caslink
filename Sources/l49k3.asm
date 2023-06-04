; BLOAD MODULE FOR 49KB ROMS PART 3 (FOR CASLINK4 PROJECT)
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
	ld	a,#41
	ld	(hl),a		; restore damaged AB
	ld	a,b
	out	(#a8),a		; set supposed mapper page in slot at 0x4000
	ld	a,c
	ld	(#ffff),a	; set supposed mapper page in slot at 0x4000
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
	ld	c,a
	and	#f0
	ld	b,a
	ld	a,c
	rrca
	rrca
	rrca
	rrca
patch1:	and	15
	or	b
	ld	(#ffff),a
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
	out	(#a8),a

	ld	hl,start6
      	ld	de,romcode+1
	ex	de,hl
	scf
	ccf
	sbc	hl,de
	ld	b,h
	ld	c,l
	ld	hl,start6
	ld	de,#f560
	push	de
	ldir
	ret

start6:	ld	hl,(execa)
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
	ld	de,(starta)
	ldir
	pop	hl
	pop	af
patch3:	and	#fc
	out	(#a8),a
	ei
	jp	(hl)
	nop

romcode:

END
