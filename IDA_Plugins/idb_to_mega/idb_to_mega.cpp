/*************************************************************************
Copyright (c) :
Project   :	   idb_2_sig plugin
**************************************************************************
MODULE	:	   idb_to_sig.cpp
PROGRAM   :
**************************************************************************
$Revision:  $
$Date:  $
$Workfile:  $
**************************************************************************
Revision History :
 Version   Author	Date	   Description
   V1.0	Quine	??????????  creation
   V1.1	Darko	04.10.2002  modification for IDA Pro v4.3 and SDK 4.3
   V1.2	Darko	05.10.2002  pat file onened in appending mode
   V1.3	Darko	21.12.2002  bug fix for reference bad address
*************************************************************************/

/*************************************************************************
Function list:
--------------------------------------------------------------------------
External functions list:
--------------------------------------------------------------------------
Special considerations:
				  Depends upon IDA SDK 4.3
*************************************************************************/

// written by Quine (quine@blacksun.res.cmu.edu)
// visit Quine's IDA Page at http://surf.to/quine_ida

// defining __NOT_ONLY_PRO_FUNCS__ keeps the STL stuff from freaking about min/max
// #define __NOT_ONLY_PRO_FUNCS__
#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <bytes.hpp>
#include <name.hpp>
#include <entry.hpp>
#include <fpro.h>
// vector must be included first because of crappy Rogue Wave STL
#include <vector>
#include <list>
#include <map>
#define MIN_SIG_LENGTH 30

//#define SINGLE_FUNC

#ifndef SINGLE_FUNC
	#define ALL_FUNCS
#endif

typedef unsigned char	   T_u_byte;   /* 8 bits: 0..255	  */
typedef signed   char	   T_s_byte;   /* 8 bits: -128..+127  */
typedef unsigned int		T_u_int;	/* >= 16 bits unsigned */
typedef signed   int		T_s_int;	/* >= 16 bits signed   */
typedef unsigned short	  T_u_int16;  /* 16 bits unsigned	*/
typedef signed   short	  T_s_int16;  /* 16 bits signed	  */

typedef unsigned long	   T_u_int32;  /* 32 bits unsigned	*/
typedef signed   long	   T_s_int32;  /* 32 bits signed	  */

typedef std::vector<bool> bool_vec;
typedef std::map<ea_t, ea_t, std::less<ea_t> > ref_map;

static int g_use_skip = true;
static int g_Fenter = true;

static const char * const SkipPrefixList[] =
{
	"unknown",
	"sub_",
	"loc_",
	"byte_",
	"dword_",
	"word_",
	NULL
};

/**********************************************************************
* Function:	QL_GetLine
* Description:
* Reads a line from file f, up to the size of the buffer.  The line in the
* buffer will NOT include line termination, although any of (CR, LF, CRLF)
* is accepted on input.  The return value is < 0 on error, 0 if the line
* was terminated abnormally (EOF, error, or out of buffer space), and
* length (of the line) if the line was terminated normally.
*
* Passing in a buffer less than 2 characters long is not a terribly bright
* idea.
* Parameters:  char *ZF_buf
*			  T_u_int32 ZF_maxLen
*			  FILE *ZF_fp
* Returns:	 T_s_int32 - buffer length or error code if < 0
**********************************************************************/
static T_s_int32 QL_GetLine(char *ZF_buf, T_u_int32 ZF_maxLen, FILE *ZF_fp)
{
	T_s_int32				   ZL_state;
	T_s_int32				   ZL_ch;
	char						*ZL_pStr;

	if ( ZF_buf == NULL || ZF_fp == NULL || ZF_maxLen == 0 )
	{
		warning("Internal IDB to SIG error at %s!\n", __LINE__);
		return -1;
	}

	memset(ZF_buf, 0, ZF_maxLen);
	for (ZL_state=0,ZL_pStr=ZF_buf; ; )
	{
		ZL_ch = qfgetc(ZF_fp);

		if ('\n' == ZL_ch)
		{
			*ZL_pStr = 0;
			return (T_s_int32)strlen(ZF_buf);/*Line terminated with \n or \r\n*/
		}

		if ( ZL_state )
		{
			*ZL_pStr = 0;
			qfseek(ZF_fp, -1L, SEEK_CUR);
			return (T_s_int32)strlen(ZF_buf);   /* Line terminated with \r */
		}

		if ( feof(ZF_fp) )
		{
			*ZL_pStr = 0;
			clearerr(ZF_fp);
			return (ZL_pStr == ZF_buf) ? -1 : -2;	   /* Error */
		}

		if ( ferror(ZF_fp) )
		{
			*ZL_pStr = 0;
			clearerr(ZF_fp);
			return -3;								  /* Error */
		}

		if ('\r' == ZL_ch)
		{
			ZL_state = 1;
		}
		else
		{
			if (--ZF_maxLen > 0)
			{
				*ZL_pStr++ = (char)(ZL_ch & 0xFF);
			}
			else
			{
				*ZL_pStr = 0;
				qfseek(ZF_fp, -1L, SEEK_CUR);
				return -4;			  /* Out of buffer space */
			}
		}
	}
} /* end of getLine */

