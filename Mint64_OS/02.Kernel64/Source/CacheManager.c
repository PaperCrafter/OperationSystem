/**
 *  file    CacheManager.c
 *  date    2009/05/17
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   ���� �ý��� ĳ�ÿ� ���õ� �ҽ� ����
 */

#include "CacheManager.h"
#include "FileSystem.h"
#include "DynamicMemory.h"

// ���� �ý��� ĳ�� �ڷᱸ��
static CACHEMANAGER gs_stCacheManager;

/**
 *  ���� �ý��� ĳ�ø� �ʱ�ȭ
 */
BOOL kInitializeCacheManager( void )
{
    int i;
    
    // �ڷᱸ�� �ʱ�ȭ
    kMemSet( &gs_stCacheManager, 0, sizeof( gs_stCacheManager ) );
    
    // ���� �ð� �ʱ�ȭ
    gs_stCacheManager.vdwAccessTime[ CACHE_CLUSTERLINKTABLEAREA ] = 0;
    gs_stCacheManager.vdwAccessTime[ CACHE_DATAAREA ] = 0;

    // ĳ�� ������ �ִ� ����
    gs_stCacheManager.vdwMaxCount[ CACHE_CLUSTERLINKTABLEAREA ] = 
        CACHE_MAXCLUSTERLINKTABLEAREACOUNT;
    gs_stCacheManager.vdwMaxCount[ CACHE_DATAAREA ] = CACHE_MAXDATAAREACOUNT;
    
    // Ŭ������ ��ũ ���̺� ������ �޸� �Ҵ�, Ŭ������ ��ũ ���̺��� 512����Ʈ�� ������
    gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] = 
        ( BYTE* ) kAllocateMemory( CACHE_MAXCLUSTERLINKTABLEAREACOUNT * 512 );
    if( gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] == NULL )
    {
        return FALSE;
    }
    
    // �Ҵ� ���� �޸� ������ ����� ĳ�� ���ۿ� ���
    for( i = 0 ; i < CACHE_MAXCLUSTERLINKTABLEAREACOUNT ; i++ )
    {
        // ĳ�� ���ۿ� �޸� ���� �Ҵ�
        gs_stCacheManager.vvstCacheBuffer[ CACHE_CLUSTERLINKTABLEAREA ][ i ].
            pbBuffer = gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] 
            + ( i * 512 );
        
        // �±׸� ��ȿ���� ���� ������ �����Ͽ� �� ������ ����
        gs_stCacheManager.vvstCacheBuffer[ CACHE_CLUSTERLINKTABLEAREA ][ i ].
            dwTag = CACHE_INVALIDTAG;
        
    }
    
    // ������ ������ �޸� �Ҵ�, ������ ������ Ŭ������ ����(4 Kbyte)�� ������
    gs_stCacheManager.vpbBuffer[ CACHE_DATAAREA ] = 
        ( BYTE* ) kAllocateMemory( CACHE_MAXDATAAREACOUNT * FILESYSTEM_CLUSTERSIZE );
    if( gs_stCacheManager.vpbBuffer[ CACHE_DATAAREA ] == NULL )
    {
        // �����ϸ� ������ �Ҵ� ���� �޸𸮸� �����ؾ� ��
        kFreeMemory( gs_stCacheManager.vpbBuffer[ CACHE_CLUSTERLINKTABLEAREA ] );
        return FALSE;
    }
    
    // �Ҵ� ���� �޸� ������ ����� ĳ�� ���ۿ� ���
    for( i = 0 ; i < CACHE_MAXDATAAREACOUNT ; i++ )
    {
        // ĳ�� ���ۿ� �޸� ���� �Ҵ�
        gs_stCacheManager.vvstCacheBuffer[ CACHE_DATAAREA ][ i ].pbBuffer = 
            gs_stCacheManager.vpbBuffer[ CACHE_DATAAREA ] + 
            ( i * FILESYSTEM_CLUSTERSIZE );
        
        // �±׸� ��ȿ���� ���� ������ �����Ͽ� �� ������ ����
        gs_stCacheManager.vvstCacheBuffer[ CACHE_DATAAREA ][ i ].dwTag = 
            CACHE_INVALIDTAG;
    }
    
    return TRUE;
}

