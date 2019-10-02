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

	;//BIOS READ Function
	mov ah, 0x02
	mov al, 0x2
	mov ch, byte [ TRACKNUMBER ]
	mov cl, byte [ SECTORNUMBER ]
	mov dh, byte [ HEADNUMBER ]
	mov dl, 0x00
	int 0x13
	jc HANDLEDISKERROR

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
DISKERRORMESSAGE:	db	'! DISK Error !', 0

;// disk read variables
SECTORNUMBER:	db	0x02
HEADNUMBER:	db	0x00
TRACKNUMBER:	db	0x00

times 510 - ( $ - $$)	db 0x00

db 0x55
db 0xAA
