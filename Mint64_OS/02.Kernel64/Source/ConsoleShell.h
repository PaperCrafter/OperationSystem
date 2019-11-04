/**
 *  file    ConsoleShell.h
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ܼ� �п� ���õ� ��� ����
 */

#ifndef __CONSOLESHELL_H__
#define __CONSOLESHELL_H__

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
#define CONSOLESHELL_MAXCOMMANDBUFFERCOUNT  300
#define CONSOLESHELL_PROMPTMESSAGE          "MINT64>"

// ���ڿ� �����͸� �Ķ���ͷ� �޴� �Լ� ������ Ÿ�� ����
typedef void ( * CommandFunction ) ( const char* pcParameter );


////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// ���� Ŀ�ǵ带 �����ϴ� �ڷᱸ��
typedef struct kShellCommandEntryStruct
{
    // Ŀ�ǵ� ���ڿ�
    char* pcCommand;
    // Ŀ�ǵ��� ����
    char* pcHelp;
    // Ŀ�ǵ带 �����ϴ� �Լ��� ������
    CommandFunction pfFunction;
} SHELLCOMMANDENTRY;

// �Ķ���͸� ó���ϱ����� ������ �����ϴ� �ڷᱸ��
typedef struct kParameterListStruct
{
    // �Ķ���� ������ ��巹��
    const char* pcBuffer;
    // �Ķ������ ����
    int iLength;
    // ���� ó���� �Ķ���Ͱ� �����ϴ� ��ġ
    int iCurrentPosition;
} PARAMETERLIST;

#pragma pack( pop )

////////////////////////////////////////////////////////////////////////////////
//
// �Լ�
//
////////////////////////////////////////////////////////////////////////////////
// ���� �� �ڵ�
void kStartConsoleShell( void );
void kExecuteCommand( const char* pcCommandBuffer );
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter );
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter );

// Ŀ�ǵ带 ó���ϴ� �Լ�
void kHelp( const char* pcParameterBuffer );
void kCls( const char* pcParameterBuffer );
void kShowTotalRAMSize( const char* pcParameterBuffer );
void kStringToDecimalHexTest( const char* pcParameterBuffer );
void kShutdown( const char* pcParamegerBuffer );
//hw3-1
void kRaiseFault( const char* pcParamegerBuffer );



#endif /*__CONSOLESHELL_H__*/
