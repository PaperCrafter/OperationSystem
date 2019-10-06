#include "Types.h"

void kPrintString( int iX, int iY, const char* pcString );
int Main(void) {
	char vcTemp[2] = {0,};
	BYTE bFlags;
	BYTE bTemp;
	int i = 0;
	kPrintString(0, 12, "Switch To IA-32e Mode Success~!!");
	kPrintString(0, 13, "IA-32e C Language Kernel Start..............[Pass]");
	
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
