/**
 *  file    Task.c
 *  date    2009/02/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �½�ũ�� ó���ϴ� �Լ��� ���õ� ����
 */

#include "Task.h"
#include "Descriptor.h"

// �����ٷ� ���� �ڷᱸ��
static SCHEDULER gs_stScheduler;
static TCBPOOLMANAGER gs_stTCBPoolManager;

//==============================================================================
//  �½�ũ Ǯ�� �½�ũ ����
//==============================================================================
/**
 *  �½�ũ Ǯ �ʱ�ȭ
 */
void kInitializeTCBPool( void )
{
    int i;
    
    kMemSet( &( gs_stTCBPoolManager ), 0, sizeof( gs_stTCBPoolManager ) );
    
    // �½�ũ Ǯ�� ��巹���� �����ϰ� �ʱ�ȭ
    gs_stTCBPoolManager.pstStartAddress = ( TCB* ) TASK_TCBPOOLADDRESS;
    kMemSet( TASK_TCBPOOLADDRESS, 0, sizeof( TCB ) * TASK_MAXCOUNT );

    // TCB�� ID �Ҵ�
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    }
    
    // TCB�� �ִ� ������ �Ҵ�� Ƚ���� �ʱ�ȭ
    gs_stTCBPoolManager.iMaxCount = TASK_MAXCOUNT;
    gs_stTCBPoolManager.iAllocatedCount = 1;
}

/**
 *  TCB�� �Ҵ� ����
 */
TCB* kAllocateTCB( void )
{
    TCB* pstEmptyTCB;
    int i;
    
    if( gs_stTCBPoolManager.iUseCount == gs_stTCBPoolManager.iMaxCount )
    {
        return NULL;
    }

    for( i = 0 ; i < gs_stTCBPoolManager.iMaxCount ; i++ )
    {
        // ID�� ���� 32��Ʈ�� 0�̸� �Ҵ���� ���� TCB
        if( ( gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID >> 32 ) == 0 )
        {
            pstEmptyTCB = &( gs_stTCBPoolManager.pstStartAddress[ i ] );
            break;
        }
    }

    // ���� 32��Ʈ�� 0�� �ƴ� ������ �����ؼ� �Ҵ�� TCB�� ����
    pstEmptyTCB->stLink.qwID = ( ( QWORD ) gs_stTCBPoolManager.iAllocatedCount << 32 ) | i;
    gs_stTCBPoolManager.iUseCount++;
    gs_stTCBPoolManager.iAllocatedCount++;
    if( gs_stTCBPoolManager.iAllocatedCount == 0 )
    {
        gs_stTCBPoolManager.iAllocatedCount = 1;
    }
    
    return pstEmptyTCB;
}

/**
 *  TCB�� ������
 */
void kFreeTCB( QWORD qwID )
{
    int i;
    
    // �½�ũ ID�� ���� 32��Ʈ�� �ε��� ������ ��
    i = qwID & 0xFFFFFFFF;
    
    // TCB�� �ʱ�ȭ�ϰ� ID ����
    kMemSet( &( gs_stTCBPoolManager.pstStartAddress[ i ].stContext ), 0, sizeof( CONTEXT ) );
    gs_stTCBPoolManager.pstStartAddress[ i ].stLink.qwID = i;
    
    gs_stTCBPoolManager.iUseCount--;
}

/**
 *  �½�ũ�� ����
 *      �½�ũ ID�� ���� ���� Ǯ���� ���� �ڵ� �Ҵ�
 */
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress )
{
    TCB* pstTask;
    void* pvStackAddress;
    
    pstTask = kAllocateTCB();
    if( pstTask == NULL )
    {
        return NULL;
    }
    
    // �½�ũ ID�� ���� ��巹�� ���, ���� 32��Ʈ�� ���� Ǯ�� ������ ���� ����
    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * 
            ( pstTask->stLink.qwID & 0xFFFFFFFF ) ) );
    
    // TCB�� ������ �� �غ� ����Ʈ�� �����Ͽ� �����ٸ��� �� �ֵ��� ��
    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, 
            TASK_STACKSIZE );
    kAddTaskToReadyList( pstTask );
    
    return pstTask;
}

/**
 *  �Ķ���͸� �̿��ؼ� TCB�� ����
 */
void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
                 void* pvStackAddress, QWORD qwStackSize )
{
    // ���ؽ�Ʈ �ʱ�ȭ
    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );
    
    // ���ÿ� ���õ� RSP, RBP �������� ����
    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize;

    // ���׸�Ʈ ������ ����
    pstTCB->stContext.vqRegister[ TASK_CSOFFSET ] = GDT_KERNELCODESEGMENT;
    pstTCB->stContext.vqRegister[ TASK_DSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_ESOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_FSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_GSOFFSET ] = GDT_KERNELDATASEGMENT;
    pstTCB->stContext.vqRegister[ TASK_SSOFFSET ] = GDT_KERNELDATASEGMENT;
    
    // RIP �������Ϳ� ���ͷ�Ʈ �÷��� ����
    pstTCB->stContext.vqRegister[ TASK_RIPOFFSET ] = qwEntryPointAddress;

    // RFLAGS ���������� IF ��Ʈ(��Ʈ 9)�� 1�� �����Ͽ� ���ͷ�Ʈ Ȱ��ȭ
    pstTCB->stContext.vqRegister[ TASK_RFLAGSOFFSET ] |= 0x0200;
    
    // ���ð� �÷��� ����
    pstTCB->pvStackAddress = pvStackAddress;
    pstTCB->qwStackSize = qwStackSize;
    pstTCB->qwFlags = qwFlags;
}

