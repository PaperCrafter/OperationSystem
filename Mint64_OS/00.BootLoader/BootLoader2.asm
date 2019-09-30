[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTORCOUNT:	dw	0x0000
TOTALSECTORCOUNT:	dw 0x02

START:
	mov ax, cs
	mov ds, ax
	mov si, 0

;// HW1 =========
;day interrup you should add code in here

call JELLER

push DAYMESSAGE
push 1
push 25
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
	mov si, 0x1100
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

	jmp 0x1100:0x0000


;// FUNCTION ==================================================
JELLER:
cmp byte[MONTH], 0x01
je  .MONTH13
cmp byte[MONTH], 0x02
je  .MONTH14

.START:
mov dx, 0x00
add dl, byte[DAY]

mov ax, 0x00 ;초기화
add al, byte[MONTH]
add al, 0x01
mov cx, 0x00 ;초기화
mov cl, 0x0D
mul cl
mov cl, 0x05
div cl
add dl, al

add dl, byte[YEAR]

mov ax, 0x00 ;초기화
mov al, byte[YEAR]
mov cl, 0x04
div cl
add dl, al

mov ax, 0x00 ;초기화
mov al, byte[CEN]
mov cl, 0x04
div cl
add dl, al

mov al, byte[CEN]
mov cl, 0x05
mul cl
add dx, ax

mov ax, dx
mov cl, 0x07
div cl

cmp    ah, 0x00
je     .D6

cmp    ah, 0x01
je     .D7

cmp    ah, 0x02
je     .D1

cmp    ah, 0x03
je     .D2

cmp    ah, 0x04
je     .D3

cmp    ah, 0x05
je     .D4

cmp    ah, 0x06
je     .D5

.D1:
mov ax, 'Mo'
mov	[DAYMESSAGE + 0], ax
mov al, 'n'
mov	[DAYMESSAGE + 2], al
ret

.D2:
mov ax, 'Tu'
mov	[DAYMESSAGE + 0], ax
mov al, 'e'
mov	[DAYMESSAGE + 2], al
ret

.D3:
mov ax, 'We'
mov	[DAYMESSAGE + 0], ax
mov al, 'd'
mov	[DAYMESSAGE + 2], al
ret

.D4:
mov ax, 'Th'
mov	[DAYMESSAGE + 0], ax
mov al, 'u'
mov	[DAYMESSAGE + 2], al
ret

.D5:
mov ax, 'Fr'
mov	[DAYMESSAGE + 0], ax
mov al, 'i'
mov	[DAYMESSAGE + 2], al
ret

.D6:
mov ax, 'Sa'
mov	[DAYMESSAGE + 0], ax
mov al, 't'
mov	[DAYMESSAGE + 2], al
ret

.D7:
mov ax, 'Su'
mov	[DAYMESSAGE + 0], ax
mov al, 'n'
mov	[DAYMESSAGE + 2], al
ret


.MONTH13:
mov byte[MONTH], 0x0d
sub byte[YEAR], 0x01
jmp .START

.MONTH14:
mov byte[MONTH], 0x0e
sub byte[YEAR], 0x01
jmp .START


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
;// date variables

YEAR: db 0x13
MONTH: db 0x09
DAY: db 0x16
CEN: db 0x14

DAYMESSAGE: db 'Day', 0

DISKERRORMESSAGE:	db	'', 0
IMAGELOADINGMESSAGE:	db	'', 0
LOADINGCOMPLETEMESSAGE: db	'[ Complete ]', 0

	;// disk read variables
SECTORNUMBER:	db	0x03
HEADNUMBER:	db	0x00
TRACKNUMBER:	db	0x00

times 510 - ( $ - $$)	db 0x00

db 0x55
db 0xAA
