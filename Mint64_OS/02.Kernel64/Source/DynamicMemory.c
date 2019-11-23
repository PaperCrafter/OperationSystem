/**
 *  file    DynmaicMemory.h
 *  date    2009/04/11
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �޸� �Ҵ�� ������ ���õ� �ҽ� ����
 */
#include "DynamicMemory.h"
#include "Utility.h"
#include "Task.h"

static DYNAMICMEMORY gs_stDynamicMemory;

/**
 *  ���� �޸� ���� �ʱ�ȭ    
 */
void kInitializeDynamicMemory( void )
{
    QWORD qwDynamicMemorySize;
    int i, j;
    BYTE* pbCurrentBitmapPosition;
    int iBlockCountOfLevel, iMetaBlockCount;

    // ���� �޸� �������� ����� �޸� ũ�⸦ �̿��Ͽ� ����� �����ϴµ�
    // �ʿ��� �޸� ũ�⸦ �ּ� ��� ������ ���
    qwDynamicMemorySize = kCalculateDynamicMemorySize();
    iMetaBlockCount = kCalculateMetaBlockCount( qwDynamicMemorySize );

    // ��ü ��� �������� ������ �ʿ��� ��Ÿ����� ������ ������ ������ ������ ���ؼ�
    // ��Ÿ ������ ����
    gs_stDynamicMemory.iBlockCountOfSmallestBlock = 
        ( qwDynamicMemorySize / DYNAMICMEMORY_MIN_SIZE ) - iMetaBlockCount;

    // �ִ� �� ���� ��� ����Ʈ�� �����Ǵ����� ���
    for( i = 0 ; ( gs_stDynamicMemory.iBlockCountOfSmallestBlock >> i ) > 0 ; i++ )
    {
        //DO Nothing
        ;
    }
    gs_stDynamicMemory.iMaxLevelCount = i;
    
    // �Ҵ�� �޸𸮰� ���� ��� ����Ʈ�� �ε����� �����ϴ� ������ �ʱ�ȭ
    gs_stDynamicMemory.pbAllocatedBlockListIndex = ( BYTE* ) DYNAMICMEMORY_START_ADDRESS;
    for( i = 0 ; i < gs_stDynamicMemory.iBlockCountOfSmallestBlock ; i++ )
    {
        gs_stDynamicMemory.pbAllocatedBlockListIndex[ i ] = 0xFF;
    }
    
    // ��Ʈ�� �ڷᱸ���� ���� ��巹�� ����
    gs_stDynamicMemory.pstBitmapOfLevel = ( BITMAP* ) ( DYNAMICMEMORY_START_ADDRESS +
        ( sizeof( BYTE ) * gs_stDynamicMemory.iBlockCountOfSmallestBlock ) );
    // ���� ��Ʈ���� ��巹���� ����
    pbCurrentBitmapPosition = ( ( BYTE* ) gs_stDynamicMemory.pstBitmapOfLevel ) + 
        ( sizeof( BITMAP ) * gs_stDynamicMemory.iMaxLevelCount );
    // ��� ����Ʈ ���� ������ ���鼭 ��Ʈ���� ���� 
    // �ʱ� ���´� ���� ū ��ϰ� ������ ��ϸ� �����ϹǷ� �������� ����ִ� ������ ����
    for( j = 0 ; j < gs_stDynamicMemory.iMaxLevelCount ; j++ )
    {
        gs_stDynamicMemory.pstBitmapOfLevel[ j ].pbBitmap = pbCurrentBitmapPosition;
        gs_stDynamicMemory.pstBitmapOfLevel[ j ].qwExistBitCount = 0;
        iBlockCountOfLevel = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> j;

        // 8�� �̻� �������� ���� ������� ��� ������ �� �����Ƿ�, ��� ����ִ� ������ ����
        for( i = 0 ; i < iBlockCountOfLevel / 8 ; i++ )
        {
            *pbCurrentBitmapPosition = 0x00;
            pbCurrentBitmapPosition++;
        }

        // 8�� ������ �������� �ʴ� ������ ��ϵ鿡 ���� ó��
        if( ( iBlockCountOfLevel % 8 ) != 0 )
        {
            *pbCurrentBitmapPosition = 0x00;
            // ���� ����� Ȧ����� ������ �� ����� ���յǾ� ���� ������� �̵����� ����
            // ���� ������ ����� ���� ��� ����Ʈ�� �����ϴ� ������ ������� ����
            i = iBlockCountOfLevel % 8;
            if( ( i % 2 ) == 1 )
            {
                *pbCurrentBitmapPosition |= ( DYNAMICMEMORY_EXIST << ( i - 1 ) );
                gs_stDynamicMemory.pstBitmapOfLevel[ j ].qwExistBitCount = 1;
            }
            pbCurrentBitmapPosition++;
        }
    }        
    
    // ��� Ǯ�� ��巹���� ���� �޸� ũ�� ����
    gs_stDynamicMemory.qwStartAddress = DYNAMICMEMORY_START_ADDRESS + 
        iMetaBlockCount * DYNAMICMEMORY_MIN_SIZE;
    gs_stDynamicMemory.qwEndAddress = kCalculateDynamicMemorySize() + 
        DYNAMICMEMORY_START_ADDRESS;
    gs_stDynamicMemory.qwUsedSize = 0;
}