/**********************************************************************
* Function:	QL_skipBackward
* Description: scrolls a number of lines backward from current position in file
*			  IT IS A NASTY ONE!!
* Parameters:  FILE *ZF_fp
*			  int ZF_NumOfRows
* Returns:	 int
**********************************************************************/
static int QL_skipBackward(FILE *ZF_fp, int ZF_NumOfRows)
{
   int						  ZL_c;

   if ( ZF_fp == NULL || ZF_NumOfRows < 0 )
   {
		warning("Internal IDB to SIG error at %s!\n", __LINE__);
		return 1;
   }

   while (ZF_NumOfRows > 0)
   {
	  if ( qftell(ZF_fp) >= 2 )
	  {
			if ( qfseek(ZF_fp, -2L, SEEK_CUR) )
			{
			   clearerr(ZF_fp);
			   qfseek(ZF_fp, 0L, SEEK_SET);
			   return 1;
			}
	  }
	  else
	  {
			qfseek(ZF_fp, 0L, SEEK_SET);
	  }

	  ZL_c = qfgetc(ZF_fp);

	  if ( feof(ZF_fp) )
	  {
		 clearerr(ZF_fp);
		 return 1;
	  }

	  if (ZL_c == '\n')
	  {
		 ZF_NumOfRows--;
	  }

	  if ( qftell(ZF_fp) == 1 )
	  {
		 qfseek(ZF_fp, 0L, SEEK_SET);
		 return 0;
	  }
   }

   return 0;
} /* end of skipBackward */

/**********************************************************************
* Function:	 QL_IsOnSkipList
* Description:  Test if an identifer is on skip list
* Parameters:   char *ZF_str - pointer to identifer
* Returns:	  true if identifer is on skip list
**********************************************************************/
bool QL_IsOnSkipList(char *ZF_str)
{
	if ( !g_use_skip )
	{
		return false;				   /* do not skip names */
	}

	for ( int i=0; (SkipPrefixList[i] != NULL) && (ZF_str != NULL); i++ )
	{
		if ( !strncmp(ZF_str, SkipPrefixList[i], strlen(SkipPrefixList[i])) )
		{
			return true;
		}
	}
	return false;
}

/**********************************************************************
* Function:	 QL_crc16
* Description:
*   crc16 is ripped straight out the c file that comes with the
*   FLAIR package
*										16   12   5
*   this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
*   This works out to be 0x1021, but the way the algorithm works
*   lets us use 0x8408 (the reverse of the bit pattern).  The high
*   bit is always assumed to be set, thus we only use 16 bits to
*   represent the 17 bit value.
* Parameters: T_u_byte *ZF_data_p - pointer to data
*			 T_s_int16 ZF_length - data lenght
* Returns:	CRC
**********************************************************************/
static T_u_int16 QL_crc16(T_u_byte *ZF_data_p, T_s_int16 ZF_length)
{
#define POLY 0x8408
	T_u_byte					ZL_i;
	T_u_int16				   ZL_data;
	T_u_int16				   ZL_crc;

	if ( ZF_length <= 0 )
	{
		return 0;
	}

	ZL_crc = 0xFFFF;
	do
	{
		ZL_data = *ZF_data_p++;
		for ( ZL_i=0; ZL_i < 8; ZL_i++ )
		{
			if ( (ZL_crc ^ ZL_data) & 1 )
			{
				ZL_crc = (ZL_crc >> 1) ^ POLY;
			}
			else
			{
				ZL_crc >>= 1;
			}
			ZL_data >>= 1;
		}
	} while ( --ZF_length );

  ZL_crc = (~ZL_crc) & 0xFFFF;
  ZL_data = ZL_crc;
  ZL_crc = ((ZL_crc << 8) & 0xFF00) | ((ZL_data >> 8) & 0xff);

  return ZL_crc;
}

