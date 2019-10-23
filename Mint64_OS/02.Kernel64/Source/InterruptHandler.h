#ifndef __INTERRUPTHANDLER_H__
#define __INTERRUPTHANDLER_H__

#include "Types.h"


void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void kPageFaultExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void kProtectionFaultExceptionHandler( int iVectorNumber, QWORD qwErrorCode );
void kCommonInterruptHandler( int iVectorNumber );
void kKeyboardHandler( int iVectorNumber );

#endif /*__INTERRUPTHANDLER_H__*/
