/**
 *  file    ConsoleShell.c
 *  date    2009/01/31
 *  author  kkamagui 
 *          Copyright(c)2008 All rights reserved by kkamagui
 *  brief   �ܼ� �п� ���õ� �ҽ� ����
 */

#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Task.h"
#include "Synchronization.h"

// Ŀ�ǵ� ���̺� ����
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
        { "help", "Show Help", kHelp },
        { "cls", "Clear Screen", kCls },
        { "totalram", "Show Total RAM Size", kShowTotalRAMSize },
        { "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
        { "shutdown", "Shutdown And Reboot OS", kShutdown },
	    { "raisefault", "Page or Protection fault at 0x1ff000", kRaiseFault },
        { "settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", kSetTimer},
        { "wait", "Wait ms Using PIT, ex)wit 100(ms)", kWaitUsingPIT},
        { "rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter},
        { "cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed},
        { "date", "Show Date And Time", kShowDateAndTime},
        { "createtask", "Create Task ex)createtask 1(type) 10(count)", kCreateTestTask },
	    { "changepriority", "Change Task Priority, ex)changepriority 1(ID) 2(Priority)", kChangeTaskPriority },
        { "tasklist", "Show Task List", kShowTaskList },
        { "killtask", "End Task, ex)killtask 1(ID) or 0xffffffff(All Task)", kKillTask },
        { "cpuload", "Show Processor Load", kCPULoad },
        { "testmutex", "Test Mutex Function", kTestMutex },
        { "testthread", "Test Thread And Process Function", kTestThread },
        { "showmatrix", "Show Matrix Screen", kShowMatrix },
};                                     

//==============================================================================
//  ���� ���� �����ϴ� �ڵ�
//==============================================================================
/**
 *  ���� ���� ����
 */
void kStartConsoleShell( void )
{
    char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];    
    int iCommandBufferIndex = 0;
    BYTE bKey;
    //int iCursorX, iCursorY;
    int iCursorX, iCursorY;

    //prompt
    kPrintf( CONSOLESHELL_PROMPTMESSAGE );
    
    // hw3 variables ============================================================
        char historyBuffer[10][ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
        int historySaveCount = -1;
        int historyCount = 0;
        int historyFull = 0;
        int historyFirstCheck = 0;
        
        int commandNum = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
        int commandCheck[commandNum];
        int commandFind = 0;
        int commandOne = 0;
        int commandMore = 0;
        kMemSet( commandCheck, 0, commandNum );
        kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
    // ============================================================            
        
        
    while( 1 )
    {        
        bKey = kGetCh();
        
        // hw3 history initialize ============================================================
        
        if( bKey != KEY_TAB ) commandMore =0;
        
        // ============================================================
        
        if( bKey == KEY_BACKSPACE )
        {
            if( iCommandBufferIndex > 0 )
            {         
                kGetCursor( &iCursorX, &iCursorY );
                kPrintStringXY( iCursorX - 1, iCursorY, " " );
                kSetCursor( iCursorX - 1, iCursorY );
                
                // fix for hw3
                vcCommandBuffer[ --iCommandBufferIndex ] = '\0';                
            }
        }
        
        else if( bKey == KEY_ENTER )
        {
            kPrintf( "\n" );
            
            if( iCommandBufferIndex > 0 )
            {
                
                vcCommandBuffer[ iCommandBufferIndex ] = '\0';
                
                //hw3 history ============================================================
                
                // history count
                int t=0;
                historySaveCount = (++historySaveCount)%10;   
                historyFull++;               
                historyCount = historySaveCount;
                
                // save
                while(vcCommandBuffer[t] != '\0'){
                historyBuffer[historySaveCount][t] = vcCommandBuffer[t];
                t++;
                }                                              
                
                historyFirstCheck = 0;
                // ============================================================
                
                kExecuteCommand( vcCommandBuffer );
            }            
            
            kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );            
            kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
            iCommandBufferIndex = 0;
        }
        
        else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
                 ( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
                 ( bKey == KEY_SCROLLLOCK ) )
        {
        	;
        }
        
        // HW3 history ============================================================
        else if( bKey == KEY_UP )
        {
        	if( historySaveCount >= 0 )
        	{   
            	// count
            	if( historyFirstCheck==1 ) historyCount--;
            	else historyFirstCheck=1;
            	
            	if( historyFull > 9 && historyCount < 0) historyCount=9;
            	else if(historyCount < 0) historyCount=historySaveCount;
            	
            	// clear
        		kGetCursor( &iCursorX, &iCursorY );
        		while (iCursorX !=0 ){
        			kPrintStringXY( --iCursorX, iCursorY, " " );
        		}   
        		
        		// initialize
        		kSetCursor( 0, iCursorY );
        		iCommandBufferIndex = 0;
        		kPrintf( CONSOLESHELL_PROMPTMESSAGE );
                
        		// print & copy
        		int t= 0;
        		while(historyBuffer[historyCount][t] != '\0'){
        			vcCommandBuffer[iCommandBufferIndex++] = historyBuffer[historyCount][t];
        			kPrintf( "%c", historyBuffer[historyCount][t++] );
        		}         	        	
        	}
        }
        
        else if( bKey == KEY_DOWN )
        {        	
        	
        	if( historySaveCount >= 0 )
            {   
        		// count
        		if( historyFirstCheck==1 ) historyCount++; 
        		else historyFirstCheck=1;         	
        		        	
        		if( historyFull > 9 && historyCount > 9) historyCount=0;
        		else if( historyFull <= 9 && historyCount > historySaveCount) historyCount=0;
        		
        		// clear
        		kGetCursor( &iCursorX, &iCursorY );
        		while (iCursorX !=0 ){
        			kPrintStringXY( --iCursorX, iCursorY, " " );
        		}
        		
        		// initialize
        		kSetCursor( 0, iCursorY );
        		iCommandBufferIndex = 0;
        		kPrintf( CONSOLESHELL_PROMPTMESSAGE );
        		
        		// print & copy
        		int t= 0;
        		while(historyBuffer[historyCount][t] != '\0'){
        			vcCommandBuffer[iCommandBufferIndex++] = historyBuffer[historyCount][t];
        			kPrintf( "%c", historyBuffer[historyCount][t++] );
            	}             
            }
        }
        
        // tap key
        else if( bKey == KEY_TAB )        	
        {                   	
        	int i;
        	if( iCommandBufferIndex > 0 )
        	{                 		
        		  
        		int iCommandLength;
        		
        		//length check
        		for( i=0; i<commandNum;i++) {
        			iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );        			
        			if(iCommandLength<(iCommandBufferIndex-1)) {
        				commandCheck[i]=2;        				
        			}
        		}
        		
        		//same check
        		for( i=0; i<commandNum ;i++ ) {
        			int t=0;
        			
        			if(commandCheck[i]==2) continue;
        			
        			while (1){        				        				        				
        				if(vcCommandBuffer[t]=='\0'){
        					commandCheck[i]=1;
        					commandOne = i;
        					commandFind++;        					
        					break;
        				}
        				
        				if(gs_vstCommandTable[ i ].pcCommand[t]!=vcCommandBuffer[t]){        					
        					break;
        				}
        				t++;
        			}         			
        		}
        		
        		// one case
        		if(commandFind==1){      
        			iCommandLength = kStrLen( gs_vstCommandTable[ commandOne ].pcCommand );
        			for(i=0;i<iCommandLength;i++){
        				if(gs_vstCommandTable[ commandOne ].pcCommand[i]!=vcCommandBuffer[i]){
        					vcCommandBuffer[iCommandBufferIndex++] = gs_vstCommandTable[ commandOne ].pcCommand[i];
        					kPrintf( "%c", gs_vstCommandTable[ commandOne ].pcCommand[i] );        					
        				}
        			}        			        			
        		}
        		
        		// more cases        		
        		else if(commandFind>1){        			        			        		
        			if(commandMore==1){
						kPrintf( "%\n");
						for(i=0;i<commandNum;i++){
							if(commandCheck[i]==1) kPrintf( "%s ", gs_vstCommandTable[ i ].pcCommand);        				
						}
						kPrintf( "%\n");
						kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );
						kPrintf( "%s", vcCommandBuffer);
        			}
        			commandMore=(commandMore+1)%2;
        		}
            }
        	
        	// initialize        	
        	for(i=0;i<commandNum;i++){
        		commandCheck[ i ]=0;       				
        	} 
        	if(commandFind<=1) commandMore=0;
        	commandOne=0;
        	commandFind=0; 
        }
        
        // ============================================================
        
        else
        {            
            if( iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
            {
                vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
                kPrintf( "%c", bKey );
            }
        }
    }
}