/**********************************************************************
* Function:	 init
* Description:
* Parameters:   none
* Returns:	  PLUGIN_OK
**********************************************************************/
int idaapi init(void)
{
	return PLUGIN_OK;
}

/**********************************************************************
* Function:	 init
* Description:  term
* Parameters:   none
* Returns:	  none
**********************************************************************/
void idaapi term(void)
{
}

/**********************************************************************
* Function:	 find_ref_loc
* Description:
*   this function finds the location of a reference within an instruction
*   or a data item
*   eg:  00401000 E8 FB 0F 00 00   call sub_402000
*   find_ref_loc(0x401000, 0x402000) would return 0x401001
*   it works for both segment relative and self-relative offsets
*   all references are assumed to be 4 bytes long
* Parameters:   ea_t item
*			   ea_t _ref
* Returns:	  ea_t
**********************************************************************/
//ea_t find_ref_loc(ea_t item, ea_t _ref)
ea_t find_ref_loc(ea_t item, ea_t _ref, FILE *f)
{
	ea_t		ZL_i;

	if ( isCode(getFlags(item)) )
	{
	//qfprintf(f, "---%d", __LINE__);	  /* check progress */
	//qflush(f);
		ua_ana0(item);
		if ( cmd.Operands[0].type == o_near )
		{
	//qfprintf(f, "---%d", __LINE__);	  /* check progress */
	//qflush(f);
			// we've got a self-relative reference
			_ref = _ref - (get_item_end(item));
		}
	}

	//qfprintf(f, "---%d", __LINE__);	  /* check progress */
	//qflush(f);
	for ( ZL_i=item; ZL_i <= get_item_end(item) - 4; ZL_i++)
	{
	//qfprintf(f, "---(%08x-%08x)%d", get_long(ZL_i), ZL_i, __LINE__);	  /* check progress */
	//qflush(f);
		if ( get_long(ZL_i) == _ref)
		{
			return ZL_i;
		}
	}
	return BADADDR;
}

/**********************************************************************
* Function:	 set_v_bytes
* Description:  marks off a string of bytes as variable
* Parameters:   bool_vec& bv
*			   int pos
*			   int len=4
* Returns:	  none
**********************************************************************/
void set_v_bytes(bool_vec& bv, int pos, int len=4)
{
	for ( int i=0; i < len; i++ )
	{
		bv[pos + i] = true;
	}
}

