//-----------------------------------------------------------------------------
//                                 Header of
//				 Buffered CRC16
//				    Dvorka
//				     1996
//-----------------------------------------------------------------------------
#ifndef __CRCRB_H
 #define __CRCRB_H

 #define CRC16
// #define CRC32

// #define PRINTEACH

 // flags
 #define  NOMEM    -1
 #define  BADMASK  -2
 #define  NOBUF    -3

 typedef unsigned char byte;
 typedef unsigned int  word;
 typedef unsigned long dword;
 // round buffer structure
 typedef struct x
  {
   byte *ptr;     // pointer to buffer
   word size;     // it's size
   word beg;      // index of the first which isn't updated
   word updt;     // how many (.ptr[beg] included )
   word in;       // index for adding data to buffer ( points to free byte )
  } rbuffer;

 #ifdef CRC16
  #define DATA     word
  #define DATASIZE 16
 #else
  #define DATA     dword
  #define DATASIZE 32
 #endif

 // all the functions which returns int returns errorlevel in it,
 // except GetCRCVal which returns errorlevel in parameter err
 // and CRC_read() which is the same as _read()
 // values:  OK == 0 if OK
 //             <  0 if ªOK

 rbuffer *InitCRC( void );
   // - initialize all the data
   // - return pointer to tab of descriptors
 int  AddCRCRB( rbuffer nbuf, byte number );
   // - adds one descriptor to tab
 void CRCDispelUndead( void );
   // - call if some fatal error occured such as I/O error ...
   // - dealocates what's possible and terminates program with value 1
   // - can be used if crc is initialized
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
 void UpdateCRCValRB( word mask );
   // - calculates crc value in round buffers
 int CRC_read( int handle, unsigned len, byte bufn );
  // - function reads data to round buffer using its descriptor
  // - returns the same values as _read()
 void CloseCRC( void );
   // - frees CRC dynamic data structures ( tabs )
 void CRCWordOfDeath( void );
   // - the same as CloseCRC() + releases all buffers
   // - can be used for int atexit(atexit_t func);
 void CRCerror( int err );
   // - prints a CRC error message

#endif