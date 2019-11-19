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

static int weight = 1000;
static int maxTotalPass = 1000000000;

//==============================================================================
//  �½�ũ Ǯ�� �½�ũ ����
//==============================================================================
/**
 *  �½�ũ Ǯ �ʱ�ȭ
 */
static void kInitializeTCBPool( void )
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
static TCB* kAllocateTCB( void )
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
static void kFreeTCB( QWORD qwID )
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
 *      ���μ��� �� ������ ��� ���� ����
 */
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress )
{
    TCB* pstTask, * pstProcess;
    void* pvStackAddress;
    BOOL bPreviousFlag;
    BYTE bPriority;
    bPriority = GETPRIORITY(qwFlags);
    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();    
    pstTask = kAllocateTCB();
    if( pstTask == NULL )
    {
        // �Ӱ迵�� ��
        kUnlockForSystemData( bPreviousFlag );
        return NULL;
    }

    // ���� ���μ��� �Ǵ� �����尡 ���� ���μ����� �˻�
    pstProcess = kGetProcessByThread( kGetRunningTask() );
    // ���� ���μ����� ���ٸ� �ƹ��� �۾��� ���� ����
    if( pstProcess == NULL )
    {
        kFreeTCB( pstTask->stLink.qwID );
        // �Ӱ� ���� ��
        kUnlockForSystemData( bPreviousFlag );
        return NULL;
    }

    // �����带 �����ϴ� ����� ���� ���� ���μ����� �ڽ� ������ ����Ʈ�� ������
    if( qwFlags & TASK_FLAGS_THREAD )
    {
        // ���� �������� ���μ����� ã�Ƽ� ������ �����忡 ���μ��� ������ ���
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pstProcess->pvMemoryAddress;
        pstTask->qwMemorySize = pstProcess->qwMemorySize;
        
        // �θ� ���μ����� �ڽ� ������ ����Ʈ�� �߰�
        kAddListToTail( &( pstProcess->stChildThreadList ), &( pstTask->stThreadLink ) );
    }
    // ���μ����� �Ķ���ͷ� �Ѿ�� ���� �״�� ����
    else
    {
        pstTask->qwParentProcessID = pstProcess->stLink.qwID;
        pstTask->pvMemoryAddress = pvMemoryAddress;
        pstTask->qwMemorySize = qwMemorySize;
    }
    
    // �������� ID�� �½�ũ ID�� �����ϰ� ����
    pstTask->stThreadLink.qwID = pstTask->stLink.qwID;   
    //allocate stride and pass
    pstTask->stride = weight / bPriority;
    pstTask->pass = 0;
    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );
    
    // �½�ũ ID�� ���� ��巹�� ���, ���� 32��Ʈ�� ���� Ǯ�� ������ ���� ����
    pvStackAddress = ( void* ) ( TASK_STACKPOOLADDRESS + ( TASK_STACKSIZE * 
            GETTCBOFFSET( pstTask->stLink.qwID ) ) );
    
    // TCB�� ������ �� �غ� ����Ʈ�� �����Ͽ� �����ٸ��� �� �ֵ��� ��
    kSetUpTask( pstTask, qwFlags, qwEntryPointAddress, pvStackAddress, 
            TASK_STACKSIZE );

    // �ڽ� ������ ����Ʈ�� �ʱ�ȭ
    kInitializeList( &( pstTask->stChildThreadList ) );

    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    
    // �½�ũ�� �غ� ����Ʈ�� ����
    kAddTaskToReadyList( pstTask );
    
    // �Ӱ� ���� ��

    kUnlockForSystemData( bPreviousFlag );
    
    return pstTask;
}

/**
 *  �Ķ���͸� �̿��ؼ� TCB�� ����
 */
