//-----------------------------------------------------------------------------
//                            Main  of project CRC
//				  CRC16/CRC32
//				 Martin Dvorak
//                                    1996
//-----------------------------------------------------------------------------
#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

#include "crc.h"

#define CRCDATAFILE "crc.dat"     // filename of the file with data
#define SZ_BUFFER   10000 // small buffer, for testing only
			  // speed test with reading from disk is nonsence
			  // --> change SZ_BUFFER in accordance with filesize

// CRC16/CRC32 has to be set in crc.h
#define TESTCHANGINGPOLNMS

#define MASK1            0x9
#define MASK2            0x6

#ifdef CRC16
 #define CRCPOLY       0x1021
 #define CRCPOLY1      0x8408
 #define INI_CRCVAL    0xFFFF
#else
 #define CRCPOLY       0xEDB88320ul
 #define INI_CRCVAL    0xFFFFFFFFul
#endif

int main( void )
{
 byte buf[SZ_BUFFER];
 int handle, bytes, i, err;
 DATA value;

 printf("\n\nFree memory before CRC:  %lu bytes", (unsigned long) coreleft());

 if ( (handle=_open(CRCDATAFILE, O_RDONLY))<0 ) { perror("Error:"); return 1; }

 if( filelength(handle)>SZ_BUFFER ) { puts("\nFile too big to fit in buffer!" ); exit(1); }

 if ((bytes = _read(handle, buf, filelength(handle) )) < 0) { printf("Read Failed.\n"); exit(1); }

 InitCRC();
 SetCRCVal( INI_CRCVAL, MASK1 | MASK2 );
 #ifdef CRC16
  #ifdef TESTCHANGINGPOLNMS
   SetCRCPoly( 0x1345, MASK1 );
  #endif
  SetCRCPoly( CRCPOLY, MASK1 );
  SetCRCPoly( CRCPOLY1, MASK2 );
 #else
  SetCRCPoly( CRCPOLY, MASK1 );
 #endif

 for( i=0; i<bytes; i++ )
 #ifdef CRC16
  UpdateCRCValByByte( buf[i], MASK1 | MASK2 );
 #else
  UpdateCRCValByByte( buf[i], MASK1 );
 #endif

 #ifdef CRC16
  puts("\n CRC16 calculation results:");
  value=GetCRCVal( 0x1, &err ); CRCerror(err);
  printf(" - slot 0001, polynomial 0x%x , value: 0x%x,", CRCPOLY, value );
  value=GetCRCVal( 0x8, &err ); CRCerror(err);
  printf("\n - slot 1000, polynomial 0x%x , value: 0x%x,", CRCPOLY, value );
  value=GetCRCVal( 0x2, &err ); CRCerror(err);
  printf("\n - slot 0010, polynomial 0x%x , value: 0x%x,", CRCPOLY1, value );
  value=GetCRCVal( 0x4, &err ); CRCerror(err);
  printf("\n - slot 0100, polynomial 0x%x , value: 0x%x,", CRCPOLY1, value );
 #else
  puts("\n CRC32 calculation results:");
  value=GetCRCVal( 0x1, &err ); CRCerror(err);
  printf(" - slot 0001, polynomial 0x%.lx , value: 0x%.lx,", CRCPOLY, value );
  value=GetCRCVal( 0x8, &err ); CRCerror(err);
  printf("\n - slot 1000, polynomial 0x%.lx , value: 0x%.lx,", CRCPOLY, value );
 #endif

 CloseCRC(); // to close old crc and init new crc you can call only InitCRC()
 _close( handle );

 printf("\nFree memory after CRC: %lu bytes", (unsigned long) coreleft());
 return 0;
}