//==============================================================================
//  �����ٷ� ����
//==============================================================================
/**
 *  �����ٷ��� �ʱ�ȭ
 *      �����ٷ��� �ʱ�ȭ�ϴµ� �ʿ��� TCB Ǯ�� init �½�ũ�� ���� �ʱ�ȭ
 */
void kInitializeScheduler( void )
{
    // �½�ũ Ǯ �ʱ�ȭ
    kInitializeTCBPool();

    // �غ� ����Ʈ �ʱ�ȭ
    kInitializeList( &( gs_stScheduler.stReadyList ) );

    // TCB�� �Ҵ� �޾� ���� ���� �½�ũ�� �����Ͽ�, ������ ������ �½�ũ�� ������ TCB�� �غ�
    gs_stScheduler.pstRunningTask = kAllocateTCB();
}

/**
 *  ���� ���� ���� �½�ũ�� ����
 */
void kSetRunningTask( TCB* pstTask )
{
    gs_stScheduler.pstRunningTask = pstTask;
}

/**
 *  ���� ���� ���� �½�ũ�� ��ȯ
 */
TCB* kGetRunningTask( void )
{
    return gs_stScheduler.pstRunningTask;
}

/**
 *  �½�ũ ����Ʈ���� �������� ������ �½�ũ�� ����
 */
TCB* kGetNextTaskToRun( void )
{
    if( kGetListCount( &( gs_stScheduler.stReadyList ) ) == 0 )
    {
        return NULL;
    }
    
    return ( TCB* ) kRemoveListFromHeader( &( gs_stScheduler.stReadyList ) );
}

/**
 *  �½�ũ�� �����ٷ��� �غ� ����Ʈ�� ����
 */
void kAddTaskToReadyList( TCB* pstTask )
{
    kAddListToTail( &( gs_stScheduler.stReadyList ), pstTask );
}

/**
 *  �ٸ� �½�ũ�� ã�Ƽ� ��ȯ
 *      ���ͷ�Ʈ�� ���ܰ� �߻����� �� ȣ���ϸ� �ȵ�
 */
void kSchedule( void )
{
    TCB* pstRunningTask, * pstNextTask;
    BOOL bPreviousFlag;
    
    // ��ȯ�� �½�ũ�� �־�� ��
    if( kGetListCount( &( gs_stScheduler.stReadyList ) ) == 0 )
    {
        return ;
    }
    
    // ��ȯ�ϴ� ���� ���ͷ�Ʈ�� �߻��Ͽ� �½�ũ ��ȯ�� �� �Ͼ�� ����ϹǷ� ��ȯ�ϴ� 
    // ���� ���ͷ�Ʈ�� �߻����� ���ϵ��� ����
    bPreviousFlag = kSetInterruptFlag( FALSE );
    // ������ ���� �½�ũ�� ����
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL )
    {
        kSetInterruptFlag( bPreviousFlag );
        return ;
    }
    
    pstRunningTask = gs_stScheduler.pstRunningTask; 
    kAddTaskToReadyList( pstRunningTask );
    
    // ���� �½�ũ�� ���� ���� ���� �½�ũ�� ������ �� ���ؽ�Ʈ ��ȯ
    gs_stScheduler.pstRunningTask = pstNextTask;
    kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );

    // ���μ��� ��� �ð��� ������Ʈ
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;

    kSetInterruptFlag( bPreviousFlag );
}

/**
 *  ���ͷ�Ʈ�� �߻����� ��, �ٸ� �½�ũ�� ã�� ��ȯ
 *      �ݵ�� ���ͷ�Ʈ�� ���ܰ� �߻����� �� ȣ���ؾ� ��
 */
BOOL kScheduleInInterrupt( void )
{
    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    
    // ��ȯ�� �½�ũ�� ������ ����
    pstNextTask = kGetNextTaskToRun();
    if( pstNextTask == NULL )
    {
        return FALSE;
    }
    
    //==========================================================================
    //  �½�ũ ��ȯ ó��   
    //      ���ͷ�Ʈ �ڵ鷯���� ������ ���ؽ�Ʈ�� �ٸ� ���ؽ�Ʈ�� ����� ������� ó��
    //==========================================================================
    pcContextAddress = ( char* ) IST_STARTADDRESS + IST_SIZE - sizeof( CONTEXT );
    
    // ���� �½�ũ�� �� IST�� �ִ� ���ؽ�Ʈ�� �����ϰ�, ���� �½�ũ�� �غ� ����Ʈ��
    // �ű�
    pstRunningTask = gs_stScheduler.pstRunningTask;
    kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
    kAddTaskToReadyList( pstRunningTask );

    // ��ȯ�ؼ� ������ �½�ũ�� Running Task�� �����ϰ� ���ؽ�Ʈ�� IST�� �����ؼ�
    // �ڵ����� �½�ũ ��ȯ�� �Ͼ���� ��
    gs_stScheduler.pstRunningTask = pstNextTask;
    kMemCpy( pcContextAddress, &( pstNextTask->stContext ), sizeof( CONTEXT ) );
    
    // ���μ��� ��� �ð��� ������Ʈ
    gs_stScheduler.iProcessorTime = TASK_PROCESSORTIME;
    return TRUE;
}

/**
 *  ���μ����� ����� �� �ִ� �ð��� �ϳ� ����
 */
void kDecreaseProcessorTime( void )
{
    if( gs_stScheduler.iProcessorTime > 0 )
    {
        gs_stScheduler.iProcessorTime--;
    }
}

/**
 *  ���μ����� ����� �� �ִ� �ð��� �� �Ǿ����� ���θ� ��ȯ
 */
BOOL kIsProcessorTimeExpired( void )
{
    if( gs_stScheduler.iProcessorTime <= 0 )
    {
        return TRUE;
    }
    return FALSE;
}