static void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
                 void* pvStackAddress, QWORD qwStackSize )
{
    // ���ؽ�Ʈ �ʱ�ȭ
    kMemSet( pstTCB->stContext.vqRegister, 0, sizeof( pstTCB->stContext.vqRegister ) );
    
    // ���ÿ� ���õ� RSP, RBP �������� ����
    pstTCB->stContext.vqRegister[ TASK_RSPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize - 8;
    pstTCB->stContext.vqRegister[ TASK_RBPOFFSET ] = ( QWORD ) pvStackAddress + 
            qwStackSize - 8;
    
    // Return Address ������ kExitTask() �Լ��� ��巹���� �����Ͽ� �½�ũ�� ��Ʈ��
    // ����Ʈ �Լ��� ���������� ���ÿ� kExitTask() �Լ��� �̵��ϵ��� ��
    *( QWORD * ) ( ( QWORD ) pvStackAddress + qwStackSize - 8 ) = ( QWORD ) kExitTask;

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
    TCB* pstTask;
    // �½�ũ Ǯ �ʱ�ȭ
    kInitializeTCBPool();   
    kInitializeList( &( gs_stScheduler.vstReadyList ) );
    kInitializeList( &( gs_stScheduler.stWaitList ) );
    
    // TCB�� �Ҵ� �޾� ������ ������ �½�ũ�� Ŀ�� ������ ���μ����� ����
    pstTask = kAllocateTCB();
    gs_stScheduler.pstRunningTask = pstTask;
    pstTask->qwFlags = TASK_FLAGS_HIGHEST | TASK_FLAGS_PROCESS | TASK_FLAGS_SYSTEM;
    pstTask->qwParentProcessID = pstTask->stLink.qwID;
    pstTask->pvMemoryAddress = ( void* ) 0x100000;
    pstTask->qwMemorySize = 0x500000;
    pstTask->pvStackAddress = ( void* ) 0x600000;
    pstTask->qwStackSize = 0x100000;
    
    // ���μ��� ������ ����ϴµ� ����ϴ� �ڷᱸ�� �ʱ�ȭ
    gs_stScheduler.qwSpendProcessorTimeInIdleTask = 0;
    gs_stScheduler.qwProcessorLoad = 0;

}

/**
 *  ���� ���� ���� �½�ũ�� ����
 */
void kSetRunningTask( TCB* pstTask )
{
    BOOL bPreviousFlag;
    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();

    gs_stScheduler.pstRunningTask = pstTask;

    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );
}

/**
 *  ���� ���� ���� �½�ũ�� ��ȯ
 */
TCB* kGetRunningTask( void )
{
    BOOL bPreviousFlag;
    TCB* pstRunningTask;
    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    
    pstRunningTask = gs_stScheduler.pstRunningTask;
    
    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );

    return pstRunningTask;
}

/**
 *  �½�ũ ����Ʈ���� �������� ������ �½�ũ�� ����
 */
static TCB* kGetNextTaskToRun( void )
{
    TCB* tmpNode;
    TCB* pstTarget = NULL;
    int iTaskCount, i, j;
    int minNodeIdx = 0;
    int minPass = 0;
    int totalPass = 0;
    
    // ť�� �½�ũ�� ������ ��� ť�� �½�ũ�� 1ȸ�� ����� ���, ��� ť�� ���μ�����
    // �纸�Ͽ� �½�ũ�� �������� ���� �� ������ NULL�� ��� �ѹ� �� ����
    //pstTarget = (TCB*)kGetHeaderFromList(&(gs_stScheduler.vstReadyList));
    iTaskCount = kGetListCount(&(gs_stScheduler.vstReadyList));

    if(iTaskCount == 0){
        return NULL;
    }

 
    //schedule
    for(i =0; i < iTaskCount; i++){
        tmpNode = (TCB*)kRemoveListFromHeader(&(gs_stScheduler.vstReadyList));
        totalPass += tmpNode->pass;
        if(i == 0 || tmpNode->pass < pstTarget->pass){
            minPass = tmpNode->pass;
            pstTarget = tmpNode;
        }
        kAddListToTail(&(gs_stScheduler.vstReadyList), tmpNode);
    }

    //prevent pass going out of range
    if(totalPass > maxTotalPass){
        for(i =0; i < iTaskCount; i++){
            tmpNode = (TCB*)kRemoveListFromHeader(&(gs_stScheduler.vstReadyList));
            tmpNode -> pass = 0;
            kAddListToTail(&(gs_stScheduler.vstReadyList), tmpNode);
        }
    }

    pstTarget->pass += pstTarget->stride;
    kRemoveList( &(gs_stScheduler.vstReadyList), pstTarget->stLink.qwID );
    return pstTarget;
}

/**
 *  �½�ũ�� �����ٷ��� �غ� ����Ʈ�� ����
 */
static BOOL kAddTaskToReadyList( TCB* pstTask )
{
    BYTE bPriority;
    
    bPriority = GETPRIORITY( pstTask->qwFlags );

    kAddListToTail( &( gs_stScheduler.vstReadyList), pstTask );
    return TRUE;
}

/**
 *  �غ� ť���� �½�ũ�� ����
 */
static TCB* kRemoveTaskFromReadyList( QWORD qwTaskID )
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
    pstTarget = kRemoveList( &( gs_stScheduler.vstReadyList), 
                     qwTaskID );
    return pstTarget;
}

/**
 *  �½�ũ�� �켱 ������ ������
 */
