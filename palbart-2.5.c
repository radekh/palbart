/******************************************************************************/
/*                                                                            */
/* Program:  PAL                                                              */
/* File:     pal.c                                                            */
/* Author:   Gary A. Messenbrink                                              */
/*           gam@rahul.net                                                    */
/*                                                                            */
/* Purpose:  A 2 pass PDP-8 pal-like assembler.                               */
/*                                                                            */
/* PAL(1)                                                                     */
/*                                                                            */
/* NAME                                                                       */
/*    pal - a PDP/8 pal-like assembler.                                       */
/*                                                                            */
/* SYNOPSIS:                                                                  */
/*    pal [ -d -l -p -r -x ] inputfile                                        */
/*                                                                            */
/* DESCRIPTION                                                                */
/*    This is a cross-assembler to for PDP/8 assembly language programs.      */
/*    It will produce an output file in bin format, rim format, and using the */
/*    appropriate pseudo-ops, a combination of rim and bin formats.           */
/*    A listing file is always produced and with an optional symbol table     */
/*    and/or a symbol cross-reference (concordance).  The permanent symbol    */
/*    table can be output in a form that may be read back in so a customized  */
/*    permanent symbol table can be produced.  Any detected errors are output */
/*    to a separate file giving the filename in which they were detected      */
/*    along with the line number, column number and error message as well as  */
/*    marking the error in the listing file.                                  */
/*    The following file name extensions are used:                            */
/*       .pal    source code (input)                                          */
/*       .lst    assembly listing (output)                                    */
/*       .bin    assembly output in DEC's bin format (output)                 */
/*       .rim    assembly output in DEC's rim format (output)                 */
/*       .err    assembly errors detected (if any) (output)                   */
/*       .prm    permanent symbol table in form suitable for reading after    */
/*               the EXPUNGE pseudo-op.                                       */
/*                                                                            */
/* OPTIONS                                                                    */
/*    -d   Dump the symbol table at end of assembly                           */
/*    -l   Allow generation of literals (default is no literal generation)    */
/*    -p   Generate a file with the permanent symbols in it.                  */
/*         (To get the current symbol table, assemble a file than has only    */
/*          a $ in it.)                                                       */
/*    -r   Produce output in rim format (default is bin format)               */
/*    -x   Generate a cross-reference (concordance) of user symbols.          */
/*                                                                            */
/* DIAGNOSTICS                                                                */
/*    Assembler error diagnostics are output to an error file and inserted    */
/*    in the listing file.  Each line in the error file has the form          */
/*                                                                            */
/*       <filename>(<line>:<col>) : error:  <message> at Loc = <loc>          */
/*                                                                            */
/*    An example error message is:                                            */
/*                                                                            */
/*       bintst.pal(17:9) : error:  undefined symbol "UNDEF" at Loc = 07616   */
/*                                                                            */
/*    The error diagnostics put in the listing start with a two character     */
/*    error code (if appropriate) and a short message.  A carat '^' is        */
/*    placed under the item in error if appropriate.                          */
/*    An example error message is:                                            */
/*                                                                            */
/*          17 07616 3000          DCA     UNDEF                              */
/*       UD undefined                      ^                                  */
/*          18 07617 1777          TAD  I  DUMMY                              */
/*                                                                            */
/*    When an indirect is generated, an at character '@' is placed after the  */
/*    the instruction value in the listing as an indicator as follows:        */
/*                                                                            */
/*          14 03716 1777@         TAD     OFFPAG                             */
/*                                                                            */
/*    Undefined symbols are marked in the symbol table listing by prepending  */
/*    a '?' to the symbol.  Redefined symbols are marked in the symbol table  */
/*    listing by prepending a '#' to the symbol.  Examples are:               */
/*                                                                            */
/*       #REDEF   04567                                                       */
/*        SWITCH  07612                                                       */
/*       ?UNDEF   00000                                                       */
/*                                                                            */
/*    Refer to the code for the diagnostic messages generated.                */
/*                                                                            */
/* BUGS                                                                       */
/*    Only a minimal effort has been made to keep the listing format          */
/*    anything like the PAL-8 listing format.                                 */
/*    The operation of the conditional assembly pseudo-ops may not function   */
/*    exactly as the DEC versions.  I did not have any examples of these so   */
/*    the implementation is my interpretation of how they should work.        */
/*                                                                            */
/*    The RIMPUNch and BINPUNch pseudo-ops do not change the binary output    */
/*    file type that was specified on startup.  This was intentional and      */
/*    and allows rim formatted data to be output prior to the actual binary   */
/*    formatted data.  On UN*X style systems, the same effect can be achieved */
/*    by using the "cat" command, but on DOS/Windows systems, doing this was  */
/*    a major chore.                                                          */
/*                                                                            */
/*    The floating point input does not generate values exactly as the DEC    */
/*    compiler does.  I worked out several examples by hand and believe that  */
/*    this implementation is slightly more accurate.  If I am mistaken,       */
/*    let me know and, if possible, a better method of generating the values. */
/*                                                                            */
/* BUILD and INSTALLATION                                                     */
/*    This program has been built and successfully executed on:               */
/*      a.  Linux (80486 CPU)using gcc                                        */
/*      b.  RS/6000 (AIX 3.2.5)                                               */
/*      c.  Borland C++ version 3.1  (large memory model)                     */
/*      d.  Borland C++ version 4.52 (large memory model)                     */
/*    with no modifications to the source code.                               */
/*                                                                            */
/*    On UNIX type systems, store the the program as the pal command          */
/*    and on PC type systems, store it as pal.exe                             */
/*                                                                            */
/* REFERENCES:                                                                */
/*    This assembler is based on the pal assember by:                         */
/*       Douglas Jones <jones@cs.uiowa.edu> and                               */
/*       Rich Coon <coon@convexw.convex.com>                                  */
/*                                                                            */
/* DISCLAIMER:                                                                */
/*    See the symbol table for the set of pseudo-ops supported.               */
/*    See the code for pseudo-ops that are not standard for PDP/8 assembly.   */
/*    Refer to DEC's "Programming Languages (for the PDP/8)" for complete     */
/*    documentation of pseudo-ops.                                            */
/*    Refer to DEC's "Introduction to Programming (for the PDP/8)" or a       */
/*    lower level introduction to the assembly language.                      */
/*                                                                            */
/* WARRANTY:                                                                  */
/*    If you don't like it the way it works or if it doesn't work, that's     */
/*    tough.  You're welcome to fix it yourself.  That's what you get for     */
/*    using free software.                                                    */
/*                                                                            */
/* COPYRIGHT NOTICE:                                                          */
/*    This is free software.  There is no fee for using it.  You may make     */
/*    any changes that you wish and also give it away.  If you can make       */
/*    a commercial product out of it, fine, but do not put any limits on      */
/*    the purchaser's right to do the same.  If you improve it or fix any     */
/*    bugs, it would be nice if you told me and offered me a copy of the      */
/*    new version.                                                            */
/*                                                                            */
/*                                                                            */
/* Amendments Record:                                                         */
/*  Version  Date    by   Comments                                            */
/*  ------- -------  ---  --------------------------------------------------- */
/*    v1.0  12Apr96  GAM  Original                                            */
/*    v1.1  18Nov96  GAM  Permanent symbol table initialization error.        */
/*    v1.2  20Nov96  GAM  Added BINPUNch and RIMPUNch pseudo-operators.       */
/*    v1.3  24Nov96  GAM  Added DUBL pseudo-op (24 bit integer constants).    */
/*    v1.4  29Nov96  GAM  Fixed bug in checksum generation.                   */
/*    v2.1  08Dec96  GAM  Added concordance processing (cross reference).     */
/*    v2.2  10Dec96  GAM  Added FLTG psuedo-op (floating point constants).    */
/*    v2.3   2Feb97  GAM  Fixed paging problem in cross reference output.     */
/*    v2.4  11Apr97  GAM  Fixed problem with some labels being put in cross   */
/*                        reference multiple times.                           */
/*    v2.5  14Aug10  RFH  Added leader length parameter to RIMPUNch and       */
/*                        BINPUNch (also special value -1)                    */
/*                                                                            */
/******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *release = "pal-2.5, 14 August 2010";

#define LINELEN              96
#define LIST_LINES_PER_PAGE  55         /* Includes 5 line page header.       */
#define NAMELEN             128
#define SYMBOL_COLUMNS        5
#define SYMLEN                7
#define SYMBOL_TABLE_SIZE  1024
#define TITLELEN             63
#define XREF_COLUMNS          8

#define ADDRESS_FIELD  00177
#define FIELD_FIELD   070000
#define INDIRECT_BIT   00400
#define LAST_PAGE_LOC  00177
#define OP_CODE        07000
#define PAGE_BIT       00200

#ifdef  PAGE_SIZE
#undef  PAGE_SIZE
#endif
#define PAGE_SIZE      00200

#define PAGE_FIELD     07600
#define PAGE_ZERO_END  00200

/* Macro to get the number of elements in an array.                           */
#define DIM(a) (sizeof(a)/sizeof(a[0]))

/* Macro to get the address plus one of the end of an array.                  */
#define BEYOND(a) ((a) + DIM(A))

#define is_blank(c) ((c==' ') || (c=='\t') || (c=='\f') || (c=='>'))
#define isend(c)   ((c=='\0')|| (c=='\n'))
#define isdone(c)  ((c=='/') || (isend(c)) || (c==';'))

/* Macros for testing symbol attributes.  Each macro evaluates to non-zero    */
/* (true) if the stated condtion is met.                                      */
/* Use these to test attributes.  The proper bits are extracted and then      */
/* tested.                                                                    */
#define M_CONDITIONAL(s) ((s & CONDITION) == CONDITION)
#define M_DEFINED(s)     ((s & DEFINED) == DEFINED)
#define M_DUPLICATE(s)   ((s & DUPLICATE) == DUPLICATE)
#define M_FIXED(s)       ((s & FIXED) == FIXED)
#define M_LABEL(s)       ((s & LABEL) == LABEL)
#define M_MRI(s)         ((s & MRI) == MRI)
#define M_MRIFIX(s)      ((s & MRIFIX) == MRIFIX)
#define M_PSEUDO(s)      ((s & PSEUDO) == PSEUDO)
#define M_REDEFINED(s)   ((s & REDEFINED) == REDEFINED)
#define M_UNDEFINED(s)   (!M_DEFINED(s))

/* This macro is used to test symbols by the conditional assembly pseudo-ops. */
#define M_DEF(s) (M_DEFINED(s))
#define M_COND(s) (M_DEFINED(s))
#define M_DEFINED_CONDITIONALLY(t) ((M_DEF(t)&&pass==1)||(!M_COND(t)&&pass==2))

typedef unsigned char BOOL;
typedef unsigned char BYTE;
typedef short    int  WORD16;
typedef long     int  WORD32;

#ifndef FALSE
  #define FALSE 0
  #define TRUE (!FALSE)
#endif

/* Line listing styles.  Used to control listing of lines.                    */
enum linestyle_t
{
  LINE, LINE_VAL, LINE_LOC_VAL, LOC_VAL
};
typedef enum linestyle_t LINESTYLE_T;

/* Symbol Types.                                                              */
/* Note that the names that have FIX as the suffix contain the FIXED bit      */
/* included in the value.                                                     */
/*                                                                            */
/* The CONDITION bit is used when processing the conditional assembly PSEUDO- */
/* OPs (e.g., IFDEF).  During pass 1 of the assembly, the symbol is either    */
/* defined or undefined.  The condition bit is set when the symbol is defined */
/* during pass 1 and reset on pass 2 at the location the symbol was defined   */
/* during pass 1.  When processing conditionals during pass 2, if the symbol  */
/* is defined and the condition bit is set, the symbol is treated as if it    */
/* were undefined.  This gives consistent behavior of the conditional         */
/* pseudo-ops during both pass 1 and pass 2.                                  */
enum symtyp
{
  UNDEFINED = 0000,
  DEFINED   = 0001,
  FIXED     = 0002,
  MRI       = 0004    | DEFINED,
  LABEL     = 0010    | DEFINED,
  REDEFINED = 0020    | DEFINED,
  DUPLICATE = 0040    | DEFINED,
  PSEUDO    = 0100    | FIXED | DEFINED,
  CONDITION = 0200    | DEFINED,
  MRIFIX    = MRI     | FIXED | DEFINED,
  DEFFIX    = DEFINED | FIXED
};
typedef enum symtyp SYMTYP;

enum pseudo_t
{
  BANK,    BINPUNCH, DECIMAL, DUBL,    EJECT,    ENPUNCH, EXPUNGE, FIELD,
  FIXMRI,  FIXTAB,   FLTG,    IFDEF,   IFNDEF,   IFNZERO, IFZERO,  NOPUNCH,
  OCTAL,   PAGE,     PAUSE,   RELOC,   RIMPUNCH, SEGMNT,  TEXT,    TITLE,
  XLIST,   ZBLOCK
};
typedef enum pseudo_t PSEUDO_T;

struct sym_t
{
  SYMTYP  type;
  char    name[SYMLEN];
  WORD16  val;
  int     xref_index;
  int     xref_count;
};
typedef struct sym_t SYM_T;

struct lpool_t
{
  BOOL    error;                /* True if error message has been printed.    */
  WORD16  loc;
  WORD16  pool[PAGE_SIZE];
};
typedef struct lpool_t LPOOL_T;

struct emsg_t
{
  char  *list;
  char  *file;
};
typedef struct emsg_t EMSG_T;

struct errsave_t
{
  char  *mesg;
  int    col;
};
typedef struct errsave_t ERRSAVE_T;

struct fltg_
{
  WORD16 exponent;
  WORD32 mantissa;
};
typedef struct fltg_ FLTG_T;

/*----------------------------------------------------------------------------*/

/* Function Prototypes                                                        */

int     binarySearch( char *name, int start, int symbol_count );
int     compareSymbols( const void *a, const void *b );
void    conditionFalse( void );
void    conditionTrue( void );
SYM_T  *defineLexeme( int start, int term, WORD16 val, SYMTYP type );
SYM_T  *defineSymbol( char *name, WORD16 val, SYMTYP type, WORD16 start);
void    endOfBinary( void );
void    errorLexeme( EMSG_T *mesg, int col );
void    errorMessage( EMSG_T *mesg, int col );
void    errorSymbol( EMSG_T *mesg, char *name, int col );
SYM_T  *eval( void );
WORD32  evalDubl( WORD32 initial_value );
FLTG_T *evalFltg( void );
SYM_T  *evalSymbol( void );
void    getArgs( int argc, char *argv[] );
WORD32  getDublExpr( void );
WORD32  getDublExprs( void );
FLTG_T *getFltgExpr( void );
FLTG_T *getFltgExprs( void );
SYM_T  *getExpr( void );
WORD16  getExprs( void );
WORD16  incrementClc( void );
void    inputDubl( void );
void    inputFltg( void );
WORD16  insertLiteral( LPOOL_T *pool, WORD16 value );
char   *lexemeToName( char *name, int from, int term );
void    listLine( void );
SYM_T  *lookup( char *name );
void    moveToEndOfLine( void );
void    nextLexBlank( void );
void    nextLexeme( void );
void    normalizeFltg( FLTG_T *fltg );
void    onePass( void );
void    printCrossReference( void );
void    printErrorMessages( void );
void    printLine(char *line, WORD16 loc, WORD16 val, LINESTYLE_T linestyle);
void    printPageBreak( void );
void    printPermanentSymbolTable( void );
void    printSymbolTable( void );
BOOL    pseudoOperators( PSEUDO_T val );
void    punchChecksum( void );
void    punchLocObject( WORD16 loc, WORD16 val );
void    punchLiteralPool( LPOOL_T *p, WORD16 lpool_page );
void    punchOutObject( WORD16 loc, WORD16 val );
void    punchLeader( int count );
void    punchObject( WORD16 val );
void    punchOrigin( WORD16 loc );
void    readLine( void );
void    saveError( char *mesg, int cc );
BOOL    testForLiteralCollision( WORD16 loc );
void    topOfForm( char *title, char *sub_title );

