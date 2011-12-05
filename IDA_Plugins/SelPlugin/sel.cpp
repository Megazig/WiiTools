/*
 *  IDA Nintendo GameCube sel Loader Module
 *  (C) Copyright 2011 Stephen Simpson
 *
 */

#include <ida.hpp>
#include <fpro.h>
#include <idp.hpp>
#include <loader.hpp>
#include <name.hpp>
#include <bytes.hpp>
#include <offset.hpp>
#include <segment.hpp>
#include <srarea.hpp>
#include <fixup.hpp>
#include <entry.hpp>
#include <auto.hpp>
#include <diskio.hpp>
#include <kernwin.hpp>
#include <nalt.hpp>
#include "sel.h"

#define DEBUG 0

/***************************************************************
* Function:	 init
* Description:
* Parameters:   none
* Returns:	  PLUGIN_OK
***************************************************************/
int idaapi init(void)
{
	return PLUGIN_OK;
}

/***************************************************************
* Function:	 term
* Description:  term
* Parameters:   none
* Returns:	  none
***************************************************************/
void idaapi term(void)
{
}

/*-----------------------------------------------------------------
 *
 *   Read the header of the sel file into memory. Swap
 *   all bytes because the file is stored as big endian.
 *
 */

int read_header(FILE *fp, selhdr *rhdr)
{
	int i;

	/* read in selheader */
	qfseek(fp, 0, SEEK_SET);
	if(qfread(fp, rhdr, sizeof(selhdr)) != sizeof(selhdr))
		return(0);

	/* convert header */
	rhdr->Prev = swap32(rhdr->Prev);
	rhdr->Next = swap32(rhdr->Next);
	rhdr->SectionCount = swap32(rhdr->SectionCount);
	rhdr->SectionOffset = swap32(rhdr->SectionOffset);
	rhdr->PathOffset = swap32(rhdr->PathOffset);
	rhdr->PathLength = swap32(rhdr->PathLength);
	for(i=0; i<6; i++)
		rhdr->Unknown[i] = swap32(rhdr->Unknown[i]);
	rhdr->InternalTableOffset = swap32(rhdr->InternalTableOffset);
	rhdr->InternalTableLength = swap32(rhdr->InternalTableLength);
	rhdr->ExternalTableOffset = swap32(rhdr->ExternalTableOffset);
	rhdr->ExternalTableLength = swap32(rhdr->ExternalTableLength);
	rhdr->ExportTableOffset = swap32(rhdr->ExportTableOffset);
	rhdr->ExportTableLength = swap32(rhdr->ExportTableLength);
	rhdr->ExportTableNames = swap32(rhdr->ExportTableNames);
	rhdr->ImportTableOffset = swap32(rhdr->ImportTableOffset);
	rhdr->ImportTableLength = swap32(rhdr->ImportTableLength);
	rhdr->ImportTableNames = swap32(rhdr->ImportTableNames);
	return(1);
}

/*-----------------------------------------------------------------
 *
 *   Read the section table sel file into memory. Swap
 *   all bytes because the file is stored as big endian.
 *
 */
int read_section_table(FILE *fp, section_entry *entries, int offset, int count)
{
	int i;

	msg("read_section_table(*,*,%08x, %d);\n", offset, count);
	/* read in section table */
	qfseek(fp, offset, SEEK_SET);
	if(qfread(fp, entries, sizeof(section_entry)*count) != sizeof(section_entry)*count) return(0);

	for(i=0; i<count; i++) {
		entries[i].Offset = swap32(entries[i].Offset);
		entries[i].Length = swap32(entries[i].Length);
	}
	return(1);
}