BOOL kChangePriority( QWORD qwTaskID, BYTE bPriority )
{
    TCB* pstTarget;
    BOOL bPreviousFlag;
    
    if( bPriority > TASK_MAXREADYLISTCOUNT )
    {
        return FALSE;
    }
    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    
    // ���� �������� �½�ũ�̸� �켱 ������ ����
    // PIT ��Ʈ�ѷ��� ���ͷ�Ʈ(IRQ 0)�� �߻��Ͽ� �½�ũ ��ȯ�� ����� �� ����� 
    // �켱 ������ ����Ʈ�� �̵�
    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        SETPRIORITY( pstTarget->qwFlags, bPriority );
        pstTarget->stride = weight / bPriority;
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
                pstTarget->stride = weight / bPriority;
            }
        }
        else
        {
            // �켱 ������ �����ϰ� �غ� ����Ʈ�� �ٽ� ����
            SETPRIORITY( pstTarget->qwFlags, bPriority );
            pstTarget->stride = weight / bPriority;
            kAddTaskToReadyList( pstTarget );
        }
    }
    // �Ӱ� ���� ��

    kUnlockForSystemData( bPreviousFlag );
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
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    pstNextTask = kGetNextTaskToRun();
    
    if( pstNextTask == NULL )
    {
        // �Ӱ� ���� ��
        kUnlockForSystemData( bPreviousFlag );
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

    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );
}

/**
 *  ���ͷ�Ʈ�� �߻����� ��, �ٸ� �½�ũ�� ã�� ��ȯ
 *      �ݵ�� ���ͷ�Ʈ�� ���ܰ� �߻����� �� ȣ���ؾ� ��
 */
BOOL kScheduleInInterrupt( void )
{
    TCB* pstRunningTask, * pstNextTask;
    char* pcContextAddress;
    BOOL bPreviousFlag;

    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    pstNextTask = kGetNextTaskToRun();

    if( pstNextTask == NULL )
    {
        // �Ӱ� ���� ��
        kUnlockForSystemData( bPreviousFlag );
        return ;
    }
 
    //==========================================================================
    //  �½�ũ ��ȯ ó��   
    //      ���ͷ�Ʈ �ڵ鷯���� ������ ���ؽ�Ʈ�� �ٸ� ���ؽ�Ʈ�� ����� ������� ó��
    //==========================================================================
    pcContextAddress = ( char* ) IST_STARTADDRESS + IST_SIZE - sizeof( CONTEXT );
    
    // ���� �������� �½�ũ�� ������ ������ �� ���ؽ�Ʈ ��ȯ
    pstRunningTask = gs_stScheduler.pstRunningTask;
    gs_stScheduler.pstRunningTask = pstNextTask;

    // ���� �½�ũ���� ��ȯ�Ǿ��ٸ� ����� Tick Count�� ������Ŵ
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
    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );

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

/**
 *  �½�ũ�� ����
 */
BOOL kEndTask( QWORD qwTaskID )
{
    TCB* pstTarget;
    BYTE bPriority;
    BOOL bPreviousFlag;
    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    
    // ���� �������� �½�ũ�̸� EndTask ��Ʈ�� �����ϰ� �½�ũ�� ��ȯ
    pstTarget = gs_stScheduler.pstRunningTask;
    if( pstTarget->stLink.qwID == qwTaskID )
    {
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        
        // �Ӱ� ���� ��
        kUnlockForSystemData( bPreviousFlag );
        
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
            // �Ӱ� ���� ��
            kUnlockForSystemData( bPreviousFlag );
            return TRUE;
        }
        
        pstTarget->qwFlags |= TASK_FLAGS_ENDTASK;
        SETPRIORITY( pstTarget->qwFlags, TASK_FLAGS_WAIT );
        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTarget );
    }
    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );
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
    BOOL bPreviousFlag;

    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    iTotalCount = kGetListCount( &( gs_stScheduler.vstReadyList));
    // ��� �غ� ť�� Ȯ���Ͽ� �½�ũ ������ ����

    kUnlockForSystemData( bPreviousFlag );
    return iTotalCount ;
}

/**
 *  ��ü �½�ũ�� ���� ��ȯ
 */ 