/*----------------------------------------------------------------------------*/

/* Table of pseudo-ops (directives) which are used to setup the symbol        */
/* table on startup and when the EXPUNGE pseudo-op is executed.               */
SYM_T pseudo[] =
{
  { PSEUDO, "BANK",   BANK    },    /* Like field, select some 32K out of 128K*/
  { PSEUDO, "BINPUN", BINPUNCH },   /* Output in Binary Loader format.        */
  { PSEUDO, "DECIMA", DECIMAL },    /* Read literal constants in base 10.     */
  { PSEUDO, "DUBL",   DUBL    },    /* Ignored (unsupported).                 */
  { PSEUDO, "EJECT",  EJECT   },    /* Eject a page in the listing.           */
  { PSEUDO, "ENPUNC", ENPUNCH },    /* Turn on object code generation.        */
  { PSEUDO, "EXPUNG", EXPUNGE },    /* Remove all symbols from symbol table.  */
  { PSEUDO, "FIELD",  FIELD   },    /* Set origin to memory field.            */
  { PSEUDO, "FIXMRI", FIXMRI  },    /* Like =, but creates mem ref instruction*/
  { PSEUDO, "FIXTAB", FIXTAB  },    /* Mark current symbols as permanent.     */
  { PSEUDO, "FLTG",   FLTG    },    /* Ignored (unsupported).                 */
  { PSEUDO, "IFDEF",  IFDEF   },    /* Assemble if symbol is defined.         */
  { PSEUDO, "IFNDEF", IFNDEF  },    /* Assemble if symbol is not defined.     */
  { PSEUDO, "IFNZER", IFNZERO },    /* Assemble if symbol value is not 0.     */
  { PSEUDO, "IFZERO", IFZERO  },    /* Assemble if symbol value is 0.         */
  { PSEUDO, "NOPUNC", NOPUNCH },    /* Turn off object code generation.       */
  { PSEUDO, "OCTAL",  OCTAL   },    /* Read literal constants in base 8.      */
  { PSEUDO, "PAGE",   PAGE    },    /* Set orign to page +1 or page n (0..37).*/
  { PSEUDO, "PAUSE",  PAUSE   },    /* Ignored                                */
  { PSEUDO, "RELOC",  RELOC   },    /* Assemble to run at a different address.*/
  { PSEUDO, "RIMPUN", RIMPUNCH },   /* Output in Read In Mode format.         */
  { PSEUDO, "SEGMNT", SEGMNT  },    /* Like page, but with page size=1K words.*/
  { PSEUDO, "TEXT",   TEXT    },    /* Pack 6 bit trimmed ASCII into memory.  */
  { PSEUDO, "TITLE",  TITLE   },    /* Use the text string as a listing title.*/
  { PSEUDO, "XLIST",  XLIST   },    /* Toggle listing generation.             */
  { PSEUDO, "ZBLOCK", ZBLOCK  }     /* Zero a block of memory.                */
};

/* Symbol Table                                                               */
/* The table is put in lexical order on startup, so symbols can be            */
/* inserted as desired into the initial table.                                */
SYM_T permanent_symbols[] =
{
  /* Memory Reference Instructions                                            */
  { MRIFIX, "AND",    00000   },    /* LOGICAL AND                            */
  { MRIFIX, "TAD",    01000   },    /* TWO'S COMPLEMENT ADD                   */
  { MRIFIX, "ISZ",    02000   },    /* INCREMENT AND SKIP IF ZERO             */
  { MRIFIX, "DCA",    03000   },    /* DEPOSIT AND CLEAR ACC                  */
  { MRIFIX, "I",      00400   },    /* INDIRECT ADDRESSING                    */
  { MRIFIX, "JMP",    05000   },    /* JUMP                                   */
  { MRIFIX, "JMS",    04000   },    /* JUMP TO SUBROUTINE                     */
  { MRIFIX, "Z",      00000   },    /* PAGE ZERO ADDRESS                      */
  /* Floating Point Interpreter Instructions                                  */
  { MRIFIX, "FEXT",   00000   },    /* FLOATING EXIT                          */
  { MRIFIX, "FADD",   01000   },    /* FLOATING ADD                           */
  { MRIFIX, "FSUB",   02000   },    /* FLOATING SUBTRACT                      */
  { MRIFIX, "FMPY",   03000   },    /* FLOATING MULTIPLY                      */
  { MRIFIX, "FDIV",   04000   },    /* FLOATING DIVIDE                        */
  { MRIFIX, "FGET",   05000   },    /* FLOATING GET                           */
  { MRIFIX, "FPUT",   06000   },    /* FLOATING PUT                           */
  { FIXED,  "FNOR",   07000   },    /* FLOATING NORMALIZE                     */
  { FIXED,  "FEXT",   00000   },    /* EXIT FROM FLOATING POINT INTERPRETER   */
  { FIXED,  "SQUARE", 00001   },    /* SQUARE C(FAC)                          */
  { FIXED,  "SQROOT", 00002   },    /* TAKE SQUARE ROOT OF C(FAC)             */
  /* Group 1 Operate Microinstrcutions                                        */
  { FIXED,  "NOP",    07000   },    /* NO OPERATION                           */
  { FIXED,  "IAC",    07001   },    /* INCREMENT AC                           */
  { FIXED,  "RAL",    07004   },    /* ROTATE AC AND LINK LEFT ONE            */
  { FIXED,  "RTL",    07006   },    /* ROTATE AC AND LINK LEFT TWO            */
  { FIXED,  "RAR",    07010   },    /* ROTATE AC AND LINK RIGHT ONE           */
  { FIXED,  "RTR",    07012   },    /* ROTATE AC AND LINK RIGHT TWO           */
  { FIXED,  "CML",    07020   },    /* COMPLEMENT LINK                        */
  { FIXED,  "CMA",    07040   },    /* COMPLEMEMNT AC                         */
  { FIXED,  "CLL",    07100   },    /* CLEAR LINK                             */
  { FIXED,  "CLA",    07200   },    /* CLEAR AC                               */
  /* Group 2 Operate Microinstructions                                        */
  { FIXED,  "BSW",    07002   },    /* Swap bytes in AC (PDP/8e)              */
  { FIXED,  "HLT",    07402   },    /* HALT THE COMPUTER                      */
  { FIXED,  "OSR",    07404   },    /* INCLUSIVE OR SR WITH AC                */
  { FIXED,  "SKP",    07410   },    /* SKIP UNCONDITIONALLY                   */
  { FIXED,  "SNL",    07420   },    /* SKIP ON NON-ZERO LINK                  */
  { FIXED,  "SZL",    07430   },    /* SKIP ON ZERO LINK                      */
  { FIXED,  "SZA",    07440   },    /* SKIP ON ZERO AC                        */
  { FIXED,  "SNA",    07450   },    /* SKIP ON NON=ZERO AC                    */
  { FIXED,  "SMA",    07500   },    /* SKIP MINUS AC                          */
  { FIXED,  "SPA",    07510   },    /* SKIP ON POSITIVE AC (ZERO IS POSITIVE) */
  /* Combined Operate Microinstructions                                       */
  { FIXED,  "CIA",    07041   },    /* COMPLEMENT AND INCREMENT AC            */
  { FIXED,  "STL",    07120   },    /* SET LINK TO 1                          */
  { FIXED,  "GLK",    07204   },    /* GET LINK (PUT LINK IN AC BIT 11)       */
  { FIXED,  "STA",    07240   },    /* SET AC TO -1                           */
  { FIXED,  "LAS",    07604   },    /* LOAD ACC WITH SR                       */
  /* MQ Instructions (PDP/8e)                                                 */
  { FIXED,  "MQL",    07421   },    /* Load MQ from AC, then clear AC.        */
  { FIXED,  "MQA",    07501   },    /* Inclusive OR MQ with AC                */
  { FIXED,  "SWP",    07521   },    /* Swap AC and MQ                         */
  { FIXED,  "ACL",    07701   },    /* Load MQ into AC                        */
  /* Program Interrupt                                                        */
  { FIXED,  "IOT",    06000   },
  { FIXED,  "ION",    06001   },    /* TURN INTERRUPT PROCESSOR ON            */
  { FIXED,  "IOF",    06002   },    /* TURN INTERRUPT PROCESSOR OFF           */
  /* Program Interrupt, PDP-8/e                                               */
  { FIXED,  "SKON",   06000   },    /* Skip if interrupt on and turn int off. */
  { FIXED,  "SRQ",    06003   },    /* Skip on interrupt request.             */
  { FIXED,  "GTF",    06004   },    /* Get interrupt flags.                   */
  { FIXED,  "RTF",    06005   },    /* Restore interrupt flags.               */
  { FIXED,  "SGT",    06006   },    /* Skip on greater than flag.             */
  { FIXED,  "CAF",    06007   },    /* Clear all flags.                       */
  /* Keyboard/Reader                                                          */
  { FIXED,  "KSF",    06031   },    /* SKIP ON KEYBOARD FLAG                  */
  { FIXED,  "KCC",    06032   },    /* CLEAR KEYBOARD FLAG                    */
  { FIXED,  "KRS",    06034   },    /* READ KEYBOARD BUFFER (STATIC)          */
  { FIXED,  "KRB",    06036   },    /* READ KEYBOARD BUFFER & CLEAR FLAG      */
  /* Teleprinter/Punch                                                        */
  { FIXED,  "TSF",    06041   },    /* SKIP ON TELEPRINTER FLAG               */
  { FIXED,  "TCF",    06042   },    /* CLEAR TELEPRINTER FLAG                 */
  { FIXED,  "TPC",    06044   },    /* LOAD TELEPRINTER & PRINT               */
  { FIXED,  "TLS",    06046   },    /* LOAD TELPRINTER & CLEAR FLAG           */
  /* High Speed Paper Tape Reader                                             */
  { FIXED,  "RSF",    06011   },    /* SKIP ON READER FLAG                    */
  { FIXED,  "RRB",    06012   },    /* READ READER BUFFER AND CLEAR FLAG      */
  { FIXED,  "RFC",    06014   },    /* READER FETCH CHARACTER                 */
  /* PC8-E High Speed Paper Tape Reader & Punch                               */
  { FIXED,  "RPE",    06010   },    /* Set interrupt enable for reader/punch  */
  { FIXED,  "PCE",    06020   },    /* Clear interrupt enable for rdr/punch   */
  { FIXED,  "RCC",    06016   },    /* Read reader buffer, clear flags & buf, */
                                    /* and fetch character.                   */
  /* High Speed Paper Tape Punch                                              */
  { FIXED,  "PSF",    06021   },    /* SKIP ON PUNCH FLAG                     */
  { FIXED,  "PCF",    06022   },    /* CLEAR ON PUNCH FLAG                    */
  { FIXED,  "PPC",    06024   },    /* LOAD PUNCH BUFFER AND PUNCH CHARACTER* */
  { FIXED,  "PLS",    06026   },    /* LOAD PUNCH BUFFER AND CLEAR FLAG       */
  /* DECtape Transport Type TU55 and DECtape Control Type TC01                */
  { FIXED,  "DTRA",   06761   },    /* Contents of status register is ORed    */
                                    /* into AC bits 0-9                       */
  { FIXED,  "DTCA",   06762   },    /* Clear status register A, all flags     */
                                    /* undisturbed                            */
  { FIXED,  "DTXA",   06764   },    /* Status register A loaded by exclusive  */
                                    /* OR from AC.  If AC bit 10=0, clear     */
                                    /* error flags; if AC bit 11=0, DECtape   */
                                    /* control flag is cleared.               */
  { FIXED,  "DTLA",   06766   },    /* Combination of DTCA and DTXA           */
  { FIXED,  "DTSF",   06771   },    /* Skip if error flag is 1 or if DECtape  */
                                    /* control flag is 1                      */
  { FIXED,  "DTRB",   06772   },    /* Contents of status register B is       */
                                    /* ORed into AC                           */
  { FIXED,  "DTLB",   06774   },    /* Memory field portion of status         */
                                    /* register B loaded from AC bits 6-8     */
  /* Disk File and Control, Type DF32                                         */
  { FIXED,  "DCMA",   06601   },    /* CLEAR DISK MEMORY REQUEST AND          */
                                    /* INTERRUPT FLAGS                        */
  { FIXED,  "DMAR",   06603   },    /* LOAD DISK FROM AC, CLEAR AC READ       */
                                    /* INTO CORE, CLEAR INTERRUPT FLAG        */
  { FIXED,  "DMAW",   06605   },    /* LOAD DISK FROM AC, WRITE ONTO DISK     */
                                    /* FROM CORE, CLEAR INTERRUPT FLAG        */
  { FIXED,  "DCEA",   06611   },    /* CLEAR DISK EXTENDED ADDRESS AND        */
  { FIXED,  "DSAC",   06612   },    /* SKIP IF ADDRESS CONFIRMED FLAG = 1     */
                                    /* MEMORY ADDRESS EXTENSION REGISTER      */
  { FIXED,  "DEAL",   06615   },    /* CLEAR DISK EXTENDED ADDRESS AND        */
                                    /* MEMORY ADDRESS EXTENSION REGISTER      */
                                    /* AND LOAD SAME FROM AC                  */
  { FIXED,  "DEAC",   06616   },    /* CLEAR AC, LOAD AC FROM DISK EXTENDED   */
                                    /* ADDRESS REGISTER, SKIP IF ADDRESS      */
                                    /* CONFIRMED FLAG = 1                     */
  { FIXED,  "DFSE",   06621   },    /* SKIP IF PARITY ERROR, DATA REQUEST     */
                                    /* LATE, OR WRITE LOCK SWITCH FLAG = 0    */
                                    /* (NO ERROR)                             */
  { FIXED,  "DFSC",   06622   },    /* SKIP IF COMPLETION FLAG = 1 (DATA      */
                                    /* TRANSFER COMPLETE)                     */
  { FIXED,  "DMAC",   06626   },    /* CLEAR AC, LOAD AC FROM DISK MEMORY     */
                                    /* ADDRESS REGISTER                       */
  /* Disk File and Control, Type RF08                                         */
  { FIXED,  "DCIM",   06611   },
  { FIXED,  "DIML",   06615   },
  { FIXED,  "DIMA",   06616   },
  { FIXED,  "DISK",   06623   },
  { FIXED,  "DCXA",   06641   },
  { FIXED,  "DXAL",   06643   },
  { FIXED,  "DXAC",   06645   },
  { FIXED,  "DMMT",   06646   },
  /* Memory Extension Control, Type 183                                       */
  { FIXED,  "CDF",    06201   },    /* CHANGE DATA FIELD                      */
  { FIXED,  "CIF",    06202   },    /* CHANGE INSTRUCTION FIELD               */
  { FIXED,  "CDI",    06203   },    /* Change data & instrution field.        */
  { FIXED,  "RDF",    06214   },    /* READ DATA FIELD                        */
  { FIXED,  "RIF",    06224   },    /* READ INSTRUCTION FIELD                 */
  { FIXED,  "RIB",    06234   },    /* READ INTERRUPT BUFFER                  */
  { FIXED,  "RMF",    06224   },    /* RESTORE MEMORY FIELD                   */
  /* Memory Parity, Type MP8/I (MP8/L)                                        */
  { FIXED,  "SMP",    06101   },    /* SKIP IF MEMORY PARITY FLAG = 0         */
  { FIXED,  "CMP",    06104   },    /* CLEAR MEMORY PAIRTY FLAG               */
  /* Memory Parity, Type MP8-E (PDP8/e)                                       */
  { FIXED,  "DPI",    06100   },    /* Disable parity interrupt.              */
  { FIXED,  "SNP",    06101   },    /* Skip if no parity error.               */
  { FIXED,  "EPI",    06103   },    /* Enable parity interrupt.               */
  { FIXED,  "CNP",    06104   },    /* Clear parity error flag.               */
  { FIXED,  "CEP",    06106   },    /* Check for even parity.                 */
  { FIXED,  "SPO",    06107   },    /* Skip on parity option.                 */
  /* Data Communications Systems, Type 680I                                   */
  { FIXED,  "TTINCR", 06401   },    /* The content of the line select         */
                                    /* register is incremented by one.        */
  { FIXED,  "TTI",    06402   },    /* The line status word is read and       */
                                    /* sampled.  If the line is active for    */
                                    /* the fourth time, the line bit is       */
                                    /* shifted into the character assembly    */
                                    /* word.  If the line bit is active for   */
                                    /* a number of times less than four,      */
                                    /* the count is incremented.  If the      */
                                    /* line is not active, the active/inac-   */
                                    /* tive status of the line is recorded    */
  { FIXED,  "TTO",    06404   },    /* The character in the AC is shifted     */
                                    /* right one position, zeros are shifted  */
                                    /* into vacated positions, and the orig-  */
                                    /* inal content of AC11 is transferred    */
                                    /* out of the computer on the TTY line.   */
  { FIXED,  "TTCL",   06411   },    /* The line select register is cleared.   */
  { FIXED,  "TTSL",   06412   },    /* The line select register is loaded by  */
                                    /* an OR transfer from the content of     */
                                    /* of AC5-11, the the AC is cleared.      */
  { FIXED,  "TTRL",   06414   },    /* The content of the line select regis-  */
                                    /* ter is read into AC5-11 by an OR       */
                                    /* transfer.                              */
  { FIXED,  "TTSKP",  06421   },    /* Skip if clock flag is a 1.             */
  { FIXED,  "TTXON",  06424   },    /* Clock 1 is enabled to request a prog-  */
                                    /* ram interrupt and clock 1 flag is      */
                                    /* cleared.                               */
  { FIXED,  "TTXOF",  06422   },    /* Clock 1 is disabled from causing a     */
                                    /* program interrupt and clock 1 flag     */
                                    /* is cleared.                            */
};      /* End-of-Symbols for Permanent Symbol Table                          */