/**
 *  ĳ�� ���ۿ��� �� ���� ã�Ƽ� ��ȯ
 */
CACHEBUFFER* kAllocateCacheBuffer( int iCacheTableIndex )
{
    CACHEBUFFER* pstCacheBuffer;
    int i;
    
    // ĳ�� ���̺��� �ִ� ������ �Ѿ�� ����
    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX )
    {
        return FALSE;
    }
    
    // ���� �ð� �ʵ尡 �ִ� ������ ���� ��ü������ ���� �ð��� ������
    kCutDownAccessTime( iCacheTableIndex );

    // �ִ� ������ŭ �˻��Ͽ� �� ���� ��ȯ
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0 ; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] ; i++ )
    {
        if( pstCacheBuffer[ i ].dwTag == CACHE_INVALIDTAG )
        {
            // �ӽ÷� ĳ�� �±׸� �����Ͽ� �Ҵ�� ������ ����
            pstCacheBuffer[ i ].dwTag = CACHE_INVALIDTAG - 1;

            // ���� �ð��� ����
            pstCacheBuffer[ i ].dwAccessTime = 
                gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ]++;
            
            return &( pstCacheBuffer[ i ] );
        }
    }
    
    return NULL;
}

/**
 *  ĳ�� ���ۿ��� �±װ� ��ġ�ϴ� ���� ã�Ƽ� ��ȯ
 */
CACHEBUFFER* kFindCacheBuffer( int iCacheTableIndex, DWORD dwTag )
{
    CACHEBUFFER* pstCacheBuffer;
    int i;
    
    // ĳ�� ���̺��� �ִ� ������ �Ѿ�� ����
    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX )
    {
        return FALSE;
    }
    
    // ���� �ð� �ʵ尡 �ִ� ������ �����ϸ� ��ü������ ���� �ð��� ����
    kCutDownAccessTime( iCacheTableIndex );
    
    // �ִ� ������ŭ �˻��Ͽ� �� ���� ��ȯ
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0 ; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] ; i++ )
    {
        if( pstCacheBuffer[ i ].dwTag == dwTag )
        {
            // ���� �ð��� ����
            pstCacheBuffer[ i ].dwAccessTime = 
                gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ]++;
            
            return &( pstCacheBuffer[ i ] );
        }
    }
    
    return NULL;
}

/**
 *  ������ �ð��� ��ü������ ����
 */
static void kCutDownAccessTime( int iCacheTableIndex )
{
    CACHEBUFFER stTemp;
    CACHEBUFFER* pstCacheBuffer;
    BOOL bSorted;
    int i, j;

    // ĳ�� ���̺��� �ִ� ������ �Ѿ�� ����
    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX )
    {
        return ;
    }

    // ���� �ð��� ���� �ִ�ġ�� ���� �ʾҴٸ� ���� �ð��� ���� �ʿ� ����
    if( gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ] < 0xfffffffe )
    {
        return ;
    }

    // ĳ�� ���۸� ���� �ð����� ������������ ������
    // ���� ����(Bouble Sort) ���
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( j = 0 ; j < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] - 1 ; j++ )
    {
        // �⺻�� ���ĵ� ������ ����
        bSorted = TRUE;
        for( i = 0 ; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] - 1 - j ;
             i++ )
        {
            // ������ �� �����͸� ���Ͽ� ���� �ð��� ū ���� ����(i+1)�� ��ġ��Ŵ
            if( pstCacheBuffer[ i ].dwAccessTime > 
                pstCacheBuffer[ i + 1 ].dwAccessTime )
            {
                // �� �����͸� ��ȯ�ϹǷ� ���ĵ��� ���� ������ ǥ��
                bSorted = FALSE;

                // i��° ĳ�ÿ� i+1��° ĳ�ø� ��ȯ
                kMemCpy( &stTemp, &( pstCacheBuffer[ i ] ), 
                        sizeof( CACHEBUFFER ) );
                kMemCpy( &( pstCacheBuffer[ i ] ), &( pstCacheBuffer[ i + 1 ] ), 
                        sizeof( CACHEBUFFER ) );
                kMemCpy( &( pstCacheBuffer[ i + 1 ] ), &stTemp, 
                        sizeof( CACHEBUFFER ) );
            }
        }

        // �� ���ĵǾ����� ������ ���� ����
        if( bSorted == TRUE )
        {
            break;
        }
    }

    // ������������ ���������Ƿ�, �ε����� �����Ҽ��� ���� �ð� ū(�ֽ�) ĳ�� ������
    // ���� �ð��� 0���� ���������� �����Ͽ� ������ ����
    for( i = 0 ; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] ; i++ )
    {
        pstCacheBuffer[ i ].dwAccessTime = i;
    }

    // ���� �ð��� ���� �ý��� ĳ�� �ڷᱸ���� �����Ͽ� �������ʹ� ����� ������ 
    // ���� �ð��� �����ϵ��� ��
    gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ] = i;
}