/*
 *  Ŀ�ǵ� ���ۿ� �ִ� Ŀ�ǵ带 ���Ͽ� �ش� Ŀ�ǵ带 ó���ϴ� �Լ��� ����
 */
void kExecuteCommand( const char* pcCommandBuffer )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    // �������� ���е� Ŀ�ǵ带 ����
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    // Ŀ�ǵ� ���̺��� �˻��ؼ� ������ �̸��� Ŀ�ǵ尡 �ִ��� Ȯ��
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        // Ŀ�ǵ��� ���̿� ������ ������ ��ġ�ϴ��� �˻�
        if( ( iCommandLength == iSpaceIndex ) &&
            ( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
                       iSpaceIndex ) == 0 ) )
        {
            gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
            break;
        }
    }

    // ����Ʈ���� ã�� �� ���ٸ� ���� ���
    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
    }
}

/**
 *  �Ķ���� �ڷᱸ���� �ʱ�ȭ
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}

/**
 *  �������� ���е� �Ķ������ ����� ���̸� ��ȯ
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    // �� �̻� �Ķ���Ͱ� ������ ����
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    
    // ������ ���̸�ŭ �̵��ϸ鼭 ������ �˻�
    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    // �Ķ���͸� �����ϰ� ���̸� ��ȯ
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

    // �Ķ������ ��ġ ������Ʈ
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    
//==============================================================================
//  Ŀ�ǵ带 ó���ϴ� �ڵ�
//==============================================================================
/**
 *  �� ������ ���
 */
