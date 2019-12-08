/**
 *  file    FileSystem.c
 *  date    2009/05/01
 *  author  kkamagui
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief  ���� �ý��ۿ� ���õ� ���� ����
 */

#include "FileSystem.h"
#include "HardDisk.h"
#include "DynamicMemory.h"
#include "Task.h"
#include "Utility.h"
#include "CacheManager.h"
#include "RAMDisk.h"


// ���� �ý��� �ڷᱸ��
static FILESYSTEMMANAGER   gs_stFileSystemManager;
// ���� �ý��� �ӽ� ����
static BYTE gs_vbTempBuffer[ FILESYSTEM_SECTORSPERCLUSTER * 512 ];

// �ϵ� ����ũ ��� ���õ� �Լ� ������ ����
fReadHDDInformation gs_pfReadHDDInformation = NULL;
fReadHDDSector gs_pfReadHDDSector = NULL;
fWriteHDDSector gs_pfWriteHDDSector = NULL;

/**
 *  ���� �ý����� �ʱ�ȭ
 */
BOOL kInitializeFileSystem( void )
{
    BOOL bCacheEnable = FALSE;

    // �ڷᱸ�� �ʱ�ȭ�� ����ȭ ��ü �ʱ�ȭ
    kMemSet( &gs_stFileSystemManager, 0, sizeof( gs_stFileSystemManager ) );
    kInitializeMutex( &( gs_stFileSystemManager.stMutex ) );

    // �ϵ� ����ũ�� �ʱ�ȭ
    if( kInitializeHDD() == TRUE )
    {
        // �ʱ�ȭ�� �����ϸ� �Լ� �����͸� �ϵ� ����ũ�� �Լ��� ����
        gs_pfReadHDDInformation = kReadHDDInformation;
        gs_pfReadHDDSector = kReadHDDSector;
        gs_pfWriteHDDSector = kWriteHDDSector;

        // ĳ�ø� Ȱ��ȭ��
        bCacheEnable = TRUE;
    }
    // �ϵ� ����ũ �ʱ�ȭ�� �����ϸ� 8Mbyte ũ���� �� ����ũ�� ����
    else if( kInitializeRDD( RDD_TOTALSECTORCOUNT ) == TRUE )
    {
        // �ʱ�ȭ�� �����ϸ� �Լ� �����͸� �� ����ũ�� �Լ��� ����
        gs_pfReadHDDInformation = kReadRDDInformation;
        gs_pfReadHDDSector = kReadRDDSector;
        gs_pfWriteHDDSector = kWriteRDDSector;

        // �� ����ũ�� �����Ͱ� �������� �����Ƿ� �Ź� ���� �ý����� ������
        if( kFormat() == FALSE )
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    // ���� �ý��� ����
    if( kMount() == FALSE )
    {
        return FALSE;
    }

    // �ڵ��� ���� ������ �Ҵ�
    gs_stFileSystemManager.pstHandlePool = ( FILE* ) kAllocateMemory(
        FILESYSTEM_HANDLE_MAXCOUNT * sizeof( FILE ) );

    // �޸��� �Ҵ��� �����ϸ� �ϵ� ����ũ�� �νĵ��� ���� ������ ����
    if( gs_stFileSystemManager.pstHandlePool == NULL )
    {
        gs_stFileSystemManager.bMounted = FALSE;
        return FALSE;
    }

    // �ڵ� Ǯ�� ���� 0���� �����Ͽ� �ʱ�ȭ
    kMemSet( gs_stFileSystemManager.pstHandlePool, 0,
            FILESYSTEM_HANDLE_MAXCOUNT * sizeof( FILE ) );

    // ĳ�� Ȱ��ȭ
    if( bCacheEnable == TRUE )
    {
        gs_stFileSystemManager.bCacheEnable = kInitializeCacheManager();
    }
    return TRUE;
}

//==============================================================================
//  ������ �Լ�(Low Level Function)
//==============================================================================
/**
 *  �ϵ� ����ũ�� MBR�� �о MINT ���� �ý������� Ȯ��
 *      MINT ���� �ý����̶��� ���� �ý��ۿ� ���õ� ���� ������ �о
 *      �ڷᱸ���� ����
 */
BOOL kMount( void )
{
    MBR* pstMBR;

    // ����ȭ ó��
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // MBR�� ����
    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }

    // �ñ׳�ó�� Ȯ���Ͽ� ���ٸ� �ڷᱸ���� �� ������ ���� ���� ����
    pstMBR = ( MBR* ) gs_vbTempBuffer;
    if( pstMBR->dwSignature != FILESYSTEM_SIGNATURE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }

    // ���� �ý��� �ν� ����
    gs_stFileSystemManager.bMounted = TRUE;

    // �� ������ ���� LBA ���巹���� ���� ���� ����
    gs_stFileSystemManager.dwReservedSectorCount = pstMBR->dwReservedSectorCount;
    gs_stFileSystemManager.dwClusterLinkAreaStartAddress =
        pstMBR->dwReservedSectorCount + 1;
    gs_stFileSystemManager.dwClusterLinkAreaSize = pstMBR->dwClusterLinkSectorCount;
    gs_stFileSystemManager.dwDataAreaStartAddress =
        pstMBR->dwReservedSectorCount + pstMBR->dwClusterLinkSectorCount + 1;
    gs_stFileSystemManager.dwTotalClusterCount = pstMBR->dwTotalClusterCount;

    // ����ȭ ó��
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}

/**
 *  �ϵ� ����ũ�� ���� �ý����� ����
 */
