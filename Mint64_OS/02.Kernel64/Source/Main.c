#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"
#include "Console.h"
#include "ConsoleShell.h"
#include "Task.h"
#include "PIT.h"

void Main( void )
{	
	char vcTemp[2] = {0,};
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;
    int iCursorX, iCursorY;

    kPrintString(0, 12, "Switch To IA-32e Mode Success~!!");
    kPrintString(0, 13, "IA-32e C Language Kernel Start..............[Pass]");    
    
    //print with changedPrintString
    changedPrintString(0, 14, "This message is printed through the video memory relocated to 0xAB8000");
    
    
    //read access to 0x1fe000
    kPrintString(0, 15, "Read from 0x1fe000 [  ]");
    readFrom(0x1fe000, 15);
    //write access to 0x1fe000
    kPrintString(0, 16, "Write to 0x1fe000  [  ]");
    writeTo(0x1fe000, 16);

    //read access to 0x1ff000
    kPrintString(0, 17, "Read from 0x1ff000 [  ]");
    //readFrom(0x1ff000, 17);
    //write access to 0x1ff000
    //kPrintString(0, 18, "Write to 0x1ff000 [  ]");
    //writeTo(0x1ff000, 18);
    
    kInitializeConsole( 0, 19 );    
    //kPrintf( "Switch To IA-32e Mode Success~!!\n" );
    //kPrintf( "IA-32e C Language Kernel Start..............[Pass]\n" );
    kPrintf( "Initialize Console..........................[Pass]\n" );
    
    kGetCursor( &iCursorX, &iCursorY );
    kPrintf( "GDT Initialize And Switch For IA-32e Mode...[    ]" );
    kInitializeGDTTableAndTSS();
    kLoadGDTR( GDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "TSS Segment Load............................[    ]" );
    kLoadTR( GDT_TSSSEGMENT );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "IDT Initialize..............................[    ]" );
    kInitializeIDTTables();    
    kLoadIDTR( IDTR_STARTADDRESS );
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass\n" );
    
    kPrintf( "Total RAM Size Check........................[    ]" );
    kCheckTotalRAMSize();
    kSetCursor( 45, iCursorY++ );
    kPrintf( "Pass], Size = %d MB\n", kGetTotalRAMSize() );
    
    kPrintf( "TCB Pool And Scheduler Initialize...........[Pass]\n" );
    iCursorY++;

    kInitializeScheduler();
    kInitializePIT( MSTOCOUNT(1), 1 );

    kPrintf( "Keyboard Activate And Queue Initialize......[    ]" );
    
    // keyboard activate
    if( kInitializeKeyboard() == TRUE )
    {
        kSetCursor( 45, --iCursorY );
        kPrintf( "Pass\n" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        kSetCursor( 45, --iCursorY );
        kPrintf( "Fail\n" );
        while( 1 ) ;
    }
    
    kPrintf( "PIC Controller And Interrupt Initialize.....[    ]" );
    
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kSetCursor( 45, iCursorY );
    kPrintf( "Pass\n" );

    //kPrintString(0, 18, "Write to 0x1ff000 [  ]");
    //writeTo(0x1ff000, 18);

    kCreateTask( TASK_FLAGS_LOWEST | TASK_FLAGS_THREAD | TASK_FLAGS_SYSTEM | TASK_FLAGS_IDLE , 0, 0, ( QWORD ) kIdleTask );
    kStartConsoleShell();
}

// HW 2
//print
void kPrintString( int iX, int iY, const char* pcString ) {
	CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
	int i;

	pstScreen += ( iY * 80 ) + iX;
	for ( i = 0 ; pcString[i] != 0 ; i++ ) {
		pstScreen[i].bCharactor = pcString[i];
	}
}

//print at AB8000
void changedPrintString( int iX, int iY, const char* pcString ) {
	CHARACTER* pstScreen = ( CHARACTER* ) 0xAB8000;
	int i;

	pstScreen += ( iY * 80 ) + iX;
	for ( i = 0 ; pcString[i] != 0 ; i++ ) {
		pstScreen[i].bCharactor = pcString[i];
	}
}

void readFrom(CHARACTER* addrPtr, int iY){
	CHARACTER* addr = addrPtr;
	CHARACTER data = *addr;
	kPrintString(20, iY, "OK");
}

void writeTo(CHARACTER* addrPtr, int iY){
	CHARACTER* addr = addrPtr;
	(*addr).bCharactor = 't';
	kPrintString(20, iY, "OK");
}