/**
 *  ���� �޸� ������ ũ�⸦ ���  
 */
static QWORD kCalculateDynamicMemorySize( void )
{
    QWORD qwRAMSize;
    
    // 3GB �̻��� �޸𸮿��� ���� �޸𸮿� ���μ����� ����ϴ� �������Ͱ�
    // �����ϹǷ� �ִ� 3GB������ ���
    qwRAMSize = ( kGetTotalRAMSize() * 1024 * 1024 );
    if( qwRAMSize > ( QWORD ) 3 * 1024 * 1024 * 1024 )
    {
        qwRAMSize = ( QWORD ) 3 * 1024 * 1024 * 1024;
    }   
    
    return qwRAMSize - DYNAMICMEMORY_START_ADDRESS;
}

/**
 *  ���� �޸� ������ �����ϴµ� �ʿ��� ������ �����ϴ� ������ ���
 *      �ּ� ��� ������ �����ؼ� ��ȯ
 */
static int kCalculateMetaBlockCount( QWORD qwDynamicRAMSize )
{
    long lBlockCountOfSmallestBlock;
    DWORD dwSizeOfAllocatedBlockListIndex;
    DWORD dwSizeOfBitmap;
    long i;
    
    // ���� ũ�Ⱑ ���� ����� ������ ����Ͽ� �̸� �������� ��Ʈ�� ������ 
    // �Ҵ�� ũ�⸦ �����ϴ� ������ ���
    lBlockCountOfSmallestBlock = qwDynamicRAMSize / DYNAMICMEMORY_MIN_SIZE;
    // �Ҵ�� ����� ���� ��� ����Ʈ�� �ε����� �����ϴµ� �ʿ��� ������ ���
    dwSizeOfAllocatedBlockListIndex = lBlockCountOfSmallestBlock * sizeof( BYTE );
    
    // ��Ʈ���� �����ϴµ� �ʿ��� ���� ���
    dwSizeOfBitmap = 0;
    for( i = 0 ; ( lBlockCountOfSmallestBlock >> i ) > 0 ; i++ )
    {
        // ��� ����Ʈ�� ��Ʈ�� �����͸� ���� ����
        dwSizeOfBitmap += sizeof( BITMAP );
        // ��� ����Ʈ�� ��Ʈ�� ũ��, ����Ʈ ������ �ø� ó��
        dwSizeOfBitmap += ( ( lBlockCountOfSmallestBlock >> i ) + 7 ) / 8;
    }
    
    // ����� �޸� ������ ũ�⸦ �ּ� ��� ũ��� �ø��ؼ� ��ȯ
    return ( dwSizeOfAllocatedBlockListIndex + dwSizeOfBitmap + 
        DYNAMICMEMORY_MIN_SIZE - 1 ) / DYNAMICMEMORY_MIN_SIZE;
}

/**
 *  �޸𸮸� �Ҵ�
 */
