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
    i = GETTCBOFFSET( qwID );
    
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
            GETTCBOFFSET(pstTask->stLink.qwID) ) );
    
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
    int i;

    // �½�ũ Ǯ �ʱ�ȭ
    kInitializeTCBPool();

    for(int i = 0; i < TASK_MAXREADYLISTCOUNT; i++){
        kInitializeList(&(gs_stScheduler.vstReadyList[ i ]));
        gs_stScheduler.viExecuteCount[ i ] = 0;
    }
    // �غ� ����Ʈ �ʱ�ȭ
    kInitializeList( &( gs_stScheduler.vstReadyList ) );

    // TCB�� �Ҵ� �޾� ���� ���� �½�ũ�� �����Ͽ�, ������ ������ �½�ũ�� ������ TCB�� �غ�
    gs_stScheduler.pstRunningTask = kAllocateTCB();
    gs_stScheduler.pstRunningTask->qwFlags = TASK_FLAGS_HIGHEST;

    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;
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
    TCB* pstTarget = NULL;
    int iTaskCount, i, j;
    
    // ť�� �½�ũ�� ������ ��� ť�� �½�ũ�� 1ȸ�� ����� ���, ��� ť�� ���μ�����
    // �纸�Ͽ� �½�ũ�� �������� ���� �� ������ NULL�� ��� �ѹ� �� ����
    for( j = 0 ; j < 2 ; j++ )
    {
        // ���� �켱 �������� ���� �켱 �������� ����Ʈ�� Ȯ���Ͽ� �����ٸ��� �½�ũ�� ����
        for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
        {
            iTaskCount = kGetListCount( &( gs_stScheduler.vstReadyList[ i ] ) );
            
            // ���� ������ Ƚ������ ����Ʈ�� �½�ũ ���� �� ������ ���� �켱 ������
            // �½�ũ�� ������
            if( gs_stScheduler.viExecuteCount[ i ] < iTaskCount )
            {
                pstTarget = ( TCB* ) kRemoveListFromHeader( 
                                        &( gs_stScheduler.vstReadyList[ i ] ) );
                gs_stScheduler.viExecuteCount[ i ]++;
                break;            
            }
            // ���� ������ Ƚ���� �� ������ ���� Ƚ���� �ʱ�ȭ�ϰ� ���� �켱 ������ �纸��
            else
            {
                gs_stScheduler.viExecuteCount[ i ] = 0;
            }
        }
        
        // ���� ������ �½�ũ�� ã������ ����
        if( pstTarget != NULL )
        {
            break;
        }
    }    
    return pstTarget;
}

/**
 *  �½�ũ�� �����ٷ��� �غ� ����Ʈ�� ����
 */