/* Global variables                                                           */
SYM_T *symtab;                  /* Symbol Table                               */
int    symbol_top;              /* Number of entries in symbol table.         */

SYM_T *fixed_symbols;           /* Start of the fixed symbol table entries.   */
int    number_of_fixed_symbols;

/*----------------------------------------------------------------------------*/

WORD16 *xreftab;                /* Start of the concordance table.            */

ERRSAVE_T error_list[20];
int     save_error_count;

LPOOL_T pz;                     /* Storage for page zero constants.           */
LPOOL_T cp;                     /* Storage for current page constants.        */

char   s_detected[] = "detected";
char   s_error[]    = "error";
char   s_errors[]   = "errors";
char   s_no[]       = "No";
char   s_page[]     = "Page";
char   s_symtable[] = "Symbol Table";
char   s_xref[]     = "Cross Reference";

/* Assembler diagnostic messages.                                             */
/* Some attempt has been made to keep continuity with the PAL-III and         */
/* MACRO-8 diagnostic messages.  If a diagnostic indicator, (e.g., IC)        */
/* exists, then the indicator is put in the listing as the first two          */
/* characters of the diagnostic message.  The PAL-III indicators where used   */
/* when there was a choice between using MACRO-8 and PAL-III indicators.      */
/* The character pairs and their meanings are:                                */
/*      DT  Duplicate Tag (symbol)                                            */
/*      IC  Illegal Character                                                 */
/*      ID  Illegal Redefinition of a symbol.  An attempt was made to give    */
/*          a symbol a new value not via =.                                   */
/*      IE  Illegal Equals  An equal sign was used in the wrong context,      */
/*          (e.g., A+B=C, or TAD A+=B)                                        */
/*      II  Illegal Indirect  An off page reference was made, but a literal   */
/*          could not be generated because the indirect bit was already set.  */
/*      IR  Illegal Reference (address is not on current page or page zero)   */
/*      ND  No $ (the program terminator) at end of file.                     */
/*      PE  Current, Non-Zero Page Exceeded (literal table flowed into code)  */
/*      RD  ReDefintion of a symbol                                           */
/*      ST  Symbol Table full                                                 */
/*      UA  Undefined Address (undefined symbol)                              */
/*      ZE  Zero Page Exceeded (see above, or out of space)                   */
EMSG_T  duplicate_label     = { "DT duplicate",  "duplicate label" };
EMSG_T  illegal_blank       = { "IC illegal blank", "illegal blank" };
EMSG_T  illegal_character   = { "IC illegal char",  "illegal character" };
EMSG_T  illegal_expression  = { "IC in expression", "illegal expression" };
EMSG_T  label_syntax        = { "IC label syntax",  "label syntax" };
EMSG_T  not_a_number        = { "IC numeric syntax", "numeric syntax of" };
EMSG_T  number_not_radix    = { "IC radix", "number not in current radix"};
EMSG_T  symbol_syntax       = { "IC symbol syntax", "symbol syntax" };
EMSG_T  illegal_equals      = { "IE illegal =",  "illegal equals" };
EMSG_T  illegal_indirect    = { "II off page",   "illegal indirect" };
EMSG_T  illegal_reference   = { "IR off page",   "illegal reference" };
EMSG_T  undefined_symbol    = { "UD undefined",  "undefined symbol" };
EMSG_T  redefined_symbol    = { "RD redefined",  "redefined symbol" };
EMSG_T  literal_overflow    = { "PE page exceeded",
                                   "current page literal capacity exceeded" };
EMSG_T  pz_literal_overflow = { "ZE page exceeded",
                                   "page zero capacity exceeded" };
EMSG_T  dubl_overflow       = { "dubl overflow",  "DUBL value overflow" };
EMSG_T  fltg_overflow       = { "fltg overflow",  "FLTG value overflow" };
EMSG_T  zblock_too_small    = { "expr too small", "ZBLOCK value too small" };
EMSG_T  zblock_too_large    = { "expr too large", "ZBLOCK value too large" };
EMSG_T  end_of_file         = { "ND no $ at EOF", "No $ at End-of-File" };
EMSG_T  no_pseudo_op        = { "not implemented",
                                   "not implemented pseudo-op" };
EMSG_T  illegal_field_value = { "expr out of range",
                                   "field value not in range of 0 through 7" };
EMSG_T  literal_gen_off     = { "literals off",
                                                "literal generation is off" };
EMSG_T  no_literal_value    = { "no value",  "no literal value" };
EMSG_T  text_string         = { "no delimiter",
                                    "text string delimiters not matched" };
EMSG_T  in_rim_mode         = { "not OK in rim mode"
                                    "FIELD pseudo-op not valid in RIM mode" };
EMSG_T  lt_expected         = { "'<' expected",  "'<' expected" };
EMSG_T  symbol_table_full   = { "ST Symbol Tbl Full",
                                                    "Symbol Table Full" };

/*----------------------------------------------------------------------------*/

FILE   *errorfile;
FILE   *infile;
FILE   *listfile;
FILE   *listsave;
FILE   *objectfile;
FILE   *objectsave;

char    errorpathname[NAMELEN];
char    filename[NAMELEN];
char    listpathname[NAMELEN];
char    objectpathname[NAMELEN];
char   *pathname;
char    permpathname[NAMELEN];

int     list_lineno;
int     list_pageno;
char    list_title[LINELEN];
BOOL    list_title_set;         /* Set if TITLE pseudo-op used.               */
char    line[LINELEN];          /* Input line.                                */
int     lineno;                 /* Current line number.                       */
int     page_lineno;            /* print line number on current page.         */
BOOL    listed;                 /* Listed flag.                               */

int     cc;                     /* Column Counter (char position in line).    */
WORD16  checksum;               /* Generated checksum                         */
BOOL    binary_data_output;     /* Set true when data has been output.        */
WORD16  clc;                    /* Location counter                           */
WORD16  cplc;                   /* Current page literal counter.              */
char    delimiter;              /* Character immediately after eval'd term.   */
int     errors;                 /* Number of errors found so far.             */
BOOL    error_in_line;          /* TRUE if error on current line.             */
int     errors_pass_1;          /* Number of errors on pass 1.                */
WORD16  field;                  /* Current field                              */
WORD16  fieldlc;                /* location counter without field portion.    */
BOOL    fltg_input;             /* TRUE when doing floating point input.      */
BOOL    indirect_generated;     /* TRUE if an off page address generated.     */
int     last_xref_lexstart;     /* Column where last xref symbol was located. */
int     last_xref_lineno;       /* Line where last xref symbol was located.   */
int     lexstartprev;           /* Where previous lexeme started.             */
int     lextermprev;            /* Where previous lexeme ended.               */
int     lexstart;               /* Index of current lexeme on line.           */
int     lexterm;                /* Index of character after current lexeme.   */
BOOL    literals_on;            /* Generate literals, defaults to none        */
int     maxcc;                  /* Current line length.                       */
BOOL    overflow;               /* Overflow flag for math routines.           */
int     pass;                   /* Number of current pass.                    */
BOOL    print_permanent_symbols;
WORD16  pzlc;                   /* Page Zero literal counter.                 */
WORD16  radix;                  /* Default number radix.                      */
WORD16  reloc;                  /* The relocation distance.                   */
BOOL    rim_mode;               /* Generate rim format, defaults to bin       */
BOOL    symtab_print;           /* Print symbol table flag                    */
BOOL    xref;

FLTG_T  fltg_ac;                /* Value holder for evalFltg()                */
SYM_T   sym_eval = { DEFINED, "", 0 };       /* Value holder for eval()       */
SYM_T   sym_getexpr = { DEFINED, "", 0 };    /* Value holder for getexpr()    */
SYM_T   sym_undefined = { UNDEFINED, "", 0 };/* Symbol Table Terminator       */


/******************************************************************************/
/*                                                                            */
/*  Function:  main                                                           */
/*                                                                            */
/*  Synopsis:  Starting point.  Controls order of assembly.                   */
/*                                                                            */
/******************************************************************************/
int main( int argc, char *argv[] )
{
  int     ix;
  int     space;

  /* Set the default values for global symbols.                               */
  binary_data_output = FALSE;
  fltg_input = FALSE;
  literals_on = FALSE;
  print_permanent_symbols = FALSE;
  rim_mode = FALSE;
  symtab_print = FALSE;
  xref = FALSE;
  pathname = NULL;

  /* Get the options and pathnames                                            */
  getArgs( argc, argv );

  /* Setup the error file in case symbol table overflows while installing the */
  /* permanent symbols.                                                       */
  errorfile = fopen( errorpathname, "w" );
  errors = 0;
  save_error_count = 0;
  pass = 0;             /* This is required for symbol table initialization.  */
  symtab = (SYM_T *) malloc( sizeof( SYM_T ) * SYMBOL_TABLE_SIZE );

  if( symtab == NULL )
  {
    fprintf( stderr, "Could not allocate memory for symbol table.\n");
    exit( -1 );
  }

  /* Place end marker in symbol table.                                        */
  symtab[0] = sym_undefined;
  symbol_top = 0;
  number_of_fixed_symbols = symbol_top;
  fixed_symbols = &symtab[symbol_top - 1];

  /* Enter the pseudo-ops into the symbol table                               */
  for( ix = 0; ix < DIM( pseudo ); ix++ )
  {
    defineSymbol( pseudo[ix].name, pseudo[ix].val, pseudo[ix].type, 0 );
  }

  /* Enter the predefined symbols into the table.                             */
  /* Also make them part of the permanent symbol table.                       */
  for( ix = 0; ix < DIM( permanent_symbols ); ix++ )
  {
    defineSymbol( permanent_symbols[ix].name,
                  permanent_symbols[ix].val,
                  permanent_symbols[ix].type | DEFFIX , 0 );
  }

  number_of_fixed_symbols = symbol_top;
  fixed_symbols = &symtab[symbol_top - 1];

  /* Do pass one of the assembly                                              */
  checksum = 0;
  pass = 1;
  page_lineno = LIST_LINES_PER_PAGE;
  onePass();
  errors_pass_1 = errors;

  /* Set up for pass two                                                      */
  rewind( infile );
  errorfile = fopen( errorpathname, "w" );
  objectfile = fopen( objectpathname, "wb" );
  objectsave = objectfile;

  listfile = fopen( listpathname, "w" );
  listsave = NULL;

  punchLeader( 0 );
  checksum = 0;

  /* Do pass two of the assembly                                              */
  errors = 0;
  save_error_count = 0;
  page_lineno = LIST_LINES_PER_PAGE;

  if( xref )
  {
    /* Get the amount of space that will be required for the concordance.     */
    for( space = 0, ix = 0; ix < symbol_top; ix++ )
    {
      symtab[ix].xref_index = space;    /* Index into concordance table.      */
      space += symtab[ix].xref_count + 1;
      symtab[ix].xref_count = 0;        /* Clear the count for pass 2.        */

    }
    /* Allocate the necessary space.                                          */
    xreftab = (WORD16 *) malloc( sizeof( WORD16 ) * space );

    /* Clear the cross reference space.                                       */
    for( ix = 0; ix < space; ix++ )
    {
      xreftab[ix] = 0;
    }
  }
  pass = 2;
  onePass();

  /* Undo effects of NOPUNCH for any following checksum                       */
  objectfile = objectsave;
  punchChecksum();

  /* Works great for trailer.                                                 */
  punchLeader( 1 );

  /* undo effects of XLIST for any following output to listing file.          */
  if( listfile == NULL )
  {
    listfile = listsave;
  }

  /* Display value of error counter.                                          */
  if( errors == 0 )
  {
    fprintf( listfile, "\n      %s %s %s\n", s_no, s_detected, s_errors );
  }
  else
  {
    fprintf( errorfile, "\n      %d %s %s\n", errors, s_detected,
                                        ( errors == 1 ? s_error : s_errors ));
    fprintf( listfile, "\n      %d %s %s\n", errors, s_detected,
                                        ( errors == 1 ? s_error : s_errors ));
    fprintf( stderr,   "      %d %s %s\n", errors, s_detected,
                                        ( errors == 1 ? s_error : s_errors ));
  }

  if( symtab_print )
  {
    printSymbolTable();
  }

  if( print_permanent_symbols )
  {
    printPermanentSymbolTable();
  }

  if( xref )
  {
    printCrossReference();
  }

  fclose( objectfile );
  fclose( listfile );
  fclose( errorfile );
  if( errors == 0 && errors_pass_1 == 0 )
  {
    remove( errorpathname );
  }

  return( errors != 0 );
} /* main()                                                                   */