void* kAllocateMemory( QWORD qwSize )
{
    QWORD qwAlignedSize;
    QWORD qwRelativeAddress;
    long lOffset;
    int iSizeArrayOffset;
    int iIndexOfBlockList;

    // �޸� ũ�⸦ ���� ����� ũ��� ����
    qwAlignedSize = kGetBuddyBlockSize( qwSize );
    if( qwAlignedSize == 0 )
    {
        return NULL;
    }
    
    // ���� ���� ������ ������� ������ ����
    if( gs_stDynamicMemory.qwStartAddress + gs_stDynamicMemory.qwUsedSize +
            qwAlignedSize > gs_stDynamicMemory.qwEndAddress )
    {
        return NULL;
    }

    // ���� ��� �Ҵ��ϰ� �Ҵ�� ����� ���� ��� ����Ʈ�� �ε����� ��ȯ
    lOffset = kAllocationBuddyBlock( qwAlignedSize );
    if( lOffset == -1 )
    {
        return NULL;
    }
    
    iIndexOfBlockList = kGetBlockListIndexOfMatchSize( qwAlignedSize );
    
    // ��� ũ�⸦ �����ϴ� ������ ������ �Ҵ�� ���� ����� ���� �� ����Ʈ�� 
    // �ε����� ����
    // �޸𸮸� ������ �� ��� ����Ʈ�� �ε����� ���
    qwRelativeAddress = qwAlignedSize * lOffset;
    iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;
    gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] = 
        ( BYTE ) iIndexOfBlockList;
    gs_stDynamicMemory.qwUsedSize += qwAlignedSize;
    
    return ( void* ) ( qwRelativeAddress + gs_stDynamicMemory.qwStartAddress );
}

/**
 *  ���� ����� ���� ����� ũ��� ���ĵ� ũ�⸦ ��ȯ
 */
static QWORD kGetBuddyBlockSize( QWORD qwSize )
{
    long i;

    for( i = 0 ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        if( qwSize <= ( DYNAMICMEMORY_MIN_SIZE << i ) )
        {
            return ( DYNAMICMEMORY_MIN_SIZE << i );
        }
    }
    return 0;
}

/**
 *  ���� ��� �˰������� �޸� ����� �Ҵ�
 *      �޸� ũ��� ���� ����� ũ��� ��û�ؾ� ��
 */
static int kAllocationBuddyBlock( QWORD qwAlignedSize )
{
    int iBlockListIndex, iFreeOffset;
    int i;
    BOOL bPreviousInterruptFlag;

    // ��� ũ�⸦ �����ϴ� ��� ����Ʈ�� �ε����� �˻�
    iBlockListIndex = kGetBlockListIndexOfMatchSize( qwAlignedSize );
    if( iBlockListIndex == -1 )
    {
        return -1;
    }
    
    // ����ȭ ó��
    bPreviousInterruptFlag = kLockForSystemData();
    
    // �����ϴ� ��� ����Ʈ���� �ֻ��� ��� ����Ʈ���� �˻��Ͽ� ����� ����
    for( i = iBlockListIndex ; i< gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        // ��� ����Ʈ�� ��Ʈ���� Ȯ���Ͽ� ����� �����ϴ��� Ȯ��
        iFreeOffset = kFindFreeBlockInBitmap( i );
        if( iFreeOffset != -1 )
        {
            break;
        }
    }
    
    // ������ ��� ����Ʈ���� �˻��ߴµ��� ������ ����
    if( iFreeOffset == -1 )
    {
        kUnlockForSystemData( bPreviousInterruptFlag );
        return -1;
    }

    // ����� ã������ �� ������ ǥ��
    kSetFlagInBitmap( i, iFreeOffset, DYNAMICMEMORY_EMPTY );

    // ���� ��Ͽ��� ����� ã�Ҵٸ� ���� ����� ����
    if( i > iBlockListIndex )
    {
        // �˻��� ��� ����Ʈ���� �˻��� ������ ��� ����Ʈ���� �������鼭 ���� �����
        // �� ������ ǥ���ϰ� ������ ����� �����ϴ� ������ ǥ����
        for( i = i - 1 ; i >= iBlockListIndex ; i-- )
        {
            // ���� ����� �� ������ ǥ��
            kSetFlagInBitmap( i, iFreeOffset * 2, DYNAMICMEMORY_EMPTY );
            // ������ ����� �����ϴ� ������ ǥ��
            kSetFlagInBitmap( i, iFreeOffset * 2 + 1, DYNAMICMEMORY_EXIST ); 
            // ���� ����� �ٽ� ����
            iFreeOffset = iFreeOffset * 2;
        }
    }    
    kUnlockForSystemData( bPreviousInterruptFlag );
    
    return iFreeOffset;
}

/**
 *  ���޵� ũ��� ���� ������ ��� ����Ʈ�� �ε����� ��ȯ
 */
