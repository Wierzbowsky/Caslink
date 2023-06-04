; PRE-BLOAD MODULE FOR MULTIFILE MODE (FOR CASLINK4 PROJECT)
; COPYRIGHT (C) 1999-2023 ALEXEY PODREZOV

	org	#C100

start:	jp	start1

loadcmd:db	#1e
	db	"bload"
	db	34
	db	"cas:binrom part 1 "
	db	34
	db	",r"
	db	13,0

caslstr:db	"[CasLink4] Loading file, please wait...",0
ramerr:	db	"[CasLink4] Loading failed: NO RAM!",0
expand:	db	0
primsl:	db	0
secsl:	db	0

start1:	ei
	ld	a,#42
	ld	(#9000),a
	ld	a,(#9000)
	cp	#42		; RAM at 9000 exists?
	jr	nz,NoRam
	xor	a
	call	#00f3		; motor off
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

NoRam:
	ld	a,(#FFFF)
	cpl
	and	#f0
	ld	(#FFFF),a	; set page for BIOS and Basic
	in	a,(#a8)
	and	#f0
	out	(#a8),a		; set required slot for BIOS and Basic
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
	ld	de,ramerr
RamE1:	ld	a,(de)
	or	a
	jr	z,RamE2
	inc	de
	call	#00a2		; display character
	inc	h
	call	#00c6		; set next position
	jr	RamE1
RamE2:	ld	hl,#0103
	call	#00c6		; set cursor position to 1:3
	call	#00c0		; beep
	call	#0156		; clears keyboard buffer
	ei
	ret

CheckRam:
	ld	a,b
	out	(#a8),a		; set supposed mapper page in slot at 0x4000
	ld	a,c
	ld	(#ffff),a	; set supposed mapper page in slot at 0x4000
	ld	a,#42
	ld	hl,#8000
	ld	(hl),a
	ld	a,(hl)
	cp	#42
	ret

start3:	di
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
	set	4,a
	res	5,a
	ld	b,a
	ld	a,(secsl)
	set	4,a
	res	5,a
	ld	c,a
	call	CheckRam	; look for RAM expansion in slot 1
	jr	z,start5a
	ld	a,(primsl)
	res	4,a
	set	5,a
	ld	b,a
	ld	a,(secsl)
	res	4,a
	set	5,a
	ld	c,a
	call	CheckRam	; look for RAM expansion in slot 2
	jr	z,start5a
	ld	a,(primsl)
	set	4,a
	set	5,a
	ld	b,a
	ld	a,(secsl)
	set	4,a
	set	4,a
	ld	c,a
	call	CheckRam	; look for RAM expansion in slot 3
	jp	nz,NoRam
start5a:
	xor	a
	ld	(#8000),a
	ld	a,b
	out	(#a8),a		; set supposed mapper page in slot at 0x4000
	ld	a,c
	ld	(#ffff),a	; set supposed mapper page in slot at 0x4000

start5c:
	ld	hl,#0103
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