static void kHelp( const char* pcCommandBuffer )
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
    kPrintf( "=========================================================\n" );
    kPrintf( "                    MINT64 Shell Help                    \n" );
    kPrintf( "=========================================================\n" );
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

    // ���� �� Ŀ�ǵ��� ���̸� ���
    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    
    // ���� ���
    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}

/**
 *  ȭ���� ���� 
 */
static void kCls( const char* pcParameterBuffer )
{
    // �� ������ ����� ������ ����ϹǷ� ȭ���� ���� ��, ���� 1�� Ŀ�� �̵�
    kClearScreen();
    kSetCursor( 0, 1 );
}

/**
 *  �� �޸� ũ�⸦ ���
 */
static void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  ���ڿ��� �� ���ڸ� ���ڷ� ��ȯ�Ͽ� ȭ�鿡 ���
 */
static void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    // �Ķ���� �ʱ�ȭ
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 )
    {
        // ���� �Ķ���͸� ����, �Ķ������ ���̰� 0�̸� �Ķ���Ͱ� ���� ���̹Ƿ�
        // ����
        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        // �Ķ���Ϳ� ���� ������ ����ϰ� 16�������� 10�������� �Ǵ��Ͽ� ��ȯ�� ��
        // ����� printf�� ���
        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

        // 0x�� �����ϸ� 16����, �׿ܴ� 10������ �Ǵ�
        if( kMemCmp( vcParameter, "0x", 2 ) == 0 )
        {
            lValue = kAToI( vcParameter + 2, 16 );
            kPrintf( "HEX Value = %q\n", lValue );
        }
        else
        {
            lValue = kAToI( vcParameter, 10 );
            kPrintf( "Decimal Value = %d\n", lValue );
        }
        
        iCount++;
    }
}

