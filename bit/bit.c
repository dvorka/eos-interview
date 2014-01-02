//-----------------------------------------------------------------------------
//                        UU comp., bit parity, Dec/Hex ..
//				  Martin Dvorak
//                                    1996
//-----------------------------------------------------------------------------
#pragma inline

#include "bit.h"

#define I asm

typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned long dword;

static union { word   w; struct x { byte lo:8; byte hi:8; }b;} Wrd;
static byte *tab,
	    invtab[256],	       // initialized by InitTab();
	    hex[]={'0','1','2','3','4','5','6','7',
		   '8','9','A','B','C','D','E','F'};

word ByteToHex( byte in )
{ Wrd.b.lo=hex[in>>4];Wrd.b.hi=hex[in&0xF];return (Wrd.w); }


byte HexToByte( word in )
{
 return (((in&0x00FF)>'9'? ((in&0x00FF)-'A'+10)<<4: ((in&0x00FF)-'0')<<4)|
	 ((in&0xFF00)>'9'<<8? (in>>8)-'A'+10 :(in>>8)-'0'));
}


dword dw3Todw4( dword in )
{
 return ( (in&0x700ul)>>4|(in&0x3F800ul)>>3|
	  (in&0x1FC0000ul)>>2|(in&0xFE000000ul)>>1 );
}


dword dw4Todw3( dword in )
{
 return ((in&0x7F000000ul)<<1|(in&0x7F0000ul)<<2|(in&0x7F00)<<3|(in&0x70)<<4);
}


void InitTab( byte *ntab )
{
 byte i; tab=ntab; for( i=0; i<=63; i++ ) invtab[tab[i]]=i; // create inv. tab
}


dword dw3Tabdw4( dword in )
{

 dword i=(in&0xFC000000ul)>>2|(in&0x03F00000ul)>>4|
	 (in&0x000FC000ul)>>6|(in&0x00003F00ul)>>8;
       in=tab[*(((char*)&i)+3)]<<8 | tab[*(((char*)&i)+2)];
       in<<=16;
 return ( in | tab[*(((char*)&i)+1)]<<8 | tab[*((char*)&i)] );
}


dword dw4Tabdw3( dword in )
{
 dword i=invtab[(in&0xFF000000ul)>>24]<<8 | invtab[(in&0xFF0000ul)>>16] ;
       i<<=16;
       i|=invtab[(in&0xFF00)>>8]<<8 | invtab[in&0xFF];
 return ( (i&0x3F000000ul)<<2 | (i&0x003F0000ul)<<4 |
	  (i&0x00003F00ul)<<6 | (i&0x0000003Ful)<<8 );
}


int Parity( byte *src, dword lbeg, dword lend, byte action )
{
 int  true=1, false=0;
 word bytes=(word)(lend>>3)-(word)(lbeg>>3);
 word lbeglo=(byte)--lbeg&7,   // number of bits in the first byte
      lendlo=(byte)--lend&7;   // number of bits in the last byte
			       // decreasing because of: 1..8 --> 0..7
      src+=(lbeg+1)>>3;

 if( (((lend+1)-(lbeg+1))&1) && (action==ODD0 || action==EVEN0) ) // odd length
  { true=0; false=1; }         // (--lend---lbeg)&1 == (lend-lbeg)&1

 // lbeg and lend in the same byte
 if( (lbeg>>3)==(lend>>3) )
  {
   I   MOV BX, src
   I   MOV AL, [BX]
   I   MOV CL, lbeglo
   I   SHL AL, CL   // left
   I   MOV DL, 7
   I   SUB DL, lendlo
   I   ADD CL, DL
   I   SHR AL, CL  // back&right
   I   TEST AL, 0xFF
   I   JP jparity
       return false;
   jparity:
       return true;
  }

 // two and more bytes
 I PUSH DI
 I PUSH SI         // save SI, DI
 I MOV DI, bytes
 I MOV BX, src
 I MOV SI, DI

 // the first byte
 I MOV AL, [BX]
 I MOV CL, lbeglo
 I SHL AL, CL
 I TEST AL, 0xFF
 I PUSHF
 I POP DX

 I TEST DI, 0xFFFF
 I JZ end

 // whole bytes
 again:
 I DEC DI
 I JZ last
 I TEST BYTE PTR [BX][DI], 0xFF
 I JP again
 I    NOT DL
 I JMP again

 last:
 I TEST BYTE PTR [BX+SI], 0xFF
 I JZ end
 I    MOV AL, [BX+SI]
 I    MOV CL, 7
 I    SUB CL, lendlo
 I    JZ end;        	         // it was whole byte
 I    SHR AL, CL
 I    TEST AL, 0xFF
 I    JP end
 I    NOT DL

 end:
 I POP SI
 I POP DI
 I AND DL, 4
 I JNZ parity
    return false;
 parity:
    return true;
}

