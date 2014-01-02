//-----------------------------------------------------------------------------
//                                 Header of
//                                CRC16/CRC32
//                               Martin Dvorak
//                                   1996
//-----------------------------------------------------------------------------
#ifndef __CRC_H
 #define __CRC_H

 #define CRC16
// #define CRC32

 #define  NOMEM    -1
 #define  BADMASK  -2

 typedef unsigned char byte;
 typedef unsigned int  word;
 typedef unsigned long dword;

 #ifdef CRC16
  #define DATA     word
  #define DATASIZE 16
 #else
  #define DATA     dword
  #define DATASIZE 32
 #endif

 // all the functions which return int return errorlevel in it,
 // except GetCRCVal which returns errorlevel in parameter err

 void InitCRC( void );
   // - initialize all the data
   // - has the same effect as InitCRC() and CloseCRC()
 int  SetCRCPoly( DATA npol, word mask );
   // - sets  new polynomials
   // - is optimized to memory
 int  SetCRCVal( DATA nval, word mask );
   // - sets crc value
 DATA GetCRCVal( word mask, int *err );
   // - brings ONE crc value
   // - is optimized to speed
 void UpdateCRCValByByte( byte val, word mask );
   // - calculates crc value
 void CloseCRC( void );
   // - frees all dynamic data structures
 void CRCerror( int err );
   // - prints a CRC error message

#endif