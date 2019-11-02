
#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"

// command table
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
        { "help", "Show Help", kHelp },
        { "cls", "Clear Screen", kCls },
        { "totalram", "Show Total RAM Size", kShowTotalRAMSize },
        { "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
        { "shutdown", "Shutdown And Reboot OS", kShutdown },
        { "raisefault", "Rais Fault at 0x1ff000", kRaisFault},        
        { "a", "testing", kStringToDecimalHexTest },
		{ "ab", "testing", kStringToDecimalHexTest },
		{ "abc", "testing", kStringToDecimalHexTest },
		{ "hint", "testing", kStringToDecimalHexTest },
		{ "temp", "testing", kStringToDecimalHexTest },
		{ "cant", "testing", kStringToDecimalHexTest },
};      


// roof
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


void kExecuteCommand( const char* pcCommandBuffer )
{
    int i, iSpaceIndex;
    int iCommandBufferLength, iCommandLength;
    int iCount;
    
    iCommandBufferLength = kStrLen( pcCommandBuffer );
    for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
    {
        if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
        {
            break;
        }
    }
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
    for( i = 0 ; i < iCount ; i++ )
    {
        iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );

        if( ( iCommandLength == iSpaceIndex ) &&
            ( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
                       iSpaceIndex ) == 0 ) )
        {
            gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
            break;
        }
    }


    if( i >= iCount )
    {
        kPrintf( "'%s' is not found.\n", pcCommandBuffer );
    }
}


void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
    pstList->pcBuffer = pcParameter;
    pstList->iLength = kStrLen( pcParameter );
    pstList->iCurrentPosition = 0;
}


int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
    int i;
    int iLength;

    
    if( pstList->iLength <= pstList->iCurrentPosition )
    {
        return 0;
    }
    

    for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
    {
        if( pstList->pcBuffer[ i ] == ' ' )
        {
            break;
        }
    }
    
    
    kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
    iLength = i - pstList->iCurrentPosition;
    pcParameter[ iLength ] = '\0';

 
    pstList->iCurrentPosition += iLength + 1;
    return iLength;
}
    


void kHelp( const char* pcCommandBuffer )
{
    int i;
    int iCount;
    int iCursorX, iCursorY;
    int iLength, iMaxCommandLength = 0;
    
    
    kPrintf( "=========================================================\n" );
    kPrintf( "                    MINT64 Shell Help                    \n" );
    kPrintf( "=========================================================\n" );
    
    iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );


    for( i = 0 ; i < iCount ; i++ )
    {
        iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
        if( iLength > iMaxCommandLength )
        {
            iMaxCommandLength = iLength;
        }
    }
    

    for( i = 0 ; i < iCount ; i++ )
    {
        kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
        kGetCursor( &iCursorX, &iCursorY );
        kSetCursor( iMaxCommandLength, iCursorY );
        kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
    }
}



void kCls( const char* pcParameterBuffer )
{
    kClearScreen();
    kSetCursor( 0, 1 );
}


void kShowTotalRAMSize( const char* pcParameterBuffer )
{
    kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}


void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
    char vcParameter[ 100 ];
    int iLength;
    PARAMETERLIST stList;
    int iCount = 0;
    long lValue;
    
    kInitializeParameter( &stList, pcParameterBuffer );
    
    while( 1 )
    {

        iLength = kGetNextParameter( &stList, vcParameter );
        if( iLength == 0 )
        {
            break;
        }

        kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
                 vcParameter, iLength );

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

void kShutdown( const char* pcParamegerBuffer )
{
    kPrintf( "System Shutdown Start...\n" );
    
    kPrintf( "Press Any Key To Reboot PC..." );
    kGetCh();
    kReboot();
}

void kRaisFault( const char* pcParameterBuffer ){
    kPrintf( "\n" );
    kPrintf( "\n");
    kPrintf( "\n");
    kPrintf( "\n" );
    writeTo(0x1ff000, 18);
}



