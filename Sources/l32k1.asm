; BLOAD MODULE FOR 32KB ROMS PART 1 (FOR CASLINK4 PROJECT)
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

caslstr:db	"[CasLink4] Loading file, please wait...",0

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
	ld	a,(primsl)
	ld	(hl),a		; number for primary slot
	ld	hl,patch4
	ld	(hl),#3e	; ld a,n
	inc	hl
	ld	a,(secsl)
	ld	(hl),a		; number for primary slot

start5c:
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
patch1:	and	15
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
patch2:	and	15
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
patch3:	nop
	nop
	out	(#a8),a
	pop	af
patch4:	nop
	nop
	ld	(#ffff),a
	ei
	ret
	nop

romcode:

END
