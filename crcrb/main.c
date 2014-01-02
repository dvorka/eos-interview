//-----------------------------------------------------------------------------
//                            Main of project CRCRB
//                               Buffered CRC16
//                                   Dvorka
//                                    1996
//-----------------------------------------------------------------------------
/*
  IMPLEMENTACE NENI DOTAZENA, JE TO POUZE PROVIZORNI IMPLEMENTACE NAVRHU,
	  KTERA MA POUZE NASTINIT JAK BY NAVRH MEL FUNGOVAT!

  What's new:
	Crcrb.prj se lisi od crc.prj pouze zpusobem ulozeni a zpracovani
  dat, samotny algoritmus vypoctu crc pro jeden byte je stejny. Cilem
  je aby se dalo najednou pocitat crc a mohla prichazet data ( proto
  jsou buffery kruhove, aby s daty nebylo treba hybat ). Data se
  muzou sazet ( treba pod prerusenim ) za zpracovavanou oblast behem
  vypoctu.
  Data jsou ulozena takto:
  - data se ukladaji do kruhovych bufferu ( byty )
  - na nacitani dat do bufferu je deklarovana specielni funkce CRC_read()
  - kazdy slot  0..15  muze ale nemusi mit vlastni buffer
  - kazdy buffer ma deskriptor, v kterem je pointer na buffer ( ptr )
    velikost bufferu ( size ), index kde se ma zacit pocitat CRC value ( beg )
    kolik bytu se ma zpracovat ( updt ) a index ( in ) ktery se pouziva
    jako index pro plneni bufferu
  - to, co se ma spocitat, se specifikuje pomoci deskriptoru
  - CRC value se opet pocita podle masky, pokud vypocet probehl OK je
    v deskriptorech OK == 0 a index beg se posune za posledni zpracovany
    byte
  - vice bufferu muze sdilet jeden velky buffer, kde  mohou sloty pracovat
    je dano pouze pocatecnim indexem ( beg ) a poctem bytu ( updt )
  - maska se muze nastavovat vzdy 0xFFFF, pokud je totiz v popisovaci
    updt == 0 buffer se preskoci --> nemusi se tedy cekat na naplneni
    bufferu vsech bitu nastavenych v masce a crc se muze pocitat
    prubezne
  - je deklarovana promenna CRCDescriptor ukazujici na pole popisovacu
    jednotlivych bufferu, ktera se inicializuje pomoci CRCInit();
    { klingerovina }
  - v deskriptorech se muze sahat na vsechno ( a to by se nemelo ).
    Melo by se volne pouzivat pouze beg, updt a in. Na ostatni cleny
    struktury by se sahat nemelo.
  - kdyz vim, ze nektere buffery nebudu pouzivat, je mozne je
    neinicializovat, pokud jim nebudu zadavat praci, tak se nic nestane
  - testoval jsem jen CRC16
  - tento main je pouze demonstracni - pokud se nahodne rozhazuji data
    do bufferu vypoctene hodnoty crc nemaji zadny smysl. Chtel jsem
    jenom naznacit jak by knihovna mohla pracovat
  - tento priklad funguje jako normalni crc, ale s cyklickym bufferem
    kdyz se nastavi #define FIRSTSLOT a odstrani #define DATACOMESRANDOM
    nebo pokud se DATACOMESRANDOM necha se musi hlidat pravidelne zpracovani,
    aby buffer nepretekl
    - kdyz nacitam nahodne muze se totiz buffer pretocit ( nacte se vicekrat
      nez se jednou zpracuje )
  - JAK PROGRAM BEZI JE VIDET KDYZ SE ZAPNE #define PRINTEACH V CRCRB.H,
    PRELOZI SE, VYSKOCI SE DO DOSu A SPUSTI SE BATCH _crcrb.bat, KTERY
    DO FILU dvorka2 VYHODI KOMPLETNI VYPIS TOHO, CO SE DEJE

  - co je spatne:
    -  system prace se strukturou CRCDescriptor:
       - predevsim by deskriptory nemely byt primo pristupne
	 ( je napriklad nepripustne menit CRCDescriptor[].ptr jinak nez
	 pomoci CRCInit() a to tady jde )
*/

#include <alloc.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

// please set CRC16 in crcrb.h
#include "crcrb.h"

#define CRCRBDATAFILE "crc.dat"

#define FIRSTSLOT
//#define DATACOMESRANDOM

#ifdef FIRSTSLOT
 #define MASK          0x0001
#else
 #define MASK          0xFFFF
#endif

// length of data readed by CRC_read()
#define LEN           17

#define CRCPOLY       0x1021
#define CRCPOLY1      0x8408
#define INI_CRCVAL    0xFFFF

#define SZ_ROUNDBUF   177

static rbuffer *CRCDescriptor;

int main( void )
{
 int     handle,
	 bytes,
	 i,
	 err,
	 numb;
 word    mask;
 DATA    value;
 rbuffer newrb;

 printf("\n\nFree memory before CRC: %lu bytes", (unsigned long) coreleft());


 CRCDescriptor=InitCRC();               // get pointer to descriptor
 SetCRCPoly( CRCPOLY, MASK );
 SetCRCVal( INI_CRCVAL, MASK );

 // init buffers
 for( i=0, mask=1; i<=15; i++, mask<<=1 )
 {
  if( mask&MASK)
   {
    if( (newrb.ptr=(byte*)malloc(SZ_ROUNDBUF))==NULL ) CRCDispelUndead();
	 newrb.size=SZ_ROUNDBUF;
    AddCRCRB( newrb, i );
   }
 }

 if ((handle = _open(CRCRBDATAFILE, O_RDONLY)) == -1) CRCDispelUndead();

 while ( !eof( handle ) )
 {
  #ifndef FIRSTSLOT
   numb=random( 16 );                       // taking some buffer 0..15
  #else
   numb=0;
  #endif

  if((i=CRC_read(handle, LEN, numb ))==0) break;

  #ifdef DATACOMESRANDOM                    // buffer can overflow!
   if( !random(77) )                        // 1:77 for update
  #endif
		    UpdateCRCValRB( MASK );
		   // beg points to the first byte which isn't updated now
		   // updts are 0
 }
 // update all rests
 UpdateCRCValRB( MASK );

 #ifdef FIRSTSLOT
  puts("\n CRC16 calculation results:");
  mask=GetCRCVal( MASK, &err ); CRCerror(err);
  printf(" - slot 0001, polynomial 0x%x , value: 0x%x,", CRCPOLY, mask );
 #endif

 // CloseCRC() + dealocating buffers ( using descriptor )
 CRCWordOfDeath();

 _close( handle );
 printf("\nFree memory after CRC: %lu bytes", (unsigned long) coreleft());
 return 0;
}






