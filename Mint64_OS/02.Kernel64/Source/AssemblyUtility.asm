
[BITS 64]         

SECTION .text     

global kInPortByte, kOutPortByte, kInPortWord, kOutPortWord
global kLoadGDTR, kLoadTR, kLoadIDTR
global kEnableInterrupt, kDisableInterrupt, kReadRFLAGS
global kReadTSC
global kSwitchContext, kHlt, kTestAndSet
global kInitializeFPU, kSaveFPUContext, kLoadFPUContext, kSetTS, kClearTS

kHlt:
    hlt
    hlt
    ret

kInPortByte:
    push rdx                

    mov rdx, rdi    
    mov rax, 0      
    in al, dx                           

    pop rdx         
    ret             


kOutPortByte:
    push rdx       
    push rax       

    mov rdx, rdi   
    mov rax, rsi   
    out dx, al                        

    pop rax         
    pop rdx
    ret             

; ��Ʈ�κ��� 2����Ʈ�� ����
;   PARAM: ��Ʈ ��ȣ
kInPortWord:
    push rdx        ; �Լ����� �ӽ÷� ����ϴ� �������͸� ���ÿ� ����
                    ; �Լ��� ������ �κп��� ���ÿ� ���Ե� ���� ���� ����
    
    mov rdx, rdi    ; RDX �������Ϳ� �Ķ���� 1(��Ʈ ��ȣ)�� ����
    mov rax, 0      ; RAX �������͸� �ʱ�ȭ
    in ax, dx       ; DX �������Ϳ� ����� ��Ʈ ��巹������ �� ����Ʈ�� �о�
                    ; AX �������Ϳ� ����, AX �������ʹ� �Լ��� ��ȯ ������ ����
    
    pop rdx         ; �Լ����� ����� ���� �������͸� ����
    ret             ; �Լ��� ȣ���� ���� �ڵ��� ��ġ�� ����
    
; ��Ʈ�� 2����Ʈ�� ��
;   PARAM: ��Ʈ ��ȣ, ������
kOutPortWord:
    push rdx        ; �Լ����� �ӽ÷� ����ϴ� �������͸� ���ÿ� ����
    push rax        ; �Լ��� ������ �κп��� ���ÿ� ���Ե� ���� ���� ����
    
    mov rdx, rdi    ; RDX �������Ϳ� �Ķ���� 1(��Ʈ ��ȣ)�� ����
    mov rax, rsi    ; RAX �������Ϳ� �Ķ���� 2(������)�� ����    
    out dx, ax      ; DX �������Ϳ� ����� ��Ʈ ��巹���� AX �������Ϳ� �����
                    ; �� ����Ʈ�� ��
    
    pop rax         ; �Լ����� ����� ���� �������͸� ����
    pop rdx
    ret             ; �Լ��� ȣ���� ���� �ڵ��� ��ġ�� ����


kLoadGDTR:
    lgdt [ rdi ]    
                    
    ret


kLoadTR:
    ltr di      
                  
    ret

kLoadIDTR:
    lidt [ rdi ]    
                    
    ret

kEnableInterrupt:
    sti     
    ret

kDisableInterrupt:
    cli            
    ret

kReadRFLAGS:
    pushfq              
    pop rax             
                           
    ret

;타임 스탬프 카운터를 읽어서 반환
; PRAM 없음
kReadTSC:
    push rdx
    
    rdtsc

    shl rdx, 32
    or rax, rdx

    pop rdx
    ret


; 테스크 관련 어셈블리어 함수
%macro KSAVECONTEXT 0       ; �Ķ���͸� ���޹��� �ʴ� KSAVECONTEXT ��ũ�� ����
    ; RBP �������ͺ��� GS ���׸�Ʈ �����ͱ��� ��� ���ÿ� ����
    push rbp
    push rax
    push rbx
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov ax, ds      ; DS ���׸�Ʈ �����Ϳ� ES ���׸�Ʈ �����ʹ� ���ÿ� ����
    push rax        ; ������ �� �����Ƿ�, RAX �������Ϳ� ������ �� ���ÿ� ����
    mov ax, es
    push rax
    push fs
    push gs 
%endmacro       ; ��ũ�� ��


; ���ؽ�Ʈ�� �����ϴ� ��ũ��
%macro KLOADCONTEXT 0   ; �Ķ���͸� ���޹��� �ʴ� KSAVECONTEXT ��ũ�� ����
    ; GS ���׸�Ʈ �����ͺ��� RBP �������ͱ��� ��� ���ÿ��� ���� ����
    pop gs
    pop fs
    pop rax
    mov es, ax      ; ES ���׸�Ʈ �����Ϳ� DS ���׸�Ʈ �����ʹ� ���ÿ��� ����
    pop rax         ; ���� ������ �� �����Ƿ�, RAX �������Ϳ� ������ �ڿ� ����
    mov ds, ax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    pop rbp        
%endmacro       ; ��ũ�� ��

