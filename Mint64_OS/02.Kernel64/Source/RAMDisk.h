/**
 *  file    RAMDisk.h
 *  date    2009/05/22
 *  author  kkamagui 
 *  brief   �� ��ũ�� ���õ� �Լ��� ������ ��� ����
 *          Copyright(c)2008 All rights reserved by kkamagui
 */

#ifndef __RAMDISK_H__
#define __RAMDISK_H__

#include "Types.h"
#include "Synchronization.h"
#include "HardDisk.h"

////////////////////////////////////////////////////////////////////////////////
//
// ��ũ��
//
////////////////////////////////////////////////////////////////////////////////
// �� ��ũ�� �� ���� ��. 8Mbyte ũ��� ����
#define RDD_TOTALSECTORCOUNT        ( 8 * 1024 * 1024 / 512)

////////////////////////////////////////////////////////////////////////////////
//
// ����ü
//
////////////////////////////////////////////////////////////////////////////////
// 1����Ʈ�� ����
#pragma pack( push, 1 )

// �� ��ũ�� �ڷᱸ���� �����ϴ� ����ü
typedef struct kRDDManagerStruct
{
    // �� ��ũ������ �Ҵ� ���� �޸� ����
    BYTE* pbBuffer;
    
    // �� ���� ��
    DWORD dwTotalSectorCount;

    // ����ȭ ��ü
    MUTEX stMutex;
} RDDMANAGER;

#pragma pack( pop)

////////////////////////////////////////////////////////////////////////////////
//
//  �Լ�
//
////////////////////////////////////////////////////////////////////////////////
BOOL kInitializeRDD( DWORD dwTotalSectorCount );
BOOL kReadRDDInformation( BOOL bPrimary, BOOL bMaster, 
        HDDINFORMATION* pstHDDInformation );
int kReadRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer );
int kWriteRDDSector( BOOL bPrimary, BOOL bMaster, DWORD dwLBA, int iSectorCount, 
        char* pcBuffer );

#endif /*__RAMDISK_H__*/