/**********************************************************************
* Function:	 make_func_sig
* Description:
*	   this is what does the real work
*	   given a starting address, a length, and a FILE pointer, it
*	   writes a pattern line to the file
* Parameters:   ea_t start_ea
*			   ulong len
*			   FILE* f
* Returns:	  none
**********************************************************************/
void make_func_sig(ea_t start_ea, ulong len, FILE* f)
{
	ea_t				ea, ref, ref_loc;
	int					first_string = 0, alen = 0;
	int					j;
	T_u_byte			crc_data[256];
	T_u_int16			crc = 0;
	bool_vec			v_bytes(len);
	std::list<ea_t>		publics;
	ref_map				refs;
	char				*p1;
	char				buf1[150];
	int					found;

	if ( len < MIN_SIG_LENGTH )
	{
		return;
	}

	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);
	ea = start_ea;
	while ( ea - start_ea < len )
	{
		flags_t flags = getFlags(ea);
		//qfprintf(f, "---[%08x]%d", ea,__LINE__);	  /* check progress */
		//qflush(f);
		if ( has_name(flags) )
		{
			publics.push_back(ea);
		}
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);
		if ( (ref = get_first_dref_from(ea)) != BADADDR )
		{
		//qfprintf(f, "---CodeRefFound(%08x)%d", ref,__LINE__);	  /* check progress */
		//qflush(f);
			// a data location is referenced
			//ref_loc = find_ref_loc(ea, ref);
			//ref_loc = find_ref_loc(ea, ref,f);
			ref_loc = ea;
		//qfprintf(f, "---(%08x|%08x)%d", ref_loc,start_ea,__LINE__);/*check progress*/
		//qflush(f);
			set_v_bytes(v_bytes, ref_loc - start_ea);
		//qfprintf(f, "---(%08x|%08x)%d", ref_loc,ref,__LINE__);	  /* check progress */
		//qflush(f);
			refs[ref_loc] = ref;
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);

			// check if there is a second data location ref'd
			if ( (ref = get_next_dref_from(ea, ref)) != BADADDR )
			{
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);
				//ref_loc = find_ref_loc(ea, ref);
				//ref_loc = find_ref_loc(ea, ref, f);
				ref_loc = ea;
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);
				set_v_bytes(v_bytes, ref_loc - start_ea);
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);
				refs[ref_loc] = ref;
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);
			}
		}
		else
		{
			// do we have a code ref?
			if ( (ref = get_first_fcref_from(ea)) != BADADDR )
			{
		//qfprintf(f, "---DataRefFound%d", __LINE__);	  /* check progress */
		//qflush(f);
				// if so, make sure it is outside of function
				if ( (ref < start_ea) || (ref >= start_ea + len) )
				{
					//ref_loc = find_ref_loc(ea, ref);
					//ref_loc = find_ref_loc(ea, ref, f);
					ref_loc = ea;
					set_v_bytes(v_bytes, ref_loc - start_ea);
					refs[ref_loc] = ref;
				}
			}

			/* check for r13 and rtoc */
			char disasm[255];
			memset(disasm, 0, 255);
			generate_disasm_line( ea , disasm , 255 );
			if ( strstr( disasm , "%r13" ) )
			{
				ref_loc = ea;
				set_v_bytes(v_bytes, ref_loc - start_ea);
			} else if ( strstr( disasm , "%rtoc" ) ) {
				ref_loc = ea;
				set_v_bytes(v_bytes, ref_loc - start_ea);
			}
		}
		//qfprintf(f, "---%d", __LINE__);	  /* check progress */
		//qflush(f);
		ea = next_not_tail(ea);
		//qfprintf(f, "---%d\n", __LINE__);	  /* check progress */
		//qflush(f);
	}

	//return;	/* MEGAEDIT */
	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);

	// write out the first string of bytes, making sure not to go past
	// the end of the function
	//first_string = (len < 32 ? len : 32);
	first_string = len;
	for ( j = 0; j < first_string; j++ )
	{
		if ( v_bytes[j] )
		{
			qfprintf(f, "..");
		}
		else
		{
			qfprintf(f, "%02X", get_byte(start_ea+j));
		}
	}
	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);

	// fill in anything less than 32
	/* MEGAEDIT /
	for ( j = 0; j < (32 - first_string); j++ )
	{
		qfprintf(f, "..");
	}
	*/
	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);

	// put together the crc data
	/* MEGAEDIT /
	int pos = 32;
	while ( (pos < len) && (!v_bytes[pos]) && (pos < (255 + 32)) )
	{
		crc_data[pos - 32] = get_byte(start_ea + pos);
		pos++;
	}
	*/
	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);

	// alen is length of the crc data
	/* MEGAEDIT /
	alen = pos - 32;
	crc = QL_crc16(crc_data, (T_s_int16)alen);
	qfprintf(f, " %02X %04X %04X", alen, crc, len);

	found = false;
	*/
	// write the publics
	for ( std::list<ea_t>::const_iterator p = publics.begin();
		  p != publics.end(); p++ )
	{
		p1 = get_true_name(BADADDR , *p, buf1, sizeof(buf1));
		if ( p1 )
		{
			found = true;
			if ( !QL_IsOnSkipList(p1) )
			{
				qfprintf(f, " :%04X %s", *p - start_ea, p1);
			}
			else
			{
				warning("Rename the function %s (it is on the skip list)!\n", p1);
				return ;				/* abandon ship */
			}
		}
	}
	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);

	if ( !found )
	{
		warning("The function has autogenerated name, rename it first!\n");
		return ;						/* abandon ship */
	}

	// write the references
	for ( ref_map::const_iterator r = refs.begin(); r != refs.end(); r++ )
	{
		p1 = get_true_name(BADADDR , (*r).second, buf1, sizeof(buf1));

		if ( p1 )
		{
									  /* V1.3, 21.12.2002 23:09 bug fix */
			if ( !QL_IsOnSkipList(p1) && ((*r).first != BADADDR) )
			{
				qfprintf(f, " ^%04X %s", (*r).first - start_ea, p1);
			}
		}
	}
	//qfprintf(f, "---(%08x)%d", len,__LINE__);	  /* check progress */
	//qflush(f);

	// and finally write out the last string with the rest of the function
	/* MEGAEDIT /
	qfprintf(f, " ");
	for ( j = pos; j < len; j++ )
	{
		if ( v_bytes[j] )
		{
			qfprintf(f, "..");
		}
		else
		{
			qfprintf(f, "%02X", get_byte(start_ea+j));
		}
	}
	*/
	qfprintf(f, "%c%c", 0x0D, 0x0A);
	qflush(f);
}