/**
 *  PC�� �����(Reboot)
 */
static void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    // Ű���� ��Ʈ�ѷ��� ���� PC�� �����
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

static void kRaiseFault( const char* pcParamegerBuffer )
{
	kPrintf( "\n" );
	kPrintf( "\n" );
	kPrintf( "\n" );

	long* ptr = 0x1ff000;
        *ptr = 0;
	
}
static void kSetTimer( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    PARAMETERLIST stList;
    long lValue;
    BOOL bPeriodic;

    // �Ķ���� �ʱ�ȭ
    kInitializeParameter( &stList, pcParameterBuffer );
    
    // milisecond ����
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
        return ;
    }
    lValue = kAToI( vcParameter, 10 );

    // Periodic ����
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
        return ;
    }    
    bPeriodic = kAToI( vcParameter, 10 );
    
    kInitializePIT( MSTOCOUNT( lValue ), bPeriodic );
    kPrintf( "Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic );
}

/**
 *  PIT ��Ʈ�ѷ��� ���� ����Ͽ� ms ���� ���  
 */
static void kWaitUsingPIT( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    long lMillisecond;
    int i;
    
    // �Ķ���� �ʱ�ȭ
    kInitializeParameter( &stList, pcParameterBuffer );
    if( kGetNextParameter( &stList, vcParameter ) == 0 )
    {
        kPrintf( "ex)wait 100(ms)\n" );
        return ;
    }
    
    lMillisecond = kAToI( pcParameterBuffer, 10 );
    kPrintf( "%d ms Sleep Start...\n", lMillisecond );
    
    // ���ͷ�Ʈ�� ��Ȱ��ȭ�ϰ� PIT ��Ʈ�ѷ��� ���� ���� �ð��� ����
    kDisableInterrupt();
    for( i = 0 ; i < lMillisecond / 30 ; i++ )
    {
        kWaitUsingDirectPIT( MSTOCOUNT( 30 ) );
    }
    kWaitUsingDirectPIT( MSTOCOUNT( lMillisecond % 30 ) );   
    kEnableInterrupt();
    kPrintf( "%d ms Sleep Complete\n", lMillisecond );
    
    // Ÿ�̸� ����
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );
}

/**
 *  Ÿ�� ������ ī���͸� ����  
 */
static void kReadTimeStampCounter( const char* pcParameterBuffer )
{
    QWORD qwTSC;
    
    qwTSC = kReadTSC();
    kPrintf( "Time Stamp Counter = %q\n", qwTSC );
}

/**
 *  ���μ����� �ӵ��� ����
 */
