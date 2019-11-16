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

// �½�ũ�� ���¸� �����ϴ� �ڷᱸ��
typedef struct kTaskControlBlockStruct
{
    // ���� �������� ��ġ�� ID
    LISTLINK stLink;
    
    // �÷���
    QWORD qwFlags;

    // ���ؽ�Ʈ
    CONTEXT stContext;

    // ������ ��巹���� ũ��
    void* pvStackAddress;
    QWORD qwStackSize;
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

// �����ٷ��� ���¸� �����ϴ� �ڷᱸ��
typedef struct kSchedulerStruct
{
    // ���� ���� ���� �½�ũ
    TCB* pstRunningTask;
    
    // ���� ���� ���� �½�ũ�� ����� �� �ִ� ���μ��� �ð�
    int iProcessorTime;
    
    // ������ �½�ũ�� �غ����� ����Ʈ
    LIST stReadyList;
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
void kInitializeTCBPool( void );
TCB* kAllocateTCB( void );
void kFreeTCB( QWORD qwID );
TCB* kCreateTask( QWORD qwFlags, QWORD qwEntryPointAddress );
void kSetUpTask( TCB* pstTCB, QWORD qwFlags, QWORD qwEntryPointAddress,
        void* pvStackAddress, QWORD qwStackSize );

//==============================================================================
//  �����ٷ� ����
//==============================================================================
void kInitializeScheduler( void );
void kSetRunningTask( TCB* pstTask );
TCB* kGetRunningTask( void );
TCB* kGetNextTaskToRun( void );
void kAddTaskToReadyList( TCB* pstTask );
void kSchedule( void );
BOOL kScheduleInInterrupt( void );
void kDecreaseProcessorTime( void );
BOOL kIsProcessorTimeExpired( void );

#endif /*__TASK_H__*/