BOOL kAddTaskToReadyList( TCB* pstTask )
{
    BYTE bPriority;
    
    bPriority = GETPRIORITY( pstTask->qwFlags );
    if( bPriority >= TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    kAddListToTail( &( gs_stScheduler.vstReadyList[ bPriority ] ), pstTask );
    return TRUE;
}

/**
 *  �غ� ť���� �½�ũ�� ����
 */
TCB* kRemoveTaskFromReadyList( QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    
    // �½�ũ ID�� ��ȿ���� ������ ����
    if( GETTCBOFFSET( qwTaskID ) >= TASK_MAXCOUNT )
    {
        return NULL;
    }
    
    // TCB Ǯ���� �ش� �½�ũ�� TCB�� ã�� ������ ID�� ��ġ�ϴ°� Ȯ��
    pstTarget = &( gs_stTCBPoolManager.pstStartAddress[ GETTCBOFFSET( qwTaskID ) ] );
    if( pstTarget->stLink.qwID != qwTaskID )
    {
        return NULL;
    }
    
    // �½�ũ�� �����ϴ� �غ� ����Ʈ���� �½�ũ ����
    bPriority = GETPRIORITY( pstTarget->qwFlags );

    pstTarget = kRemoveList( &( gs_stScheduler.vstReadyList[ bPriority ]), 
                     qwTaskID );
    return pstTarget;
}

/**
 *  �½�ũ�� �켱 ������ ������
 */
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority )
{
    TCB* pstTarget;
    
    if( bPriority > TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    // ���� �������� �½�ũ�̸� �켱 ������ ����
    // PIT ��Ʈ�ѷ��� ���ͷ�Ʈ(IRQ 0)�� �߻��Ͽ� �½�ũ ��ȯ�� ����� �� ����� 
    // �켱 ������ ����Ʈ�� �̵�
    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        SETPRIORITY( pstTarget->qwFlags, bPriority );
    }
    // �������� �½�ũ�� �ƴϸ� �غ� ����Ʈ���� ã�Ƽ� �ش� �켱 ������ ����Ʈ�� �̵�
    else
    {
        // �غ� ����Ʈ���� �½�ũ�� ã�� ���ϸ� ���� �½�ũ�� ã�Ƽ� �켱 ������ ����
        pstTarget = kRemoveTaskFromReadyList( qwTaskID );
        if( pstTarget == NULL )
        {
            // �½�ũ ID�� ���� ã�Ƽ� ����
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                // �켱 ������ ����
                SETPRIORITY( pstTarget->qwFlags, bPriority );
            }
        }
        else
        {
            // �켱 ������ �����ϰ� �غ� ����Ʈ�� �ٽ� ����
            SETPRIORITY( pstTarget->qwFlags, bPriority );
            kAddTaskToReadyList( pstTarget );
        }
    }
    return TRUE;    
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
    if( kGetReadyTaskCount() < 1 )
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
    
    // ���� �������� �½�ũ�� ������ ������ �� ���ؽ�Ʈ ��ȯ
    pstRunningTask = gs_stScheduler.pstRunningTask; 
    gs_stScheduler.pstRunningTask = pstNextTask;
    
    // ���� �½�ũ���� ��ȯ�Ǿ��ٸ� ����� ���μ��� �ð��� ������Ŵ
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += 
            TASK_PROCESSORTIME - gs_stScheduler.iProcessorTime;
    }
    
    // �½�ũ ���� �÷��װ� ������ ��� ���ؽ�Ʈ�� ������ �ʿ䰡 �����Ƿ�, ��� ����Ʈ��
    // �����ϰ� ���ؽ�Ʈ ��ȯ
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstRunningTask );
        kSwitchContext( NULL, &( pstNextTask->stContext ) );
    }
    else
    {
        kAddTaskToReadyList( pstRunningTask );
        kSwitchContext( &( pstRunningTask->stContext ), &( pstNextTask->stContext ) );
    }

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
    
    // ���� �������� �½�ũ�� ������ ������ �� ���ؽ�Ʈ ��ȯ
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    // ���� �½�ũ���� ��ȯ�Ǿ��ٸ� ����� ���μ��� �ð��� ������Ŵ
    if( ( pstRunningTask->qwFlags & TASK_FLAGS_IDLE ) == TASK_FLAGS_IDLE )
    {
        gs_stScheduler.qwSpendProcessorTimeInIdleTask += TASK_PROCESSORTIME;
    }    
    
    // �½�ũ ���� �÷��װ� ������ ���, ���ؽ�Ʈ�� �������� �ʰ� ��� ����Ʈ���� ����
    if( pstRunningTask->qwFlags & TASK_FLAGS_ENDTASK )
    {    
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstRunningTask );
    }
    // �½�ũ�� ������� ������ IST�� �ִ� ���ؽ�Ʈ�� �����ϰ�, ���� �½�ũ�� �غ� ����Ʈ��
    // �ű�
    else
    {
        kMemCpy( &( pstRunningTask->stContext ), pcContextAddress, sizeof( CONTEXT ) );
        kAddTaskToReadyList( pstRunningTask );
    }
    // ��ȯ�ؼ� ������ �½�ũ�� Running Task�� �����ϰ� ���ؽ�Ʈ�� IST�� �����ؼ�
    // �ڵ����� �½�ũ ��ȯ�� �Ͼ���� ��
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


BOOL kEndTask( QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    
    // ���� �������� �½�ũ�̸� EndTask ��Ʈ�� �����ϰ� �½�ũ�� ��ȯ
    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        
        kSchedule();
        
        // �½�ũ�� ��ȯ �Ǿ����Ƿ� �Ʒ� �ڵ�� ���� ������� ����
        while( 1 ) ;
    }
    // ���� ���� �½�ũ�� �ƴϸ� �غ� ť���� ���� ã�Ƽ� ��� ����Ʈ�� ����
    else
    {
        // �غ� ����Ʈ���� �½�ũ�� ã�� ���ϸ� ���� �½�ũ�� ã�Ƽ� �½�ũ ���� ��Ʈ��
        // ����
        pstTarget = kRemoveTaskFromReadyList( qwTaskID );
        if( pstTarget == NULL )
        {
            // �½�ũ ID�� ���� ã�Ƽ� ����
            pstTarget = kGetTCBInTCBPool( GETTCBOFFSET( qwTaskID ) );
            if( pstTarget != NULL )
            {
                pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
                SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
            }
            return FALSE;
        }
        
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTarget );
    }
    return TRUE;
}

/**
 *  �½�ũ�� �ڽ��� ������
 */
void kExitTask( void )
{
    kEndTask( gs_stScheduler.pstRunningTask->stLink.qwID );
}

/**
 *  �غ� ť�� �ִ� ��� �½�ũ�� ���� ��ȯ
 */