/**********************************************************************
* Function:	 get_pat_file
* Description:  open and prepare output file for write
* Parameters:   none
* Returns:	  FILE*
**********************************************************************/
inline FILE* get_pat_file(void)
{
	char*					   ZL_filename;
	FILE*					   ZL_fp;
	static char				 ZL_SaveName[255];

	if (g_Fenter)
	{
		g_Fenter = false;
		ZL_filename = askfile_c(true, "*.mega", "Enter the name of the pattern file:");
	}
	else
	{
		ZL_filename = askfile_c(true, ZL_SaveName, "Enter the name of the pattern file:");
	}

	ZL_fp = qfopen(ZL_filename, "r+b");

	if ( !ZL_fp )
	{
		ZL_fp = qfopen(ZL_filename, "w+b");
	}

	if ( !ZL_fp )
	{
		warning("Could not open %s for writing!\n", ZL_filename);
	}
	else
	{
		 qstrncpy(ZL_SaveName, ZL_filename, 255);  /* save file name for next askfile_c dialog */
		 /* this section tests if pat file exists and overwtite '---' at the end */
		 qfseek(ZL_fp, 0L, SEEK_END);	   /* go to end_of_file */
		 if ( qftell(ZL_fp) != 0 )
		 {
			/* pat file is not empty */
			char				ZL_buf[50];
			int				 ZL_i;
			long				ZL_pos;

			do
			{
				QL_skipBackward(ZL_fp, 1);	  /* one line back */
				ZL_pos = qftell(ZL_fp);
				ZL_i = QL_GetLine(ZL_buf, sizeof(ZL_buf), ZL_fp);
				if ( ZL_i == 0 )			/* skip empty lines at the end of file */
				{
					QL_skipBackward(ZL_fp, 1);  /* one more line back */
				}
			} while ( ZL_i == 0 );		  /* skip empty lines at the end of file */

			if ( ZL_i < 0 )
			{
				warning("Something is wrong with %s or '---' is missing!\n", ZL_filename);
				qfclose(ZL_fp);
				return NULL;				/* abandon ship */
			}

			if ( ZL_i > 0 )
			{
				if ( strcmp(ZL_buf, "---") )
				{
					warning("%s is not a valid PAT file!\n", ZL_filename);
					qfclose(ZL_fp);
					return NULL;			/* abandon ship */
				}
			}
			qfseek(ZL_fp, ZL_pos, SEEK_SET);	/* overwrite '---' */
		 }
	}
	return ZL_fp;
}