static void kMeasureProcessorSpeed( const char* pcParameterBuffer )
{
    int i;
    QWORD qwLastTSC, qwTotalTSC = 0;
        
    kPrintf( "Now Measuring." );
    
    // 10�� ���� ��ȭ�� Ÿ�� ������ ī���͸� �̿��Ͽ� ���μ����� �ӵ��� ���������� ����
    kDisableInterrupt();    
    for( i = 0 ; i < 200 ; i++ )
    {
        qwLastTSC = kReadTSC();
        kWaitUsingDirectPIT( MSTOCOUNT( 50 ) );
        qwTotalTSC += kReadTSC() - qwLastTSC;

        kPrintf( "." );
    }
    // Ÿ�̸� ����
    kInitializePIT( MSTOCOUNT( 1 ), TRUE );    
    kEnableInterrupt();
    
    kPrintf( "\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000 );
}

/**
 *  RTC ��Ʈ�ѷ��� ����� ���� �� �ð� ������ ǥ��
 */
static void kShowDateAndTime( const char* pcParameterBuffer )
{
    BYTE bSecond, bMinute, bHour;
    BYTE bDayOfWeek, bDayOfMonth, bMonth;
    WORD wYear;

    // RTC ��Ʈ�ѷ����� �ð� �� ���ڸ� ����
    kReadRTCTime( &bHour, &bMinute, &bSecond );
    kReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );
    
    kPrintf( "Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth,
             kConvertDayOfWeekToString( bDayOfWeek ) );
    kPrintf( "Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}


//TCB자료구조와 스택 정의
static TCB gs_vstTask[ 2 ] = { 0, };
static QWORD gs_vstStack[ 1024 ] = { 0, };

/**
 *  �½�ũ ��ȯ�� �׽�Ʈ�ϴ� �½�ũ
 */
static void kTestTask( void )
{
    int i = 0;
    
    while( 1 )
    {
        // �޽����� ����ϰ� Ű �Է��� ���
        kPrintf( "[%d] This message is from kTestTask. Press any key to switch "
                 "kConsoleShell~!!\n", i++ );
        kGetCh();
        
        // ������ Ű�� �ԷµǸ� �½�ũ�� ��ȯ
        kSwitchContext( &( gs_vstTask[ 1 ].stContext ), &( gs_vstTask[ 0 ].stContext ) );
    }
}


/**
 *  �½�ũ 1
 *      ȭ�� �׵θ��� ���鼭 ���ڸ� ���
 */
static void kTestTask1( void )
{
    BYTE bData;
    int i = 0, iX = 0, iY = 0, iMargin, j;
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    TCB* pstRunningTask;
    
    // �ڽ��� ID�� �� ȭ�� ���������� ���
    pstRunningTask = kGetRunningTask();
    iMargin = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) % 10;
    

    // ȭ�� �� �����̸� ���鼭 ���� ���
    for(int j =0; j < 20000; j++)
    {
        switch( i )
        {
        case 0:
            iX++;
            if( iX >= ( CONSOLE_WIDTH - iMargin ) )
            {
                i = 1;
            }
            break;
            
        case 1:
            iY++;
            if( iY >= ( CONSOLE_HEIGHT - iMargin ) )
            {
                i = 2;
            }
            break;
            
        case 2:
            iX--;
            if( iX < iMargin )
            {
                i = 3;
            }
            break;
            
        case 3:
            iY--;
            if( iY < iMargin )
            {
                i = 0;
            }
            break;
        }
        
        // ���� �� ���� ����
        pstScreen[ iY * CONSOLE_WIDTH + iX ].bCharactor = bData;
        pstScreen[ iY * CONSOLE_WIDTH + iX ].bAttribute = bData & 0x0F;
        bData++;
        
        // �ٸ� �½�ũ�� ��ȯ
        //kSchedule();
    }
    kExitTask();
}

/**
 *  �½�ũ 2
 *      �ڽ��� ID�� �����Ͽ� Ư�� ��ġ�� ȸ���ϴ� �ٶ����� ���
 */
static void kTestTask2( void )
{
    int i = 0, iOffset;
    CHARACTER* pstScreen = ( CHARACTER* ) CONSOLE_VIDEOMEMORYADDRESS;
    TCB* pstRunningTask;
    char vcData[ 4 ] = { '-', '\\', '|', '/' };
    
    // �ڽ��� ID�� �� ȭ�� ���������� ���
    pstRunningTask = kGetRunningTask();
    iOffset = ( pstRunningTask->stLink.qwID & 0xFFFFFFFF ) * 2;
    iOffset = CONSOLE_WIDTH * CONSOLE_HEIGHT - 
        ( iOffset % ( CONSOLE_WIDTH * CONSOLE_HEIGHT ) );

    while( 1 )
    {
        // ȸ���ϴ� �ٶ����� ǥ��
        pstScreen[ iOffset ].bCharactor = vcData[ i % 4 ];
        // ���� ����
        pstScreen[ iOffset ].bAttribute = ( iOffset % 15 ) + 1;
        i++;
        
        // �ٸ� �½�ũ�� ��ȯ
        //kSchedule();
    }
}

/**
 *  �½�ũ�� �����ؼ� ��Ƽ �½�ŷ ����
 */
static void kCreateTestTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcType[ 30 ];
    char vcCount[ 30 ];
    int i;
    
    // �Ķ���͸� ����
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcType );
    kGetNextParameter( &stList, vcCount );

    switch( kAToI( vcType, 10 ) )
    {
    // Ÿ�� 1 �½�ũ ����
    case 1:
        for( i = 0 ; i < kAToI( vcCount, 10 ) ; i++ )
        {    
            if( kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask1 ) == NULL )
            {
                break;
            }
        }
        
        kPrintf( "Task1 %d Created\n", i );
        break;
        
    // Ÿ�� 2 �½�ũ ����
    case 2:
    default:
        for( i = 0 ; i < kAToI( vcCount, 10 ) ; i++ )
        {    
            if( kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 ) == NULL )
            {
                break;
            }
        }
        
        kPrintf( "Task2 %d Created\n", i );
        break;
    }    
}   
 

