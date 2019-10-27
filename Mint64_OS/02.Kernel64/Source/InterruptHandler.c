#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Console.h"


void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };
  
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;
    
    kPrintStringXY( 0, 0, "====================================================" );
    kPrintStringXY( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintStringXY( 0, 2, "                    Vector:                         " );
    kPrintStringXY( 27, 2, vcBuffer );
    kPrintStringXY( 0, 3, "====================================================" );

    while( 1 ) ;
}

void kPageFaultExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    int iCursorX, iCursorY;
    kGetCursor( &iCursorX, &iCursorY );

    char vcBuffer[ 9 ] = { '0', 'x', '0', '0', '0', '0', '0', '0'};
    int i = 7;
    
    while(iVectorNumber >= 1){
        if(iVectorNumber%16 >= 10) vcBuffer[i] = 'A' + iVectorNumber%16 - 10;
        else vcBuffer[i] = '0' + iVectorNumber%16;
        iVectorNumber = iVectorNumber/16;
        i--;
    }

    kPrintStringXY( 0, iCursorY-4, "====================================================" );
    kPrintStringXY( 0, iCursorY-3, "                 Page Fault Occurs~!!!!             " );
    kPrintStringXY( 0, iCursorY-2, "                   Address:                         " );
    kPrintStringXY( 27, iCursorY-2, vcBuffer );
    kPrintStringXY( 0, iCursorY-1, "====================================================" );

    while( 1 ) ;
}

void kProtectionFaultExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    int iCursorX, iCursorY;
    kGetCursor( &iCursorX, &iCursorY );

    char vcBuffer[ 9 ] = { '0', 'x', '0', '0', '0', '0', '0', '0'};
    int i = 7;
    
    while(iVectorNumber >= 1){
        if(iVectorNumber%16 >= 10) vcBuffer[i] = 'A' + iVectorNumber%16 - 10;
        else vcBuffer[i] = '0' + iVectorNumber%16;
        iVectorNumber = iVectorNumber/16;
        i--;
    }
    
    kPrintStringXY( 0, iCursorY-4, "====================================================" );
    kPrintStringXY( 0, iCursorY-3, "                 Protection fault Occurs~!          " );
    kPrintStringXY( 0, iCursorY-2, "                   Address:                        " );
    kPrintStringXY( 27, iCursorY-2, vcBuffer );
    kPrintStringXY( 0, iCursorY-1, "====================================================" );

    while( 1 ) ;
}


void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintStringXY( 70, 0, vcBuffer );
            
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}


//keyboad handler
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;


    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintStringXY( 0, 0, vcBuffer );

    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}