/******************************************************************************/
/*                                                                            */
/*  Function:  getArgs                                                        */
/*                                                                            */
/*  Synopsis:  Parse command line, set flags accordingly and setup input and  */
/*             output files.                                                  */
/*                                                                            */
/******************************************************************************/
void getArgs( int argc, char *argv[] )
{
  int  len;
  int  ix, jx;

  /* Set the defaults                                                         */
  errorfile = NULL;
  infile = NULL;
  listfile = NULL;
  listsave = NULL;
  objectfile = NULL;
  objectsave = NULL;

  for( ix = 1; ix < argc; ix++ )
  {
    if( argv[ix][0] == '-' )
    {
      for( jx = 1; argv[ix][jx] != 0; jx++ )
      {
        switch( argv[ix][jx] )
        {
        case 'd':
          symtab_print = TRUE;
          break;

        case 'r':
          rim_mode = TRUE;
          break;

        case 'l':
          literals_on = TRUE;
          break;

        case 'p':
          print_permanent_symbols = TRUE;
          break;

        case 'x':
          xref = TRUE;
          break;

        case 'v':
          fprintf( stderr, "%s\n", release );
          fflush( stderr );
          exit( -1 );
          break;

        default:
          fprintf( stderr, "%s: unknown flag: %s\n", argv[0], argv[ix] );
        case 'h':
          fprintf( stderr, " -d -- dump symbol table\n" );
          fprintf( stderr, " -h -- show this help\n" );
          fprintf( stderr, " -l -- generate literals\n" );
          fprintf( stderr, " -r -- output rim format file\n" );
          fprintf( stderr, " -p -- output permanent symbols to file\n" );
          fprintf( stderr, " -v -- display version\n" );
          fprintf( stderr, " -x -- output cross reference to file\n" );
          fflush( stderr );
          exit( -1 );
        } /* end switch                                                       */
      } /* end for                                                            */
    }
    else
    {
      if( pathname != NULL )
      {
        fprintf( stderr, "%s: too many input files\n", argv[0] );
        exit( -1 );
      }
      pathname = &argv[ix][0];
    }
  } /* end for                                                                */

  if( pathname == NULL )
  {
    fprintf( stderr, "%s:  no input file specified\n", argv[0] );
    exit( -1 );
  }

  len = strlen( pathname );
  if( len > NAMELEN - 5 )
  {
    fprintf( stderr, "%s: pathname \"%s\" too long\n", argv[0], pathname );
    exit( -1 );
  }

  /* Now open the input file.                                                 */
  if(( infile = fopen( pathname, "r" )) == NULL )
  {
    fprintf( stderr, "%s: cannot open \"%s\"\n", argv[0], pathname );
    exit( -1 );
  }

  /* Now make the pathnames                                                   */
  /* Find last '.', if it exists.                                             */
  jx = len - 1;
  while( pathname[jx] != '.'  && pathname[jx] != '/'
      && pathname[jx] != '\\' && jx >= 0 )
  {
    jx--;
  }

  switch( pathname[jx] )
  {
  case '.':
    break;

  case '/':
  case '\\':
    jx = len;
    break;

  default:
    break;
  }

  /* Add the pathname extensions.                                             */
  strncpy( objectpathname, pathname, jx );
  objectpathname[jx] = '\0';
  strcat( objectpathname, rim_mode ? ".rim" : ".bin" );

  strncpy( listpathname, pathname, jx );
  listpathname[jx] = '\0';
  strcat( listpathname, ".lst" );

  strncpy( errorpathname, pathname, jx );
  errorpathname[jx] = '\0';
  strcat( errorpathname, ".err" );

  strncpy( permpathname, pathname, jx );
  permpathname[jx] = '\0';
  strcat( permpathname, ".prm" );

  /* Extract the filename from the path.                                      */
  if( isalpha( pathname[0] ) && pathname[1] == ':' && pathname[2] != '\\' )
  {
    pathname[1] = '\\';         /* MS-DOS style pathname                      */
  }

  jx = len - 1;
  while( pathname[jx] != '/' && pathname[jx] != '\\' && jx >= 0 )
  {
    jx--;
  }
  strcpy( filename, &pathname[jx + 1] );

} /* getArgs()                                                                */


/******************************************************************************/
/*                                                                            */
/*  Function:  onePass                                                        */
/*                                                                            */
/*  Synopsis:  Do one assembly pass.                                          */
/*                                                                            */
/******************************************************************************/
void onePass()
{
  char    name[SYMLEN];
  WORD16  newclc;
  BOOL    scanning_line;
  int     start;
  SYM_T  *sym;
  int     term;
  WORD16  val;

  clc = 0200;                   /* Default starting address is 200 octal.     */
  field = 0;
  fieldlc = 0;
  reloc = 0;
  cp.loc = 00200;               /* Points to end of page for [] operands.     */
  pz.loc = 00200;               /* Points to end of page for () operands.     */
  cp.error = FALSE;
  pz.error = FALSE;
  listed = TRUE;
  lineno = 0;
  list_pageno = 0;
  list_lineno = 0;
  last_xref_lexstart = 0;
  last_xref_lineno = 0;
  list_title_set = FALSE;
  radix = 8;                    /* Initial radix is octal (base 8).           */

  while( TRUE )
  {
    readLine();
    nextLexeme();

    scanning_line = TRUE;
    while( scanning_line )
    {
      if( isend( line[lexstart] ))
      {
        scanning_line = FALSE;
      }
      else
      {
        switch( line[lexstart] )
        {
        case '/':
          scanning_line = FALSE;
          break;

        case ';':
          nextLexeme();
          break;

        case '$':
          endOfBinary();
          return;

        case '*':
          nextLexeme();             /* Skip '*', (set origin symbol)          */
          newclc = ((getExpr())->val & 07777 ) | field;
          /* Do not change Current Location Counter if an error occurred.     */
          if( !error_in_line )
          {
            if(( newclc & 07600 ) != ( clc & 07600 ))
            {
              /* Current page has changed.                                    */
              punchLiteralPool( &cp, clc - 1 );
            }
            clc = newclc - reloc;
            fieldlc = clc & 07777;

            if( !rim_mode )
            {
              /* Not rim mode, put out origin.                                */
              punchOrigin( clc );
            }
            printLine( line, 0, fieldlc, LINE_VAL );
          }
          break;

        default:
          switch( line[lexterm] )
          {
          case ',':
            if( isalpha( line[lexstart] ))
            {
              /* Use lookup so symbol will not be counted as reference.       */
              sym = lookup( lexemeToName( name, lexstart, lexterm ));
              if( M_DEFINED( sym->type ))
              {
                if( sym->val != clc && pass == 2 )
                {
                  errorSymbol( &duplicate_label, sym->name, lexstart );
                }
                sym->type = sym->type | DUPLICATE;
              }
              /* Must call define on pass 2 to generate concordance.          */
              defineLexeme( lexstart, lexterm, ( clc + reloc ), LABEL );
            }
            else
            {
              errorLexeme( &label_syntax, lexstart );
            }
            nextLexeme();           /* skip label                             */
            nextLexeme();           /* skip comma                             */
            break;

          case '=':
            if( isalpha( line[lexstart] ))
            {
              start = lexstart;
              term = lexterm;
              delimiter = line[lexterm];
              nextLexBlank();       /* skip symbol                            */
              nextLexBlank();       /* skip trailing =                        */
              val = getExprs();
              defineLexeme( start, term, val, DEFINED );
              printLine( line, 0, val, LINE_VAL );
            }
            else
            {
              errorLexeme( &symbol_syntax, lexstartprev );
              nextLexeme();         /* skip symbol                            */
              nextLexeme();         /* skip trailing =                        */
              getExprs();           /* skip expression                        */
            }
            break;

          default:
            if( isalpha( line[lexstart] ))
            {
              sym = evalSymbol();
              val = sym->val;
              if( M_PSEUDO( sym->type ))
              {
                nextLexeme();         /* Skip symbol                          */
                scanning_line = pseudoOperators( (PSEUDO_T)val & 07777 );
              }
              else
              {
                /* Identifier is not a pseudo-op, interpret as load value     */
                punchOutObject( clc, getExprs() & 07777 );
                incrementClc();
              }
            }
            else
            {
              /* Identifier is a value, interpret as load value               */
              punchOutObject( clc, getExprs() & 07777 );
              incrementClc();
            }
            break;
          } /* end switch                                                     */
          break;
        } /* end switch                                                       */
      } /* end if                                                             */
    } /* end while( scanning_line )                                           */
  } /* end while( TRUE )                                                      */
} /* onePass()                                                                */


/******************************************************************************/
/*                                                                            */
/*  Function:  getExprs                                                       */
/*                                                                            */
/*  Synopsis:  Or together a list of blank separated expressions, from the    */
/*             current lexeme onward.  Leave the current lexeme as            */
/*             the last one in the list.                                      */
/*                                                                            */
/******************************************************************************/
WORD16 getExprs()
{
  SYM_T  *symv;
  SYM_T  *symt;
  WORD16  temp;
  SYMTYP  temp_type;
  WORD16  value;
  SYMTYP  value_type;

  symv = getExpr();
  value = symv->val;
  value_type = symv->type;

  while( TRUE )
  {
    if( isdone( line[lexstart] ))
    {
      return( value );
    }
    switch( line[lexstart] )
    {
    case ')':
    case ']':
      return( value );

    default:
      break;
    }

    /* Interpret space as logical or                                          */
    symt = getExpr();
    temp = symt->val & 07777;
    temp_type = symt->type;

    switch( value_type )
    {
    case MRI:
    case MRIFIX:
      /* Previous symbol was a Memory Reference Instruction.                  */
      switch( temp_type )
      {
      case MRI:
      case MRIFIX:
        /* Current symbol is also a Memory Reference Instruction.             */
        value |= temp;          /* Just OR the MRI instructions.              */
        break;

      default:
        /* Now have the address part of the MRI instruction.                  */
        if( temp < 00200 )
        {
          value |= temp;        /* Page zero MRI.                             */
        }
        else if( (( fieldlc + reloc ) & 07600 ) <= temp
             && temp <= (( fieldlc + reloc ) | 0177 ))
        {
          value |= ( PAGE_BIT | (temp & ADDRESS_FIELD )); /* Current page MRI */
        }
        else
        {
          if(( value & INDIRECT_BIT ) == INDIRECT_BIT )
          {
            /* Already indirect, can't generate                               */
            errorSymbol( &illegal_indirect, symt->name, lexstartprev );
          }
          else
          {
            if( literals_on )
            {
              /* Now fix off page reference.                                  */
              /* Search current page literal pool for needed value.           */
              /* Set Indirect Current Page                                    */
              value |= ( 00600 | insertLiteral( &cp, temp ));
              indirect_generated = TRUE;
            }
            else
            {
              errorSymbol( &illegal_reference, symt->name, lexstartprev );
              value |= ( temp & 0177 );
            }
          }
        }
        break;
      }
      break;

    default:
        value |= temp;          /* Normal 12 bit value.                       */
        break;
    }
  } /* end while                                                              */
} /* getExprs()                                                               */


/******************************************************************************/
/*                                                                            */
/*  Function:  getExpr                                                        */
/*                                                                            */
/*  Synopsis:  Get an expression, from the current lexeme onward, leave the   */
/*             current lexeme as the one after the expression.  Expressions   */
/*             contain terminal symbols (identifiers) separated by operators. */
/*                                                                            */
/******************************************************************************/
SYM_T *getExpr()
{
  delimiter = line[lexterm];

  if( line[lexstart] == '-' )
  {
    nextLexBlank();
    sym_getexpr = *(eval());
    sym_getexpr.val = ( - sym_getexpr.val );
  }
  else
  {
    sym_getexpr = *(eval());
  }


  if( is_blank( delimiter ))
  {
    return( &sym_getexpr );
  }

  /* Here we assume the current lexeme is the operator separating the         */
  /* previous operator from the next, if any.                                 */
  while( TRUE )
  {
    /* assert line[lexstart] == delimiter                                     */
    if( is_blank( delimiter ))
    {
      return( &sym_getexpr );
    }

    switch( line[lexstart] )
    {
    case '+':                   /* add                                        */
      nextLexBlank();           /* skip over the operator                     */
      sym_getexpr.val += (eval())->val;
      break;

    case '-':                   /* subtract                                   */
      nextLexBlank();           /* skip over the operator                     */
      sym_getexpr.val -= (eval())->val;
      break;

    case '^':                   /* multiply                                   */
      nextLexBlank();           /* skip over the operator                     */
      sym_getexpr.val *= (eval())->val;
      break;

    case '%':                   /* divide                                     */
      nextLexBlank();           /* skip over the operator                     */
      sym_getexpr.val /= (eval())->val;
      break;

    case '&':                   /* and                                        */
      nextLexBlank();           /* skip over the operator                     */
      sym_getexpr.val &= (eval())->val;
      break;

    case '!':                   /* or                                         */
      nextLexBlank();           /* skip over the operator                     */
      sym_getexpr.val |= (eval())->val;
      break;

    default:
      if( isend( line[lexstart] ))
      {
        return( &sym_getexpr );
      }

      switch( line[lexstart] )
      {
      case '/':
      case ';':
      case ')':
      case ']':
      case '<':
        break;

      case '=':
        errorMessage( &illegal_equals, lexstart );
        moveToEndOfLine();
        sym_getexpr.val = 0;
        break;

      default:
        errorMessage( &illegal_expression, lexstart );
        moveToEndOfLine();
        sym_getexpr.val = 0;
        break;
      }
      return( &sym_getexpr );
    }
  } /* end while                                                              */
} /* getExpr()                                                                */


/******************************************************************************/
/*                                                                            */
/*  Function:  eval                                                           */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme, set delimiter and advance.*/
/*                                                                            */
/******************************************************************************/
SYM_T *eval()
{
  WORD16  digit;
  int     from;
  WORD16  loc;
  SYM_T  *sym;
  WORD16  val;

  val = 0;

  delimiter = line[lexterm];
  if( isalpha( line[lexstart] ))
  {
    sym = evalSymbol();
    if( M_UNDEFINED( sym->type ) && pass == 2 )
    {
      errorSymbol( &undefined_symbol, sym->name, lexstart );
      nextLexeme();
      return( sym );
    }
    else
    {
      nextLexeme();
      return( sym );
    }
  }
  else if( isdigit( line[lexstart] ))
  {
    from = lexstart;
    val = 0;
    while( from < lexterm )
    {
      if( isdigit( line[from] ))
      {
        digit = (WORD16) line[from++] - (WORD16) '0';
        if( digit < radix )
        {
          val = val * radix + digit;
        }
        else
        {
          errorLexeme( &number_not_radix, from - 1 );
          val = 0;
          from = lexterm;
        }
      }
      else
      {
        errorLexeme( &not_a_number, lexstart );
        val = 0;
        from = lexterm;
      }
    }
    nextLexeme();
    sym_eval.val = val;
    return( &sym_eval );
  }
  else
  {
    switch( line[lexstart] )
    {
    case '"':                   /* Character literal                          */
      if( cc + 2 < maxcc )
      {
        val = line[lexstart + 1] | 0200;
        delimiter = line[lexstart + 2];
        cc = lexstart + 2;
      }
      else
      {
        errorMessage( &no_literal_value, lexstart );
      }
      nextLexeme();
      break;

    case '.':                   /* Value of Current Location Counter          */
      val = clc + reloc;
      nextLexeme();
      break;

    case '[':                   /* Generate literal on page zero.             */
      if( !literals_on )
      {
        errorMessage( &literal_gen_off, lexstart );
      }
      nextLexBlank();           /* Skip bracket                               */
      val = (getExpr())->val & 07777;
      if( line[lexstart] == ']' )
      {
        nextLexBlank();         /* Skip end bracket                           */
      }
      else
      {
        /* errorMessage( "parens", lexstart );                                */
      }
      sym_eval.val = literals_on ? insertLiteral( &pz, val ) : 0;
      return( &sym_eval );

    case '(':                   /* Generate literal on current page.          */
      if( !literals_on )
      {
        errorMessage( &literal_gen_off, lexstart );
      }

      nextLexBlank();           /* Skip paren                                 */
      val = getExprs() & 07777;

      if( line[lexstart] == ')' )
      {
        nextLexBlank();         /* Skip end paren                             */
      }
      else
      {
        /* errorMessage( "parens", NULL );                                    */
      }

      loc = literals_on ? insertLiteral( &cp, val ) : 0;
      sym_eval.val = loc + (( clc + reloc ) & 077600 );
      return( &sym_eval );

    default:
      switch( line[lexstart] )
      {
      case '=':
        errorMessage( &illegal_equals, lexstart );
        moveToEndOfLine();
        break;

      default:
        errorMessage( &illegal_character, lexstart );
        break;
      }
      val = 0;                  /* On error, set value to zero.               */
      nextLexBlank();           /* Go past illegal character.                 */
    }
  }
  sym_eval.val = val;
  return( &sym_eval );
} /* eval()                                                                   */


