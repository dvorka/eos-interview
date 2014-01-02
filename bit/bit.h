//-----------------------------------------------------------------------------
//				   Header of
//                        UU comp., bit parity, Dec/Hex ..
//				 Martin Dvorak
//                                    1996
//-----------------------------------------------------------------------------
#ifndef __BIT_H
 #define __BIT_H

 // actions for Parity()
 #define EVEN0 00
 #define ODD0  01
 #define EVEN1 10
 #define ODD1  11

 typedef unsigned char byte;
 typedef unsigned int  word;
 typedef unsigned long dword;

  word  ByteToHex( byte in );
  byte  HexToByte( word in );
  dword dw3Todw4( dword in );
  dword dw4Todw3( dword in );
  void  InitTab( byte *p );
  dword dw3Tabdw4( dword in );
  dword dw4Tabdw3( dword in );
  // max src[] length is 64kB
  int   Parity( byte *src, dword lbeg, dword lend, byte action );

#endif