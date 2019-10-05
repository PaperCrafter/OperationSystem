
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define O_BINARY 0
#define BYTESOFSECTOR  512

// �Լ� ����
int AdjustInSectorSize( int iFd, int iSourceSize );
void WriteKernelInformation( int iTargetFd, int iTotalKernelSectorCount, 
        int iKernel32SectorCount );
int CopyFile( int iSourceFd, int iTargetFd );

/**
 *  Main �Լ�
*/
int main(int argc, char* argv[])
{
    int iSourceFd;
    int iTargetFd;
    int iBootLoaderSize;
    int iKernel32SectorCount;
    int iKernel64SectorCount;
    int iSourceSize;
        
    // Ŀ�ǵ� ���� �ɼ� �˻�
    if( argc < 4 )
    {
        fprintf( stderr, "[ERROR] ImageMaker.exe BootLoader.bin Kernel32.bin Kernel64.bin\n" );
        exit( -1 );
    }
    
    // Disk.img ������ ����
    if( ( iTargetFd = open( "Disk.img", O_RDWR | O_CREAT |  O_TRUNC |
            O_BINARY, S_IREAD | S_IWRITE ) ) == -1 )
    {
        fprintf( stderr , "[ERROR] Disk.img open fail.\n" );
        exit( -1 );
    }

    //--------------------------------------------------------------------------
    // ��Ʈ �δ� ������ ��� ��� ������ ��ũ �̹��� ���Ϸ� ����
    //--------------------------------------------------------------------------
    printf( "[INFO] Copy boot loader to image file\n" );
    if( ( iSourceFd = open( argv[ 1 ], O_RDONLY | O_BINARY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 1 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    // ���� ũ�⸦ ���� ũ���� 512����Ʈ�� ���߱� ���� ������ �κ��� 0x00 ���� ä��
    iBootLoaderSize = AdjustInSectorSize( iTargetFd , iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
            argv[ 1 ], iSourceSize, iBootLoaderSize );

    //--------------------------------------------------------------------------
    // 32��Ʈ Ŀ�� ������ ��� ��� ������ ��ũ �̹��� ���Ϸ� ����
    //--------------------------------------------------------------------------
    printf( "[INFO] Copy protected mode kernel to image file\n" );
    if( ( iSourceFd = open( argv[ 2 ], O_RDONLY | O_BINARY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 2 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    // ���� ũ�⸦ ���� ũ���� 512����Ʈ�� ���߱� ���� ������ �κ��� 0x00 ���� ä��
    iKernel32SectorCount = AdjustInSectorSize( iTargetFd, iSourceSize );
    printf( "[INFO] %s size = [%d] and sector Count = [%d]\n",
                argv[ 2 ], iSourceSize, iKernel32SectorCount );

    //--------------------------------------------------------------------------
    // 64��Ʈ Ŀ�� ������ ��� ��� ������ ��ũ �̹��� ���Ϸ� ����
    //--------------------------------------------------------------------------
    printf( "[INFO] Copy IA-32e mode kernel to image file\n" );
    if( ( iSourceFd = open( argv[ 3 ], O_RDONLY | O_BINARY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 3 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    // ���� ũ�⸦ ���� ũ���� 512����Ʈ�� ���߱� ���� ������ �κ��� 0x00 ���� ä��
    iKernel64SectorCount = AdjustInSectorSize( iTargetFd, iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
                argv[ 3 ], iSourceSize, iKernel64SectorCount );
    
    //--------------------------------------------------------------------------
    // ��ũ �̹����� Ŀ�� ������ ����
    //--------------------------------------------------------------------------
    printf( "[INFO] Start to write kernel information\n" );
    // ��Ʈ������ 5��° ����Ʈ���� Ŀ�ο� ���� ������ ����
    WriteKernelInformation( iTargetFd, iKernel32SectorCount + iKernel64SectorCount,
            iKernel32SectorCount );
    printf( "[INFO] Image file create complete\n" );

    close( iTargetFd );
    return 0;
}

/**
 *  ���� ��ġ���� 512����Ʈ ��� ��ġ���� ���߾� 0x00���� ä��
*/
int AdjustInSectorSize( int iFd, int iSourceSize )
{
    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;
    
    if( iAdjustSizeToSector != 0 )
    {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        printf( "[INFO] File size [%lu] and fill [%u] byte\n", iSourceSize, 
            iAdjustSizeToSector );
        for( i = 0 ; i < iAdjustSizeToSector ; i++ )
        {
            write( iFd , &cCh , 1 );
        }
    }
    else
    {
        printf( "[INFO] File size is aligned 512 byte\n" );
    }
    
    // ���� ���� �ǵ�����
    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTESOFSECTOR;
    return iSectorCount;
}

/**
 *  ��Ʈ �δ��� Ŀ�ο� ���� ������ ����
*/
void WriteKernelInformation( int iTargetFd, int iTotalKernelSectorCount, 
        int iKernel32SectorCount )
{
    unsigned short usData;
    long lPosition;
    
    // ������ ���ۿ��� 5����Ʈ ������ ��ġ�� Ŀ���� �� ���� �� ������ ��Ÿ��
    lPosition = lseek( iTargetFd, 5, SEEK_SET );
    if( lPosition == -1 )
    {
        fprintf( stderr, "lseek fail. Return value = %d, errno = %d, %d\n", 
            lPosition, errno, SEEK_SET );
        exit( -1 );
    }
    
    // ��Ʈ �δ��� ������ �� ���� �� �� ��ȣ ��� Ŀ���� ���� �� ����
    usData = ( unsigned short ) iTotalKernelSectorCount;
    write( iTargetFd, &usData, 2 );
    usData = ( unsigned short ) iKernel32SectorCount;
    write( iTargetFd, &usData, 2 );

    printf( "[INFO] Total sector count except boot loader [%d]\n", 
        iTotalKernelSectorCount );
    printf( "[INFO] Total sector count of protected mode kernel [%d]\n", 
        iKernel32SectorCount );
}

/**
 *  �ҽ� ����(Source FD)�� ������ ��ǥ ����(Target FD)�� �����ϰ� �� ũ�⸦ �ǵ�����
*/
int CopyFile( int iSourceFd, int iTargetFd )
{
    int iSourceFileSize;
    int iRead;
    int iWrite;
    char vcBuffer[ BYTESOFSECTOR ];

    iSourceFileSize = 0;
    while( 1 )
    {
        iRead   = read( iSourceFd, vcBuffer, sizeof( vcBuffer ) );
        iWrite  = write( iTargetFd, vcBuffer, iRead );

        if( iRead != iWrite )
        {
            fprintf( stderr, "[ERROR] iRead != iWrite.. \n" );
            exit(-1);
        }
        iSourceFileSize += iRead;
        
        if( iRead != sizeof( vcBuffer ) )
        {
            break;
        }
    }
    return iSourceFileSize;
} 