/**
 *  �½�ũ�� �켱 ������ ����
 */
static void kChangeTaskPriority( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    char vcPriority[ 30 ];
    QWORD qwID;
    BYTE bPriority;
    
    // �Ķ���͸� ����
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    kGetNextParameter( &stList, vcPriority );
    
    // �½�ũ�� �켱 ������ ����
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    
    bPriority = kAToI( vcPriority, 10 );
    
    kPrintf( "Change Task Priority ID [0x%q] Priority[%d] ", qwID, bPriority );
    if( kChangePriority( qwID, bPriority ) == TRUE )
    {
        kPrintf( "Success\n" );
    }
    else
    {
        kPrintf( "Fail\n" );
    }
}

/**
 *  ���� ������ ��� �½�ũ�� ������ ���
 */
static void kShowTaskList( const char* pcParameterBuffer )
{
    int i;
    TCB* pstTCB;
    int iCount = 0;
    
    kPrintf( "=========== Task Total Count [%d] ===========\n", kGetTaskCount() );
    for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
    {
        // TCB�� ���ؼ� TCB�� ��� ���̸� ID�� ���
        pstTCB = kGetTCBInTCBPool( i );
        if( ( pstTCB->stLink.qwID >> 32 ) != 0 )
        {
            // �½�ũ�� 10�� ��µ� ������, ��� �½�ũ ������ ǥ������ ���θ� Ȯ��
            if( ( iCount != 0 ) && ( ( iCount % 10 ) == 0 ) )
            {
                kPrintf( "Press any key to continue... ('q' is exit) : " );
                if( kGetCh() == 'q' )
                {
                    kPrintf( "\n" );
                    break;
                }
                kPrintf( "\n" );
            }
            
            kPrintf( "[%d] Task ID[0x%Q], Priority[%d], Flags[0x%Q], Thread[%d]\n", 1 + iCount++,
                     pstTCB->stLink.qwID, GETPRIORITY( pstTCB->qwFlags ), 
                     pstTCB->qwFlags, kGetListCount( &( pstTCB->stChildThreadList ) ) );
            kPrintf( "    Parent PID[0x%Q], Memory Address[0x%Q], Size[0x%Q]\n",
                    pstTCB->qwParentProcessID, pstTCB->pvMemoryAddress, pstTCB->qwMemorySize );
        }
    }
}

/**
 *  �½�ũ�� ����
 */