/**
 *  ĳ�� ���ۿ��� ������ ���� ã��
 */
CACHEBUFFER* kGetVictimInCacheBuffer( int iCacheTableIndex )
{
    DWORD dwOldTime;
    CACHEBUFFER* pstCacheBuffer;
    int iOldIndex;
    int i;

    // ĳ�� ���̺��� �ִ� ������ �Ѿ�� ����
    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX )
    {
        return FALSE;
    }
    
    // ���� �ð��� �ִ�� �ؼ� ���� �ð��� ���� ������(���� ����) ĳ�� ���۸� �˻�
    iOldIndex = -1;
    dwOldTime = 0xFFFFFFFF;

    // ĳ�� ���ۿ��� ��� ������ �ʰų� �������� ���� ������ ���� ã�Ƽ� ��ȯ
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0 ; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] ; i++ )
    {
        // �� ĳ�� ���۰� ������ �� ���� ��ȯ
        if( pstCacheBuffer[ i ].dwTag == CACHE_INVALIDTAG )
        {
            iOldIndex = i;
            break;
        }

        // ���� �ð��� ���� ������ �����Ǿ����� �����ص�
        if( pstCacheBuffer[ i ].dwAccessTime < dwOldTime )
        {
            dwOldTime = pstCacheBuffer[ i ].dwAccessTime;
            iOldIndex = i;
        }
    }

    // ĳ�� ���۸� ã�� ���ϸ� ������ �߻��� ����
    if( iOldIndex == -1 )
    {
        kPrintf( "Cache Buffer Find Error\n" );
        
        return NULL;
    }

    // ���õ� ĳ�� ������ ���� �ð��� ����
    pstCacheBuffer[ iOldIndex ].dwAccessTime = 
        gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ]++;
    
    return &( pstCacheBuffer[ iOldIndex ] );
}

/**
 *  ĳ�� ������ ������ ��� ���
 */
void kDiscardAllCacheBuffer( int iCacheTableIndex )
{
    CACHEBUFFER* pstCacheBuffer;
    int i;
    
    // ĳ�� ���۸� ��� �� ������ ����
    pstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    for( i = 0 ; i < gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ] ; i++ )
    {
        pstCacheBuffer[ i ].dwTag = CACHE_INVALIDTAG;
    }
    
    // ���� �ð��� �ʱ�ȭ
    gs_stCacheManager.vdwAccessTime[ iCacheTableIndex ] = 0;
}

/**
 *  ĳ�� ������ �����Ϳ� �ִ� ������ ��ȯ
 */
BOOL kGetCacheBufferAndCount( int iCacheTableIndex, 
        CACHEBUFFER** ppstCacheBuffer, int* piMaxCount )
{
    // ĳ�� ���̺��� �ִ� ������ �Ѿ�� ����
    if( iCacheTableIndex > CACHE_MAXCACHETABLEINDEX )
    {
        return FALSE;
    }
    
    // ĳ�� ������ �����Ϳ� �ִ��� ��ȯ
    *ppstCacheBuffer = gs_stCacheManager.vvstCacheBuffer[ iCacheTableIndex ];
    *piMaxCount = gs_stCacheManager.vdwMaxCount[ iCacheTableIndex ];
    return TRUE;
}