/******************************************************************
* Function:	 run
* Description:  entry function of the plugin
* Parameters:   int arg
* Returns:	  none
******************************************************************/
void idaapi run(int ZF_arg)
{
	selhdr rhdr;
	uint snum;
	int i;
	int text;
	FILE* fp = NULL;

	const char* section_names[] = { \
			"None", \
			"_f_init", \
			"_f_text", \
			"_f_ctors", \
			"_f_dtors", \
			"_f_rodata", \
			"_f_data", \
			"_f_bss", \
			"_f_sdata", \
			"_f_sdata2", \
			"_f_zero", \
			"_f_sbss", \
			"_f_sbss2", \
			"_f_zero2" \
	};

	/* Hello here I am */
	msg("---------------------------------------\n");
	msg("Nintendo Sel Loader Plugin 0.1\n");
	msg("---------------------------------------\n");

	char tBuf1[0x80] = {0};
	get_input_file_path(tBuf1, 0x80);

	char *tBuf3 = NULL;
	char *tBuf4 = NULL;
	qsplitfile(tBuf1, &tBuf3, &tBuf4);

	char filename[0x80] = {0};
	const char* ext = ".sel";
	set_file_ext(filename, 0x80, tBuf3, ext);

	fp = qfopen(filename, "r+b");
	if(!fp) {
		msg("Error opening file %s\n", filename);
		return;
	}
  
	/* read sel header into memory */
	if (read_header(fp, &rhdr)==0) {
		msg("Error reading sel header into memory\n");
		return;
	}
  
	/* get section list */
	unsigned int sections[14] = {0};
	unsigned int off = rhdr.ExportTableOffset;
	unsigned int len = rhdr.ExportTableLength;
	unsigned int nam = rhdr.ExportTableNames;
	for(unsigned int ii = off; ii < off+len; ii += sizeof(export_table_entry)) {
		export_table_entry ent;
		qfseek(fp, ii, SEEK_SET);
		qfread(fp, &ent, sizeof(export_table_entry));
		ent.name_off = swap32(ent.name_off);
		ent.section_off = swap32(ent.section_off);
		ent.section_num = swap32(ent.section_num);
		ent.elf_hash = swap32(ent.elf_hash);
		if(ent.section_num != 0xfff1)
			continue;
		char nom[0x50];
		qfseek(fp, nam+ent.name_off, SEEK_SET);
		qfread(fp, &nom, 0x50);
		if(!memcmp(nom, "_f_init", 7)) {
			sections[1] = ent.section_off;
		}else if(!memcmp(nom, "_f_text", 7)) {
			sections[2] = ent.section_off;
		}else if(!memcmp(nom, "_f_ctors", 8)) {
			sections[3] = ent.section_off;
		}else if(!memcmp(nom, "_f_dtors", 8)) {
			sections[4] = ent.section_off;
		}else if(!memcmp(nom, "_f_rodata", 9)) {
			sections[5] = ent.section_off;
		}else if(!memcmp(nom, "_f_data", 7)) {
			sections[6] = ent.section_off;
		}else if(!memcmp(nom, "_f_bss", 6)) {
			sections[7] = ent.section_off;
		}else if(!memcmp(nom, "_f_sdata", 8)) {
			sections[8] = ent.section_off;
		}else if(!memcmp(nom, "_f_sdata2", 9)) {
			sections[9] = ent.section_off;
		}else if(!memcmp(nom, "_f_zero", 7)) {
			sections[10] = ent.section_off;
		}else if(!memcmp(nom, "_f_sbss", 7)) {
			sections[11] = ent.section_off;
		}else if(!memcmp(nom, "_f_sbss2", 8)) {
			sections[12] = ent.section_off;
		}else if(!memcmp(nom, "_f_zero2", 8)) {
			sections[13] = ent.section_off;
		}
	}

	/* create segments */
	//delete segments
	/*
	int segment_qty = get_segm_qty();
	segment_t *segment;
	for(size_t count = segment_qty - 1; count < segment_qty; count++) {
		segment = getnseg(0);
		ea_t start = segment->startEA;
		ea_t end   = segment->endEA;
		msg("Deleting segment (%08x-%08x) ...\n", start, end);
		del_segm( start, SEGDEL_PERM );
	}
	*/
	//create new ones
	for(unsigned int ii = 1; ii < 14; ii++ ) {
		if(!sections[ii])
			continue;
		ea_t start = sections[ii];
		//FIXME
		ea_t end   = BADADDR;
		//msg("Creating a new segment  (%08x-%08x) ...", start, end);
		if( (ii==1) || (ii==2) || (ii==5) ) {
			add_segm(1, sections[ii], end, section_names[ii], "CODE");
		}else if( (ii==7) || (ii==11) || (ii==12) ) {
			add_segm(1, sections[ii], end, section_names[ii], "BSS");
		}else{
			add_segm(1, sections[ii], end, section_names[ii], "DATA");
		}
		//msg(" ...");
		/* set adderssing mode to 32 bit */
		set_segm_addressing( getseg(sections[ii]), 1 );
		//msg(" OK\n");
	}

	/* loop through symbols */
	/* set naming */
	unsigned int which = 0;
	for(unsigned int ii = off; ii < off+len; ii += sizeof(export_table_entry)) {
		which++;
#if DEBUG
		if (which > 0x100)
			break;
#endif
		export_table_entry ent;
		qfseek(fp, ii, SEEK_SET);
		qfread(fp, &ent, sizeof(export_table_entry));
		ent.name_off = swap32(ent.name_off);
		ent.section_off = swap32(ent.section_off);
		ent.section_num = swap32(ent.section_num);
		ent.elf_hash = swap32(ent.elf_hash);
		char nom[0x80];
		qfseek(fp, nam+ent.name_off, SEEK_SET);
		qfread(fp, &nom, 0x80);
		if( ent.section_num > 13 ) {
			msg("FATAL ERROR: invalid section ID (%d) for %d\n", ent.section_num, which);
			continue;
		}
		if( sections[ent.section_num] == 0 ) {
			msg("FATAL ERROR: no address for section ID (%d) [%s]\n", ent.section_num, nom);
			continue;
		}
		if( (ent.section_num==1) || (ent.section_num==2) || (ent.section_num==5) ) {
			add_func(sections[ent.section_num]+ent.section_off, BADADDR);
		}
		set_name( sections[ent.section_num]+ent.section_off, nom, SN_PUBLIC);
	}

	qfclose(fp);
}

//-----------------------------------------------------------------
char comment[] = "This is gets segments and naming from a SEL file.";
char help[]	= "Import SEL file\n\n";
char wanted_name[]   = "SEL Loader.";
char wanted_hotkey[] = "Alt-S";

//-----------------------------------------------------------------
//
//	  PLUGIN DESCRIPTION BLOCK
//
//-----------------------------------------------------------------

extern "C" plugin_t PLUGIN =
{
  IDP_INTERFACE_VERSION,
  0,				// plugin flags
  init,				// initialize
  term,				// terminate. this pointer may be NULL.
  run,				// invoke plugin
  comment,			// long comment about the plugin
					// it could appear in the status line
					// or as a hint
  help,				// multiline help about the plugin
  wanted_name,		// the preferred short name of the plugin
  wanted_hotkey		// the preferred hotkey to run the plugin
};