static int kGetBlockListIndexOfMatchSize( QWORD qwAlignedSize )
{
    int i;

    for( i = 0 ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        if( qwAlignedSize <= ( DYNAMICMEMORY_MIN_SIZE << i ) )
        {
            return i;
        }
    }
    return -1;
}

/**
 *  ��� ����Ʈ�� ��Ʈ�ʸ� �˻��� ��, ����� �����ϸ� ����� �������� ��ȯ
 */
static int kFindFreeBlockInBitmap( int iBlockListIndex )
{
    int i, iMaxCount;
    BYTE* pbBitmap;
    QWORD* pqwBitmap;

    // ��Ʈ�ʿ� �����Ͱ� �������� �ʴ´ٸ� ����
    if( gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount == 0 )
    {
        return -1;
    }
    
    // ��� ����Ʈ�� �����ϴ� �� ����� ���� ���� ��, �� ������ŭ ��Ʈ���� �˻�
    iMaxCount = gs_stDynamicMemory.iBlockCountOfSmallestBlock >> iBlockListIndex;
    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    for( i = 0 ; i < iMaxCount ; )
    {
        // QWORD�� 8 * 8��Ʈ => 64��Ʈ�̹Ƿ�, 64��Ʈ�� �Ѳ����� �˻��ؼ� 1�� ��Ʈ��
        // �ִ� �� Ȯ����
        if( ( ( iMaxCount - i ) / 64 ) > 0 )
        {
            pqwBitmap = ( QWORD* ) &( pbBitmap[ i / 8 ] );
            // ���� 8����Ʈ�� ��� 0�̸� 8����Ʈ ��� ����
            if( *pqwBitmap == 0 )
            {
                i += 64;
                continue;
            }
        }                
        
        if( ( pbBitmap[ i / 8 ] & ( DYNAMICMEMORY_EXIST << ( i % 8 ) ) ) != 0 )
        {
            return i;
        }
        i++;
    }
    return -1;
}

/**
 *  ��Ʈ�ʿ� �÷��׸� ����
 */
static void kSetFlagInBitmap( int iBlockListIndex, int iOffset, BYTE bFlag )
{
    BYTE* pbBitmap;

    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    if( bFlag == DYNAMICMEMORY_EXIST )
    {
        // �ش� ��ġ�� �����Ͱ� ��� �ִٸ� ���� ����
        if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) == 0 )
        {
            gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount++;
        }
        pbBitmap[ iOffset / 8 ] |= ( 0x01 << ( iOffset % 8 ) );
    }
    else 
    {
        // �ش� ��ġ�� �����Ͱ� �����Ѵٸ� ���� ����
        if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) != 0 )
        {
            gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].qwExistBitCount--;
        }
        pbBitmap[ iOffset / 8 ] &= ~( 0x01 << ( iOffset % 8 ) );
    }
}

/**
 *  �Ҵ� ���� �޸𸮸� ����
 */
BOOL kFreeMemory( void* pvAddress )
{
    QWORD qwRelativeAddress;
    int iSizeArrayOffset;
    QWORD qwBlockSize;
    int iBlockListIndex;
    int iBitmapOffset;

    if( pvAddress == NULL )
    {
        return FALSE;
    }

    // �Ѱ� ���� ��巹���� ��� Ǯ�� �������� �ϴ� ��巹���� ��ȯ�Ͽ� �Ҵ��ߴ�
    // ����� ũ�⸦ �˻�
    qwRelativeAddress = ( ( QWORD ) pvAddress ) - gs_stDynamicMemory.qwStartAddress;
    iSizeArrayOffset = qwRelativeAddress / DYNAMICMEMORY_MIN_SIZE;

    // �Ҵ�Ǿ����� ������ ��ȯ �� ��
    if( gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] == 0xFF )
    {
        return FALSE;
    }

    // �Ҵ�� ����� ���� ��� ����Ʈ�� �ε����� ����� ���� �ʱ�ȭ�ϰ�, �Ҵ�� 
    // ����� ���Ե� ��� ����Ʈ�� �˻�
    iBlockListIndex = ( int ) gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ];
    gs_stDynamicMemory.pbAllocatedBlockListIndex[ iSizeArrayOffset ] = 0xFF;
    // ���� ����� �ּ� ũ�⸦ ��� ����Ʈ �ε����� ����Ʈ�Ͽ� �Ҵ�� ����� ũ�� ���
    qwBlockSize = DYNAMICMEMORY_MIN_SIZE << iBlockListIndex;

    // ��� ����Ʈ ���� ��� �������� ���ؼ� ��� ����
    iBitmapOffset = qwRelativeAddress / qwBlockSize;
    if( kFreeBuddyBlock( iBlockListIndex, iBitmapOffset ) == TRUE )
    {
        gs_stDynamicMemory.qwUsedSize -= qwBlockSize;
        return TRUE;
    }
    
    return FALSE;
}

