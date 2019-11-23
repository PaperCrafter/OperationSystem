#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"
#include "Utility.h"
#include "Task.h"
#include "Descriptor.h"
#include "AssemblyUtility.h"
#include "HardDisk.h"

static inline void invlpg(int* m)
{
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}


/**
 *  �������� ����ϴ� ���� �ڵ鷯
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
    
    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "                    Vector:                         " );
    kPrintStringXY( 27, 2, vcBuffer );
    kPrintStringXY( 0, 3, "====================================================" );

    while( 1 ) ;
}

void kPageFaultExceptionHandler( int iVectorNumber, QWORD qwErrorcode )
{
	int iCursorX, iCursorY;
   	kGetCursor( &iCursorX, &iCursorY );
	char vcBuffer[ 9 ] = { '0', 'x', '0', '0', '0', '0', '0', '0'};
	int i = 7;

	while(iVectorNumber >= 1)
	{
	        if(iVectorNumber%16 >= 10) 
			vcBuffer[i] = 'A' + iVectorNumber%16 - 10;
	        else 
			vcBuffer[i] = '0' + iVectorNumber%16;

	        iVectorNumber = iVectorNumber/16;
	        i--;
	}

	int *fault_addr = ( int* ) iVectorNumber; // TLB ����

	if((qwErrorcode%2) == 0 )
	{
	        kPrintStringXY( 0, iCursorY-4, "====================================================" );
	        kPrintStringXY( 0, iCursorY-3, "                 Page Fault Occurs~!!!!             " );
	        kPrintStringXY( 0, iCursorY-2, "                   Address:                         " );
	        kPrintStringXY( 27, iCursorY-2, vcBuffer );
	        kPrintStringXY( 0, iCursorY-1, "====================================================" );

		PTENTRY* pstPTEntry;
      		pstPTEntry = ( PTENTRY* )0x142000;

      		DWORD dwMappingAddress;
      		dwMappingAddress = PAGE_TABLESIZE * 511;

      		i = 511;

      		kSetPageEntryData((pstPTEntry + i), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT, 0);
	}
	else
	{
	        kPrintStringXY( 0, iCursorY-4, "====================================================" );
	        kPrintStringXY( 0, iCursorY-3, "               Protection Fault Occurs~!!!!         " );
	        kPrintStringXY( 0, iCursorY-2, "                   Address:                         " );
	        kPrintStringXY( 27, iCursorY-2, vcBuffer );
	        kPrintStringXY( 0, iCursorY-1, "====================================================" );

		PTENTRY* pstPTEntry;
      		pstPTEntry = ( PTENTRY* )0x142000;

      		DWORD dwMappingAddress;
      		dwMappingAddress = PAGE_TABLESIZE * 511;

      		i = 511;

      		kSetPageEntryData((pstPTEntry + i), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT, 0);
	}

	//TLB Invalitdation
	invlpg(fault_addr);
    
}


/**
 *  �������� ����ϴ� ���ͷ�Ʈ �ڵ鷯
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // EOI ����
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  Ű���� ���ͷ�Ʈ�� �ڵ鷯
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // Ű���� ��Ʈ�ѷ����� �����͸� �о ASCII�� ��ȯ�Ͽ� ť�� ����
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    // EOI ����
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperflags ) {
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) | dwUpperflags;
}


void kTimerHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iTimerInterruptCount = 0;
    timer++;     //
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;

    vcBuffer[ 8 ] = '0' + g_iTimerInterruptCount;
    g_iTimerInterruptCount = ( g_iTimerInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );

    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );

    g_qwTickCount++;
    
    kDecreaseProcessorTime();

    if( kIsProcessorTimeExpired() == TRUE){
        kScheduleInInterrupt();
    }
}

/**
 *  Device Not Available ������ �ڵ鷯
 */
void kDeviceNotAvailableHandler( int iVectorNumber )
{
    TCB* pstFPUTask, * pstCurrentTask;
    QWORD qwLastFPUTaskID;

    //=========================================================================
    // FPU ���ܰ� �߻������� �˸����� �޽����� ����ϴ� �κ�
    char vcBuffer[] = "[EXC:  , ]";
    static int g_iFPUInterruptCount = 0;

    // ���� ���͸� ȭ�� ������ ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iFPUInterruptCount;
    g_iFPUInterruptCount = ( g_iFPUInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );    
    //=========================================================================
    
    // CR0 ��Ʈ�� ���������� TS ��Ʈ�� 0���� ����
    kClearTS();

    // ������ FPU�� ����� �½�ũ�� �ִ��� Ȯ���Ͽ�, �ִٸ� FPU�� ���¸� �½�ũ�� ����
    qwLastFPUTaskID = kGetLastFPUUsedTaskID();
    pstCurrentTask = kGetRunningTask();
    
    // ������ FPU�� ����� ���� �ڽ��̸� �ƹ��͵� �� ��
    if( qwLastFPUTaskID == pstCurrentTask->stLink.qwID )
    {
        return ;
    }
    // FPU�� ����� �½�ũ�� ������ FPU ���¸� ����
    else if( qwLastFPUTaskID != TASK_INVALIDID )
    {
        pstFPUTask = kGetTCBInTCBPool( GETTCBOFFSET( qwLastFPUTaskID ) );
        if( ( pstFPUTask != NULL ) && ( pstFPUTask->stLink.qwID == qwLastFPUTaskID ) )
        {
            kSaveFPUContext( pstFPUTask->vqwFPUContext );
        }
    }
    
    // ���� �½�ũ�� FPU�� ����� ���� �ִ� �� Ȯ���Ͽ� FPU�� ����� ���� ���ٸ� 
    // �ʱ�ȭ�ϰ�, ��������� �ִٸ� ����� FPU ���ؽ�Ʈ�� ����
    if( pstCurrentTask->bFPUUsed == FALSE )
    {
        kInitializeFPU();
        pstCurrentTask->bFPUUsed = TRUE;
    }
    else
    {
        kLoadFPUContext( pstCurrentTask->vqwFPUContext );
    }
    
    // FPU�� ����� �½�ũ ID�� ���� �½�ũ�� ����
    kSetLastFPUUsedTaskID( pstCurrentTask->stLink.qwID );
}

void kHDDHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iHDDInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // ���ͷ�Ʈ�� �߻������� �˸����� �޽����� ����ϴ� �κ�
    // ���ͷ�Ʈ ���͸� ȭ�� ���� ���� 2�ڸ� ������ ���
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // �߻��� Ƚ�� ���
    vcBuffer[ 8 ] = '0' + g_iHDDInterruptCount;
    g_iHDDInterruptCount = ( g_iHDDInterruptCount + 1 ) % 10;
    // ���� ���� �ִ� �޽����� ��ġ�� �ʵ��� (10, 0)�� ���
    kPrintStringXY( 10, 0, vcBuffer );
    //=========================================================================

    // ù ��° PATA ��Ʈ�� ���ͷ�Ʈ ����(IRQ 14) ó��
    if( iVectorNumber - PIC_IRQSTARTVECTOR == 14 )
    {
        // ù ��° PATA ��Ʈ�� ���ͷ�Ʈ �߻� ���θ� TRUE�� ����
        kSetHDDInterruptFlag( TRUE, TRUE );
    }
    // �� ��° PATA ��Ʈ�� ���ͷ�Ʈ ����(IRQ 15) ó��
    else
    {
        // �� ��° PATA ��Ʈ�� ���ͷ�Ʈ �߻� ���θ� TRUE�� ����
        kSetHDDInterruptFlag( FALSE, TRUE );
    }
    
    // EOI ����
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}