/******************************************************************************/
/*                                                                            */
/*  Function:  inputDubl                                                      */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme as a double word.          */
/*                                                                            */
/******************************************************************************/
void inputDubl()
{
  WORD32 dublvalue;
  BOOL   scanning_line;

  scanning_line = TRUE;
  do
  {
    while( scanning_line )
    {
      if( isend( line[lexstart] ))
      {
        scanning_line = FALSE;
      }
      else
      {
        switch( line[lexstart] )
        {
        case '/':
          scanning_line = FALSE;
          break;

        case ';':
          nextLexeme();
          break;

        case '+':
          delimiter = line[lexterm];
          nextLexBlank();
        case '-':
        default:
          if( isdigit( line[lexstart] ) || line[lexstart] == '-' )
          {
            dublvalue = getDublExprs();
            punchOutObject( clc, (WORD16)(( dublvalue >> 12 ) & 07777 ));
            incrementClc();
            punchOutObject( clc, (WORD16)( dublvalue & 07777 ));
            incrementClc();
          }
          else
          {
            return;             /* Non-numeric input, back to assembly.       */
          }
          break;
        } /* end switch                                                       */
      } /* end if                                                             */

      if( error_in_line )
      {
        return;                 /* Error occurred, exit DUBL input mode.      */
      }
    } /* end while( scanning_line )                                           */
    readLine();
    nextLexeme();
    scanning_line = TRUE;
  }
  while( TRUE );
} /* inputDubl()                                                              */


/******************************************************************************/
/*                                                                            */
/*  Function:  getDublExprs                                                   */
/*                                                                            */
/*  Synopsis:  Get a DUBL expression.                                         */
/*                                                                            */
/******************************************************************************/
WORD32 getDublExprs()
{
  WORD32 dublvalue;

  dublvalue = getDublExpr();

  while( TRUE )
  {
    if( isdone( line[lexstart] ))
    {
      return( dublvalue );
    }
    else
    {
      errorMessage( &illegal_expression, lexstart - 1 );
      return( 0 );
    }
  } /* end while                                                              */
} /* getDublExprs()                                                           */


/******************************************************************************/
/*                                                                            */
/*  Function:  getDublExpr                                                    */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme as a double word.  The     */
/*             number is always considered to have a decimal radix.           */
/*                                                                            */
/******************************************************************************/
WORD32 getDublExpr()
{
  WORD32 dublvalue;

  delimiter = line[lexterm];
  if( line[lexstart] == '-' )
  {
    nextLexBlank();
    dublvalue = evalDubl( 0 );
    nextLexeme();
    /* Test for any value greater than 23 bits in length.                     */
    if( (unsigned long int)dublvalue > 040000000L )
    {
      errorMessage( &dubl_overflow, lexstart );
      dublvalue = 0;
    }
    dublvalue = -dublvalue;
  }
  else
  {
    dublvalue = evalDubl( 0 );
    nextLexeme();
    /* Test for any value greater than 23 bits in length.                     */
    if( (unsigned long int)dublvalue > 037777777L )
    {
      errorMessage( &dubl_overflow, lexstart );
      dublvalue = 0;
    }
  }

  if( is_blank( delimiter ))
  {
    return( dublvalue );
  }

  /* Here we assume the current lexeme is the operator separating the         */
  /* previous operator from the next, if any.                                 */
  while( TRUE )
  {
    /* assert line[lexstart] == delimiter                                     */
    if( is_blank( delimiter ))
    {
      errorMessage( &illegal_expression, lexstart );
      moveToEndOfLine();
      dublvalue = 0;
      return( dublvalue );
    }

    switch( line[lexstart] )
    {
    case '+':                   /* add                                        */
    case '-':                   /* subtract                                   */
    case '^':                   /* multiply                                   */
    case '%':                   /* divide                                     */
    case '&':                   /* and                                        */
    case '!':                   /* or                                         */
      errorMessage( &illegal_expression, lexstart );
      moveToEndOfLine();
      dublvalue = 0;
      break;

    default:
      if( isend( line[lexstart] ))
      {
        return( dublvalue );
      }

      switch( line[lexstart] )
      {
      case '/':
      case ';':
        break;

      default:
        errorMessage( &illegal_expression, lexstart );
        moveToEndOfLine();
        dublvalue = 0;
        break;
      }
      return( dublvalue );
    }
  } /* end while                                                              */
} /* getDublExpr()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  evalDubl                                                       */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme as a double word.  The     */
/*             number is always considered to have a decimal radix.           */
/*                                                                            */
/******************************************************************************/
WORD32 evalDubl( WORD32 initial_value )
{
  WORD32  digit;
  int     from;
  WORD32  dublvalue;
  WORD32  olddublvalue;

  overflow = FALSE;
  delimiter = line[lexterm];
  from = lexstart;
  dublvalue = initial_value;

  while( from < lexterm )
  {
    if( isdigit( line[from] ))
    {
      olddublvalue = dublvalue;
      digit = (WORD32)( line[from++] - '0' );
      dublvalue = dublvalue * 10 + digit;
      if( dublvalue < olddublvalue )
      {
        overflow = TRUE;
      }
    }
    else
    {
      errorLexeme( &not_a_number, from );
      dublvalue = 0;
      from = lexterm;
    }
  }
  return( dublvalue );
} /* evalDubl()                                                               */


/******************************************************************************/
/*                                                                            */
/*  Function:  inputFltg                                                      */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme as a Floating Point const. */
/*                                                                            */
/******************************************************************************/
void inputFltg()
{
  FLTG_T *fltg;
  BOOL    scanning_line;

  fltg_input = TRUE;            /* Set lexeme scanner for floating point.     */
  scanning_line = TRUE;
  while( TRUE )
  {
    while( scanning_line )
    {
      if( isend( line[lexstart] ))
      {
        scanning_line = FALSE;
      }
      else
      {
        switch( line[lexstart] )
        {
        case '/':
          scanning_line = FALSE;
          break;

        case ';':
          nextLexeme();
          break;

        case '+':
          delimiter = line[lexterm];
          nextLexBlank();
        case '-':
        default:
          if( isdigit( line[lexstart] ) || line[lexstart] == '-' )
          {
            fltg = getFltgExprs();
            punchOutObject( clc, ( fltg->exponent & 07777 ));
            incrementClc();
            punchOutObject( clc, (WORD16)(( fltg->mantissa >> 12 ) & 07777 ));
            incrementClc();
            punchOutObject( clc, (WORD16)( fltg->mantissa & 07777 ));
            incrementClc();
          }
          else
          {
            fltg_input = FALSE; /* Reset lexeme scanner.                      */
            return;             /* Non-numeric input, back to assembly.       */
          }
          break;
        } /* end switch                                                       */
      } /* end if                                                             */

      if( error_in_line )
      {
        fltg_input = FALSE;     /* Reset lexeme scanner.                      */
        return;                 /* Error occurred, exit FLTG input mode.      */
      }
    } /* end while( scanning_line )                                           */
    readLine();
    nextLexeme();
    scanning_line = TRUE;
  }
} /* inputFltg()                                                              */


/******************************************************************************/
/*                                                                            */
/*  Function:  getFltgExprs                                                   */
/*                                                                            */
/*  Synopsis:  Get a FLTG expression.                                         */
/*                                                                            */
/******************************************************************************/
FLTG_T *getFltgExprs()
{
  FLTG_T *fltg;

  fltg = getFltgExpr();

  while( TRUE )
  {
    if( isdone( line[lexstart] ))
    {
      return( fltg );
    }
    else
    {
      errorMessage( &illegal_expression, lexstart - 1 );
      return( 0 );
    }
  } /* end while                                                              */
} /* getFltgExprs()                                                           */


/******************************************************************************/
/*                                                                            */
/*  Function:  getFltgExpr                                                    */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme as a double word.  The     */
/*             number is always considered to have a decimal radix.           */
/*                                                                            */
/******************************************************************************/
FLTG_T *getFltgExpr()
{
  FLTG_T *fltg;

  delimiter = line[lexterm];
  fltg = evalFltg();
  /* Test for any value greater than 23 bits in length.                     */
  if( (unsigned long int)fltg->mantissa> 077777777L )
  {
    errorMessage( &fltg_overflow, lexstart );
  }

  if( is_blank( delimiter ))
  {
    return( fltg );
  }

  /* Here we assume the current lexeme is the operator separating the         */
  /* previous operator from the next, if any.                                 */
  while( TRUE )
  {
    /* assert line[lexstart] == delimiter                                     */
    if( is_blank( delimiter ))
    {
      errorMessage( &illegal_expression, lexstart );
      moveToEndOfLine();
      fltg = 0;
      return( fltg );
    }

    switch( line[lexstart] )
    {
    case '+':                   /* add                                        */
    case '-':                   /* subtract                                   */
    case '^':                   /* multiply                                   */
    case '%':                   /* divide                                     */
    case '&':                   /* and                                        */
    case '!':                   /* or                                         */
      errorMessage( &illegal_expression, lexstart );
      moveToEndOfLine();
      fltg = NULL;
      break;

    default:
      if( isend( line[lexstart] ))
      {
        return( fltg );
      }

      switch( line[lexstart] )
      {
      case '/':
      case ';':
        break;

      default:
        errorMessage( &illegal_expression, lexstart );
        moveToEndOfLine();
        fltg = NULL;
        break;
      }
      return( fltg );
    }
  } /* end while                                                              */
} /* getFltgExpr()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  evalFltg                                                       */
/*                                                                            */
/*  Synopsis:  Get the value of the current lexeme as a floating point value. */
/*             Floating point input is alwasy considered decimal.             */
/*             The general format of a floating point number is:              */
/*                +-ddd.dddE+-dd  where each d is a decimal digit.            */
/*                                                                            */
/******************************************************************************/
FLTG_T *evalFltg()
{
  int      current_state;
  int      current_col;
  WORD16   exponent;
  FLTG_T  *fltg;
  WORD32   input_value;
  BOOL     negate;
  BOOL     negate_exponent;
  int      next_state;
  int      right_digits;

  /* This uses a lexical analyzer to parse the floating point format.         */
  static BYTE state_table[][10] =
  {
    /*  0   1   2   3   4   5   6  Oolumn index                               */
    /*  +   -   d   .   E  sp   p      State  Comment                         */
     {  2,  1,  3,  4, 10, 10, 10  },  /*  0  Initial state.                  */
     { 11, 11,  3,  4, 11, 11, 11  },  /*  1  -                               */
     { 11, 11,  3,  4, 11, 11, 11  },  /*  2  +                               */
     { 10, 10, 10,  4,  6, 10, 10  },  /*  3  # (+-ddd)                       */
     { 11, 11,  5, 11, 11, 10, 10  },  /*  4  . (+-ddd.)                      */
     { 11, 11, 11, 11,  6, 10, 11  },  /*  5  # (+-ddd.ddd)                   */
     {  8,  7,  9, 11, 11, 11, 11  },  /*  6  E (+-ddd.dddE)                  */
     { 11, 11,  9, 11, 11, 11, 11  },  /*  7  - (+-ddd.dddE-                  */
     { 11, 11,  9, 11, 11, 11, 11  },  /*  8  + (+-ddd.dddE+                  */
     { 11, 11, 11, 11, 11, 10, 11  }   /*  9  # (+-ddd.dddE+-dd               */
                                       /* 10  Completion state                */
                                       /* 11  Error state.                    */
  };

  delimiter = line[lexterm];
  fltg = &fltg_ac;
  fltg->exponent = 0;
  fltg->mantissa = 0;
  input_value = 0;
  negate = FALSE;
  negate_exponent = FALSE;
  next_state = 0;
  exponent = 0;
  right_digits = 0;
  current_state = 0;

  while( TRUE )
  {
    /* Classify character.  This is the column index.                         */
    switch( line[lexstart] )
    {
    case '+':
      current_col = 0;
      break;

    case '-':
      current_col = 1;
      break;

    case '.':
      current_col = 3;
      break;

    case 'E':  case 'e':
      current_col = 4;
      break;

    default:
      if( isdigit( line[lexstart] ))
      {
        current_col = 2;
      }
      else if( isdone( line[lexstart] ))
      {
        current_col = 5;
      }
      else
      {
        current_col = 6;
      }
      break;
    }

    next_state = state_table[current_state][current_col];

    switch( next_state )
    {
    case 1:                             /*  -                                 */
      negate = TRUE;
    case 2:                             /*  +                                 */
      delimiter = line[lexterm];        /* Move past the + or - character.    */
      nextLexBlank();
      break;

    case 3:                             /* Number  (+-ddd)                    */
      input_value = evalDubl( 0 );      /* Integer part of the number.        */
      nextLexeme();                     /* Move past previous lexeme.         */
      break;

    case 4:
      delimiter = line[lexterm];
      nextLexBlank();                   /* Move past the . character.         */
      break;

    case 5:                             /* .  (+-ddd.ddd)                     */
                                        /* Fractional part of the number.     */
      input_value = evalDubl( input_value );
      right_digits = lexterm - lexstart;/* Digit count to right of decimal.   */
      nextLexeme();                     /* Move past previous lexeme.         */
      break;

    case 6:                             /* E  (+-ddd.dddE)                    */
      delimiter = line[lexterm];        /* Move past the E.                   */
      nextLexBlank();
      break;

    case 7:                             /* -  (+-ddd.dddE-)                   */
      negate_exponent = TRUE;
    case 8:                             /* +  (+-ddd.dddE+)                   */
      delimiter = line[lexterm];        /* Move past the + or - character.    */
      nextLexBlank();
      break;

    case 9:                             /* #  (+-ddd.dddE+-dd)                */
      exponent = (int)evalDubl( 0 );    /* Exponent of floating point number. */
      if( negate_exponent ) { exponent = - exponent; }
      nextLexeme();                     /* Move past previous lexeme.         */
      break;

    case 10:                            /* Floating number parsed, convert    */
                                        /* the number.                        */
      /* Get the exponent for the number as input.                            */
      exponent -= right_digits;

      /* Remove trailing zeros and adjust the exponent accordingly.           */
      while(( input_value % 10 ) == 0 )
      {
        input_value /= 10;
        exponent++;
      }

      /* Convert the number to floating point.  The number is calculated with */
      /* a 27 bit mantissa to improve precision.  The extra 3 bits are        */
      /* discarded after the result has been calculated.                      */
      fltg->exponent = 26;
      fltg->mantissa = input_value << 3;
      normalizeFltg( fltg );


      while( exponent != 0 )
      {
        if( exponent < 0 )
        {
          /* Decimal point is to the left. */
          fltg->mantissa /= 10;
          normalizeFltg( fltg );
          exponent++;
        }
        else if( exponent > 0 )
        {
          /* Decimal point is to the right. */
          fltg->mantissa *= 10;
          normalizeFltg( fltg );
          exponent--;
        }
      }

      /* Discard the extra precsion used for calculating the number.          */
      fltg->mantissa >>= 3;
      fltg->exponent -= 3;
      if( negate )
      {
        fltg->mantissa = (- fltg->mantissa ) & 077777777L;
      }
      return( fltg );

    case 11:                            /* Error in format.                   */
      /* Not a properly constructued floating point number.                   */
      return( fltg );
    default:
      break;
    }
    /* Set state for next pass through the loop.                              */
    current_state = next_state;
  }
} /* evalFltg()                                                               */