int kGetReadyTaskCount( void )
{
    int iTotalCount = 0;
    int i;
    
    // ��� �غ� ť�� Ȯ���Ͽ� �½�ũ ������ ����
    for( i = 0 ; i < TASK_MAXREADYLISTCOUNT ; i++ )
    {
        iTotalCount += kGetListCount( &( gs_stScheduler.vstReadyList[ i ] ) );
    }
    
    return iTotalCount ;
}

/**
 *  ��ü �½�ũ�� ���� ��ȯ
 */ 
int kGetTaskCount( void )
{
    int iTotalCount;
    
    // �غ� ť�� �½�ũ ���� ���� ��, ��� ť�� �½�ũ ���� ���� ���� ���� �½�ũ ���� ����
    iTotalCount = kGetReadyTaskCount();
    iTotalCount += kGetListCount( &( gs_stScheduler.stWaitList ) ) + 1;

    return iTotalCount;
}

/**
 *  TCB Ǯ���� �ش� �������� TCB�� ��ȯ
 */
TCB* kGetTCBInTCBPool( int iOffset )
{
    if( ( iOffset < -1 ) && ( iOffset > TASK_MAXCOUNT ) )
    {
        return NULL;
    }
    
    return &( gs_stTCBPoolManager.pstStartAddress[ iOffset ] );
}

/**
 *  �½�ũ�� �����ϴ��� ���θ� ��ȯ
 */
BOOL kIsTaskExist( QWORD qwID )
{
    TCB* pstTCB;
    
    // ID�� TCB�� ��ȯ
    pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
    // TCB�� ���ų� ID�� ��ġ���� ������ �������� �ʴ� ����
    if( ( pstTCB == NULL ) || ( pstTCB->stLink.qwID != qwID ) )
    {
        return FALSE;
    }
    return TRUE;
}

/**
 *  ���μ����� ������ ��ȯ
 */
QWORD kGetProcessorLoad( void )
{
    return gs_stScheduler.qwProcessorLoad;
}

//==============================================================================
//  ���� �½�ũ ����
//==============================================================================
/**
 *  ���� �½�ũ
 *      ��� ť�� ���� ������� �½�ũ�� ����
 */
void kIdleTask( void )
{
    TCB* pstTask;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    
    // ���μ��� ��뷮 ����� ���� ���� ������ ����
    qwLastSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
    qwLastMeasureTickCount = kGetTickCount();
    
    while( 1 )
    {
        // ���� ���¸� ����
        qwCurrentMeasureTickCount = kGetTickCount();
        qwCurrentSpendTickInIdleTask = gs_stScheduler.qwSpendProcessorTimeInIdleTask;
        
        // ���μ��� ��뷮�� ���
        // 100 - ( ���� �½�ũ�� ����� ���μ��� �ð� ) * 100 / ( �ý��� ��ü���� 
        // ����� ���μ��� �ð� )
        if( qwCurrentMeasureTickCount - qwLastMeasureTickCount == 0 )
        {
            gs_stScheduler.qwProcessorLoad = 0;
        }
        else
        {
            gs_stScheduler.qwProcessorLoad = 100 - 
                ( qwCurrentSpendTickInIdleTask - qwLastSpendTickInIdleTask ) * 
                100 /( qwCurrentMeasureTickCount - qwLastMeasureTickCount );
        }
        
        // ���� ���¸� ���� ���¿� ����
        qwLastMeasureTickCount = qwCurrentMeasureTickCount;
        qwLastSpendTickInIdleTask = qwCurrentSpendTickInIdleTask;

        // ���μ����� ���Ͽ� ���� ���� ��
        kHaltProcessorByLoad();
        
        // ��� ť�� ������� �½�ũ�� ������ �½�ũ�� ������
        if( kGetListCount( &( gs_stScheduler.stWaitList ) ) >= 0 )
        {
            while( 1 )
            {
                pstTask = kRemoveListFromHeader( &( gs_stScheduler.stWaitList ) );
                if( pstTask == NULL )
                {
                    break;
                }
                kPrintf( "IDLE: Task ID[0x%q] is completely ended.\n", 
                        pstTask->stLink.qwID );
                kFreeTCB( pstTask->stLink.qwID );
            }
        }
        
        kSchedule();
    }
}

/**
 *  ������ ���μ��� ���Ͽ� ���� ���μ����� ���� ��
 */
void kHaltProcessorByLoad( void )
{
    if( gs_stScheduler.qwProcessorLoad < 40 )
    {
        kHlt();
        kHlt();
        kHlt();
    }
    else if( gs_stScheduler.qwProcessorLoad < 80 )
    {
        kHlt();
        kHlt();
    }
    else if( gs_stScheduler.qwProcessorLoad < 95 )
    {
        kHlt();
    }
}