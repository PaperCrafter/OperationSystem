/**
 *  file    DynmaicMemory.h
 *  date    2009/04/11
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �޸� �Ҵ�� ������ ���õ� ��� ����
 */

#ifndef __DYNAMICMEMORY_H__
#define __DYNAMICMEMORY_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// ���� �޸� ������ ���� ��巹��, 1Mbyte ������ ����
#define DYNAMICMEMORY_START_ADDRESS     ( ( TASK_STACKPOOLADDRESS + \
        ( TASK_STACKSIZE * TASK_MAXCOUNT ) + 0xfffff ) & 0xfffffffffff00000 )
// ���� ����� �ּ� ũ��, 1KB
#define DYNAMICMEMORY_MIN_SIZE          ( 1 * 1024 )

// ��Ʈ���� �÷���
#define DYNAMICMEMORY_EXIST             0x01
#define DYNAMICMEMORY_EMPTY             0x00

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// ��Ʈ���� �����ϴ� �ڷᱸ��
typedef struct kBitmapStruct
{
    BYTE* pbBitmap;
    QWORD qwExistBitCount;
} BITMAP;

// ���� ����� �����ϴ� �ڷᱸ��
typedef struct kDynamicMemoryManagerStruct
{
    // ��� ����Ʈ�� �� ������ ���� ũ�Ⱑ ���� ���� ����� ����, �׸��� �Ҵ�� �޸� ũ��
    int iMaxLevelCount;
    int iBlockCountOfSmallestBlock;
    QWORD qwUsedSize;
    
    // ��� Ǯ�� ���� ��巹���� ������ ��巹��
    QWORD qwStartAddress;
    QWORD qwEndAddress;
    
    // �Ҵ�� �޸𸮰� ���� ��� ����Ʈ�� �ε����� �����ϴ� ������ ��Ʈ�� �ڷᱸ���� 
    // ��巹��
    BYTE* pbAllocatedBlockListIndex;
    BITMAP* pstBitmapOfLevel;
} DYNAMICMEMORY;

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
void kInitializeDynamicMemory( void );
void* kAllocateMemory( QWORD qwSize );
BOOL kFreeMemory( void* pvAddress );
void kGetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, 
        QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, 
        QWORD* pqwUsedMemorySize ); 
DYNAMICMEMORY* kGetDynamicMemoryManager( void );

static QWORD kCalculateDynamicMemorySize( void );
static int kCalculateMetaBlockCount( QWORD qwDynamicRAMSize );
static int kAllocationBuddyBlock( QWORD qwAlignedSize );
static QWORD kGetBuddyBlockSize( QWORD qwSize );
static int kGetBlockListIndexOfMatchSize( QWORD qwAlignedSize );
static int kFindFreeBlockInBitmap( int iBlockListIndex );
static void kSetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag );
static BOOL kFreeBuddyBlock( int iBlockListIndex, int iBlockOffset );
static BYTE kGetFlagInBitmap( int iBlockListIndex, int iOffset );

#endif /*__DYNAMICMEMORY_H__*/