/******************************************************************************/
/*                                                                            */
/*  Function:  normalizeFltg                                                  */
/*                                                                            */
/*  Synopsis:  Normalize a PDP-8 double precision floating point number.      */
/*                                                                            */
/******************************************************************************/
void normalizeFltg( FLTG_T *fltg )
{
  /* Normalize the floating point number.                                     */
  if( fltg->mantissa != 0 )
  {
    if(( fltg->mantissa & ~0x3FFFFFFL ) == 0 )
    {
      while(( fltg->mantissa & ~0x1FFFFFFL ) == 0 )

      {
        fltg->mantissa <<= 1;
        fltg->exponent--;
      }
    }
    else
    {
      while(( fltg->mantissa & ~0x3FFFFFFL ) != 0 )
      {
        fltg->mantissa >>= 1;
        fltg->exponent++;
      }
    }
  }
  else
  {
    fltg->exponent = 0;
  }
  return;
}


/******************************************************************************/
/*                                                                            */
/*  Function:  incrementClc                                                   */
/*                                                                            */
/*  Synopsis:  Set the next assembly location.  Test for collision with       */
/*             the literal tables.                                            */
/*                                                                            */
/******************************************************************************/
WORD16 incrementClc()
{
  testForLiteralCollision( clc );

  /* Incrementing the location counter is not to change field setting.        */
  clc = ( clc & 070000 ) + (( clc + 1 ) & 07777 );
  fieldlc = clc & 07777;
  return( clc );
} /* incrementClc()                                                           */


/******************************************************************************/
/*                                                                            */
/*  Function:  testForLiteralCollision                                        */
/*                                                                            */
/*  Synopsis:  Test the given location for collision with the literal tables. */
/*                                                                            */
/******************************************************************************/
BOOL testForLiteralCollision( WORD16 loc )
{
  WORD16  pagelc;
  BOOL    result = FALSE;
  WORD16  tmppage;

  tmppage = loc & 07600;
  pagelc  = loc & 00177;

  if( tmppage == 0 )
  {
    if( pagelc >= pz.loc && !pz.error )
    {
      errorMessage( &pz_literal_overflow, -1 );
      pz.error = TRUE;
      result = TRUE;
    }
  }
  else
  {
    if( pagelc >= cp.loc && !cp.error )
    {
      errorMessage( &literal_overflow, -1 );
      cp.error = TRUE;
      result = TRUE;
    }
  }
  return( result );
} /* testForLiteralCollision()                                                */


/******************************************************************************/
/*                                                                            */
/*  Function:  readLine                                                       */
/*                                                                            */
/*  Synopsis:  Get next line of input.  Print previous line if needed.        */
/*                                                                            */
/******************************************************************************/
void readLine()
{
  WORD16  ix;
  WORD16  iy;
  char   inpline[LINELEN];

  listLine();                   /* List previous line if needed.              */
  lineno++;                     /* Count lines read.                          */
  indirect_generated = FALSE;   /* Mark no indirect address generated.        */
  listed = FALSE;               /* Mark as not listed.                        */
  cc = 0;                       /* Initialize column counter.                 */
  lexstartprev = 0;

  if(( fgets( inpline, LINELEN - 1, infile )) == NULL )
  {
    inpline[0] = '$';
    inpline[1] = '\n';
    inpline[2] = '\0';
    error_in_line = TRUE;
  }
  else
  {
    error_in_line = FALSE;
  }

  /* Remove any tabs from the input line by inserting the required number     */
  /* of spaces to simulate 8 character tab stops.                             */
  for( ix = 0, iy = 0; inpline[ix] != '\0'; ix++ )
  {
    switch( inpline[ix] )
    {
    case '\t':
      do
      {
        line[iy] = ' ';
        iy++;
      }
      while(( iy % 8 ) != 0 );
      break;

    default:
      line[iy] = inpline[ix];
      iy++;
      break;
    }
  }
  line[iy] = '\0';

  /* If the line is terminated by CR-LF, remove, the CR.                      */
  if( line[iy - 2] == '\r' )
  {
    iy--;
    line[iy - 1] = line[iy - 0];
    line[iy] = '\0';
  }
  maxcc = iy;                   /* Save the current line length.              */

  /* Save the first line for possible use as the listing title.               */
  if( lineno == 1 )
  {
    strcpy( list_title, line );
  }
} /* readLine()                                                               */


/******************************************************************************/
/*                                                                            */
/*  Function:  listLine                                                       */
/*                                                                            */
/*  Synopsis:  Output a line to the listing file.                             */
/*                                                                            */
/******************************************************************************/
void listLine()
/* generate a line of listing if not already done!                            */
{
  if( listfile != NULL && listed == FALSE )
  {
    printLine( line, 0, 0, LINE );
  }
} /* listLine()                                                               */


/******************************************************************************/
/*                                                                            */
/*  Function:  printPageBreak                                                 */
/*                                                                            */
/*  Synopsis:  Output a Top of Form and listing header if new page necessary. */
/*                                                                            */
/******************************************************************************/
void printPageBreak()
{
  if( page_lineno >= LIST_LINES_PER_PAGE )
         /*  ( list_lineno % LIST_LINES_PER_PAGE ) == 0 ) */
  {
    if( !list_title_set )
    {
      /* strcpy( list_title, line ); */
      if( list_title[strlen(list_title) - 1] == '\n' )
      {
        list_title[strlen(list_title) - 1] = '\0';
      }
      if( strlen( list_title ) > TITLELEN )
      {
        list_title[TITLELEN] = '\0';
      }
      list_title_set = TRUE;
    }
    topOfForm( list_title, NULL );
  }
} /* printPageBreak()                                                         */


/******************************************************************************/
/*                                                                            */
/*  Function:  printLine                                                      */
/*                                                                            */
/*  Synopsis:  Output a line to the listing file with new page if necessary.  */
/*                                                                            */
/******************************************************************************/
void printLine( char *line, WORD16 loc, WORD16 val, LINESTYLE_T linestyle )
{
  if( listfile == NULL )
  {
    save_error_count = 0;
    return;
  }

  printPageBreak();

  list_lineno++;
  page_lineno++;
  switch( linestyle )
  {
  default:
  case LINE:
    fprintf(listfile, "%5d             ", lineno );
    fputs( line, listfile );
    listed = TRUE;
    break;

  case LINE_VAL:
    fprintf(listfile, "%5d       %4.4o  ", lineno, val );
    fputs( line, listfile );
    listed = TRUE;
    break;

  case LINE_LOC_VAL:
    if( !listed )
    {
      if( indirect_generated )
      {
        fprintf( listfile, "%5d %5.5o %4.4o@ ", lineno, loc, val );
      }
      else
      {
        fprintf( listfile, "%5d %5.5o %4.4o  ", lineno, loc, val );
      }
      fputs( line, listfile );
      listed = TRUE;
    }
    else
    {
      fprintf( listfile, "      %5.5o %4.4o\n", loc, val );
    }
    break;

  case LOC_VAL:
    fprintf( listfile, "      %5.5o %4.4o\n", loc, val );
    break;
  }
  printErrorMessages();
} /* printLine()                                                              */


/******************************************************************************/
/*                                                                            */
/*  Function:  printErrorMessages                                             */
/*                                                                            */
/*  Synopsis:  Output any error messages from the current list of errors.     */
/*                                                                            */
/******************************************************************************/
void printErrorMessages()
{
  WORD16  ix;
  WORD16  iy;

  if( listfile != NULL )
  {
    /* If any errors, display them now.                                       */
    for( iy = 0; iy < save_error_count; iy++ )
    {
      printPageBreak();
      fprintf( listfile, "%-18.18s", error_list[iy].mesg );
      if( error_list[iy].col >= 0 )
      {
        for( ix = 0; ix < error_list[iy].col; ix++ )
        {
          if( line[ix] == '\t' )
          {
            putc( '\t', listfile );
          }
          else
          {
            putc( ' ', listfile );
          }
        }
        fputs( "^", listfile );
        list_lineno++;
        page_lineno++;
      }
      fputs( "\n", listfile );
    }
  }
  save_error_count = 0;
} /* printErrorMessages()                                                     */


/******************************************************************************/
/*                                                                            */
/*  Function:  endOfBinary                                                    */
/*                                                                            */
/*  Synopsis:  Outputs both literal tables at the end of a binary segment.    */
/*                                                                            */
/******************************************************************************/
void endOfBinary()
{
  /* Points to end of page for () operands.                                   */
  punchLiteralPool( &cp, clc - 1 );
  /* Points to end of page zero for [] operands.                              */
  punchLiteralPool( &pz, 0 );
  if( error_in_line )
  {
    listed = TRUE;
    clc = ( clc & 070000 ) + (( clc - 1 ) & 07777 );
    errorMessage( &end_of_file, -1 );
    clc = ( clc & 070000 ) + (( clc + 1 ) & 07777 );
  }
  else
  {
    listLine();             /* List line if not done yet.                     */
  }
  return;
} /* endOfBinary()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  punchChecksum                                                  */
/*                                                                            */
/*  Synopsis:  Output a checksum if the current mode requires it and an       */
/*             object file exists.                                            */
/*                                                                            */
/******************************************************************************/
void punchChecksum()
{
  /* If the assembler has output any BIN data output the checksum.            */
  if( binary_data_output && !rim_mode )
  {
    punchLocObject( 0, checksum );
  }
  binary_data_output = FALSE;
  checksum = 0;
} /* punchChecksum()                                                          */


/******************************************************************************/
/*                                                                            */
/*  Function:  punchLeader                                                    */
/*                                                                            */
/*  Synopsis:  Generate 2 feet of leader on object file, as per DEC           */
/*             documentation.  Paper tape has 10 punches per inch.            */
/*                                                                            */
/******************************************************************************/
void punchLeader( int count )
{
  int ix;

  /* If value is zero, set to the default of 2 feet of leader.                */
  count = ( count == 0 ) ? 240 : count;

  if( objectfile != NULL )
  {
    for( ix = 0; ix < count; ix++ )
    {
      fputc( 0200, objectfile );
    }
  }
} /* punchLeader()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  punchOrigin                                                    */
/*                                                                            */
/*  Synopsis:  Output an origin to the object file.                           */
/*                                                                            */
/******************************************************************************/
void punchOrigin( WORD16 loc )
{
  punchObject((( loc >> 6 ) & 0077 ) | 0100 );
  punchObject( loc & 0077 );
} /* punchOrigin()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  punchObject                                                    */
/*                                                                            */
/*  Synopsis:  Put one character to object file and include it in checksum.   */
/*                                                                            */
/******************************************************************************/
void punchObject( WORD16 val )
{
  val &= 0377;
  if( objectfile != NULL )
  {
    fputc( val, objectfile );
  }
  checksum += val;
  binary_data_output = TRUE;
} /* punchObject()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  punchOutObject                                                 */
/*                                                                            */
/*  Synopsis:  Output the current line and then then punch value to the       */
/*             object file.                                                   */
/*                                                                            */
/******************************************************************************/
void punchOutObject( WORD16 loc, WORD16 val )
{
  printLine( line, ( field | loc ), val, LINE_LOC_VAL );
  punchLocObject( loc, val );
} /* punchOutObject()                                                         */

/******************************************************************************/
/*                                                                            */
/*  Function:  punchLocObject                                                 */
/*                                                                            */
/*  Synopsis:  Output the word (with origin if rim format) to the object file.*/
/*                                                                            */
/******************************************************************************/
void punchLocObject( WORD16 loc, WORD16 val )
{
  if( rim_mode )
  {
    punchOrigin( loc );
  }
  punchObject(( val >> 6 ) & 0077 );
  punchObject( val & 0077 );
} /* punchLocObject()                                                         */


/******************************************************************************/
/*                                                                            */
/*  Function:  punchLiteralPool                                               */
/*                                                                            */
/*  Synopsis:  Output the current page data.                                  */
/*                                                                            */
/******************************************************************************/
void punchLiteralPool( LPOOL_T *p, WORD16 lpool_page )
{
  WORD16  loc;
  WORD16  tmplc;

  lpool_page &= 07600;

  if( p->loc < 00200 )
  {
    if( !rim_mode )
    {
      /* Put out origin if not in rim mode.                                   */
      punchOrigin( p->loc | lpool_page );
    }
    /* Put the literals in the object file.                                    */
    for( loc = p->loc; loc < 00200; loc++ )
    {
      tmplc = loc + lpool_page;
      printLine( line, (field | tmplc), p->pool[loc], LOC_VAL );
      punchLocObject( tmplc, p->pool[loc] );
    }
    p->error = FALSE;
    p->loc = 00200;
  }
} /* punchLiteralPool()                                                       */


/******************************************************************************/
/*                                                                            */
/*  Function:  insertLiteral                                                  */
/*                                                                            */
/*  Synopsis:  Add a value to the given literal pool if not already in pool.  */
/*             Return the location of the value in the pool.                  */
/*                                                                            */
/******************************************************************************/
WORD16 insertLiteral( LPOOL_T *pool, WORD16 value )
{
  WORD16  ix;
  LPOOL_T *p;

  p = pool;

  /* If page zero is the current page, make sure that literals are inserted   */
  /* in the page zero literal table.                                          */
  if(( clc & 07600 ) == 0 )
  {
     p = &pz;
  }

  /* Search the literal pool for any occurence of the needed value.           */
  ix = PAGE_SIZE - 1;
  while( ix >= p->loc && p->pool[ix] != value )
  {
    ix--;
  }

  /* Check if value found in literal pool. If not, then insert value.         */
  if( ix < p->loc )
  {
    (p->loc)--;
    p->pool[p->loc] = value;
    ix = p->loc;
  }
  return( ix );
} /* insertLiteral()                                                          */