/**
 *  ��� ����Ʈ�� ���� ����� ����
 */
static BOOL kFreeBuddyBlock( int iBlockListIndex, int iBlockOffset )
{
    int iBuddyBlockOffset;
    int i;
    BOOL bFlag;
    BOOL bPreviousInterruptFlag;

    // ����ȭ ó��
    bPreviousInterruptFlag = kLockForSystemData();
    
    // ��� ����Ʈ�� ������ ������ ����� �˻��Ͽ� ������ �� ���� ������ �ݺ�
    for( i = iBlockListIndex ; i < gs_stDynamicMemory.iMaxLevelCount ; i++ )
    {
        // ���� ����� �����ϴ� ���·� ����
        kSetFlagInBitmap( i, iBlockOffset, DYNAMICMEMORY_EXIST );
        
        // ����� �������� ¦��(����)�̸� Ȧ��(������)�� �˻��ϰ�, Ȧ���̸� ¦����
        // ��Ʈ���� �˻��Ͽ� ������ ����� �����Ѵٸ� ����
        if( ( iBlockOffset % 2 ) == 0 )
        {
            iBuddyBlockOffset = iBlockOffset + 1;
        }
        else
        {
            iBuddyBlockOffset = iBlockOffset - 1;
        }
        bFlag = kGetFlagInBitmap( i, iBuddyBlockOffset );

        // ����� ��������� ����
        if( bFlag == DYNAMICMEMORY_EMPTY )
        {
            break;
        }
        
        // ������� �Դٸ� ������ ����� �����ϹǷ�, ����� ����
        // ����� ��� �� ������ ����� ���� ������� �̵�
        kSetFlagInBitmap( i, iBuddyBlockOffset, DYNAMICMEMORY_EMPTY );
        kSetFlagInBitmap( i, iBlockOffset, DYNAMICMEMORY_EMPTY );
        
        // ���� ��� ����Ʈ�� ��� ���������� �����ϰ�, ���� ������ ���� ��Ͽ���
        // �ٽ� �ݺ�
        iBlockOffset = iBlockOffset/ 2;
    }
    
    kUnlockForSystemData( bPreviousInterruptFlag );
    return TRUE;
}

/**
 *  ��� ����Ʈ�� �ش� ��ġ�� ��Ʈ���� ��ȯ
*/
static BYTE kGetFlagInBitmap( int iBlockListIndex, int iOffset )
{
    BYTE* pbBitmap;
    
    pbBitmap = gs_stDynamicMemory.pstBitmapOfLevel[ iBlockListIndex ].pbBitmap;
    if( ( pbBitmap[ iOffset / 8 ] & ( 0x01 << ( iOffset % 8 ) ) ) != 0x00 )
    {
        return DYNAMICMEMORY_EXIST;
    }
    
    return DYNAMICMEMORY_EMPTY;
}

/**
 *  ���� �޸� ������ ���� ������ ��ȯ
 */
void kGetDynamicMemoryInformation( QWORD* pqwDynamicMemoryStartAddress, 
        QWORD* pqwDynamicMemoryTotalSize, QWORD* pqwMetaDataSize, 
        QWORD* pqwUsedMemorySize )
{
    *pqwDynamicMemoryStartAddress = DYNAMICMEMORY_START_ADDRESS;
    *pqwDynamicMemoryTotalSize = kCalculateDynamicMemorySize();    
    *pqwMetaDataSize = kCalculateMetaBlockCount( *pqwDynamicMemoryTotalSize ) * 
        DYNAMICMEMORY_MIN_SIZE;
    *pqwUsedMemorySize = gs_stDynamicMemory.qwUsedSize;
}

/**
 *  ���� �޸� ������ �����ϴ� �ڷᱸ���� ��ȯ
 */
DYNAMICMEMORY* kGetDynamicMemoryManager( void )
{
    return &gs_stDynamicMemory;
}
