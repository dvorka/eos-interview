//-----------------------------------------------------------------------------
//				 Buffered CRC16
//				     Dvorka
//                                    1996
//-----------------------------------------------------------------------------
#include <alloc.h>
#include <io.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>

#include "crcrb.h"

#define  OK        0     // functions returns 0 if success
#define  TRUE      1
#define  FALSE     !TRUE
#define  SZ_CRCTAB 257
#define  COUNTER   256   // index in CRCtab[] where is stored number of
			 // pointers which points to it
#ifdef CRC16
 #define MK_TAB    0x8000         // 1<<15
#else
 #define MK_TAB    0x80000000ul   // 1<<31
#endif

static word  CRCconv[]=    // array which uses BinSearch to convert 16-bit
			   // value to array index - GetCRCVal()
		       { 1, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7,
			 1<<8, 1<<9, 1<<10, 1<<11, 1<<12, 1<<14, 1<<15
		       };
static DATA CRCval[16],       // array of CRC values
	    CRCpoly[16],      // array of CRC polynomials
	    *CRCtab[]=        // array of pointers to tabs
		       { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static rbuffer CRCdata[16];   // array of buffer descriptors


rbuffer *InitCRC( void  )
{
 byte i;
 setmem( CRCpoly, 16*sizeof(DATA), 0 );
 setmem( CRCval , 16*sizeof(DATA), 0 );
 setmem( CRCdata, 16*sizeof(rbuffer), 0 );
 for( i=0; i<=15; i++ )
   if( CRCtab[i] ) // exists >=1
    { if( !(--CRCtab[i][COUNTER]) ) free(CRCtab[i]); CRCtab[i]=0;}

 return CRCdata;
}


int  AddCRCRB( rbuffer nbuf, byte number )
{
 CRCdata[number]=nbuf;
 if( CRCdata[number].ptr==NULL ) return NOBUF;
 CRCdata[number].beg=0;
 CRCdata[number].updt=0;
 CRCdata[number].in=0;
 #ifdef PRINTEACH
  printf("\nFree memory:  %lu bytes, buffer number: %d", (unsigned long) coreleft(), number);
 #endif
 return OK;
}


void CRCDispelUndead( void )
{
 byte i;
 for( i=0; i<=15; i++ )
  if( CRCtab[i] )
   {
    // release tabs
    if( !(--CRCtab[i][COUNTER]) ) free( CRCtab[i] ); CRCtab[i]=0;
    // release buffers
    if( CRCdata[i].ptr ) free( CRCdata[i].ptr );
   }
 puts("\nError: program terminated!");
 exit( 1 );
}


int SetCRCPoly( DATA npol, word mask )
{
 word  i, j;
 DATA  val, dat;
 DATA  *actualtab=NULL; // pointer to the CRCtab of npol
 byte  found, di, dj;

 if( !mask ) return BADMASK;

 for( i=1, di=0; di<=15; i<<=1, di++ )
  if( i&mask )
   {
    if(!(CRCpoly[di]==npol && CRCtab[i])) // if doesn't exist the same pol with tab
     {
      // free old tab if exists
      if( CRCtab[di] )      // exist >= 1
	if( !(--CRCtab[di][COUNTER]) ) free(CRCtab[di]);

      CRCtab[di]=0; // there is no tab now

      if( actualtab ) // tab was created or exists
       { CRCtab[di]=actualtab; CRCtab[di][COUNTER]++; CRCpoly[di]=npol; }
      else
       {
	// searching the same polynomial
	found=FALSE;
	for( j=0; j<=15; j++ )
	 if( CRCpoly[j]==npol )
	  { actualtab=CRCtab[di]=CRCtab[j]; CRCtab[j][COUNTER]++;
		  CRCpoly[di]=npol; found=TRUE; break;  }

	if( !found ) // making new one
	 {
	  if( (CRCtab[di]=(DATA*)malloc(SZ_CRCTAB*sizeof(DATA)))==NULL ) return NOMEM;
	  for ( dat=0; dat<=255; dat++ )
	   {
	    for ( j=0, val=dat<<(DATASIZE-8); j<=7; j++ )
	     val=val&MK_TAB?(val<<1)^npol:val<<1;
	    CRCtab[di][dat]=val; // save new value
	   }
	  CRCtab[di][COUNTER]=1; // there is 1 pointer now
	  CRCpoly[di]=npol;
	  actualtab=CRCtab[di];
	 } // make
       }   // else
     }     // !=
   }
 return OK;
}


int SetCRCVal( DATA nval, word mask )
{
 word j=1; byte i;
 if( mask )
  {  for( i=0; i<=15; i++,j<<=1 )
      if(j&mask) CRCval[i]=nval; return OK; }
 else return BADMASK;
}


DATA GetCRCVal( word mask, int *err )
{
 byte beg=0, end=0xF, i;
 if( !((mask-1)&mask) && mask )      // ONE bit is set
  {
   // searching index by BinSearch
   do
    { i=(beg+end)>>1; if( mask>CRCconv[i] ) beg=i+1; else end=i-1; }
   while( CRCconv[i]!=mask );
   return CRCval[i];
  }
 else { *err=BADMASK; return BADMASK; }
}


void UpdateCRCValByByte( byte val, word mask )
{
 word i; byte j;
 for( i=1, j=0; j<=15 ; i<<=1, j++ )
  if( i&mask )
   CRCval[j]=(CRCval[j]<<8) ^ CRCtab[j][(CRCval[j]>>(DATASIZE-8))^val];

 #ifdef PRINTEACH
   printf("%x\n",CRCval[0]);
 #endif
}


void UpdateCRCValRB( word mask )
{
 byte j;
 word i, k, m, rest;  // number of bytes!
 dword sum;

 for( i=1, j=0; j<=15 ; i<<=1, j++ )
  if( i&mask && CRCdata[j].updt )
   {
    #ifdef PRINTEACH
     printf("\nCRC: updating: %d bytes,",CRCdata[j].updt );
    #endif

    sum=CRCdata[j].beg; sum+=CRCdata[j].updt;  // Rage Against The Overflow

    if( sum > CRCdata[j].size )
     // two fragments ; '+' can overflow
     {
      rest = sum - CRCdata[j].size;
      m = CRCdata[j].updt - rest;     // m, rest: how many bytes

      for( k=CRCdata[j].beg; m>0; m--, k++)
       CRCval[j]=(CRCval[j]<<8) ^ CRCtab[j][(CRCval[j]>>(DATASIZE-8))^CRCdata[j].ptr[k]];

      if( rest ) // nutne ?
       for( k=0; k<rest; k++)
	CRCval[j]=(CRCval[j]<<8) ^ CRCtab[j][(CRCval[j]>>(DATASIZE-8))^CRCdata[j].ptr[k]];
     }
    else
      for( k=CRCdata[j].beg; CRCdata[j].updt>0; CRCdata[j].updt--, k++)
       CRCval[j]=(CRCval[j]<<8) ^ CRCtab[j][(CRCval[j]>>(DATASIZE-8))^CRCdata[j].ptr[k]];
    // k points behind updated
    CRCdata[j].beg= k==CRCdata[j].size?0:k;
    CRCdata[j].updt=0;
    #ifdef PRINTEACH
     printf("buf: %d, value %x", j, CRCval[j]);
    #endif
   }
}


int CRC_read( int handle, unsigned len, byte bufn )
{
  word i, r, rest;
  dword sum;

    sum=CRCdata[bufn].in; sum+=len;  // Rage Against The Overflow
    // in points to free byte, must be filled
    if( sum > CRCdata[bufn].size )
     {   // two fragments
      rest = sum - CRCdata[bufn].size;
      i = len - rest;     // i, r: how many bytes

      r=_read( handle, CRCdata[bufn].ptr+CRCdata[bufn].in, i );
       if( r==0xFFFF ) return r;  // error
       if( r!=i ) { CRCdata[bufn].updt+=r; CRCdata[bufn].in += r; return r; }; // reads && eof
      i=_read( handle, CRCdata[bufn].ptr, rest );
       if( i==0xFFFF ) return i;     // error
      CRCdata[bufn].in = i;    // in poinst to free
      CRCdata[bufn].updt+=r+i; // inc number of data to update

      #ifdef PRINTEACH
       printf("\nCRC_READ reads to %d .buffer %d bytes, total: %d, in %d", bufn, len, CRCdata[bufn].updt, CRCdata[bufn].in );
      #endif
      return (r+i);
     }
    else // one fragment
     {
      r=_read( handle, CRCdata[bufn].ptr+CRCdata[bufn].in, len );
      // read some or eof         II
      if( r!=0xFFFF )          // \/ situation: in==0; len==size
       CRCdata[bufn].in += r==CRCdata[bufn].size?0:r;
       CRCdata[bufn].updt+=r;  // inc number of data to update

      #ifdef PRINTEACH
       printf("\nCRC_READ reads to %d .buffer %d bytes, total: %d, in %d", bufn, len, CRCdata[bufn].updt, CRCdata[bufn].in );
      #endif
      return r;
     }
}


void CloseCRC( void )
{
 byte i;
 for( i=0; i<=15; i++ )
  if( CRCtab[i] )
   { if( !(--CRCtab[i][COUNTER]) ) free( CRCtab[i] ); CRCtab[i]=0; }
 setmem( CRCpoly, 16*sizeof(DATA), 0 );
 setmem( CRCval , 16*sizeof(DATA), 0 );
}


void CRCWordOfDeath( void )
{
 byte i;
 for( i=0; i<=15; i++ )
  if( CRCtab[i] )
   {
    if( !(--CRCtab[i][COUNTER]) ) free( CRCtab[i] ); CRCtab[i]=0;
    if( CRCdata[i].ptr ) free( CRCdata[i].ptr );
   }
 setmem( CRCpoly, 16*sizeof(DATA), 0 );
 setmem( CRCval , 16*sizeof(DATA), 0 );
 setmem( CRCdata, 16*sizeof(rbuffer), 0 );
}


void CRCerror( int val )
{
  switch( val ) {
   case NOMEM  : puts(" CRCerror: Not enought memory!"); break;
   case BADMASK: puts(" CRCerror: Given mask has no sense!"); break;
   case NOBUF  : puts(" CRCerror: Pointer to specified buffer is NULL!"); break;
   case OK     : /* success */ break;
  }
}