static void kKillTask( const char* pcParameterBuffer )
{
    PARAMETERLIST stList;
    char vcID[ 30 ];
    QWORD qwID;
    TCB* pstTCB;
    int i;
    
    // �Ķ���͸� ����
    kInitializeParameter( &stList, pcParameterBuffer );
    kGetNextParameter( &stList, vcID );
    
    // �½�ũ�� ����
    if( kMemCmp( vcID, "0x", 2 ) == 0 )
    {
        qwID = kAToI( vcID + 2, 16 );
    }
    else
    {
        qwID = kAToI( vcID, 10 );
    }
    
    // Ư�� ID�� �����ϴ� ���
    if( qwID != 0xFFFFFFFF )
    {
        pstTCB = kGetTCBInTCBPool( GETTCBOFFSET( qwID ) );
        qwID = pstTCB->stLink.qwID;

        // �ý��� �׽�Ʈ�� ����
        if( ( ( qwID >> 32 ) != 0 ) && ( ( pstTCB->qwFlags & TASK_FLAGS_SYSTEM ) == 0x00 ) )
        {
            kPrintf( "Kill Task ID [0x%q] ", qwID );
            if( kEndTask( qwID ) == TRUE )
            {
                kPrintf( "Success\n" );
            }
            else
            {
                kPrintf( "Fail\n" );
            }
        }
        else
        {
            kPrintf( "Task does not exist or task is system task\n" );
        }
    }
    // �ܼ� �а� ���� �½�ũ�� �����ϰ� ��� �½�ũ ����
    else
    {
        for( i = 0 ; i < TASK_MAXCOUNT ; i++ )
        {
            pstTCB = kGetTCBInTCBPool( i );
            qwID = pstTCB->stLink.qwID;

            // �ý��� �׽�Ʈ�� ���� ��Ͽ��� ����
            if( ( ( qwID >> 32 ) != 0 ) && ( ( pstTCB->qwFlags & TASK_FLAGS_SYSTEM ) == 0x00 ) )
            {
                kPrintf( "Kill Task ID [0x%q] ", qwID );
                if( kEndTask( qwID ) == TRUE )
                {
                    kPrintf( "Success\n" );
                }
                else
                {
                    kPrintf( "Fail\n" );
                }
            }
        }
    }
}

/**
 *  ���μ����� ������ ǥ��
 */
static void kCPULoad( const char* pcParameterBuffer )
{
    kPrintf( "Processor Load : %d%%\n", kGetProcessorLoad() );
}
    
// ���ؽ� �׽�Ʈ�� ���ؽ��� ����
static MUTEX gs_stMutex;
static volatile QWORD gs_qwAdder;

/**
 *  ���ؽ��� �׽�Ʈ�ϴ� �½�ũ
 */
static void kPrintNumberTask( void )
{
    int i;
    int j;
    QWORD qwTickCount;

    // 50ms ���� ����Ͽ� �ܼ� ���� ����ϴ� �޽����� ��ġ�� �ʵ��� ��
    qwTickCount = kGetTickCount();
    while( ( kGetTickCount() - qwTickCount ) < 50 )
    {
        kSchedule();
    }    
    
    // ������ ���鼭 ���ڸ� ���
    for( i = 0 ; i < 5 ; i++ )
    {
        kLock( &( gs_stMutex ) );
        kPrintf( "Task ID [0x%Q] Value[%d]\n", kGetRunningTask()->stLink.qwID,
                gs_qwAdder );
        
        gs_qwAdder += 1;
        kUnlock( & ( gs_stMutex ) );
    
        // ���μ��� �Ҹ� �ø����� �߰��� �ڵ�
        for( j = 0 ; j < 30000 ; j++ ) ;
    }
    
    // ��� �½�ũ�� ������ ������ 1��(100ms) ���� ���
    qwTickCount = kGetTickCount();
    while( ( kGetTickCount() - qwTickCount ) < 1000 )
    {
        kSchedule();
    }    
    
    // �½�ũ ����
    kExitTask();
}

/**
 *  ���ؽ��� �׽�Ʈ�ϴ� �½�ũ ����
 */
static void kTestMutex( const char* pcParameterBuffer )
{
    int i;
    
    gs_qwAdder = 1;
    
    // ���ؽ� �ʱ�ȭ
    kInitializeMutex( &gs_stMutex );
    
    for( i = 0 ; i < 3 ; i++ )
    {
        // ���ؽ��� �׽�Ʈ�ϴ� �½�ũ�� 3�� ����
        kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kPrintNumberTask );
    }    
    kPrintf( "Wait Util %d Task End...\n", i );
    kGetCh();
}

