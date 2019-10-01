[ORG 0x00]  ;
[BITS 16]   ;

SECTION .text
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x1100;
    
    mov ds, ax
    mov es, ax

    ; BIOS A20 Activate
	mov ax, 0x2401
    int 0x15

    jc .A20GATEERROR
    jmp .A20GATESUCCESS

.A20GATEERROR:
    in al, 0x92
    or al, 0x02
    and al, 0xFE
    out 0x92, al

.A20GATESUCCESS:
    cli
    lgdt[GDTR]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   보호모드로 진입
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

mov eax, 0x4000003B
mov cr0, eax

jmp dword 0x08: (PROTECTEDMODE -$$ + 0x11000)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   함수 코드 영역
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]
PROTECTEDMODE:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ;
    mov ss, ax
    mov esp, 0xFFFE
    mov ebp, 0xFFFE

    ;과제 1. RAM 크기 출력
    push (RAMSIZEMESSAGE - $$ + 0x11000)
    push 3
    push 0
    call PRINTMESSAGE
    add esp, 12 

    ;
    push (SWITCHSUCCESSMESSAGE - $$ + 0x11000)
    push 4
    push 0
    call PRINTMESSAGE
    add esp, 12


    jmp dword 0x08:0x11200

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;메세지를 출력하는 함수
;스택에 x좌표, y좌표, 문자열 

PRINTMESSAGE:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push eax
    push ecx
    push edx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; X, Y좌표를 비디오 메모리와 어드레스를 계산함
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;
    mov eax, dword[ebp+12]
    mov esi, 160
    mul esi
    mov edi, eax

    ;
    mov eax, dword[ebp + 8]
    mov esi, 2
    mul esi
    add edi, eax

    ;
    mov esi, dword[ebp + 16]

.MESSAGELOOP:
    mov cl, byte[esi]

    cmp cl, 0
    je .MESSAGEEND

    mov byte [edi+0xB8000], cl

    add esi, 1
    add edi, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 아래의 데이터들을 8바이트에 맞춰 정렬하기 위해 추가
align 8, db 0

;GDTR
GDTR:
    dw GDTEND - GDT -1
    dd (GDT -$$ + 0x11000)

GDT:
    NULLDescriptor:
        dw 0X0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    CODEDESCRIPTOR:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xCF
        db 0x00

    DATADESCRIPTOR:
        dw 0XFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xCF
        db 0x00
        
GDTEND:

SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0
RAMSIZEMESSAGE: db 'RAM Size XX MB', 0

times 512 -($ - $$) db 0x00



