#include "Types.h"

#define TRUE ((void *) 1)

void kPrintString(int iX, int iY, const char* pcString);
BOOL kInitializeKernel64Area( void );
BOOL kIsMemoryEnough( void );

//Main function
void Main(void){
	
	kPrintString( 0, 5, "C Language Kernel Start.....................[Pass]" );

	    //check minimum memory size

		kPrintString( 0, 6, "Minimum Memory Size Check...................[    ]" );
	        if( kIsMemoryEnough() == FALSE )
	        {
	            kPrintString( 45, 6, "Fail" );
	            kPrintString( 0, 7, "Not Enough Memory~!! MINT64 OS Requires Over "
	                    "64Mbyte Memory~!!" );
	            while( 1 ) ;
	        }
	        else
	        {
	            kPrintString( 45, 6, "Pass" );
	        }

	    //initialization
	    kPrintString( 0, 7, "IA-32e Kernel Area Initialize...............[    ]" );
	    if( kInitializeKernel64Area() == FALSE )
	    {
	    	kPrintString( 45, 7, "Fail" );
	    	kPrintString( 0, 8, "Kernel Area Initialization Fail~!!" );
	    	while( 1 ) ;
	    }
	    kPrintString( 45, 7, "Pass" );


    while(1);    
}

//functin printing string
void kPrintString(int iX, int iY, const char* pcString){
    CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
    int i;
    
    pstScreen +=(iY*80) + iX;
    for(i = 0; pcString[i] != 0; i++){
        pstScreen[i].bCharactor = pcString[i];
    }
}

//initializing
BOOL kInitializeKernel64Area( void )
{
    DWORD* pdwCurrentAddress;

    pdwCurrentAddress = ( DWORD* ) 0x100000;

    while( ( DWORD ) pdwCurrentAddress < 0x600000 )
    {
        *pdwCurrentAddress = 0x00;

      if( *pdwCurrentAddress != 0 )
      {
    	  return FALSE;
      }

      pdwCurrentAddress++;
    }
    return TRUE;
}

//checking memory
BOOL kIsMemoryEnough( void )
{
    DWORD* pdwCurrentAddress;
    pdwCurrentAddress = ( DWORD* ) 0x100000;

    while( ( DWORD ) pdwCurrentAddress < 0x4000000 )
    {
        *pdwCurrentAddress = 0x12345678;

        if( *pdwCurrentAddress != 0x12345678 )
        {
           return FALSE;
        }

        pdwCurrentAddress += ( 0x100000 / 4 );
    }
    return TRUE;
}

