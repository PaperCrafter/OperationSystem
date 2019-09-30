#ifndef __TYPES_H__
#define __TYPES_H__

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int
#define QWORD unsigned long
#define BOOL unsigned char

#define TRYE 1
#define FALSE 0
#define NULL 0

#pragma pack(push, 1)

//data structure which construct textmode in videomode
typedef struct kCharactorStruct
{
    BYTE bCharactor;
    BYTE bAttribute;
}CHARACTER;

#pragma pack(pop)
#endif /*__TYPES_H__*/