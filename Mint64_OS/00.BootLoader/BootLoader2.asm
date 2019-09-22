[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTORCOUNT:	dw	0x0000
TOTALSECTORCOUNT:	dw 1024

START:
	mov ax, cs
	mov ds, ax
	mov si, 0

;// HW1 =========
;day interrup you should add code in here
	mov ah, 0x04
	int 0x1A


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
	mov si, 0x2000
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

	jmp 0x2000:0x0000


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
DATE: db 'Current Data: 00/00/0000', 0

DISKERRORMESSAGE:	db	'! DISK Error !', 0
IMAGELOADINGMESSAGE:	db	'---OS Image Loading---', 0
LOADINGCOMPLETEMESSAGE: db	'[ Complete ]', 0

	;// disk read variables
SECTORNUMBER:	db	0x03
HEADNUMBER:	db	0x00
TRACKNUMBER:	db	0x00

times 510 - ( $ - $$)	db 0x00

db 0x55
db 0xAA