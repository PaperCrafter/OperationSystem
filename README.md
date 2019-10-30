OperationSystem
===========

##### This is project is for operation system assignment

# 진행상황
-------
## 진행내역

### 확인하셔야 할 사항
```
1. 부트로더의 엔트리포인트는 0 입니다.
2. 2번 부트로더의 엔트리 포인트는 0x90000 입니다.
3. 커널의 엔트리 포인트는 0x10000 입니다.
4. 이후 구현되는 시스템의 엔트리 포인트는 0x10200 입니다.
5. 페이지 테이블의 끝은 0x143000 입니다.
```

### assignment1
```
1. 2 stage bootLoader -complete-
2. print year, day, month, century -complete-
```
---------------------

### assignment2
```
1. BIOS를 통해 시스템의 메모리 맵 정보를 알아낸 후, 사용 가능 메모리 크기를 추출한다. (INT 15h /AX=E820h 사용)
> 완료
2. 페이지 테이블 이용 
> 완료
3. 5 구현
> 완료
```
------------------

### Trouble Shooting guide 
```
1. 부트로더 크기가 더 필요한 경우 2번 부트로더의 SECTORNUMBER:	db	0x04 
부분에서 추가적으로 필요한 메모리 512 단위로 숫자를  1씩 증가시켜 주시면 됩니다.
더불어 times 1022 - ( $ - $$)	db 0x00 부분도 512*(n-2) -2 만큼 설정해 주시면 됩니다.

2. 8장 내용 커널 이미지가 전부 읽히지 않는 문제 발생 : 부트로더 2의 al 레지스터의 mov al, 0x01 => 0x02로 바꾸어 주었습니다

```
## 진행 중인 사항

### assignment3 
```
1. Exception handler에 page fualt, protection falut 추가 및 raisefault 명령어 구현
>완료
2. page fualt, protection falut present bit에 따라 구별해서 발생
>완료
3.
4.
```