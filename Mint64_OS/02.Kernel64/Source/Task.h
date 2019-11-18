/**
 *  file    Task.h
 *  date    2009/02/19
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �½�ũ�� ó���ϴ� �Լ��� ���õ� ����
 */

#ifndef __TASK_H__
#define __TASK_H__

#include "Types.h"
#include "List.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// SS, RSP, RFLAGS, CS, RIP + ISR���� �����ϴ� 19���� ��������
#define TASK_REGISTERCOUNT     ( 5 + 19 )
#define TASK_REGISTERSIZE       8

// Context �ڷᱸ���� �������� ������
#define TASK_GSOFFSET           0
#define TASK_FSOFFSET           1
#define TASK_ESOFFSET           2
#define TASK_DSOFFSET           3
#define TASK_R15OFFSET          4
#define TASK_R14OFFSET          5
#define TASK_R13OFFSET          6
#define TASK_R12OFFSET          7
#define TASK_R11OFFSET          8
#define TASK_R10OFFSET          9
#define TASK_R9OFFSET           10
#define TASK_R8OFFSET           11
#define TASK_RSIOFFSET          12
#define TASK_RDIOFFSET          13
#define TASK_RDXOFFSET          14
#define TASK_RCXOFFSET          15
#define TASK_RBXOFFSET          16
#define TASK_RAXOFFSET          17
#define TASK_RBPOFFSET          18
#define TASK_RIPOFFSET          19
#define TASK_CSOFFSET           20
#define TASK_RFLAGSOFFSET       21
#define TASK_RSPOFFSET          22
#define TASK_SSOFFSET           23

// �½�ũ Ǯ�� ��巹��
#define TASK_TCBPOOLADDRESS     0x800000
#define TASK_MAXCOUNT           1024

// ���� Ǯ�� ������ ũ��
#define TASK_STACKPOOLADDRESS   ( TASK_TCBPOOLADDRESS + sizeof( TCB ) * TASK_MAXCOUNT )
#define TASK_STACKSIZE          8192

// ��ȿ���� ���� �½�ũ ID
#define TASK_INVALIDID          0xFFFFFFFFFFFFFFFF

// �½�ũ�� �ִ�� �� �� �ִ� ���μ��� �ð�(5 ms)
#define TASK_PROCESSORTIME      5

// �غ� ����Ʈ�� ��
#define TASK_MAXREADYLISTCOUNT  5

// �½�ũ�� �켱 ����
#define TASK_FLAGS_HIGHEST            0
#define TASK_FLAGS_HIGH               1
#define TASK_FLAGS_MEDIUM             2
#define TASK_FLAGS_LOW                3
#define TASK_FLAGS_LOWEST             4
#define TASK_FLAGS_WAIT               0xFF          

// �½�ũ�� �÷���
#define TASK_FLAGS_ENDTASK            0x8000000000000000
#define TASK_FLAGS_SYSTEM             0x4000000000000000
#define TASK_FLAGS_PROCESS            0x2000000000000000
#define TASK_FLAGS_THREAD             0x1000000000000000
#define TASK_FLAGS_IDLE               0x0800000000000000

// �Լ� ��ũ��
#define GETPRIORITY( x )        ( ( x ) & 0xFF )
#define SETPRIORITY( x, priority )  ( ( x ) = ( ( x ) & 0xFFFFFFFFFFFFFF00 ) | \
        ( priority ) )
#define GETTCBOFFSET( x )       ( ( x ) & 0xFFFFFFFF )

// �ڽ� ������ ��ũ�� ����� stThreadLink �������� �½�ũ �ڷᱸ��(TCB) ��ġ�� 
// ����Ͽ� ��ȯ�ϴ� ��ũ��
#define GETTCBFROMTHREADLINK( x )   ( TCB* ) ( ( QWORD ) ( x ) - offsetof( TCB, \
                                      stThreadLink ) )


////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// ���ؽ�Ʈ�� ���õ� �ڷᱸ��
typedef struct kContextStruct
{
    QWORD vqRegister[ TASK_REGISTERCOUNT ];
} CONTEXT;

