#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"

void kPrintString( int iX, int iY, const char* pcString );
void changedPrintString( int iX, int iY, const char* pcString );
void readFrom(CHARACTER* addrPtr, int iY);
void writeTo(CHARACTER* addrPtr, int iY);

int Main(void) {
	char vcTemp[2] = {0,};
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;
	
	kPrintString(0, 12, "Switch To IA-32e Mode Success~!!");
	kPrintString(0, 13, "IA-32e C Language Kernel Start..............[Pass]");
	//print with changedPrintString
	changedPrintString(0, 14, "This message is printed through the video memory relocated to 0xAB8000");
	//read access to 0x1fe000
	kPrintString(0, 15, "Read from 0x1fe000 [  ]");
	readFrom(0x1fe000, 15);
	//write access to 0x1fe000
	kPrintString(0, 16, "Write to 0x1fe000 [  ]");
	writeTo(0x1fe000, 16);
	//read access to 0x1ff000
	kPrintString(0, 17, "Read from 0x1ff000 [  ]");
	readFrom(0x1ff000, 17);
	
	//write access to 0x1ff000
	//kPrintString(0, 18, "Write to 0x1ff000 [  ]");
	//writeTo(0x1ff000, 18);
	
	
	// keyboard activate
	kPrintString(0, 18, "Keyboard Activate...........................[    ]");

    if( kActivateKeyboard() == TRUE )
    {
        kPrintString( 45, 18, "Pass" );
        kChangeKeyboardLED( FALSE, FALSE, FALSE );
    }
    else
    {
        kPrintString( 45, 18, "Fail" );
        while( 1 ) ;
    }
    
    kPrintString( 0, 19, "PIC Controller And Interrupt Initialize.....[    ]" );
    kInitializePIC();
    kMaskPICInterrupt( 0 );
    kEnableInterrupt();
    kPrintString( 45, 19, "Pass" );
    
    while( 1 )
    {
        if( kIsOutputBufferFull() == TRUE )
        {
            bTemp = kGetKeyboardScanCode();         
            if( kConvertScanCodeToASCIICode( bTemp, &( vcTemp[ 0 ] ), &bFlags ) == TRUE )
            {                
                if( bFlags & KEY_FLAGS_DOWN )
                {
                    kPrintString( i++, 20, vcTemp );             
                    if( vcTemp[ 0 ] == '0' )
                    {
                        bTemp = bTemp / 0;
                    }
                }
            }
        }
    }	
}

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
	kPrintString(19, iY, "OK");
}