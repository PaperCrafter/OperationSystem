/**
 *  file    PIT.c
 *  date    2009/02/08
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   PIT ��Ʈ�ѷ��� ���õ� ��� ����
 */

#include "PIT.h"

/*
 *  PIT�� �ʱ�ȭ
 */
void kInitializePIT( WORD wCount, BOOL bPeriodic )
{
    // PIT ��Ʈ�� ��������(��Ʈ 0x43)�� ���� �ʱ�ȭ�Ͽ� ī��Ʈ�� ���� �ڿ�
    // ��� 0�� ���̳ʸ� ī���ͷ� ����
    kOutPortByte( PIT_PORT_CONTROL, PIT_COUNTER0_ONCE );
    
    // ���� ������ �ֱ�� �ݺ��ϴ� Ÿ�̸Ӷ�� ��� 2�� ����
    if( bPeriodic == TRUE )
    {
        // PIT ��Ʈ�� ��������(��Ʈ 0x43)�� ��� 2�� ���̳ʸ� ī���ͷ� ����
        kOutPortByte( PIT_PORT_CONTROL, PIT_COUNTER0_PERIODIC );
    }    
    
    // ī���� 0(��Ʈ 0x40)�� LSB -> MSB ������ ī���� �ʱ� ���� ����
    kOutPortByte( PIT_PORT_COUNTER0, wCount );
    kOutPortByte( PIT_PORT_COUNTER0, wCount >> 8 );
}

/**
 *  ī���� 0�� ���� ���� ��ȯ
 */
WORD kReadCounter0( void )
{
    BYTE bHighByte, bLowByte;
    WORD wTemp = 0;
    
    // PIT ��Ʈ�� ��������(��Ʈ 0x43)�� ��ġ Ŀ�ǵ带 �����Ͽ� ī���� 0�� �ִ�
    // ���� ���� ����
    kOutPortByte( PIT_PORT_CONTROL, PIT_COUNTER0_LATCH );
    
    // ī���� 0(��Ʈ 0x40)���� LSB -> MSB ������ ī���� ���� ����
    bLowByte = kInPortByte( PIT_PORT_COUNTER0 );
    bHighByte = kInPortByte( PIT_PORT_COUNTER0 );

    // ���� ���� 16��Ʈ�� ���Ͽ� ��ȯ
    wTemp = bHighByte;
    wTemp = ( wTemp << 8 ) | bLowByte;
    return wTemp;
}

/**
 *  ī���� 0�� ���� �����Ͽ� ���� �ð� �̻� ���
 *      �Լ��� ȣ���ϸ� PIT ��Ʈ�ѷ��� ������ �ٲ�Ƿ�, ���Ŀ� PIT ��Ʈ�ѷ��� �缳��
 *      �ؾ� ��
 *      ��Ȯ�ϰ� �����Ϸ��� �Լ� ��� ���� ���ͷ�Ʈ�� ��Ȱ��ȭ �ϴ� ���� ����
 *      �� 50ms���� ���� ����
 */
void kWaitUsingDirectPIT( WORD wCount )
{
    WORD wLastCounter0;
    WORD wCurrentCounter0;
    
    // PIT ��Ʈ�ѷ��� 0~0xFFFF���� �ݺ��ؼ� ī�����ϵ��� ����
    kInitializePIT( 0, TRUE );
    
    // ���ݺ��� wCount �̻� ������ ������ ���
    wLastCounter0 = kReadCounter0();
    while( 1 )
    {
        // ���� ī���� 0�� ���� ��ȯ
        wCurrentCounter0 = kReadCounter0();
        if( ( ( wLastCounter0 - wCurrentCounter0 ) & 0xFFFF ) >= wCount )
        {
            break;
        }
    }
}
