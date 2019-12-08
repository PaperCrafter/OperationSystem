 [BITS 32]

 global kReadCPUID, kSwitchAndExecute64bitKernel

 SECTION .text

 kReadCPUID:
 	push ebp
 	mov ebp, esp
 	push eax
 	push ebx
 	push ecx
 	push edx
 	push esi

 	mov eax, dword [ ebp + 8 ]
 	cpuid

	
	; save parameter
 	mov esi, dword [ ebp + 12 ]
 	mov dword [ esi ], eax

 	mov esi, dword [ ebp + 16 ]
 	mov dword [ esi ], ebx

 	mov esi, dword [ ebp + 20 ]
 	mov dword [ esi ], ecx

 	mov esi, dword [ ebp + 24 ]
 	mov dword [ esi ], edx

 	pop esi
 	pop edx
 	pop ecx
 	pop ebx
 	pop eax
 	pop ebp
 	ret

 ;switch mode
 kSwitchAndExecute64bitKernel:
 	
 	mov eax, cr4
 	or eax, 0x620
 	mov cr4, eax

 	mov eax, 0x100000
 	mov cr3, eax

 	mov ecx, 0xC0000080
 	rdmsr

 	or eax, 0x0100

 	wrmsr
 	
 	mov eax, cr0
 	or eax, 0xE0000000
 	xor eax, 0x60000000
	;add wp 1 
 	add eax, 0x00010000
 	
 	mov cr0, eax

 	jmp 0x08:0x200000

	
mov eax, cr0            ; EAX �������Ϳ� CR0 ��Ʈ�� �������͸� ����
    or eax, 0xE000000E      ; NW ��Ʈ(��Ʈ 29), CD ��Ʈ(��Ʈ 30), PG ��Ʈ(��Ʈ 31),
                            ; TS ��Ʈ(��Ʈ 3), EM ��Ʈ(��Ʈ 2), MP ��Ʈ(��Ʈ 1)��                        ; ��� 1�� ����
    xor eax, 0x60000004     ; NW ��Ʈ(��Ʈ 29), CD ��Ʈ(��Ʈ 30), EM ��Ʈ(��Ʈ 2),
    
	;add wp 1 
 	add eax, 0x00010000
	                        ; EM ��Ʈ(��Ʈ 2)�� XOR�Ͽ� 0���� ����
    mov cr0, eax            ; ������ ���� �ٽ� CR0 ��Ʈ�� �������Ϳ� ����
    
    jmp 0x08:0x200000   ; CS ���׸�Ʈ �����͸� IA-32e ���� �ڵ� ���׸�Ʈ ��ũ���ͷ�
                        ; ��ü�ϰ� 0x200000(2MB) ��巹���� �̵�
    
    ; ����� ������� ����
    jmp $
