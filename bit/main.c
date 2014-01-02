//-----------------------------------------------------------------------------
//                          Main module of BIT project
//				 Martin Dvorak
//                                    1996
//-----------------------------------------------------------------------------
#include <conio.h>
#include <stdio.h>

#include "bit.h"

// #define DECHEX
// #define DW34
// #define PARITY
#define DWORDTAB

int main( void )
{

 #ifdef DECHEX
  byte j; int i;
  for( j=0; j<255; j++ )
   {
    printf(" Number: %d   ", j);
    i=ByteToHex( j ); printf(" Ascii: %c%c  ", (char)i,*(((char*)&i)+1) );
    i=HexToByte( i ); printf(" Decimal: %d\n", (unsigned char )i);
   }
 #elif defined( DW34 )
  dword i;
  i=dw3Todw4( 0xFFFFFF00ul );
  i=dw4Todw3( i );
 #elif defined( PARITY )
  char p[]={ 163,38,170,255,108 }, i=23, j=23, k;
  k=Parity( p, i, j, EVEN1 );
  printf(" Parity from %d. bit to %d. bit with param. %d is %d\n", i, j, EVEN1, k);
 #elif defined ( DWORDTAB )
  dword i=0xDeFace00ul,j;
  byte tab[64], k; for( k=0; k<=63; k++ ) tab[k]=k;  // initialize tab
  InitTab( tab );
  printf("\n Init 3 bytes: %.lx\n",i);
  j=dw3Tabdw4( i );     printf(" 4bytes: %.lx\n",j);
  i=dw4Tabdw3( j );     printf(" 3bytes: %.lx\n",i);
 #endif

 return 0;
}