int kGetTaskCount( void )
{
    int iTotalCount;
    BOOL bPreviousFlag;
    
    // �غ� ť�� �½�ũ ���� ���� ��, ��� ť�� �½�ũ ���� ���� ���� ���� �½�ũ ���� ����
    iTotalCount = kGetReadyTaskCount();
    
    // �Ӱ� ���� ����
    bPreviousFlag = kLockForSystemData();
    
    iTotalCount += kGetListCount( &( gs_stScheduler.stWaitList ) ) + 1;

    // �Ӱ� ���� ��
    kUnlockForSystemData( bPreviousFlag );
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

/**
 *  �����尡 �Ҽӵ� ���μ����� ��ȯ
 */
static TCB* kGetProcessByThread( TCB* pstThread )
{
    TCB* pstProcess;
    
    // ���� ���� ���μ����̸� �ڽ��� ��ȯ
    if( pstThread->qwFlags & TASK_FLAGS_PROCESS )
    {
        return pstThread;
    }
    
    // ���� ���μ����� �ƴ϶��, �θ� ���μ����� ������ �½�ũ ID�� ���� 
    // TCB Ǯ���� �½�ũ �ڷᱸ�� ����
    pstProcess = kGetTCBInTCBPool( GETTCBOFFSET( pstThread->qwParentProcessID ) );

    // ���� ���μ����� ���ų�, �½�ũ ID�� ��ġ���� �ʴ´ٸ� NULL�� ��ȯ
    if( ( pstProcess == NULL ) || ( pstProcess->stLink.qwID != pstThread->qwParentProcessID ) )
    {
        return NULL;
    }
    
    return pstProcess;
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
    TCB* pstTask, * pstChildThread, * pstProcess;
    QWORD qwLastMeasureTickCount, qwLastSpendTickInIdleTask;
    QWORD qwCurrentMeasureTickCount, qwCurrentSpendTickInIdleTask;
    BOOL bPreviousFlag;
    int i, iCount;
    QWORD qwTaskID;
    void* pstThreadLink;
    
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
                // �Ӱ� ���� ����
                bPreviousFlag = kLockForSystemData();                
                pstTask = kRemoveListFromHeader( &( gs_stScheduler.stWaitList ) );
                if( pstTask == NULL )
                {
                    // �Ӱ� ���� ��
                    kUnlockForSystemData( bPreviousFlag );
                    break;
                }
                
                if( pstTask->qwFlags & TASK_FLAGS_PROCESS )
                {
                    // ���μ����� ������ �� �ڽ� �����尡 �����ϸ� �����带 ��� 
                    // �����ϰ�, �ٽ� �ڽ� ������ ����Ʈ�� ����
                    iCount = kGetListCount( &( pstTask->stChildThreadList ) );
                    for( i = 0 ; i < iCount ; i++ )
                    {
                        // ������ ��ũ�� ��巹������ ���� �����带 �����Ŵ
                        pstThreadLink = ( TCB* ) kRemoveListFromHeader( 
                                &( pstTask->stChildThreadList ) );
                        if( pstThreadLink == NULL )
                        {
                            break;
                        }
                        
                        // �ڽ� ������ ����Ʈ�� ����� ������ �½�ũ �ڷᱸ���� �ִ� 
                        // stThreadLink�� ���� ��巹���̹Ƿ�, �½�ũ �ڷᱸ���� ����
                        // ��巹���� ���Ϸ��� ������ ����� �ʿ���
                        pstChildThread = GETTCBFROMTHREADLINK( pstThreadLink );

                        // �ٽ� �ڽ� ������ ����Ʈ�� �����Ͽ� �ش� �����尡 ����� ��
                        // �ڽ� �����尡 ���μ����� ã�� ������ ����Ʈ���� �����ϵ��� ��
                        kAddListToTail( &( pstTask->stChildThreadList ),
                                &( pstChildThread->stThreadLink ) );

                        // �ڽ� �����带 ã�Ƽ� ����
                        kEndTask( pstChildThread->stLink.qwID );
                    }
                    
                    // ���� �ڽ� �����尡 �����ִٸ� �ڽ� �����尡 �� ����� ������
                    // ��ٷ��� �ϹǷ� �ٽ� ��� ����Ʈ�� ����
                    if( kGetListCount( &( pstTask->stChildThreadList ) ) > 0 )
                    {
                        kAddListToTail( &( gs_stScheduler.stWaitList ), pstTask );

                        // �Ӱ� ���� ��
                        kUnlockForSystemData( bPreviousFlag );
                        continue;
                    }
                    // ���μ����� �����ؾ� �ϹǷ� �Ҵ� ���� �޸� ������ ����
                    else
                    {
                        // TODO: ���Ŀ� �ڵ� ����
                    }
                }                
                else if( pstTask->qwFlags & TASK_FLAGS_THREAD )
                {
                    // �������� ���μ����� �ڽ� ������ ����Ʈ���� ����
                    pstProcess = kGetProcessByThread( pstTask );
                    if( pstProcess != NULL )
                    {
                        kRemoveList( &( pstProcess->stChildThreadList ), pstTask->stLink.qwID );
                    }
                }
                
                qwTaskID = pstTask->stLink.qwID;
                kFreeTCB( qwTaskID );
                // �Ӱ� ���� ��
                kUnlockForSystemData( bPreviousFlag );
                
                kPrintf( "IDLE: Task ID[0x%q] is completely ended.\n", 
                        qwTaskID );
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
