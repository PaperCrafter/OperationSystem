#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"


static inline void invlpg(int* m)
{
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}


/**
 *  공통으로 사용하는 예외 핸들러
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
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

	int *fault_addr = ( int* ) iVectorNumber; // TLB 관련

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
 *  공통으로 사용하는 인터럽트 핸들러
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
    //=========================================================================
    
    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  키보드 인터럽트의 핸들러
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );
    //=========================================================================

    // 키보드 컨트롤러에서 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperflags ) {
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) | dwUpperflags;
}