// �½�ũ(���μ��� �� ������)�� ���¸� �����ϴ� �ڷᱸ��
typedef struct kTaskControlBlockStruct
{
    // ���� �������� ��ġ�� ID
    LISTLINK stLink;
    
    // �÷���
    QWORD qwFlags;
    
    // ���μ��� �޸� ������ ���۰� ũ��
    void* pvMemoryAddress;
    QWORD qwMemorySize;

    //==========================================================================
    // ���� ������ ����
    //==========================================================================
    // �ڽ� �������� ��ġ�� ID
    LISTLINK stThreadLink;
    
    // �ڽ� �������� ����Ʈ
    LIST stChildThreadList;
    
    // �θ� ���μ����� ID
    QWORD qwParentProcessID;
    
    // ���ؽ�Ʈ
    CONTEXT stContext;

    // ������ ��巹���� ũ��
    void* pvStackAddress;
    QWORD qwStackSize;

    //티켓 수
    int tickets
    //
} TCB;

// TCB Ǯ�� ���¸� �����ϴ� �ڷᱸ��
typedef struct kTCBPoolManagerStruct
{
    // �½�ũ Ǯ�� ���� ����
    TCB* pstStartAddress;
    int iMaxCount;
    int iUseCount;
    
    // TCB�� �Ҵ�� Ƚ��
    int iAllocatedCount;
} TCBPOOLMANAGER;

//로터리 스케줄러 자료구조
typedef struct kSchedulerStruct
{
    // ���� ���� ���� �½�ũ
    TCB* pstRunningTask;
    
    // ���� ���� ���� �½�ũ�� ����� �� �ִ� ���μ��� �ð�
    int iProcessorTime;
    
    // ������ �½�ũ�� �غ����� ����Ʈ, �½�ũ�� �켱 ������ ���� ����
    //in lottery it dosent have to be an layerd architecture
    LIST stReadyList;

    // ������ �½�ũ�� ������� ����Ʈ
    LIST stWaitList;
    
    // �� �켱 �������� �½�ũ�� ������ Ƚ���� �����ϴ� �ڷᱸ��
    int viExecuteCount[ TASK_MAXREADYLISTCOUNT ];
    
    // ���μ��� ���ϸ� ����ϱ� ���� �ڷᱸ��
    QWORD qwProcessorLoad;
    
    // ���� �½�ũ(Idle Task)���� ����� ���μ��� �ð�
    QWORD qwSpendProcessorTimeInIdleTask;
    //총 전역 티켓량
    int globaltotaltickets;
    //카운터
    int counter;
    //winner
    int winner;
    //총 티켓 갯수
    int totaltickets;

} SCHEDULER;


#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
//==============================================================================
//  �½�ũ Ǯ�� �½�ũ ����
//==============================================================================
static void kInitializeTCBPool( void );
static TCB* kAllocateTCB( void );
static void kFreeTCB( QWORD qwID );
TCB* kCreateTask( QWORD qwFlags, void* pvMemoryAddress, QWORD qwMemorySize, 
                  QWORD qwEntryPointAddress );
static void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
        void* pvStackAddress, QWORD qwStackSize );

//==============================================================================
//  �����ٷ� ����
//==============================================================================
void kInitializeScheduler( void );
void kSetRunningTask( TCB* pstTask );
TCB* kGetRunningTask( void );
static TCB* kGetNextTaskToRun( void );
static BOOL kAddTaskToReadyList( TCB* pstTask );
void kSchedule( void );
BOOL kScheduleInInterrupt( void );
void kDecreaseProcessorTime( void );
BOOL kIsProcessorTimeExpired( void );
static TCB* kRemoveTaskFromReadyList( QWORD qwTaskID );
BOOL kChangePriority( QWORD qwID, BYTE bPriority );
BOOL kEndTask( QWORD qwTaskID );
void kExitTask( void );
int kGetReadyTaskCount( void );
int kGetTaskCount( void );
TCB* kGetTCBInTCBPool( int iOffset );
BOOL kIsTaskExist( QWORD qwID );
QWORD kGetProcessorLoad( void );
static TCB* kGetProcessByThread( TCB* pstThread );

//==============================================================================
//  ���� �½�ũ ����
//==============================================================================
void kIdleTask( void );
void kHaltProcessorByLoad( void );



static int rand();
#endif /*__TASK_H__*/
