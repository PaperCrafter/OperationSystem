[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

TOTALSECTORCOUNT:	dw 1024

START:
	mov ax, 0x07C0
	mov ds, ax
	mov ax, 0xB800
	mov es, ax

	;// stack create
	mov ax, 0x0000
	mov ss, ax
	mov sp, 0xFFFE
	mov bp, 0xFFFE

	mov si, 0

;// MESSAGE CLEAR ==================================================
.SCREENCLEARLOOP:
	mov byte [ es: si ], 0
	mov byte [ es: si + 1 ], 0x0F

	add si, 2

	cmp si, 80 * 25 * 2
	jl .SCREENCLEARLOOP


;// PRINT HELLO MESSAGE ==================================================

	push MESSAGE1
	push 0
	push 0
	call PRINTMESSAGE
	add	sp, 6

;// HW1 =========

	mov ah, 0x04
	int 0x1A

	;month
	MOV BH, DH
	SHR BH, 4
	ADD BH, 30H
	MOV [DATE+14], BH
	MOV BH, DH
	AND BH, 0FH
	ADD BH, 30H
	MOV [DATE+15], BH

	;day
	MOV BH, DL
	SHR BH, 4
	ADD BH, 30H
	MOV [DATE+17], BH
	MOV BH, DL
	AND BH, 0FH
	ADD BH, 30H
	MOV [DATE+18], BH

	;century
	MOV BH, CH
	SHR BH, 4
	ADD BH, 30H
	MOV [DATE+20], BH
	MOV BH, CH
	AND BH, 0FH
	ADD BH, 30H
	MOV [DATE+21], BH

	;year
	MOV BH, CL
	SHR BH, 4
	ADD BH, 30H
	MOV [DATE+22], BH
	MOV BH, CL
	AND BH, 0FH
	ADD BH, 30H
	MOV [DATE+23], BH

	push DATE
	push 1
	push 0
	call PRINTMESSAGE
	add sp, 6

;// PRINT OS LOAD MESSAGE ==================================================
	push IMAGELOADINGMESSAGE
	push 2
	push 0
	call PRINTMESSAGE
	add sp, 6

;// RESET DISK ==================================================
RESETDISK:
	mov ax, 0
	mov dl, 0
	int 0x13
	jc HANDLEDISKERROR

	;// READ SECTOR
	mov si, 0x1000
	mov es, si
	mov bx, 0x0000

	mov di, word [ TOTALSECTORCOUNT ]

;// READ DATA ==================================================
READDATA:
	cmp di, 0
	je READEND
	sub di, 0x1

	;//BIOS READ Function
	mov ah, 0x02
	mov al, 0x1
	mov ch, byte [ TRACKNUMBER ]
	mov cl, byte [ SECTORNUMBER ]
	mov dh, byte [ HEADNUMBER ]
	mov dl, 0x00
	int 0x13
	jc HANDLEDISKERROR

	add si, 0x0020
	mov es, si

	mov al, byte [ SECTORNUMBER ]
	add al, 0x01
	mov byte [ SECTORNUMBER ], al
	cmp al, 19
	jl READDATA

	xor byte [ HEADNUMBER ], 0x01
	mov byte [ SECTORNUMBER ], 0x01

	cmp byte [ HEADNUMBER ], 0x00
	jne READDATA

	add byte [ TRACKNUMBER ], 0x01
	jmp READDATA

READEND:
	;// print message
	push LOADINGCOMPLETEMESSAGE
	push 2
	push 30
	call PRINTMESSAGE
	add	sp, 6

	jmp 0x1000:0x0000


;// FUNCTION ==================================================

HANDLEDISKERROR:
	push DISKERRORMESSAGE
	push 2
	push 20
	call PRINTMESSAGE

	jmp $

PRINTMESSAGE:
	push bp
	mov bp, sp

	push es
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xB800
	mov es, ax

	mov ax, word [ bp + 6 ]
	mov si, 160
	mul si
	mov di, ax

	mov ax, word [ bp + 4 ]
	mov si, 2
	mul si
	add di, ax

	mov si, word [ bp + 8 ]

.MESSAGELOOP:
	mov cl, byte [ si ]
	cmp cl, 0
	je .MESSAGEEND

	mov byte [ es: di ], cl

	add si, 1
	add di, 2

	jmp .MESSAGELOOP

.MESSAGEEND:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp
	ret

;// DATA ==================================================
MESSAGE1:	db 'OS 64BIT Boot Loader Start.', 0

DATE: db 'Current Data: 00/00/0000', 0

DISKERRORMESSAGE:	db	'! DISK Error !', 0
IMAGELOADINGMESSAGE:	db	'---OS Image Loading---', 0
LOADINGCOMPLETEMESSAGE: db	'[ Complete ]', 0

	;// disk read variables
SECTORNUMBER:	db	0x02
HEADNUMBER:	db	0x00
TRACKNUMBER:	db	0x00

times 510 - ( $ - $$)	db 0x00

db 0x55
db 0xAA