/**********************************************************************
* Function:	 run
* Description:  entry function of the plugin
* Parameters:   int arg
* Returns:	  none
**********************************************************************/
void idaapi run(int ZF_arg)
{
	FILE*					   ZL_fp;
	func_t*					 ZL_func;
	int						 ZL_i;

	g_use_skip = true;

#if 0
	switch ( ZF_arg )
	{
		case 10:	// write all publics and do NOT skip names
			g_use_skip = false;
		case 0:						 // write all publics
			ZL_fp = get_pat_file();
			if ( !ZL_fp )
			{
				return;
			}
			for ( ZL_i=0; ZL_i < get_func_qty(); ZL_i++ )
			{
				ZL_func = getn_func(ZL_i);
				if ( !ZL_func )
				{
					continue;
				}
				if ( has_name(getFlags(ZL_func->startEA)) && !(ZL_func->flags & FUNC_LIB) )
				{
					make_func_sig(ZL_func->startEA,
								  ZL_func->endEA - ZL_func->startEA, ZL_fp);
				}
			}
			break;

		case 11:	// write all entry points and do NOT skip names
			g_use_skip = false;
		case 1:						 // write all entry points
			ZL_fp = get_pat_file();
			if ( !ZL_fp )
			{
				return;
			}
			for ( ZL_i=1; ZL_i < get_entry_qty(); ZL_i++ )
			{
				ZL_func = get_func(get_entry(get_entry_ordinal(ZL_i)));
				if ( !ZL_func )
				{
					continue;
				}
				make_func_sig(ZL_func->startEA,
							  ZL_func->endEA - ZL_func->startEA, ZL_fp);
			}
			break;

		case 21:	// write current function and do NOT skip names
			g_use_skip = false;
		case 2:						 // write current function
			ZL_fp = get_pat_file();
			if ( !ZL_fp )
			{
				return;
			}
			ZL_func = get_func(get_screen_ea());
			if ( ZL_func )
			{
				make_func_sig(ZL_func->startEA,
							  ZL_func->endEA - ZL_func->startEA, ZL_fp);
			}
			else
			{
				warning("Cursor is NOT in valid function!\n");
			}
			break;

		default:
			warning("Internal IDB to SIG error!\n");
			return;
	}
#endif

/* MEGAEDIT */
	ZL_fp = get_pat_file();
	if ( !ZL_fp ) { return; }

#ifdef SINGLE_FUNC
	ZL_func = get_func(get_screen_ea());
	if ( ZL_func )
	{
		make_func_sig(ZL_func->startEA, ZL_func->endEA - ZL_func->startEA, ZL_fp);
	} else {
		warning("Cursor is NOT in a valid function!\n");
	}
#endif
#ifdef ALL_FUNCS
	for ( ZL_i=0; ZL_i < get_func_qty(); ZL_i++ )
	{
		ZL_func = getn_func(ZL_i);
		if ( !ZL_func ) { continue; }

		if ( has_name(getFlags(ZL_func->startEA)) && !(ZL_func->flags & FUNC_LIB) )
		{
			make_func_sig(ZL_func->startEA, ZL_func->endEA - ZL_func->startEA, ZL_fp);
		}
	}
#endif
/* MEGAEDIT */

	qfprintf(ZL_fp, "---%c%c", 0x0D, 0x0A);	  /* terminate pat file */

	qflush(ZL_fp);
	qfclose(ZL_fp);
}

//--------------------------------------------------------------------------
char comment[] = "This is converts a function or set of functions to a MEGA file.";
char help[]	= "Convert a function or functions to a MEGA file\n\n";
char wanted_name[]   = "IDB to MEGA all public fun.";
char wanted_hotkey[] = "Alt-0";

//--------------------------------------------------------------------------
//
//	  PLUGIN DESCRIPTION BLOCK
//
//--------------------------------------------------------------------------

extern "C" plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,					// plugin flags
  init,				 // initialize
  term,				 // terminate. this pointer may be NULL.
  run,				  // invoke plugin
  comment,			  // long comment about the plugin
						// it could appear in the status line
						// or as a hint
  help,				 // multiline help about the plugin
  wanted_name,		  // the preferred short name of the plugin
  wanted_hotkey		 // the preferred hotkey to run the plugin
};