BOOL kFormat( void )
{
    HDDINFORMATION* pstHDD;
    MBR* pstMBR;
    DWORD dwTotalSectorCount, dwRemainSectorCount;
    DWORD dwMaxClusterCount, dwClsuterCount;
    DWORD dwClusterLinkSectorCount;
    DWORD i;

    // ����ȭ ó��
    kLock( &( gs_stFileSystemManager.stMutex ) );

    //==========================================================================
    //  �ϵ� ����ũ ������ �о ��Ÿ ������ ũ���� Ŭ�������� ������ ����
    //==========================================================================
    // �ϵ� ����ũ�� ������ ��� �ϵ� ����ũ�� �� ���� ���� ����
    pstHDD = ( HDDINFORMATION* ) gs_vbTempBuffer;
    if( gs_pfReadHDDInformation( TRUE, TRUE, pstHDD ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }
    dwTotalSectorCount = pstHDD->dwTotalSectors;

    // ��ü ���� ���� 4Kbyte, �� Ŭ������ ũ���� ������ �ִ� Ŭ������ ���� ����
    dwMaxClusterCount = dwTotalSectorCount / FILESYSTEM_SECTORSPERCLUSTER;

    // �ִ� Ŭ�������� ���� ���߾� Ŭ������ ��ũ ���̺��� ���� ���� ����
    // ��ũ �����ʹ� 4����Ʈ�̹Ƿ�, �� ���Ϳ��� 128���� ���. ������ �� ������
    // 128�� ���� �� �ø��Ͽ� Ŭ������ ��ũ�� ���� ���� ����
    dwClusterLinkSectorCount = ( dwMaxClusterCount + 127 ) / 128;

    // ������ ������ ���� �������� �����Ƿ�, ����ũ ��ü �������� MBR ������ Ŭ������
    // ��ũ ���̺� ������ ũ�⸦ �� �������� ���� ������ ������ ��
    // �ش� ������ Ŭ������ ũ���� ������ ���� Ŭ�������� ������ ����
    dwRemainSectorCount = dwTotalSectorCount - dwClusterLinkSectorCount - 1;
    dwClsuterCount = dwRemainSectorCount / FILESYSTEM_SECTORSPERCLUSTER;

    // ���� ���� ������ Ŭ������ ���� ���߾� �ٽ� �ѹ� ����
    dwClusterLinkSectorCount = ( dwClsuterCount + 127 ) / 128;

    //==========================================================================
    // ������ ������ MBR�� ���� ����, ��Ʈ �����͸� �������� ���� 0���� �ʱ�ȭ�Ͽ�
    // ���� �ý����� ����
    //==========================================================================
    // MBR ���� �б�
    if( gs_pfReadHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }

    // ��Ƽ�� ������ ���� �ý��� ���� ����
    pstMBR = ( MBR* ) gs_vbTempBuffer;
    kMemSet( pstMBR->vstPartition, 0, sizeof( pstMBR->vstPartition ) );
    pstMBR->dwSignature = FILESYSTEM_SIGNATURE;
    pstMBR->dwReservedSectorCount = 0;
    pstMBR->dwClusterLinkSectorCount = dwClusterLinkSectorCount;
    pstMBR->dwTotalClusterCount = dwClsuterCount;

    // MBR ������ 1 ���͸� ��
    if( gs_pfWriteHDDSector( TRUE, TRUE, 0, 1, gs_vbTempBuffer ) == FALSE )
    {
        // ����ȭ ó��
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return FALSE;
    }

    // MBR ���ĺ��� ��Ʈ �����͸����� ���� 0���� �ʱ�ȭ
    kMemSet( gs_vbTempBuffer, 0, 512 );
    for( i = 0 ; i < ( dwClusterLinkSectorCount + FILESYSTEM_SECTORSPERCLUSTER );
         i++ )
    {
        // ��Ʈ �����͸�(Ŭ������ 0)�� �̹� ���� �ý����� �����ϰ� �����Ƿ�,
        // �Ҵ��� ������ ǥ��
        if( i == 0 )
        {
            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_LASTCLUSTER;
        }
        else
        {
            ( ( DWORD* ) ( gs_vbTempBuffer ) )[ 0 ] = FILESYSTEM_FREECLUSTER;
        }

        // 1 ���;� ��
        if( gs_pfWriteHDDSector( TRUE, TRUE, i + 1, 1, gs_vbTempBuffer ) == FALSE )
        {
            // ����ȭ ó��
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return FALSE;
        }
    }

    // ĳ�� ���۸� ���� ����
    if( gs_stFileSystemManager.bCacheEnable == TRUE )
    {
        kDiscardAllCacheBuffer( CACHE_CLUSTERLINKTABLEAREA );
        kDiscardAllCacheBuffer( CACHE_DATAAREA );
    }

    // ����ȭ ó��
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}

/**
 *  ���� �ý��ۿ� ������ �ϵ� ����ũ�� ������ ��ȯ
 */
BOOL kGetHDDInformation( HDDINFORMATION* pstInformation)
{
    BOOL bResult;

    // ����ȭ ó��
    kLock( &( gs_stFileSystemManager.stMutex ) );

    bResult = gs_pfReadHDDInformation( TRUE, TRUE, pstInformation );

    // ����ȭ ó��
    kUnlock( &( gs_stFileSystemManager.stMutex ) );

    return bResult;
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿��� �� ���͸� ����
 */
static BOOL kReadClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    // ĳ�� ���ο� ���� �ٸ� �б� �Լ� ȣ��
    if( gs_stFileSystemManager.bCacheEnable == FALSE )
    {
        kInternalReadClusterLinkTableWithoutCache( dwOffset, pbBuffer );
    }
    else
    {
        kInternalReadClusterLinkTableWithCache( dwOffset, pbBuffer );
    }
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿��� �� ���͸� ����
 *      ���������� �����ϴ� �Լ�, ĳ�� ���� ����
 */
static BOOL kInternalReadClusterLinkTableWithoutCache( DWORD dwOffset,
        BYTE* pbBuffer )
{
    // Ŭ������ ��ũ ���̺� ������ ���� ���巹���� ����
    return gs_pfReadHDDSector( TRUE, TRUE, dwOffset +
              gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿��� �� ���͸� ����
 *      ���������� �����ϴ� �Լ�, ĳ�� ����
 */
static BOOL kInternalReadClusterLinkTableWithCache( DWORD dwOffset,
        BYTE* pbBuffer )
{
    CACHEBUFFER* pstCacheBuffer;

    // ���� ĳ�ÿ� �ش� Ŭ������ ��ũ ���̺��� �ִ��� Ȯ��
    pstCacheBuffer = kFindCacheBuffer( CACHE_CLUSTERLINKTABLEAREA, dwOffset );

    // ĳ�� ���ۿ� �ִٸ� ĳ���� ������ ����
    if( pstCacheBuffer != NULL )
    {
        kMemCpy( pbBuffer, pstCacheBuffer->pbBuffer, 512 );
        return TRUE;
    }

    // ĳ�� ���ۿ� ���ٸ� �ϵ� ����ũ���� ���� ����
    if( kInternalReadClusterLinkTableWithoutCache( dwOffset, pbBuffer ) == FALSE )
    {
        return FALSE;
    }

    // ĳ�ø� �Ҵ� �޾Ƽ� ĳ�� ������ ����
    pstCacheBuffer = kAllocateCacheBufferWithFlush( CACHE_CLUSTERLINKTABLEAREA );
    if( pstCacheBuffer == NULL )
    {
        return FALSE;
    }

    // ĳ�� ���ۿ� ���� ������ ������ ��, �±� ������ ����
    kMemCpy( pstCacheBuffer->pbBuffer, pbBuffer, 512 );
    pstCacheBuffer->dwTag = dwOffset;

    // �б⸦ ���������Ƿ� ������ ������ �������� ���� ������ ǥ��
    pstCacheBuffer->bChanged = FALSE;
    return TRUE;
}

/**
 *  Ŭ������ ��ũ ���̺� ������ ĳ�� ���� �Ǵ� ������ ������ ĳ�� ���ۿ��� �Ҵ�
 *      �� ĳ�� ���۰� ���� ���� ������ �� �߿� �ϳ��� ������ ���� �� ����
 */
static CACHEBUFFER* kAllocateCacheBufferWithFlush( int iCacheTableIndex )
{
    CACHEBUFFER* pstCacheBuffer;

    // ĳ�� ���ۿ� ���ٸ� ĳ�ø� �Ҵ� �޾Ƽ� ĳ�� ������ ����
    pstCacheBuffer = kAllocateCacheBuffer( iCacheTableIndex );
    // ĳ�ø� �Ҵ� ���� �� ���ٸ� ĳ�� ���ۿ��� ������ ���� ã�� ���� �� ����
    if( pstCacheBuffer == NULL )
    {
        pstCacheBuffer = kGetVictimInCacheBuffer( iCacheTableIndex );
        // ������ ĳ�� ���۵� �Ҵ� ���� �� ������ ����
        if( pstCacheBuffer == NULL )
        {
            kPrintf( "Cache Allocate Fail~!!!!\n" );
            return NULL;
        }

        // ĳ�� ������ �����Ͱ� �����Ǿ��ٸ� �ϵ� ����ũ�� �Űܾ� ��
        if( pstCacheBuffer->bChanged == TRUE )
        {
            switch( iCacheTableIndex )
            {
                // Ŭ������ ��ũ ���̺� ������ ĳ���� ����
            case CACHE_CLUSTERLINKTABLEAREA:
                // ���Ⱑ �����Ѵٸ� ����
                if( kInternalWriteClusterLinkTableWithoutCache(
                    pstCacheBuffer->dwTag, pstCacheBuffer->pbBuffer ) == FALSE )
                {
                    kPrintf( "Cache Buffer Write Fail~!!!!\n" );
                    return NULL;
                }
                break;

                // ������ ������ ĳ���� ����
            case CACHE_DATAAREA:
                // ���Ⱑ �����Ѵٸ� ����
                if( kInternalWriteClusterWithoutCache(
                    pstCacheBuffer->dwTag, pstCacheBuffer->pbBuffer ) == FALSE )
                {
                    kPrintf( "Cache Buffer Write Fail~!!!!\n" );
                    return NULL;
                }
                break;

                // ��Ÿ�� ����
            default:
                kPrintf( "kAllocateCacheBufferWithFlush Fail\n" );
                return NULL;
                break;
            }
        }
    }
    return pstCacheBuffer;
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿� �� ���͸� ��
 */
static BOOL kWriteClusterLinkTable( DWORD dwOffset, BYTE* pbBuffer )
{
    // ĳ�� ���ο� ���� �ٸ� ���� �Լ� ȣ��
    if( gs_stFileSystemManager.bCacheEnable == FALSE )
    {
        return kInternalWriteClusterLinkTableWithoutCache( dwOffset, pbBuffer );
    }
    else
    {
        return kInternalWriteClusterLinkTableWithCache( dwOffset, pbBuffer );
    }
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿� �� ���͸� ��
 *      ���������� �����ϴ� �Լ�, ĳ�� ���� �� ��
 */
static BOOL kInternalWriteClusterLinkTableWithoutCache( DWORD dwOffset,
        BYTE* pbBuffer )
{
    // Ŭ������ ��ũ ���̺� ������ ���� ���巹���� ����
    return gs_pfWriteHDDSector( TRUE, TRUE, dwOffset +
               gs_stFileSystemManager.dwClusterLinkAreaStartAddress, 1, pbBuffer );
}

/**
 *  Ŭ������ ��ũ ���̺� ���� �����¿� �� ���͸� ��
 *      ���������� �����ϴ� �Լ�, ĳ�� ����
 */
static BOOL kInternalWriteClusterLinkTableWithCache( DWORD dwOffset,
        BYTE* pbBuffer )
{
    CACHEBUFFER* pstCacheBuffer;

    // ĳ�ÿ� �ش� Ŭ������ ��ũ ���̺��� �ִ��� Ȯ��
    pstCacheBuffer = kFindCacheBuffer( CACHE_CLUSTERLINKTABLEAREA, dwOffset );

    // ĳ�� ���ۿ� �ִٸ� ĳ�ÿ� ��
    if( pstCacheBuffer != NULL )
    {
        kMemCpy( pstCacheBuffer->pbBuffer, pbBuffer, 512 );

        // ���⸦ ���������Ƿ� ������ ������ ������ ������ ǥ��
        pstCacheBuffer->bChanged = TRUE;
        return TRUE;
    }

    // ĳ�� ���ۿ� ���ٸ� ĳ�� ���۸� �Ҵ� �޾Ƽ� ĳ�� ������ ����
    pstCacheBuffer = kAllocateCacheBufferWithFlush( CACHE_CLUSTERLINKTABLEAREA );
    if( pstCacheBuffer == NULL )
    {
        return FALSE;
    }

    // ĳ�� ���ۿ� ����, �±� ������ ����
    kMemCpy( pstCacheBuffer->pbBuffer, pbBuffer, 512 );
    pstCacheBuffer->dwTag = dwOffset;

    // ���⸦ ���������Ƿ� ������ ������ ������ ������ ǥ��
    pstCacheBuffer->bChanged = TRUE;

    return TRUE;
}

/**
 *  ������ ������ �����¿��� �� Ŭ�����͸� ����
 */
static BOOL kReadCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    // ĳ�� ���ο� ���� �ٸ� �б� �Լ� ȣ��
    if( gs_stFileSystemManager.bCacheEnable == FALSE )
    {
        kInternalReadClusterWithoutCache( dwOffset, pbBuffer );
    }
    else
    {
        kInternalReadClusterWithCache( dwOffset, pbBuffer );
    }
}

/**
 *  ������ ������ �����¿��� �� Ŭ�����͸� ����
 *      ���������� �����ϴ� �Լ�, ĳ�� ���� �� ��
 */
static BOOL kInternalReadClusterWithoutCache( DWORD dwOffset, BYTE* pbBuffer )
{
    // ������ ������ ���� ���巹���� ����
    return gs_pfReadHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) +
              gs_stFileSystemManager.dwDataAreaStartAddress,
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

/**
 *  ������ ������ �����¿��� �� Ŭ�����͸� ����
 *      ���������� �����ϴ� �Լ�, ĳ�� ����
 */
static BOOL kInternalReadClusterWithCache( DWORD dwOffset, BYTE* pbBuffer )
{
    CACHEBUFFER* pstCacheBuffer;

    // ĳ�ÿ� �ش� ������ Ŭ�����Ͱ� �ִ��� Ȯ��
    pstCacheBuffer = kFindCacheBuffer( CACHE_DATAAREA, dwOffset );

    // ĳ�� ���ۿ� �ִٸ� ĳ���� ������ ����
    if( pstCacheBuffer != NULL )
    {
        kMemCpy( pbBuffer, pstCacheBuffer->pbBuffer, FILESYSTEM_CLUSTERSIZE );
        return TRUE;
    }

    // ĳ�� ���ۿ� ���ٸ� �ϵ� ����ũ���� ���� ����
    if( kInternalReadClusterWithoutCache( dwOffset, pbBuffer ) == FALSE )
    {
        return FALSE;
    }

    // ĳ�� ���۸� �Ҵ� �޾Ƽ� ĳ�� ������ ����
    pstCacheBuffer = kAllocateCacheBufferWithFlush( CACHE_DATAAREA );
    if( pstCacheBuffer == NULL )
    {
        return FALSE;
    }

    // ĳ�� ���ۿ� ���� ������ ������ ��, �±� ������ ����
    kMemCpy( pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE );
    pstCacheBuffer->dwTag = dwOffset;

    // �б⸦ ���������Ƿ� ������ ������ �������� ���� ������ ǥ��
    pstCacheBuffer->bChanged = FALSE;
    return TRUE;
}

/**
 *  ������ ������ �����¿� �� Ŭ�����͸� ��
 */
static BOOL kWriteCluster( DWORD dwOffset, BYTE* pbBuffer )
{
    // ĳ�� ���ο� ���� �ٸ� ���� �Լ� ȣ��
    if( gs_stFileSystemManager.bCacheEnable == FALSE )
    {
        kInternalWriteClusterWithoutCache( dwOffset, pbBuffer );
    }
    else
    {
        kInternalWriteClusterWithCache( dwOffset, pbBuffer );
    }
}

/**
 *  ������ ������ �����¿� �� Ŭ�����͸� ��
 *      ���������� �����ϴ� �Լ�, ĳ�� ���� �� ��
 */
static BOOL kInternalWriteClusterWithoutCache( DWORD dwOffset, BYTE* pbBuffer )
{
    // ������ ������ ���� ���巹���� ����
    return  gs_pfWriteHDDSector( TRUE, TRUE, ( dwOffset * FILESYSTEM_SECTORSPERCLUSTER ) +
              gs_stFileSystemManager.dwDataAreaStartAddress,
              FILESYSTEM_SECTORSPERCLUSTER, pbBuffer );
}

/**
 *  ������ ������ �����¿� �� Ŭ�����͸� ��
 *      ���������� �����ϴ� �Լ�, ĳ�� ����
 */
static BOOL kInternalWriteClusterWithCache( DWORD dwOffset, BYTE* pbBuffer )
{
    CACHEBUFFER* pstCacheBuffer;

    // ĳ�� ���ۿ� �ش� ������ Ŭ�����Ͱ� �ִ��� Ȯ��
    pstCacheBuffer = kFindCacheBuffer( CACHE_DATAAREA, dwOffset );

    // ĳ�� ���ۿ� �ִٸ� ĳ�ÿ� ��
    if( pstCacheBuffer != NULL )
    {
        kMemCpy( pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE );

        // ���⸦ ���������Ƿ� ������ ������ ������ ������ ǥ��
        pstCacheBuffer->bChanged = TRUE;

        return TRUE;
    }

    // ĳ�� ���ۿ� ���ٸ� ĳ�ø� �Ҵ� �޾Ƽ� ĳ�� ������ ����
    pstCacheBuffer = kAllocateCacheBufferWithFlush( CACHE_DATAAREA );
    if( pstCacheBuffer == NULL )
    {
        return FALSE;
    }

    // ĳ�� ���ۿ� ����, �±� ������ ����
    kMemCpy( pstCacheBuffer->pbBuffer, pbBuffer, FILESYSTEM_CLUSTERSIZE );
    pstCacheBuffer->dwTag = dwOffset;

    // ���⸦ ���������Ƿ� ������ ������ ������ ������ ǥ��
    pstCacheBuffer->bChanged = TRUE;

    return TRUE;
}

/**
 *  Ŭ������ ��ũ ���̺� �������� �� Ŭ�����͸� �˻���
 */
static DWORD kFindFreeCluster( void )
{
    DWORD dwLinkCountInSector;
    DWORD dwLastSectorOffset, dwCurrentSectorOffset;
    DWORD i, j;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FILESYSTEM_LASTCLUSTER;
    }

    // ���������� Ŭ�����͸� �Ҵ��� Ŭ������ ��ũ ���̺��� ���� �������� ������
    dwLastSectorOffset = gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset;

    // ���������� �Ҵ��� ��ġ���� ������ ���鼭 �� Ŭ�����͸� �˻�
    for( i = 0 ; i < gs_stFileSystemManager.dwClusterLinkAreaSize ; i++ )
    {
        // Ŭ������ ��ũ ���̺��� ������ �����̸� ��ü ���͸�ŭ ���� ���� �ƴ϶�
        // ���� Ŭ�������� ����ŭ ������ ���ƾ� ��
        if( ( dwLastSectorOffset + i ) ==
            ( gs_stFileSystemManager.dwClusterLinkAreaSize - 1 ) )
        {
            dwLinkCountInSector = gs_stFileSystemManager.dwTotalClusterCount % 128;
        }
        else
        {
            dwLinkCountInSector = 128;
        }

        // �̹��� �о��� �� Ŭ������ ��ũ ���̺��� ���� �������� ���ؼ� ����
        dwCurrentSectorOffset = ( dwLastSectorOffset + i ) %
            gs_stFileSystemManager.dwClusterLinkAreaSize;
        if( kReadClusterLinkTable( dwCurrentSectorOffset, gs_vbTempBuffer ) == FALSE )
        {
            return FILESYSTEM_LASTCLUSTER;
        }

        // ���� ������ ������ ���鼭 �� Ŭ�����͸� �˻�
        for( j = 0 ; j < dwLinkCountInSector ; j++ )
        {
            if( ( ( DWORD* ) gs_vbTempBuffer )[ j ] == FILESYSTEM_FREECLUSTER )
            {
                break;
            }
        }

        // ã�Ҵٸ� Ŭ������ �ε����� ��ȯ
        if( j != dwLinkCountInSector )
        {
            // ���������� Ŭ�����͸� �Ҵ��� Ŭ������ ��ũ ���� ���� �������� ����
            gs_stFileSystemManager.dwLastAllocatedClusterLinkSectorOffset =
                dwCurrentSectorOffset;

            // ���� Ŭ������ ��ũ ���̺��� �������� �����Ͽ� Ŭ������ �ε����� ����
            return ( dwCurrentSectorOffset * 128 ) + j;
        }
    }

    return FILESYSTEM_LASTCLUSTER;
}

/**
 *  Ŭ������ ��ũ ���̺��� ���� ����
 */
static BOOL kSetClusterLinkData( DWORD dwClusterIndex, DWORD dwData )
{
    DWORD dwSectorOffset;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FALSE;
    }

    // �� ���Ϳ� 128���� Ŭ������ ��ũ�� ����Ƿ� 128�� ������ ���� ��������
    // ���� �� ����
    dwSectorOffset = dwClusterIndex / 128;

    // �ش� ���͸� �о ��ũ ������ ������ ��, �ٽ� ����
    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ] = dwData;

    if( kWriteClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    return TRUE;
}

/**
 *  Ŭ������ ��ũ ���̺��� ���� ��ȯ
 */
static BOOL kGetClusterLinkData( DWORD dwClusterIndex, DWORD* pdwData )
{
    DWORD dwSectorOffset;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return FALSE;
    }

    // �� ���Ϳ� 128���� Ŭ������ ��ũ�� ����Ƿ� 128�� ������ ���� ��������
    // ���� �� ����
    dwSectorOffset = dwClusterIndex / 128;

    if( dwSectorOffset > gs_stFileSystemManager.dwClusterLinkAreaSize )
    {
        return FALSE;
    }


    // �ش� ���͸� �о ��ũ ������ ��ȯ
    if( kReadClusterLinkTable( dwSectorOffset, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    *pdwData = ( ( DWORD* ) gs_vbTempBuffer )[ dwClusterIndex % 128 ];
    return TRUE;
}


/**
 *  ��Ʈ �����͸����� �� �����͸� ��Ʈ���� ��ȯ
 */
static int kFindFreeDirectoryEntry( void )
{
    DIRECTORYENTRY* pstEntry;
    int i;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return -1;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return -1;
    }

    // ��Ʈ �����͸� �ȿ��� ������ ���鼭 �� ��Ʈ��, �� ���� Ŭ������ ��ȣ�� 0��
    // ��Ʈ���� �˻�
    pstEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    {
        if( pstEntry[ i ].dwStartClusterIndex == 0 )
        {
            return i;
        }
    }
    return -1;
}

/**
 *  ��Ʈ �����͸��� �ش� �ε����� �����͸� ��Ʈ���� ����
 */
static BOOL kSetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;

    // ���� �ý����� �ν����� ���߰ų� �ε����� �ùٸ��� ������ ����
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� �ִ� �ش� �����͸� ����
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstRootEntry + iIndex, pstEntry, sizeof( DIRECTORYENTRY ) );

    // ��Ʈ �����͸��� ��
    if( kWriteCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  ��Ʈ �����͸��� �ش� �ε����� ��ġ�ϴ� �����͸� ��Ʈ���� ��ȯ
 */
static BOOL kGetDirectoryEntryData( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;

    // ���� �ý����� �ν����� ���߰ų� �ε����� �ùٸ��� ������ ����
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� �ִ� �ش� �����͸� ����
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstEntry, pstRootEntry + iIndex, sizeof( DIRECTORYENTRY ) );
    return TRUE;
}

/**
 *  ��Ʈ �����͸����� ���� �̸��� ��ġ�ϴ� ��Ʈ���� ã�Ƽ� �ε����� ��ȯ
 */
static int kFindDirectoryEntry( const char* pcFileName, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;
    int i;
    int iLength;

    // ���� �ý����� �ν����� �������� ����
    if( gs_stFileSystemManager.bMounted == FALSE )
    {
        return -1;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return -1;
    }

    iLength = kStrLen( pcFileName );
    // ��Ʈ �����͸� �ȿ��� ������ ���鼭 ���� �̸��� ��ġ�ϴ� ��Ʈ���� ��ȯ
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    for( i = 0 ; i < FILESYSTEM_MAXDIRECTORYENTRYCOUNT ; i++ )
    {
        if( kMemCmp( pstRootEntry[ i ].vcFileName, pcFileName, iLength ) == 0 )
        {
            kMemCpy( pstEntry, pstRootEntry + i, sizeof( DIRECTORYENTRY ) );
            return i;
        }
    }
    return -1;
}

/**
 *  ���� �ý����� ������ ��ȯ
 */
void kGetFileSystemInformation( FILESYSTEMMANAGER* pstManager )
{
    kMemCpy( pstManager, &gs_stFileSystemManager, sizeof( gs_stFileSystemManager ) );
}

//==============================================================================
//  ������ �Լ�(High Level Function)
//==============================================================================
/**
 *  �����ִ� �ڵ��� �Ҵ�
 */
static void* kAllocateFileDirectoryHandle( void )
{
    int i;
    FILE* pstFile;

    // �ڵ� Ǯ(Handle Pool)�� ���� �˻��Ͽ� �����ִ� �ڵ��� ��ȯ
    pstFile = gs_stFileSystemManager.pstHandlePool;
    for( i = 0 ; i < FILESYSTEM_HANDLE_MAXCOUNT ; i++ )
    {
        // �����ִٸ� ��ȯ
        if( pstFile->bType == FILESYSTEM_TYPE_FREE )
        {
            pstFile->bType = FILESYSTEM_TYPE_FILE;
            return pstFile;
        }

        // �������� �̵�
        pstFile++;
    }

    return NULL;
}

/**
 *  ������ �ڵ��� ��ȯ
 */
static void kFreeFileDirectoryHandle( FILE* pstFile )
{
    // ��ü ������ �ʱ�ȭ
    kMemSet( pstFile, 0, sizeof( FILE ) );

    // �����ִ� Ÿ������ ����
    pstFile->bType = FILESYSTEM_TYPE_FREE;
}

/**
 *  ������ ����
 */
static BOOL kCreateFile( const char* pcFileName, DIRECTORYENTRY* pstEntry,
        int* piDirectoryEntryIndex )
{
    DWORD dwCluster;

    // �� Ŭ�����͸� ã�Ƽ� �Ҵ��� ������ ����
    dwCluster = kFindFreeCluster();
    if( ( dwCluster == FILESYSTEM_LASTCLUSTER ) ||
        ( kSetClusterLinkData( dwCluster, FILESYSTEM_LASTCLUSTER ) == FALSE ) )
    {
        return FALSE;
    }

    // �� �����͸� ��Ʈ���� �˻�
    *piDirectoryEntryIndex = kFindFreeDirectoryEntry();
    if( *piDirectoryEntryIndex == -1 )
    {
        // ������ ���� �Ҵ� ���� Ŭ�����͸� ��ȯ�ؾ� ��
        kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        return FALSE;
    }

    // �����͸� ��Ʈ���� ����
    //hw추가구현 파일 생성시에 권한, 소유자ID 추가
    //문제점 : userID를 받아와서 저장해주어야하는데 구현 못하였음.
    kMemCpy( pstEntry->vcFileName, pcFileName, kStrLen( pcFileName ) + 1 );
    pstEntry->dwStartClusterIndex = dwCluster;
    pstEntry->dwFileSize = 0;
    kMemCpy( pstEntry->vcAuthority, "rwr-", kStrLen( "rwr-" ) + 1 );
    kMemCpy( pstEntry->vcOwnerID, "root", kStrLen( "root" ) + 1 );

    // �����͸� ��Ʈ���� ����
    if( kSetDirectoryEntryData( *piDirectoryEntryIndex, pstEntry ) == FALSE )
    {
        // ������ ���� �Ҵ� ���� Ŭ�����͸� ��ȯ�ؾ� ��
        kSetClusterLinkData( dwCluster, FILESYSTEM_FREECLUSTER );
        return FALSE;
    }
    return TRUE;
}

/**
 *  �Ķ����ͷ� �Ѿ��� Ŭ�����ͺ��� ������ ������ ������ Ŭ�����͸� ���� ��ȯ
 */
static BOOL kFreeClusterUntilEnd( DWORD dwClusterIndex )
{
    DWORD dwCurrentClusterIndex;
    DWORD dwNextClusterIndex;

    // Ŭ������ �ε����� �ʱ�ȭ
    dwCurrentClusterIndex = dwClusterIndex;

    while( dwCurrentClusterIndex != FILESYSTEM_LASTCLUSTER )
    {
        // ���� Ŭ�������� �ε����� ������
        if( kGetClusterLinkData( dwCurrentClusterIndex, &dwNextClusterIndex )
                == FALSE )
        {
            return FALSE;
        }

        // ���� Ŭ�����͸� �� ������ �����Ͽ� ����
        if( kSetClusterLinkData( dwCurrentClusterIndex, FILESYSTEM_FREECLUSTER )
                == FALSE )
        {
            return FALSE;
        }

        // ���� Ŭ������ �ε����� ���� Ŭ�������� �ε����� �ٲ�
        dwCurrentClusterIndex = dwNextClusterIndex;
    }

    return TRUE;
}

/**
 *  ������ ���ų� ����
 */
FILE* kOpenFile( const char* pcFileName, const char* pcMode )
{
    DIRECTORYENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;
    DWORD dwSecondCluster;
    FILE* pstFile;

    // ���� �̸� �˻�
    iFileNameLength = kStrLen( pcFileName );
    if( ( iFileNameLength > ( sizeof( stEntry.vcFileName ) - 1 ) ) ||
        ( iFileNameLength == 0 ) )
    {
        return NULL;
    }

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    //==========================================================================
    // ������ ���� �����ϴ°� Ȯ���ϰ�, ���ٸ� �ɼ��� ���� ������ ����
    //==========================================================================
    iDirectoryEntryOffset = kFindDirectoryEntry( pcFileName, &stEntry );
    if( iDirectoryEntryOffset == -1 )
    {
        // ������ ���ٸ� �б�(r, r+) �ɼ��� ����
        if( pcMode[ 0 ] == 'r' )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return NULL;
        }

        // ������ �ɼǵ��� ������ ����
        if( kCreateFile( pcFileName, &stEntry, &iDirectoryEntryOffset ) == FALSE )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return NULL;
        }
    }
    //==========================================================================
    // ������ ������ ������ �ϴ� �ɼ��̸� ���Ͽ� ������ Ŭ�����͸� ���� �����ϰ�
    // ���� ũ�⸦ 0���� ����
    //==========================================================================
    else if( pcMode[ 0 ] == 'w' )
    {
        // ���� Ŭ�������� ���� Ŭ�����͸� ã��
        if( kGetClusterLinkData( stEntry.dwStartClusterIndex, &dwSecondCluster )
                == FALSE )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return NULL;
        }

        // ���� Ŭ�����͸� ������ Ŭ�����ͷ� ����
        if( kSetClusterLinkData( stEntry.dwStartClusterIndex,
               FILESYSTEM_LASTCLUSTER ) == FALSE )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return NULL;
        }

        // ���� Ŭ�����ͺ��� ������ Ŭ�����ͱ��� ���� ����
        if( kFreeClusterUntilEnd( dwSecondCluster ) == FALSE )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return NULL;
        }

        // ������ ������ ���� ���������Ƿ�, ũ�⸦ 0���� ����
        stEntry.dwFileSize = 0;
        if( kSetDirectoryEntryData( iDirectoryEntryOffset, &stEntry ) == FALSE )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return NULL;
        }
    }

    //==========================================================================
    // ���� �ڵ��� �Ҵ� �޾� �����͸� ������ �� ��ȯ
    //==========================================================================
    // ���� �ڵ��� �Ҵ� �޾� ������ ����
    pstFile = kAllocateFileDirectoryHandle();
    if( pstFile == NULL )
    {
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return NULL;
    }

    // ���� �ڵ鿡 ���� ������ ����
    pstFile->bType = FILESYSTEM_TYPE_FILE;
    pstFile->stFileHandle.iDirectoryEntryOffset = iDirectoryEntryOffset;
    pstFile->stFileHandle.dwFileSize = stEntry.dwFileSize;
    pstFile->stFileHandle.dwStartClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwPreviousClusterIndex = stEntry.dwStartClusterIndex;
    pstFile->stFileHandle.dwCurrentOffset = 0;

    // ���� �߰� �ɼ�(a)�� �����Ǿ� ������, ������ ������ �̵�
    if( pcMode[ 0 ] == 'a' )
    {
        kSeekFile( pstFile, 0, FILESYSTEM_SEEK_END );
    }

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return pstFile;
}