; Current Context�� ���� ���ؽ�Ʈ�� �����ϰ� Next Task���� ���ؽ�Ʈ�� ����
;   PARAM: Current Context, Next Context
kSwitchContext:
    push rbp        ; ���ÿ� RBP �������͸� �����ϰ� RSP �������͸� RBP�� ����
    mov rbp, rsp
    
    ; Current Context�� NULL�̸� ���ؽ�Ʈ�� ������ �ʿ� ����
    pushfq          ; �Ʒ��� cmp�� ����� RFLAGS �������Ͱ� ������ �ʵ��� ���ÿ� ����
    cmp rdi, 0      ; Current Context�� NULL�̸� ���ؽ�Ʈ �������� �ٷ� �̵�
    je .LoadContext 
    popfq           ; ���ÿ� ������ RFLAGS �������͸� ����

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ���� �½�ũ�� ���ؽ�Ʈ�� ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    push rax            ; ���ؽ�Ʈ ������ ���������� ����� RAX �������͸� ���ÿ� ����
    
    ; SS, RSP, RFLAGS, CS, RIP �������� ������� ����
    mov ax, ss                          ; SS �������� ����
    mov qword[ rdi + ( 23 * 8 ) ], rax

    mov rax, rbp                        ; RBP�� ����� RSP �������� ����
    add rax, 16                         ; RSP �������ʹ� push rbp�� Return Address��
    mov qword[ rdi + ( 22 * 8 ) ], rax  ; ������ ������ ����
    
    pushfq                              ; RFLAGS �������� ����
    pop rax
    mov qword[ rdi + ( 21 * 8 ) ], rax

    mov ax, cs                          ; CS �������� ����
    mov qword[ rdi + ( 20 * 8 ) ], rax
    
    mov rax, qword[ rbp + 8 ]           ; RIP �������͸� Return Address�� �����Ͽ� 
    mov qword[ rdi + ( 19 * 8 ) ], rax  ; ���� ���ؽ�Ʈ ���� �ÿ� �� �Լ��� ȣ���� 
                                        ; ��ġ�� �̵��ϰ� ��
    
    ; ������ �������͸� ������ �� ���ͷ�Ʈ�� �߻����� ��ó�� ������ ���ؽ�Ʈ�� ��� ����
    pop rax
    pop rbp
    
    ; ���� ���κп� SS, RSP, RFLAGS, CS, RIP �������͸� ���������Ƿ�, ���� ������
    ; push ���ɾ�� ���ؽ�Ʈ�� �����ϱ� ���� ������ ����
    add rdi, ( 19 * 8 )
    mov rsp, rdi
    sub rdi, ( 19 * 8 )
    
    ; ������ �������͸� ��� Context �ڷᱸ���� ����
    KSAVECONTEXT

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; ���� �½�ũ�� ���ؽ�Ʈ ����
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.LoadContext:
    mov rsp, rsi
    
    ; Context �ڷᱸ������ �������͸� ����
    KLOADCONTEXT
    iretq

kTestAndSet:
    mov rax, rsi        ; �� ��° �Ķ������ ���� ���� RAX �������Ϳ� ���� 
    
    ; RAX �������Ϳ� ����� ���� ���� ù ��° �Ķ������ �޸� ��巹���� ����
    ; ���Ͽ� �� ���� ���ٸ� �� ��° �Ķ������ ���� ù ��° �Ķ���Ͱ� ����Ű��
    ; ��巹���� ����
    lock cmpxchg byte [ rdi ], dl   
    je .SUCCESS         ; ZF ��Ʈ�� 1�̸� ���ٴ� ���̹Ƿ� .SUCCESS�� �̵�

.NOTSAME:               ; Destination�� Compare�� �ٸ� ���
    mov rax, 0x00
    ret
    
.SUCCESS:               ; Destination�� Compare�� ���� ���
    mov rax, 0x01
    ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   FPU ���� ��������� �Լ�
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;    
; FPU�� �ʱ�ȭ
;   PAPAM: ����
kInitializeFPU:
    finit               ; FPU �ʱ�ȭ�� ����
    ret
    
; FPU ���� �������͸� ���ؽ�Ʈ ���ۿ� ����
;   PARAM: Buffer Address
kSaveFPUContext:
    fxsave  [ rdi ]     ; ù ��° �Ķ���ͷ� ���޵� ���ۿ� FPU �������͸� ����
    ret
    
; FPU ���� �������͸� ���ؽ�Ʈ ���ۿ��� ����
;   PARAM: Buffer Address
kLoadFPUContext:
    fxrstor [ rdi ]     ; ù ��° �Ķ���ͷ� ���޵� ���ۿ��� FPU �������͸� ����
    ret

; CR0 ��Ʈ�� ���������� TS ��Ʈ�� 1�� ����
;   PARAM: ����
kSetTS:
    push rax            ; ���ÿ� RAX ���������� ���� ����

    mov rax, cr0        ; CR0 ��Ʈ�� ���������� ���� RAX �������ͷ� ����
    or rax, 0x08        ; TS ��Ʈ(��Ʈ 7)�� 1�� ����
    mov cr0, rax        ; TS ��Ʈ�� 1�� ������ ���� CR0 ��Ʈ�� �������ͷ� ����

    pop rax             ; ���ÿ��� RAX ���������� ���� ����
    ret
    
; CR0 ��Ʈ�� ���������� TS ��Ʈ�� 0���� ����
;   PARAM: ����
kClearTS:
    clts                ; CR0 ��Ʈ�� �������Ϳ��� TS ��Ʈ�� 0���� ����
    ret    