/******************************************************************************/
/*                                                                            */
/*  Function:  printSymbolTable                                               */
/*                                                                            */
/*  Synopsis:  Output the symbol table.                                       */
/*                                                                            */
/******************************************************************************/
void printSymbolTable()
{
  int    col;
  int    cx;
  char  *fmt;
  int    ix;
  char   mark;
  int    page;
  int    row;
  int    symbol_base;
  int    symbol_lines;

  symbol_base = number_of_fixed_symbols;

  for( page=0, list_lineno=0, col=0, ix=symbol_base; ix < symbol_top; page++ )
  {
    topOfForm( list_title, s_symtable );
    symbol_lines = LIST_LINES_PER_PAGE - page_lineno;

    for( row = 0; page_lineno < LIST_LINES_PER_PAGE && ix < symbol_top; row++)
    {
      list_lineno++;
      page_lineno++;
      fprintf( listfile, "%5d", list_lineno );

      for( col = 0; col < SYMBOL_COLUMNS && ix < symbol_top; col++ )
      {
        /* Get index of symbol for the current line and column                         */
        cx = symbol_lines * ( SYMBOL_COLUMNS * page + col ) + row;
        cx += symbol_base;

        /* Make sure that there is a symbol to be printed.                    */
        if( number_of_fixed_symbols <= cx && cx < symbol_top )
        {
          switch( symtab[cx].type & LABEL )
          {
          case LABEL:
            fmt = " %c%-6.6s %5.5o ";
            break;

          default:
            fmt = " %c%-6.6s  %4.4o ";
            break;
          }

          switch( symtab[cx].type & ( DEFINED | REDEFINED ))
          {
          case UNDEFINED:
            mark = '?';
            break;

          case REDEFINED:
            mark = '#';
            break;

          default:
            mark = ' ';
            break;
          }
          fprintf( listfile, fmt, mark, symtab[cx].name, symtab[cx].val );
          ix++;
        }
      }
      fprintf( listfile, "\n" );
    }
  }
} /* printSymbolTable()                                                       */


/******************************************************************************/
/*                                                                            */
/*  Function:  printPermanentSymbolTable                                      */
/*                                                                            */
/*  Synopsis:  Output the permanent symbol table to a file suitable for       */
/*             being input after the EXPUNGE pseudo-op.                       */
/*                                                                            */
/******************************************************************************/
void printPermanentSymbolTable()
{
  int     ix;
  FILE   *permfile;
  char  *s_type;

  if(( permfile = fopen( permpathname, "w" )) == NULL )
  {
    exit( 2 );
  }

  fprintf( permfile, "/ PERMANENT SYMBOL TABLE\n/\n" );
  fprintf( permfile, "        EXPUNGE\n/\n" );
  /* Print the memory reference instructions first.                           */
  s_type = "FIXMRI";
  for( ix = 0; ix < symbol_top; ix++ )
  {
    if( M_MRI( symtab[ix].type ))
    {
      fprintf( permfile, "%-7s %s=%4.4o\n",
                                    s_type, symtab[ix].name, symtab[ix].val );
    }
  }

  s_type = " ";
  for( ix = 0; ix < symbol_top; ix++ )
  {
    if( M_FIXED( symtab[ix].type ))
    {
      if( !M_MRI( symtab[ix].type ) && !M_PSEUDO( symtab[ix].type ))
      {
        fprintf( permfile, "%-7s %s=%4.4o\n",
                                    s_type, symtab[ix].name, symtab[ix].val );
      }
    }
  }
  fprintf( permfile, "/\n        FIXTAB\n" );
  fclose( permfile );
} /* printPermanentSymbolTable()                                              */


/******************************************************************************/
/*                                                                            */
/*  Function:  printCrossReference                                            */
/*                                                                            */
/*  Synopsis:  Output a cross reference (concordance) for the file being      */
/*             assembled.                                                     */
/*                                                                            */
/******************************************************************************/
void printCrossReference()
{
  int    ix;
  int    symbol_base;
  int    xc;
  int    xc_index;
  int    xc_refcount;
  int    xc_cols;

  /* Force top of form for first page.                                        */
  page_lineno = LIST_LINES_PER_PAGE;

  list_lineno = 0;
  symbol_base = number_of_fixed_symbols;

  for( ix = symbol_base; ix < symbol_top; ix++ )
  {
    list_lineno++;
    page_lineno++;
    if( page_lineno >= LIST_LINES_PER_PAGE )
    {
      topOfForm( list_title, s_xref );
    }

    fprintf( listfile, "%5d", list_lineno );

    /* Get reference count & index into concordance table for this symbol.    */
    xc_refcount = symtab[ix].xref_count;
    xc_index = symtab[ix].xref_index;
    /* Determine how to label symbol on concordance.                          */
    switch( symtab[ix].type & ( DEFINED | REDEFINED ))
    {
    case UNDEFINED:
      fprintf( listfile, " U         ");
      break;

    case REDEFINED:
      fprintf( listfile, " M  %5d  ", xreftab[xc_index] );
      break;

    default:
      fprintf( listfile, " A  %5d  ", xreftab[xc_index] );
      break;
    }
    fprintf( listfile, "%-6.6s  ", symtab[ix].name );

    /* Output the references, 8 numbers per line after symbol name.           */
    for( xc_cols = 0, xc = 1; xc < xc_refcount + 1; xc++, xc_cols++ )
    {
      if( xc_cols >= XREF_COLUMNS )
      {
        xc_cols = 0;
        page_lineno++;
        if( page_lineno >= LIST_LINES_PER_PAGE )
        {
          topOfForm( list_title, s_xref);
        }
        list_lineno++;
        fprintf( listfile, "\n%5d%-19s", list_lineno, " " );
      }
      fprintf( listfile, "  %5d", xreftab[xc_index + xc] );
    }
    fprintf( listfile, "\n" );
  }
} /* printCrossReference()                                                    */


/******************************************************************************/
/*                                                                            */
/*  Function:  topOfForm                                                      */
/*                                                                            */
/*  Synopsis:  Prints title and sub-title on top of next page of listing.     */
/*                                                                            */
/******************************************************************************/
void topOfForm( char *title, char *sub_title )
{
  char temp[10];

  list_pageno++;
  strcpy( temp, s_page );
  sprintf( temp, "%s %d", s_page, list_pageno );

  /* Output a top of form if not the first page of the listing.               */
  if( list_pageno > 1 )
  {
    fprintf( listfile, "\f" );
  }
  fprintf( listfile, "\n\n\n      %-63s %10s\n", title, temp );

  /* Reset the current page line counter.                                     */
  page_lineno = 3;
  if( sub_title != NULL )
  {
    fprintf( listfile, "%80s\n", sub_title );
    page_lineno++;
  }
  else
  {
    fprintf( listfile, "\n" );
    page_lineno++;
  }
  fprintf( listfile, "\n" );
  page_lineno++;
} /* topOfForm()                                                              */


/******************************************************************************/
/*                                                                            */
/*  Function:  lexemeToName                                                   */
/*                                                                            */
/*  Synopsis:  Convert the current lexeme into a string.                      */
/*                                                                            */
/******************************************************************************/
char *lexemeToName( char *name, int from, int term )
{
  int  to;

  to = 0;

  while( from < term && to < ( SYMLEN - 1 ))
  {
    name[to++] = toupper( line[from++] );
  }

  while( to < SYMLEN )
  {
    name[to++] = '\0';
  }
  return( name );
} /* lexemeToName()                                                           */

/******************************************************************************/
/*                                                                            */
/*  Function:  defineLexeme                                                   */
/*                                                                            */
/*  Synopsis:  Put lexeme into symbol table with a value.                     */
/*                                                                            */
/******************************************************************************/
SYM_T *defineLexeme( int     start,     /* start of lexeme being defined.     */
                     int     term,      /* end+1 of lexeme being defined.     */
                     WORD16  val,       /* value of lexeme being defined.     */
                     SYMTYP  type )     /* how symbol is being defined.       */
{
  char  name[SYMLEN];

  lexemeToName( name, start, term);
  return( defineSymbol( name, val, type, start ));
} /* defineLexeme()                                                           */


/******************************************************************************/
/*                                                                            */
/*  Function:  defineSymbol                                                   */
/*                                                                            */
/*  Synopsis:  Define a symbol in the symbol table, enter symbol name if not  */
/*             not already in table.                                          */
/*                                                                            */
/******************************************************************************/
SYM_T *defineSymbol( char *name, WORD16 val, SYMTYP type, WORD16 start )
{
  SYM_T  *sym;
  int     xref_count;

  if( strlen( name ) < 1 )
  {
    return( &sym_undefined );   /* Protect against non-existent names.        */
  }
  sym = lookup( name );
  if( M_FIXED( sym->type ))
  {
      return( sym );            /* Can't modify permanent symbols.            */
  }

  xref_count = 0;               /* Set concordance for normal defintion.      */

  if( M_DEFINED( sym->type ))
  {
    if( pass == 2 && sym->val != val )
    {
      /* Generate diagnostic if redefining a symbol.                          */
      if( M_REDEFINED( sym->type ))
      {
        errorSymbol( &redefined_symbol, sym->name, start );
      }
      type = type | REDEFINED;
      sym->xref_count++;      /* Referenced suymbol, count it.                */
      xref_count = sym->xref_count;
    }
  }

  if( pass == 2 && xref )
  {
    /* Put the definition line number in the concordance table.               */
    /* Defined symbols are not counted as references.                         */
    xreftab[sym->xref_index] = lineno;
    /* Put the line number in the concordance table.                          */
    xreftab[sym->xref_index + xref_count] = lineno;
  }

  /* Now set the value and the type.                                          */
  sym->val = ( type == LABEL) ? val : val & 07777;
  sym->type = ( pass == 1 ) ? ( type | CONDITION ) : type;
  return( sym );
} /* defineSymbol()                                                           */


/******************************************************************************/
/*                                                                            */
/*  Function:  lookup                                                         */
/*                                                                            */
/*  Synopsis:  Find a symbol in table.  If not in table, enter symbol in      */
/*             table as undefined.  Return address of symbol in table.        */
/*                                                                            */
/******************************************************************************/
SYM_T *lookup( char *name )
{
  int     ix;                   /* Insertion index                            */
  int     lx;                   /* Left index                                 */
  int     rx;                   /* Right index                                */

  /* First search the permanent symbols.                                      */
  lx = 0;
  ix = binarySearch( name, lx, number_of_fixed_symbols );

  /* If symbol not in permanent symbol table.                                 */
  if( ix < 0 )
  {
    /* Now try the user symbol table.                                         */
    ix = binarySearch( name, number_of_fixed_symbols, symbol_top );

    /* If symbol not in user symbol table.                                    */
    if( ix < 0 )
    {
      /* Must put symbol in table if index is negative.                       */
      ix = ~ix;
      if( symbol_top + 1 >= SYMBOL_TABLE_SIZE )
      {
        errorSymbol( &symbol_table_full, name, lexstart );
        exit( 1 );
      }

      for( rx = symbol_top; rx >= ix; rx-- )
      {
        symtab[rx + 1] = symtab[rx];
      }
      symbol_top++;

      /* Enter the symbol as UNDEFINED with a value of zero.                  */
      strcpy( symtab[ix].name, name );
      symtab[ix].type = UNDEFINED;
      symtab[ix].val  = 0;
      symtab[ix].xref_count = 0;
      if( xref && pass == 2 )
      {
        xreftab[symtab[ix].xref_index] = 0;
      }
    }
  }

  return( &symtab[ix] );        /* Return the location of the symbol.         */
} /* lookup()                                                                 */


/******************************************************************************/
/*                                                                            */
/*  Function:  binarySearch                                                   */
/*                                                                            */
/*  Synopsis:  Searches the symbol table within the limits given.  If the     */
/*             symbol is not in the table, it returns the insertion point.    */
/*                                                                            */
/******************************************************************************/
int binarySearch( char *name, int start, int symbol_count )
{
  int     lx;                   /* Left index                                 */
  int     mx;                   /* Middle index                               */
  int     rx;                   /* Right index                                */
  int     compare;              /* Results of comparison                      */

  lx = start;
  rx = symbol_count - 1;
  while( lx <= rx )
  {
    mx = ( lx + rx ) / 2;   /* Find center of search area.                    */

    compare = strcmp( name, symtab[mx].name );

    if( compare < 0 )
    {
      rx = mx - 1;
    }
    else if( compare > 0 )
    {
      lx = mx + 1;
    }
    else
    {
      return( mx );         /* Found a match in symbol table.                 */
    }
  } /* end while                                                              */
  return( ~lx );            /* Return insertion point.                        */
} /* binarySearch()                                                           */


/******************************************************************************/
/*                                                                            */
/*  Function:  compareSymbols                                                 */
/*                                                                            */
/*  Synopsis:  Used to presort the symbol table when starting assembler.      */
/*                                                                            */
/******************************************************************************/
int compareSymbols( const void *a, const void *b )
{
  return( strcmp( ((SYM_T *) a)->name, ((SYM_T *) b)->name ));
} /* compareSymbols()                                                         */


/******************************************************************************/
/*                                                                            */
/*  Function:  evalSymbol                                                     */
/*                                                                            */
/*  Synopsis:  Get the pointer for the symbol table entry if exists.          */
/*             If symbol doesn't exist, return a pointer to the undefined sym */
/*                                                                            */
/******************************************************************************/
SYM_T *evalSymbol()
{
  char   name[SYMLEN];
  SYM_T *sym;

  sym = lookup( lexemeToName( name, lexstart, lexterm ));

  /* The symbol goes in the concordance iff it is in a different position in  */
  /* the assembler source file.                                               */
  if( lexstart != last_xref_lexstart ||  lineno != last_xref_lineno )
  {
    sym->xref_count++;          /* Count the number of references to symbol.  */
    last_xref_lexstart = lexstart;
    last_xref_lineno = lineno;

    /* Put the line number in the concordance table.                          */
    if( xref && pass == 2 )
    {
      xreftab[sym->xref_index + sym->xref_count] = lineno;
    }
  }
  return( sym );
} /* evalSymbol()                                                             */


/******************************************************************************/
/*                                                                            */
/*  Function:  moveToEndOfLine                                                */
/*                                                                            */
/*  Synopsis:  Move the parser input to the end of the current input line.    */
/*                                                                            */
/******************************************************************************/
void moveToEndOfLine()
{
  while( !isend( line[cc] )) cc++;
  lexstart = cc;
  lexterm = cc;
  lexstartprev = lexstart;
} /* moveToEndOfLine()                                                        */

/******************************************************************************/
/*                                                                            */
/*  Function:  nextLexeme                                                     */
/*                                                                            */
/*  Synopsis:  Get the next lexical element from input line.                  */
/*                                                                            */
/******************************************************************************/
void nextLexeme()
{
  /* Save start column of previous lexeme for diagnostic messages.            */
  lexstartprev = lexstart;
  lextermprev = lexterm;

  while( is_blank( line[cc] )) { cc++; }
  lexstart = cc;

  if( isalnum( line[cc] ))
  {
    while( isalnum( line[cc] )) { cc++; }
  }
  else if( isend( line[cc] ))
  {
    /* End-of-Line, don't advance cc!                                         */
  }
  else
  {
    switch( line[cc] )
    {
    case '"':     /* Quoted letter                                            */
      if( cc + 2 < maxcc )
      {
        cc++;
        cc++;
      }
      else
      {
        errorMessage( &no_literal_value, lexstart );
        cc++;
      }
      break;

    case '/':     /* Comment, don't advance cc!                               */
      break;

    default:      /* All other punctuation.                                   */
      cc++;
      break;
    }
  }
  lexterm = cc;
} /* nextLexeme()                                                             */


