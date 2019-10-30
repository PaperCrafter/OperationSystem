#include "Page.h"

void kInitializePageTables(void) {
	PML4TENTRY *pstPML4TEntry;
	PDPTENTRY* pstPDPTEntry;
	PDENTRY* pstPDEntry;
	PTENTRY* pstPTEntry;
	DWORD dwMappingAddress;
	int i;


	pstPML4TEntry = (PML4TENTRY*) 0x100000;
	kSetPageEntryData( pstPML4TEntry, 0x00, 0x101000, PAGE_FLAGS_DEFAULT, 0 );
	for ( i = 1; i < PAGE_MAXENTRYCOUNT; i++ ) {
		kSetPageEntryData( pstPML4TEntry + i, 0, 0, 0, 0 );
	}

	pstPDPTEntry = ( PDPTENTRY* ) 0x101000;
	for ( i = 0; i < 64; i++) {
		kSetPageEntryData( pstPDPTEntry + i, 0, 0x102000 + ( i * PAGE_TABLESIZE ), PAGE_FLAGS_DEFAULT, 0 );
	}
	for ( i = 64; i < PAGE_MAXENTRYCOUNT; i++) {
		kSetPageEntryData( pstPDPTEntry + i, 0, 0, 0, 0);
	}

	pstPDEntry = ( PDENTRY* ) 0x102000;
	dwMappingAddress = 0;
	for ( i = 0; i < PAGE_MAXENTRYCOUNT * 64; i++) {
		if(i == 0) kSetPageEntryData(&(pstPDEntry[i]), 0, 0x142000, PAGE_FLAGS_DEFAULT, 0);
		else kSetPageEntryData( pstPDEntry + i, ( i * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0 );
		dwMappingAddress += PAGE_DEFAULTSIZE;
	}
	//0x000A0000 => 0x00000000
	kSetPageEntryData( pstPDEntry + 5, ( 0 * ( PAGE_DEFAULTSIZE >> 20 ) ) >> 12, dwMappingAddress, PAGE_FLAGS_DEFAULT | PAGE_FLAGS_PS, 0 );
	dwMappingAddress += PAGE_DEFAULTSIZE;

	//page direacoty table -> page table(4kb)
	//원래라면 512*64*64를 매핑해줘야 하나 2메가 페이지 하나만 4k로 나누어 주면 되기 때문에 512로 매핑
	pstPTEntry = ( PTENTRY* )0x142000;
	dwMappingAddress = 0;
	for(i = 0; i < PAGE_MAXENTRYCOUNT; i++)
	{
		//pagefault
		//if(i == 511) kSetPageEntryData((pstPTEntry + i), 0, dwMappingAddress, 0, 0);
		//protection fault
		if(i == 511) kSetPageEntryData((pstPTEntry + i), 0, dwMappingAddress, 0, 0);
		else kSetPageEntryData((pstPTEntry + i), 0, dwMappingAddress, PAGE_FLAGS_DEFAULT, 0);
		dwMappingAddress += PAGE_TABLESIZE;
	}
}

void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress, DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperflags ) {
	pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
	pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) | dwUpperflags;
}
