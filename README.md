OperationSystem
===========

##### This is project is for operation system assignment

# 진행상황
-------

### 04장 내용까지 구현 완료
```
1. bootLoader testing complete
```

### 05장 까지 구현, 작동 확인
```
1. bootLoader sucsessfully load testkernel to memory
```

### 확인하셔야 할 사항
```
1. 부트로더의 엔트리포인트는 0 입니다.
2. 2번 부트로더의 엔트리 포인트는 0x10000 입니다.
3. 커널의 엔트리 포인트는 0x11000 입니다.
4. 이후 구현되는 시스템의 엔트리 포인트는 0x11200 입니다.
```

### assignment1
```
1. 2 stage bootLoader -complete-
2. print year, day, month, century -complete-
```

### Trouble Shooting
```
부트로더 크기가 더 필요한 경우 2번 부트로더의 SECTORNUMBER:	db	0x04 부분에서 추가적으로 필요한 메모리 512 단위로 숫자를  1씩 증가시켜 주시면 됩니다.
더불어 times 1022 - ( $ - $$)	db 0x00 부분도 512*(n-2) -2 만큼 설정해 주시면 됩니다.

```

### 06장 정상 구동 확인
```

```

### 07장 구동 확인
```
1. 책이 윈도우 기반이라 일부 동작하지 않는 명령어 들이 있어 수정하였습니다.
```

### assignment2
```
1. BIOS를 통해 시스템의 메모리 맵 정보를 알아낸 후, 사용 가능 메모리 크기를 추출한다. (INT 15h /AX=E820h 사용)
2. 페이지 테이블 이용 
```