/**
 *  ������ �о� ���۷� ����
 */
DWORD kReadFile( void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    DWORD dwTotalCount;
    DWORD dwReadCount;
    DWORD dwOffsetInCluster;
    DWORD dwCopySize;
    FILEHANDLE* pstFileHandle;
    DWORD dwNextClusterIndex;

    // �ڵ��� ���� Ÿ���� �ƴϸ� ����
    if( ( pstFile == NULL ) ||
        ( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    {
        return 0;
    }
    pstFileHandle = &( pstFile->stFileHandle );

    // ������ ���̰ų� ������ Ŭ�������̸� ����
    if( ( pstFileHandle->dwCurrentOffset == pstFileHandle->dwFileSize ) ||
        ( pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER ) )
    {
        return 0;
    }

    // ���� ���� �����ؼ� ������ ���� �� �ִ� ���� ����
    dwTotalCount = MIN( dwSize * dwCount, pstFileHandle->dwFileSize -
                        pstFileHandle->dwCurrentOffset );

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // ������ ����ŭ �� ���� ������ �ݺ�
    dwReadCount = 0;
    while( dwReadCount != dwTotalCount )
    {
        //======================================================================
        // Ŭ�����͸� �о ���ۿ� ����
        //======================================================================
        // ���� Ŭ�����͸� ����
        if( kReadCluster( pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer )
                == FALSE )
        {
            break;
        }

        // Ŭ������ ������ ���� �����Ͱ� �����ϴ� �������� ����
        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;

        // ���� Ŭ�����Ϳ� ������ �ִٸ� ���� Ŭ�����Ϳ��� ���� ��ŭ �а� ����
        // Ŭ�����ͷ� �̵�
        dwCopySize = MIN( FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster,
                          dwTotalCount - dwReadCount );
        kMemCpy( ( char* ) pvBuffer + dwReadCount,
                gs_vbTempBuffer + dwOffsetInCluster, dwCopySize );

        // ���� ����Ʈ ���� ���� �������� ��ġ�� ����
        dwReadCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        //======================================================================
        // ���� Ŭ�����͸� ������ �� �о����� ���� Ŭ�����ͷ� �̵�
        //======================================================================
        if( ( pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE ) == 0 )
        {
            // ���� Ŭ�������� ��ũ �����͸� ã�� ���� Ŭ�����͸� ����
            if( kGetClusterLinkData( pstFileHandle->dwCurrentClusterIndex,
                                     &dwNextClusterIndex ) == FALSE )
            {
                break;
            }

            // Ŭ������ ������ ����
            pstFileHandle->dwPreviousClusterIndex =
                pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );

    // ���� ���ڵ� ���� ��ȯ
    return ( dwReadCount / dwSize );
}

/**
 *  ��Ʈ �����͸����� �����͸� ��Ʈ�� ���� ����
 */
static BOOL kUpdateDirectoryEntry( FILEHANDLE* pstFileHandle )
{
    DIRECTORYENTRY stEntry;

    // �����͸� ��Ʈ�� �˻�
    if( ( pstFileHandle == NULL ) ||
        ( kGetDirectoryEntryData( pstFileHandle->iDirectoryEntryOffset, &stEntry)
            == FALSE ) )
    {
        return FALSE;
    }

    // ���� ũ���� ���� Ŭ�����͸� ����
    stEntry.dwFileSize = pstFileHandle->dwFileSize;
    stEntry.dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;

    // ������ �����͸� ��Ʈ���� ����
    if( kSetDirectoryEntryData( pstFileHandle->iDirectoryEntryOffset, &stEntry )
            == FALSE )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  ������ �����͸� ���Ͽ� ��
 */
DWORD kWriteFile( const void* pvBuffer, DWORD dwSize, DWORD dwCount, FILE* pstFile )
{
    DWORD dwWriteCount;
    DWORD dwTotalCount;
    DWORD dwOffsetInCluster;
    DWORD dwCopySize;
    DWORD dwAllocatedClusterIndex;
    DWORD dwNextClusterIndex;
    FILEHANDLE* pstFileHandle;

    // �ڵ��� ���� Ÿ���� �ƴϸ� ����
    if( ( pstFile == NULL ) ||
        ( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    {
        return 0;
    }
    pstFileHandle = &( pstFile->stFileHandle );

    // �� ����Ʈ ��
    dwTotalCount = dwSize * dwCount;

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // �� �� ������ �ݺ�
    dwWriteCount = 0;
    while( dwWriteCount != dwTotalCount )
    {
        //======================================================================
        // ���� Ŭ�����Ͱ� ������ ���̸� Ŭ�����͸� �Ҵ��Ͽ� ����
        //======================================================================
        if( pstFileHandle->dwCurrentClusterIndex == FILESYSTEM_LASTCLUSTER )
        {
            // �� Ŭ������ �˻�
            dwAllocatedClusterIndex = kFindFreeCluster();
            if( dwAllocatedClusterIndex == FILESYSTEM_LASTCLUSTER )
            {
                break;
            }

            // �˻��� Ŭ�����͸� ������ Ŭ�����ͷ� ����
            if( kSetClusterLinkData( dwAllocatedClusterIndex, FILESYSTEM_LASTCLUSTER )
                    == FALSE )
            {
                break;
            }

            // ������ ������ Ŭ�����Ϳ� �Ҵ��� Ŭ�����͸� ����
            if( kSetClusterLinkData( pstFileHandle->dwPreviousClusterIndex,
                                     dwAllocatedClusterIndex ) == FALSE )
            {
                // ������ ���� �Ҵ��� Ŭ�����͸� ����
                kSetClusterLinkData( dwAllocatedClusterIndex, FILESYSTEM_FREECLUSTER );
                break;
            }

            // ���� Ŭ�����͸� �Ҵ��� ������ ����
            pstFileHandle->dwCurrentClusterIndex = dwAllocatedClusterIndex;

            // ���� �Ҵ��޾����� �ӽ� Ŭ������ ���۸� 0���� ä��
            kMemSet( gs_vbTempBuffer, 0, FILESYSTEM_LASTCLUSTER );
        }
        //======================================================================
        // �� Ŭ�����͸� ä���� ���ϸ� Ŭ�����͸� �о �ӽ� Ŭ������ ���۷� ����
        //======================================================================
        else if( ( ( pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE ) != 0 ) ||
                 ( ( dwTotalCount - dwWriteCount ) < FILESYSTEM_CLUSTERSIZE ) )
        {
            // ��ü Ŭ�����͸� ����� ���찡 �ƴϸ� �κи� �������� �ϹǷ�
            // ���� Ŭ�����͸� ����
            if( kReadCluster( pstFileHandle->dwCurrentClusterIndex,
                              gs_vbTempBuffer ) == FALSE )
            {
                break;
            }
        }

        // Ŭ������ ������ ���� �����Ͱ� �����ϴ� �������� ����
        dwOffsetInCluster = pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE;

        // ���� Ŭ�����Ϳ� ������ �ִٸ� ���� Ŭ�����Ϳ��� ���� ��ŭ ���� ����
        // Ŭ�����ͷ� �̵�
        dwCopySize = MIN( FILESYSTEM_CLUSTERSIZE - dwOffsetInCluster,
                          dwTotalCount - dwWriteCount );
        kMemCpy( gs_vbTempBuffer + dwOffsetInCluster, ( char* ) pvBuffer +
                 dwWriteCount, dwCopySize );

        // �ӽ� ���ۿ� ���Ե� ���� ����ũ�� ��
        if( kWriteCluster( pstFileHandle->dwCurrentClusterIndex, gs_vbTempBuffer )
                == FALSE )
        {
            break;
        }

        // �� ����Ʈ ���� ���� �������� ��ġ�� ����
        dwWriteCount += dwCopySize;
        pstFileHandle->dwCurrentOffset += dwCopySize;

        //======================================================================
        // ���� Ŭ�������� ������ �� ������ ���� Ŭ�����ͷ� �̵�
        //======================================================================
        if( ( pstFileHandle->dwCurrentOffset % FILESYSTEM_CLUSTERSIZE ) == 0 )
        {
            // ���� Ŭ�������� ��ũ �����ͷ� ���� Ŭ�����͸� ����
            if( kGetClusterLinkData( pstFileHandle->dwCurrentClusterIndex,
                                     &dwNextClusterIndex ) == FALSE )
            {
                break;
            }

            // Ŭ������ ������ ����
            pstFileHandle->dwPreviousClusterIndex =
                pstFileHandle->dwCurrentClusterIndex;
            pstFileHandle->dwCurrentClusterIndex = dwNextClusterIndex;
        }
    }

    //==========================================================================
    // ���� ũ�Ⱑ ���ߴٸ� ��Ʈ �����͸��� �ִ� �����͸� ��Ʈ�� ������ ����
    //==========================================================================
    if( pstFileHandle->dwFileSize < pstFileHandle->dwCurrentOffset )
    {
        pstFileHandle->dwFileSize = pstFileHandle->dwCurrentOffset;
        kUpdateDirectoryEntry( pstFileHandle );
    }

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );

    // �� ���ڵ� ���� ��ȯ
    return ( dwWriteCount / dwSize );
}

/**
 *  ������ Count��ŭ 0���� ä��
 */
BOOL kWriteZero( FILE* pstFile, DWORD dwCount )
{
    BYTE* pbBuffer;
    DWORD dwRemainCount;
    DWORD dwWriteCount;

    // �ڵ��� NULL�̸� ����
    if( pstFile == NULL )
    {
        return FALSE;
    }

    // �ӵ� ������ ���� �޸𸮸� �Ҵ� �޾� Ŭ������ ������ ���� ����
    // �޸𸮸� �Ҵ�
    pbBuffer = ( BYTE* ) kAllocateMemory( FILESYSTEM_CLUSTERSIZE );
    if( pbBuffer == NULL )
    {
        return FALSE;
    }

    // 0���� ä��
    kMemSet( pbBuffer, 0, FILESYSTEM_CLUSTERSIZE );
    dwRemainCount = dwCount;

    // Ŭ������ ������ �ݺ��ؼ� ���� ����
    while( dwRemainCount != 0 )
    {
        dwWriteCount = MIN( dwRemainCount , FILESYSTEM_CLUSTERSIZE );
        if( kWriteFile( pbBuffer, 1, dwWriteCount, pstFile ) != dwWriteCount )
        {
            kFreeMemory( pbBuffer );
            return FALSE;
        }
        dwRemainCount -= dwWriteCount;
    }
    kFreeMemory( pbBuffer );
    return TRUE;
}

/**
 *  ���� �������� ��ġ�� �̵�
 */
int kSeekFile( FILE* pstFile, int iOffset, int iOrigin )
{
    DWORD dwRealOffset;
    DWORD dwClusterOffsetToMove;
    DWORD dwCurrentClusterOffset;
    DWORD dwLastClusterOffset;
    DWORD dwMoveCount;
    DWORD i;
    DWORD dwStartClusterIndex;
    DWORD dwPreviousClusterIndex;
    DWORD dwCurrentClusterIndex;
    FILEHANDLE* pstFileHandle;

    // �ڵ��� ���� Ÿ���� �ƴϸ� ����
    if( ( pstFile == NULL ) ||
        ( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    {
        return 0;
    }
    pstFileHandle = &( pstFile->stFileHandle );

    //==========================================================================
    // Origin�� Offset�� �����Ͽ� ���� ������ �������� ���� �����͸� �Űܾ� �� ��ġ��
    // ����
    //==========================================================================
    // �ɼǿ� ������ ���� ��ġ�� ����
    // �����̸� ������ ���� �������� �̵��ϸ� �����̸� ������ �� �������� �̵�
    switch( iOrigin )
    {
    // ������ ������ �������� �̵�
    case FILESYSTEM_SEEK_SET:
        // ������ ó���̹Ƿ� �̵��� �������� �����̸� 0���� ����
        if( iOffset <= 0 )
        {
            dwRealOffset = 0;
        }
        else
        {
            dwRealOffset = iOffset;
        }
        break;

    // ���� ��ġ�� �������� �̵�
    case FILESYSTEM_SEEK_CUR:
        // �̵��� �������� �����̰� ���� ���� �������� ������ ũ�ٸ�
        // �� �̻� �� �� �����Ƿ� ������ ó������ �̵�
        if( ( iOffset < 0 ) &&
            ( pstFileHandle->dwCurrentOffset <= ( DWORD ) -iOffset ) )
        {
            dwRealOffset = 0;
        }
        else
        {
            dwRealOffset = pstFileHandle->dwCurrentOffset + iOffset;
        }
        break;

    // ������ ���κ��� �������� �̵�
    case FILESYSTEM_SEEK_END:
        // �̵��� �������� �����̰� ���� ���� �������� ������ ũ�ٸ�
        // �� �̻� �� �� �����Ƿ� ������ ó������ �̵�
        if( ( iOffset < 0 ) &&
            ( pstFileHandle->dwFileSize <= ( DWORD ) -iOffset ) )
        {
            dwRealOffset = 0;
        }
        else
        {
            dwRealOffset = pstFileHandle->dwFileSize + iOffset;
        }
        break;
    }

    //==========================================================================
    // ������ �����ϴ� Ŭ�������� ������ ���� ���� �������� ��ġ�� �����Ͽ�
    // �Ű��� ���� �����Ͱ� ��ġ�ϴ� Ŭ�����ͱ��� Ŭ������ ��ũ�� Ž��
    //==========================================================================
    // ������ ������ Ŭ�������� ������
    dwLastClusterOffset = pstFileHandle->dwFileSize / FILESYSTEM_CLUSTERSIZE;
    // ���� �����Ͱ� �Ű��� ��ġ�� Ŭ������ ������
    dwClusterOffsetToMove = dwRealOffset / FILESYSTEM_CLUSTERSIZE;
    // ���� ���� �����Ͱ� �ִ� Ŭ�������� ������
    dwCurrentClusterOffset = pstFileHandle->dwCurrentOffset / FILESYSTEM_CLUSTERSIZE;

    // �̵��ϴ� Ŭ�������� ��ġ�� ������ ������ Ŭ�������� �������� �Ѿ��
    // ���� Ŭ�����Ϳ��� ���������� �̵��� ��, Write �Լ��� �̿��ؼ� �������� ��������
    // ä��
    if( dwLastClusterOffset < dwClusterOffsetToMove )
    {
        dwMoveCount = dwLastClusterOffset - dwCurrentClusterOffset;
        dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    }
    // �̵��ϴ� Ŭ�������� ��ġ�� ���� Ŭ�����Ϳ� ���ų� ������ ��ġ��
    // �ִٸ� ���� Ŭ�����͸� �������� ���̸�ŭ�� �̵��ϸ� �ȴ�.
    else if( dwCurrentClusterOffset <= dwClusterOffsetToMove )
    {
        dwMoveCount = dwClusterOffsetToMove - dwCurrentClusterOffset;
        dwStartClusterIndex = pstFileHandle->dwCurrentClusterIndex;
    }
    // �̵��ϴ� Ŭ�������� ��ġ�� ���� Ŭ������ ������ �ִٸ�, ù ��° Ŭ�����ͺ���
    // �̵��ϸ鼭 �˻�
    else
    {
        dwMoveCount = dwClusterOffsetToMove;
        dwStartClusterIndex = pstFileHandle->dwStartClusterIndex;
    }

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // Ŭ�����͸� �̵�
    dwCurrentClusterIndex = dwStartClusterIndex;
    for( i = 0 ; i < dwMoveCount ; i++ )
    {
        // ���� ����
        dwPreviousClusterIndex = dwCurrentClusterIndex;

        // ���� Ŭ�������� �ε����� ����
        if( kGetClusterLinkData( dwPreviousClusterIndex, &dwCurrentClusterIndex ) ==
            FALSE )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return -1;
        }
    }

    // Ŭ�����͸� �̵������� Ŭ������ ������ ����
    if( dwMoveCount > 0 )
    {
        pstFileHandle->dwPreviousClusterIndex = dwPreviousClusterIndex;
        pstFileHandle->dwCurrentClusterIndex = dwCurrentClusterIndex;
    }
    // ù ��° Ŭ�����ͷ� �̵��ϴ� ������ �ڵ��� Ŭ������ ���� ���� Ŭ�����ͷ� ����
    else if( dwStartClusterIndex == pstFileHandle->dwStartClusterIndex )
    {
        pstFileHandle->dwPreviousClusterIndex = pstFileHandle->dwStartClusterIndex;
        pstFileHandle->dwCurrentClusterIndex = pstFileHandle->dwStartClusterIndex;
    }

    //==========================================================================
    // ���� �����͸� �����ϰ� ���� �������� ������ ũ�⸦ �Ѿ��ٸ� ������ �κ���
    // 0���� ä���� ������ ũ�⸦ �ø�
    //==========================================================================
    // ���� ������ ũ�⸦ �Ѿ ���� �����Ͱ� �̵��ߴٸ�, ���� ����������
    // ���� ũ�⸸ŭ 0���� ä����
    if( dwLastClusterOffset < dwClusterOffsetToMove )
    {
        pstFileHandle->dwCurrentOffset = pstFileHandle->dwFileSize;
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );

        // ���� ũ�⸸ŭ 0���� ä��
        if( kWriteZero( pstFile, dwRealOffset - pstFileHandle->dwFileSize )
                == FALSE )
        {
            return 0;
        }
    }

    pstFileHandle->dwCurrentOffset = dwRealOffset;

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );

    return 0;
}

/**
 *  ������ ����
 */
int kCloseFile( FILE* pstFile )
{
    // �ڵ� Ÿ���� ������ �ƴϸ� ����
    if( ( pstFile == NULL ) ||
        ( pstFile->bType != FILESYSTEM_TYPE_FILE ) )
    {
        return -1;
    }

    // �ڵ��� ��ȯ
    kFreeFileDirectoryHandle( pstFile );
    return 0;
}

/**
 *  �ڵ� Ǯ�� �˻��Ͽ� ������ �����ִ����� Ȯ��
 */
BOOL kIsFileOpened( const DIRECTORYENTRY* pstEntry )
{
    int i;
    FILE* pstFile;

    // �ڵ� Ǯ�� ���� ���巹������ ������ ���� ���ϸ� �˻�
    pstFile = gs_stFileSystemManager.pstHandlePool;
    for( i = 0 ; i < FILESYSTEM_HANDLE_MAXCOUNT ; i++ )
    {
        // ���� Ÿ�� �߿��� ���� Ŭ�����Ͱ� ��ġ�ϸ� ��ȯ
        if( ( pstFile[ i ].bType == FILESYSTEM_TYPE_FILE ) &&
            ( pstFile[ i ].stFileHandle.dwStartClusterIndex ==
              pstEntry->dwStartClusterIndex ) )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//음
BOOL kGetDirectory( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;

    // ���� �ý����� �ν����� ���߰ų� �ε����� �ùٸ��� ������ ����
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� �ִ� �ش� �����͸� ����
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstEntry, pstRootEntry + iIndex, sizeof( DIRECTORYENTRY ) );
    return TRUE;
}
BOOL kSetDirectory( int iIndex, DIRECTORYENTRY* pstEntry )
{
    DIRECTORYENTRY* pstRootEntry;

    // ���� �ý����� �ν����� ���߰ų� �ε����� �ùٸ��� ������ ����
    if( ( gs_stFileSystemManager.bMounted == FALSE ) ||
        ( iIndex < 0 ) || ( iIndex >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }

    // ��Ʈ �����͸��� �ִ� �ش� �����͸� ����
    pstRootEntry = ( DIRECTORYENTRY* ) gs_vbTempBuffer;
    kMemCpy( pstRootEntry + iIndex, pstEntry, sizeof( DIRECTORYENTRY ) );

    // ��Ʈ �����͸��� ��
    if( kWriteCluster( 0, gs_vbTempBuffer ) == FALSE )
    {
        return FALSE;
    }
    return TRUE;
}
/**
 *  ������ ����
 */
int kRemoveFile( const char* pcFileName )
{
    DIRECTORYENTRY stEntry;
    int iDirectoryEntryOffset;
    int iFileNameLength;

    // ���� �̸� �˻�
    iFileNameLength = kStrLen( pcFileName );
    if( ( iFileNameLength > ( sizeof( stEntry.vcFileName ) - 1 ) ) ||
        ( iFileNameLength == 0 ) )
    {
        return NULL;
    }

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // ������ �����ϴ°� Ȯ��
    iDirectoryEntryOffset = kFindDirectoryEntry( pcFileName, &stEntry );
    if( iDirectoryEntryOffset == -1 )
    {
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return -1;
    }

    // �ٸ� �½�ũ���� �ش� ������ ���� �ִ��� �ڵ� Ǯ�� �˻��Ͽ� Ȯ��
    // ������ ���� ������ ������ �� ����
    if( kIsFileOpened( &stEntry ) == TRUE )
    {
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return -1;
    }

    // ������ �����ϴ� Ŭ�����͸� ���� ����
    if( kFreeClusterUntilEnd( stEntry.dwStartClusterIndex ) == FALSE )
    {
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return -1;
    }

    // �����͸� ��Ʈ���� �� ������ ����
    kMemSet( &stEntry, 0, sizeof( stEntry ) );
    if( kSetDirectoryEntryData( iDirectoryEntryOffset, &stEntry ) == FALSE )
    {
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return -1;
    }

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return 0;
}

/**
 *  �����͸��� ��
 */
DIR* kOpenDirectory( const char* pcDirectoryName )
{
    DIR* pstDirectory;
    DIRECTORYENTRY* pstDirectoryBuffer;

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // ��Ʈ �����͸� �ۿ� �����Ƿ� �����͸� �̸��� �����ϰ� �ڵ鸸 �Ҵ��޾Ƽ� ��ȯ
    pstDirectory = kAllocateFileDirectoryHandle();
    if( pstDirectory == NULL )
    {
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return NULL;
    }

    // ��Ʈ �����͸��� ������ ���۸� �Ҵ�
    pstDirectoryBuffer = ( DIRECTORYENTRY* ) kAllocateMemory( FILESYSTEM_CLUSTERSIZE );
    if( pstDirectory == NULL )
    {
        // �����ϸ� �ڵ��� ��ȯ�ؾ� ��
        kFreeFileDirectoryHandle( pstDirectory );
        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return NULL;
    }

    // ��Ʈ �����͸��� ����
    if( kReadCluster( 0, ( BYTE* ) pstDirectoryBuffer ) == FALSE )
    {
        // �����ϸ� �ڵ��� �޸𸮸� ���� ��ȯ�ؾ� ��
        kFreeFileDirectoryHandle( pstDirectory );
        kFreeMemory( pstDirectoryBuffer );

        // ����ȭ
        kUnlock( &( gs_stFileSystemManager.stMutex ) );
        return NULL;

    }

    // �����͸� Ÿ������ �����ϰ� ���� �����͸� ��Ʈ���� �������� �ʱ�ȭ
    pstDirectory->bType = FILESYSTEM_TYPE_DIRECTORY;
    pstDirectory->stDirectoryHandle.iCurrentOffset = 0;
    pstDirectory->stDirectoryHandle.pstDirectoryBuffer = pstDirectoryBuffer;

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return pstDirectory;
}

/**
 *  �����͸� ��Ʈ���� ��ȯ�ϰ� �������� �̵�
 */
struct kDirectoryEntryStruct* kReadDirectory( DIR* pstDirectory )
{
    DIRECTORYHANDLE* pstDirectoryHandle;
    DIRECTORYENTRY* pstEntry;

    // �ڵ� Ÿ���� �����͸��� �ƴϸ� ����
    if( ( pstDirectory == NULL ) ||
        ( pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY ) )
    {
        return NULL;
    }
    pstDirectoryHandle = &( pstDirectory->stDirectoryHandle );

    // �������� ������ Ŭ�����Ϳ� �����ϴ� �ִ����� �Ѿ�� ����
    if( ( pstDirectoryHandle->iCurrentOffset < 0 ) ||
        ( pstDirectoryHandle->iCurrentOffset >= FILESYSTEM_MAXDIRECTORYENTRYCOUNT ) )
    {
        return NULL;
    }

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // ��Ʈ �����͸��� �ִ� �ִ� �����͸� ��Ʈ���� ������ŭ �˻�
    pstEntry = pstDirectoryHandle->pstDirectoryBuffer;
    while( pstDirectoryHandle->iCurrentOffset < FILESYSTEM_MAXDIRECTORYENTRYCOUNT )
    {
        // ������ �����ϸ� �ش� �����͸� ��Ʈ���� ��ȯ
        if( pstEntry[ pstDirectoryHandle->iCurrentOffset ].dwStartClusterIndex
                != 0 )
        {
            // ����ȭ
            kUnlock( &( gs_stFileSystemManager.stMutex ) );
            return &( pstEntry[ pstDirectoryHandle->iCurrentOffset++ ] );
        }

        pstDirectoryHandle->iCurrentOffset++;
    }

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return NULL;
}

/**
 *  �����͸� �����͸� �����͸��� ó������ �̵�
 */
void kRewindDirectory( DIR* pstDirectory )
{
    DIRECTORYHANDLE* pstDirectoryHandle;

    // �ڵ� Ÿ���� �����͸��� �ƴϸ� ����
    if( ( pstDirectory == NULL ) ||
        ( pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY ) )
    {
        return ;
    }
    pstDirectoryHandle = &( pstDirectory->stDirectoryHandle );

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // �����͸� ��Ʈ���� �����͸� 0���� �ٲ���
    pstDirectoryHandle->iCurrentOffset = 0;

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
}


/**
 *  ���� �����͸��� ����
 */
int kCloseDirectory( DIR* pstDirectory )
{
    DIRECTORYHANDLE* pstDirectoryHandle;

    // �ڵ� Ÿ���� �����͸��� �ƴϸ� ����
    if( ( pstDirectory == NULL ) ||
        ( pstDirectory->bType != FILESYSTEM_TYPE_DIRECTORY ) )
    {
        return -1;
    }
    pstDirectoryHandle = &( pstDirectory->stDirectoryHandle );

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // ��Ʈ �����͸��� ���۸� �����ϰ� �ڵ��� ��ȯ
    kFreeMemory( pstDirectoryHandle->pstDirectoryBuffer );
    kFreeFileDirectoryHandle( pstDirectory );

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );

    return 0;
}


/**
 *  ���� �ý��� ĳ�ø� ���� �ϵ� ����ũ�� ��
 */
BOOL kFlushFileSystemCache( void )
{
    CACHEBUFFER* pstCacheBuffer;
    int iCacheCount;
    int i;

    // ĳ�ð� ��Ȱ��ȭ �Ǿ��ٸ� �Լ��� ������ �ʿ䰡 ����
    if( gs_stFileSystemManager.bCacheEnable == FALSE )
    {
        return TRUE;
    }

    // ����ȭ
    kLock( &( gs_stFileSystemManager.stMutex ) );

    // Ŭ������ ��ũ ���̺� ������ ĳ�� ������ ��� ������ ���� ĳ�� ���۸� ����
    // ����ũ�� ��
    kGetCacheBufferAndCount( CACHE_CLUSTERLINKTABLEAREA, &pstCacheBuffer,
            &iCacheCount );
    for( i = 0 ; i < iCacheCount ; i++ )
    {
        // ĳ���� ������ ���ߴٸ� �±׿� ������ ��ġ�� ���� ��
        if( pstCacheBuffer[ i ].bChanged == TRUE )
        {
            if( kInternalWriteClusterLinkTableWithoutCache(
                pstCacheBuffer[ i ].dwTag, pstCacheBuffer[ i ].pbBuffer ) == FALSE )
            {
                return FALSE;
            }
            // ������ ������ �ϵ� ����ũ�� �����Ƿ�, �������� ���� ������ ����
            pstCacheBuffer[ i ].bChanged = FALSE;
        }
    }

    // ������ ������ ĳ�� ������ ��� ������ ���� ĳ�� ���۸� ���� ����ũ�� ��
    kGetCacheBufferAndCount( CACHE_DATAAREA, &pstCacheBuffer, &iCacheCount );
    for( i = 0 ; i < iCacheCount ; i++ )
    {
        // ĳ���� ������ ���ߴٸ� �±׿� ������ ��ġ�� ���� ��
        if( pstCacheBuffer[ i ].bChanged == TRUE )
        {
            if( kInternalWriteClusterWithoutCache( pstCacheBuffer[ i ].dwTag,
                pstCacheBuffer[ i ].pbBuffer ) == FALSE )
            {
                return FALSE;
            }
            // ������ ������ �ϵ� ����ũ�� �����Ƿ�, �������� ���� ������ ����
            pstCacheBuffer[ i ].bChanged = FALSE;
        }
    }

    // ����ȭ
    kUnlock( &( gs_stFileSystemManager.stMutex ) );
    return TRUE;
}