/**
 *  �½�ũ 2�� �ڽ��� ������� �����ϴ� �½�ũ
 */
static void kCreateThreadTask( void )
{
    int i;
    
    for( i = 0 ; i < 3 ; i++ )
    {
        kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_THREAD, 0, 0, ( QWORD ) kTestTask2 );
    }
    
    while( 1 )
    {
        kSleep( 1 );
    }
}

/**
 *  �����带 �׽�Ʈ�ϴ� �½�ũ ����
 */
static void kTestThread( const char* pcParameterBuffer )
{
    TCB* pstProcess;
    
    pstProcess = kCreateTask( TASK_FLAGS_LOW | TASK_FLAGS_PROCESS, ( void * )0xEEEEEEEE, 0x1000, 
                              ( QWORD ) kCreateThreadTask );
    if( pstProcess != NULL )
    {
        kPrintf( "Process [0x%Q] Create Success\n", pstProcess->stLink.qwID ); 
    }
    else
    {
        kPrintf( "Process Create Fail\n" );
    }
}

// ������ �߻���Ű�� ���� ����
static volatile QWORD gs_qwRandomValue = 0;

/**
 *  ������ ������ ��ȯ
 */
QWORD kRandom( void )
{
    gs_qwRandomValue = ( gs_qwRandomValue * 412153 + 5571031 ) >> 16;
    return gs_qwRandomValue;
}

/**
 *  ö�ڸ� �귯������ �ϴ� ������
 */
static void kDropCharactorThread( void )
{
    int iX, iY;
    int i;
    char vcText[ 2 ] = { 0, };

    iX = kRandom() % CONSOLE_WIDTH;
    
    while( 1 )
    {
        // ��� �����
        kSleep( kRandom() % 20 );
        
        if( ( kRandom() % 20 ) < 16 )
        {
            vcText[ 0 ] = ' ';
            for( i = 0 ; i < CONSOLE_HEIGHT - 1 ; i++ )
            {
                kPrintStringXY( iX, i , vcText );
                kSleep( 50 );
            }
        }        
        else
        {
            for( i = 0 ; i < CONSOLE_HEIGHT - 1 ; i++ )
            {
                vcText[ 0 ] = i + kRandom();
                kPrintStringXY( iX, i, vcText );
                kSleep( 50 );
            }
        }
    }
}

/**
 *  �����带 �����Ͽ� ��Ʈ���� ȭ��ó�� �����ִ� ���μ���
 */
static void kMatrixProcess( void )
{
    int i;
    
    for( i = 0 ; i < 300 ; i++ )
    {
        if( kCreateTask( TASK_FLAGS_THREAD | TASK_FLAGS_LOW, 0, 0, 
                         ( QWORD ) kDropCharactorThread ) == NULL )
        {
            break;
        }
        
        kSleep( kRandom() % 5 + 5 );
    }
    
    kPrintf( "%d Thread is created\n", i );

    // Ű�� �ԷµǸ� ���μ��� ����
    kGetCh();
}

/**
 *  ��Ʈ���� ȭ���� ������
 */
static void kShowMatrix( const char* pcParameterBuffer )
{
    TCB* pstProcess;
    
    pstProcess = kCreateTask( TASK_FLAGS_PROCESS | TASK_FLAGS_LOW, ( void* ) 0xE00000, 0xE00000, 
                              ( QWORD ) kMatrixProcess );
    if( pstProcess != NULL )
    {
        kPrintf( "Matrix Process [0x%Q] Create Success\n" );

        // �½�ũ�� ���� �� ������ ���
        while( ( pstProcess->stLink.qwID >> 32 ) != 0 )
        {
            kSleep( 100 );
        }
    }
    else
    {
        kPrintf( "Matrix Process Create Fail\n" );
    }
}