/******************************************************************************/
/*                                                                            */
/*  Function:  nextLexBlank                                                   */
/*                                                                            */
/*  Synopsis:  Used to prevent illegal blanks in expressions.                 */
/*                                                                            */
/******************************************************************************/
void nextLexBlank()
{
  nextLexeme();
  if( is_blank( delimiter ))
  {
    errorMessage( &illegal_blank, lexstart - 1 );
  }
  delimiter = line[lexterm];
} /* nextLexBlank()                                                           */



/******************************************************************************/
/*                                                                            */
/*  Function:  pseudoOperators                                                */
/*                                                                            */
/*  Synopsis:  Process pseudo-ops (directives).                               */
/*                                                                            */
/******************************************************************************/
BOOL pseudoOperators( PSEUDO_T val )
{
  int     count;
  int     delim;
  int     index;
  int     ix;
  int     lexstartsave;
  WORD16  newfield;
  WORD16  oldclc;
  int     pack;
  BOOL    status;
  SYM_T  *sym;
  FILE   *temp;
  int     term;
  WORD16  value;
  int	  leaderlen;

  status = TRUE;
  switch( (PSEUDO_T) val )
  {
  case BANK:
    errorSymbol( &no_pseudo_op, "BANK", lexstartprev );
    /* should select a different 32K out of 128K                              */
    break;

  case BINPUNCH:
    /* If there has been data output and this is a mode switch, set up to     */
    /* output data in BIN mode.                                               */
    if( binary_data_output && rim_mode )
    {
      lexstartsave = lexstartprev;
      if( isdone( line[lexstart] ))
        {
	  leaderlen = 8; /* If not specified, the short leader/trailer will be 8. */
	}
      else	
	{
	  leaderlen = (getExpr())->val;      /* BINPUN leaderlen              */
	}


      cp.loc = 00200;           /* Clear the literal tables.                  */
      cp.error = FALSE;
      pz.loc = 00200;
      pz.error = FALSE;
      punchLeader( leaderlen ); /* Generate a short leader/trailer.           */
      checksum = 0;
      binary_data_output = FALSE;
    }
    rim_mode = FALSE;
    break;

  case DECIMAL:
    radix = 10;
    break;

  case DUBL:
    inputDubl();
    break;

  case EJECT:
    page_lineno = LIST_LINES_PER_PAGE;  /* This will force a page break.      */
    status = FALSE;             /* This will force reading of next line       */
    break;

  case ENPUNCH:
    if( pass == 2 )
    {
      objectfile = objectsave;
    }
    break;

  case EXPUNGE:                 /* Erase symbol table                         */
    if( pass == 1 )
    {
      symtab[0] = sym_undefined;
      symbol_top = 0;
      number_of_fixed_symbols = symbol_top;
      fixed_symbols = &symtab[symbol_top - 1];

      /* Enter the pseudo-ops into the symbol table.                          */
      for( ix = 0; ix < DIM( pseudo ); ix++ )
      {
        defineSymbol( pseudo[ix].name, pseudo[ix].val, pseudo[ix].type, 0 );
      }
      number_of_fixed_symbols = symbol_top;
      fixed_symbols = &symtab[symbol_top - 1];
    }
    break;

  case FIELD:
    punchLiteralPool( &cp, clc - 1 );
    punchLiteralPool( &pz, 0 );
    newfield = field >> 12;
    lexstartsave = lexstartprev;
    if( isdone( line[lexstart] ))
    {
      newfield += 1;            /* Blank FIELD directive.                     */
    }
    else
    {
      newfield = (getExpr())->val;      /* FIELD with argument.               */
    }

    if( rim_mode )
    {
      errorMessage( &in_rim_mode, lexstartsave ); /* Can't change fields.     */
    }
    else if( newfield > 7 || newfield < 0 )
    {
      errorMessage( &illegal_field_value, lexstartprev );
    }
    else
    {
      value = (( newfield & 0007 ) << 3 ) | 00300;
      punchObject( value );
      checksum -= value;        /* Field punches are not added to checksum.   */
      field = newfield << 12;
    }

    clc = 0200 | field;
    fieldlc = clc & 07777;

    if( !rim_mode )
    {
      punchOrigin( clc );
    }
    break;

  case FIXMRI:
    if( line[lexterm] == '=' && isalpha( line[lexstart] ))
    {
      lexstartsave = lexstart;
      term = lexterm;
      nextLexeme();           /* Skip symbol.                                 */
      nextLexeme();           /* Skip trailing =                              */
      defineLexeme( lexstartsave, term, getExprs(), MRI );
    }
    else
    {
      errorLexeme( &symbol_syntax, lexstart );
      nextLexeme();           /* Skip symbol.                                 */
      nextLexeme();           /* Skip trailing =                              */
      (void) getExprs();      /* Skip expression.                             */
    }
    break;

  case FIXTAB:
    /* Mark all current symbols as permanent symbols.                         */
    for( ix = 0; ix < symbol_top; ix++ )
    {
      symtab[ix].type = symtab[ix].type | FIXED;
    }
    number_of_fixed_symbols = symbol_top;
    fixed_symbols = &symtab[symbol_top - 1];

    /* Re-sort the symbol table                                               */
    qsort( symtab, symbol_top, sizeof(symtab[0]), compareSymbols );
    break;

  case FLTG:
    inputFltg();
    /* errorSymbol( &no_pseudo_op, "FLTG", lexstartprev ); */
    break;

  case IFDEF:
    if( isalpha( line[lexstart] ))
    {
      sym = evalSymbol();
      nextLexeme();
      if( M_DEFINED_CONDITIONALLY( sym->type ))
      {
        conditionTrue();
      }
      else
      {
        conditionFalse();
      }
    }
    else
    {
      errorLexeme( &label_syntax, lexstart );
    }
    break;

  case IFNDEF:
    if( isalpha( line[lexstart] ))
    {
      sym = evalSymbol();
      nextLexeme();
      if( M_DEFINED_CONDITIONALLY( sym->type ))
      {
        conditionFalse();
      }
      else
      {
        conditionTrue();
      }
    }
    else
    {
      errorLexeme( &label_syntax, lexstart );
    }
    break;

  case IFNZERO:
    if( (getExpr())->val == 0 )
    {
      conditionFalse();
    }
    else
    {
      conditionTrue();
    }
    break;

  case IFZERO:
    if( (getExpr())->val == 0 )
    {
      conditionTrue();
    }
    else
    {
      conditionFalse();
    }
    break;

  case NOPUNCH:
    if( pass == 2 )
    {
      objectfile = NULL;
    }
    break;

  case OCTAL:
    radix = 8;
    break;

  case PAGE:
    punchLiteralPool( &cp, clc - 1 );
    oldclc = clc;
    if( isdone( line[lexstart] ))
    {
      clc = ( clc + 0177 ) & 077600;            /* No argumnet.               */
      fieldlc = clc & 07777;
    }
    else
    {
      value = (getExpr())->val;
      clc = ( field << 12 ) + (( value & 037 ) << 7 );
      fieldlc = clc & 07777;
    }
    testForLiteralCollision( clc );

    if( !rim_mode && clc != oldclc )
    {
      punchOrigin( clc );
    }
    break;

  case PAUSE:
    break;

  case RELOC:
    if( isdone( line[lexstart] ))
    {
       reloc = 0;               /* Blank RELOC directive.                     */
    }
    else
    {
      value = (getExpr())->val; /* RELOC with argument.                       */
      reloc = value - ( clc + reloc );
    }
    break;

  case RIMPUNCH:
    /* If the assembler has output any BIN data, output the literal tables    */
    /* and the checksum for what has been assembled and setup for RIM mode.   */
    if( binary_data_output && !rim_mode )
    {
      lexstartsave = lexstartprev;
      if( isdone( line[lexstart] ))
      {
	leaderlen = 8; /* If not specified, the short leader/trailer will be 8. */
      }
      else	
      {
	leaderlen = (getExpr())->val;      /* BINPUN leaderlen              */
      }

      endOfBinary();
      if (leaderlen != -1)
      {
	punchChecksum();
      }
      punchLeader( leaderlen );         /* Generate a short leader/trailer.           */
    }
    rim_mode = TRUE;
    break;

  case SEGMNT:
    punchLiteralPool( &cp, clc - 1 );
    if( isdone( line[lexstart] ))
    {                           /* No argument.                               */
      clc = ( clc & 06000 ) + 02000;
      fieldlc = clc & 07777;
    }
    else
    {
      getExpr();
      clc = ( val & 003 ) << 10;
      fieldlc = clc & 07777;
    }
    if( !rim_mode )
    {
      punchOrigin( clc );
    }
    testForLiteralCollision( clc );
    break;

  case TEXT:
    delim = line[lexstart];
    pack = 0;
    count = 0;
    index = lexstart + 1;
    while( line[index] != delim && !isend( line[index] ))
    {
      pack = ( pack << 6 ) | ( line[index] & 077 );
      count++;
      if( count > 1 )
      {
        punchOutObject( clc, pack );
        incrementClc();
        count = 0;
        pack = 0;
      }
      index++;
    }

    if( count != 0 )
    {
      punchOutObject( clc, pack << 6 );
      incrementClc();
    }
    else
    {
      punchOutObject( clc, 0 );
      incrementClc();
    }

    if( isend( line[index] ))
    {
      cc = index;
      lexterm = cc;
      errorMessage( &text_string, cc );
    }
    else
    {
      cc = index + 1;
      lexterm = cc;
    }
    nextLexeme();
    break;

  case TITLE:
    delim = line[lexstart];
    ix = lexstart + 1;
    /* Find string delimiter.                                                 */
    do
    {
      if( list_title[ix] == delim && list_title[ix + 1] == delim )
      {
        ix++;
      }
      ix++;
    } while( line[ix] != delim && !isend(line[ix]) );

    if( line[ix] == delim )
    {
      count = 0;
      ix = lexstart + 1;
      do
      {
        if( list_title[ix] == delim && list_title[ix + 1] == delim )
        {
          ix++;
        }
        list_title[count] = line[ix];
        count++;
        ix++;
        list_title[count] = '\0';
      } while( line[ix] != delim && !isend(line[ix]) );

      if( strlen( list_title ) > TITLELEN )
      {
        list_title[TITLELEN] = '\0';
      }

      cc = ix + 1;
      lexterm = cc;
      page_lineno = LIST_LINES_PER_PAGE;/* Force top of page for new titles.  */
      list_title_set = TRUE;
    }
    else
    {
      cc = ix;
      lexterm = cc;
      errorMessage( &text_string, cc );
    }

    nextLexeme();
    break;

  case XLIST:
    if( isdone( line[lexstart] ))
    {
      temp = listfile;          /* Blank XLIST directive.                     */
      listfile = listsave;
      listsave = temp;
    }
    else
    {
      if( (getExpr())->val == 0 )
      {
        if( listfile == NULL )
        {
          listfile = listsave;
          listsave = NULL;
        }
      }
      else
      {
        if( listfile != NULL )
        {
          listsave = listfile;
          listfile = NULL;
        }
      }
    }
    break;

  case ZBLOCK:
    value = (getExpr())->val;
    if( value < 0 )
    {
      errorMessage( &zblock_too_small, lexstartprev );
    }
    else if( value + ( clc & 07777 ) - 1 > 07777 )
    {
      errorMessage( &zblock_too_large, lexstartprev );
    }
    else
    {
      for( ; value > 0; value-- )
      {
        punchOutObject( clc, 0 );
        incrementClc();
      }
    }

    break;

  default:
    break;
  } /* end switch for pseudo-ops                                              */
  return( status );
} /* pseudoOperators()                                                        */


/******************************************************************************/
/*                                                                            */
/*  Function:  conditionFalse                                                 */
/*                                                                            */
/*  Synopsis:  Called when a false conditional has been evaluated.            */
/*             Lex should be the opening <; ignore all text until             */
/*             the closing >.                                                 */
/*                                                                            */
/******************************************************************************/
void conditionFalse()
{
  int     level;

  if( line[lexstart] == '<' )
  {
    /* Invariant: line[cc] is the next unexamined character.                  */
    level = 1;
    while( level > 0 )
    {
      if( isend( line[cc] ))
      {
        readLine();
      }
      else
      {
        switch( line[cc] )
        {
        case '>':
          level--;
          cc++;
          break;

        case '<':
          level++;
          cc++;
          break;

        case '$':
          level = 0;
          cc++;
          break;

        default:
          cc++;
          break;
        } /* end switch                                                       */
      } /* end if                                                             */
    } /* end while                                                            */
    nextLexeme();
  }
  else
  {
    errorMessage( &lt_expected, lexstart );
  }
} /* conditionFalse()                                                         */

/******************************************************************************/
/*                                                                            */
/*  Function:  conditionTrue                                                  */
/*                                                                            */
/*  Synopsis:  Called when a true conditional has been evaluated.             */
/*             Lex should be the opening <; skip it and setup for             */
/*             normal assembly.                                               */
/*                                                                            */
/******************************************************************************/
void conditionTrue()
{
  if( line[lexstart] == '<' )
  {
    nextLexeme();               /* Skip the opening '<'                       */
  }
  else
  {
    errorMessage( &lt_expected, lexstart );
  }
} /* conditionTrue()                                                          */


/******************************************************************************/
/*                                                                            */
/*  Function:  errorLexeme                                                    */
/*                                                                            */
/*  Synopsis:  Display an error message using the current lexical element.    */
/*                                                                            */
/******************************************************************************/
void errorLexeme( EMSG_T *mesg, int col )
{
  char   name[SYMLEN];

  errorSymbol( mesg, lexemeToName( name, lexstart, lexterm ), col );
} /* errorLexeme()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  errorSymbol                                                    */
/*                                                                            */
/*  Synopsis:  Display an error message with a given string.                  */
/*                                                                            */
/******************************************************************************/
void errorSymbol( EMSG_T *mesg, char *name, int col )
{
  char   linecol[12];
  char  *s;

  if( pass == 2 )
  {
    s = ( name == NULL ) ? "" : name ;
    errors++;
    sprintf( linecol, "(%d:%d)", lineno, col + 1 );
    fprintf( errorfile, "%s%-9s : error:  %s \"%s\" at Loc = %5.5o\n",
                                      filename, linecol, mesg->file, s, clc );
    saveError( mesg->list, col );
  }
  error_in_line = TRUE;
} /* errorSymbol()                                                            */


/******************************************************************************/
/*                                                                            */
/*  Function:  errorMessage                                                   */
/*                                                                            */
/*  Synopsis:  Display an error message without a name argument.              */
/*                                                                            */
/******************************************************************************/
void errorMessage( EMSG_T *mesg, int col )
{
  char   linecol[12];

  if( pass == 2 )
  {
    errors++;
    sprintf( linecol, "(%d:%d)", lineno, col + 1 );
    fprintf( errorfile, "%s%-9s : error:  %s at Loc = %5.5o\n",
                                         filename, linecol, mesg->file, clc );
    saveError( mesg->list, col );
  }
  error_in_line = TRUE;
} /* errorMessage()                                                           */

/******************************************************************************/
/*                                                                            */
/*  Function:  saveError                                                      */
/*                                                                            */
/*  Synopsis:  Save the current error in a list so it may displayed after the */
/*             the current line is printed.                                   */
/*                                                                            */
/******************************************************************************/
void saveError( char *mesg, int col )
{
  if( save_error_count < DIM( error_list ))
  {
    error_list[save_error_count].mesg = mesg;
    error_list[save_error_count].col = col;
    save_error_count++;
  }
  error_in_line = TRUE;

  if( listed )
  {
    printErrorMessages();
  }
} /* saveError()                                                              */

/* End-of-File                                                                */
