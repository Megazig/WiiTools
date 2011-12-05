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

typedef unsigned int u32;

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

u32 get_next_segment(u32* segments, u32 count, u32 addr)
{
	u32 next = BADADDR;
	for(u32 ii = 0; ii < count; ii++) {
		if((segments[ii] > addr) && (segments[ii] < next))
			next = segments[ii];
	}
	return next;
}

u32 check_previous_segment_ends(u32* segments, u32 count, u32 start, u32 end)
{
	bool changed = false;
	u32 next = end;
	for(u32 ii = 0; ii < count; ii++) {
		if( (changed == true) && (segments[ii] > next) ) {
			next = segments[ii];
			continue;
		}
		if( (segments[ii] > start) && (next == BADADDR) ) {
			next = segments[ii];
			changed = true;
			continue;
		}
	}
	return next;
}

void fixup_functions()
{
	int segment_qty = get_segm_qty();
	segment_t *segment;
	for(size_t count = 0; count < segment_qty; count++) {
		segment = getnseg(count);
		char tBuf[0x20] = {0};
		get_segm_class(segment, tBuf, 0x20);
		if(memcmp(tBuf, "CODE", 4))
			continue;
		ea_t start = segment->startEA;
		ea_t end   = segment->endEA;
		for(u32 ii = start; ii < end; ii += 4) {
			if(!get_func(ii))
				add_func(ii, BADADDR);
		}
	}
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
			"_f_sbss", \
			"_f_sdata2", \
			"_f_zero", \
			"_f_sdata", \
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
		char* chosen_name = askfile_c(1, "*.sel", "Please choose a .sel file");
		fp = qfopen(chosen_name, "r+b");
		if(!fp)
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
		if(!memcmp(nom, "_f_init", 8)) {
			sections[1] = ent.section_off;
		}else if(!memcmp(nom, "_f_text", 8)) {
			sections[2] = ent.section_off;
		}else if(!memcmp(nom, "_f_ctors", 9)) {
			sections[3] = ent.section_off;
		}else if(!memcmp(nom, "_f_dtors", 9)) {
			sections[4] = ent.section_off;
		}else if(!memcmp(nom, "_f_rodata", 10)) {
			sections[5] = ent.section_off;
		}else if(!memcmp(nom, "_f_data", 8)) {
			sections[6] = ent.section_off;
		}else if(!memcmp(nom, "_f_bss", 7)) {
			sections[7] = ent.section_off;
		}else if(!memcmp(nom, "_f_sbss", 8)) {
			sections[8] = ent.section_off;
		}else if(!memcmp(nom, "_f_sdata2", 10)) {
			sections[9] = ent.section_off;
		}else if(!memcmp(nom, "_f_zero", 8)) {
			sections[10] = ent.section_off;
		}else if(!memcmp(nom, "_f_sdata", 9)) {
			sections[11] = ent.section_off;
		}else if(!memcmp(nom, "_f_sbss2", 9)) {
			sections[12] = ent.section_off;
		}else if(!memcmp(nom, "_f_zero2", 9)) {
			sections[13] = ent.section_off;
		}
	}

	/* SHOW A WAIT BOX TO LET EU KNOW WE MEAN BUSINESS */
	show_wait_box("Doing something cool");

	/* create segments */
	int segment_qty = get_segm_qty();
	int old_segment_qty = segment_qty;
	segment_t *segment;
	//get current segment addresses
	unsigned int previous_segment_strt[0x20] = {0};
	unsigned int previous_segment_ends[0x20] = {0};
	for(size_t count = 0; count < segment_qty; count++) {
		segment = getnseg(count);
		ea_t start = segment->startEA;
		ea_t end   = segment->endEA;
		previous_segment_strt[count] = start;
		previous_segment_ends[count] = end;
	}
	//delete segments
	for(size_t count = 0; count < segment_qty; count++) {
		if(count == 0)
			continue;
		segment = getnseg(count);
		ea_t start = segment->startEA;
		ea_t end   = segment->endEA;
		/*
		char tSegName[0x80] = {0};
		get_segm_name(segment, tSegName, 0x80);
		if(!memcmp(tSegName, ".bss", 4)) {
			segment_qty -= 1;
			count--;
			del_segm(start, SEGDEL_KEEP);
		}
		*/
		segment_qty -= 1;
		count--;
		del_segm( start, SEGDEL_KEEP );
	}
	//create new ones
	for(unsigned int ii = 1; ii < 14; ii++ ) {
		if(!sections[ii])
			continue;
		ea_t start = sections[ii];
		//FIXME
		ea_t end   = get_next_segment(sections, 14, start);
		end = check_previous_segment_ends(previous_segment_ends, old_segment_qty, start, end);
		if( (ii==1) || (ii==2) ) {
			add_segm(1, sections[ii], end, section_names[ii], "CODE");
		}else if( (ii==7) || (ii==11) || (ii==12) ) {
			add_segm(1, sections[ii], end, section_names[ii], "BSS");
		}else{
			add_segm(1, sections[ii], end, section_names[ii], "DATA");
		}
		/* set adderssing mode to 32 bit */
		set_segm_addressing( getseg(sections[ii]), 1 );
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
			msg("ERROR: invalid section ID (%d) for %d\n", ent.section_num, which);
			continue;
		}
		if( sections[ent.section_num] == 0 ) {
			msg("ERROR: no address for section ID (%d) [%s]\n", ent.section_num, nom);
			continue;
		}
		if( (ent.section_num==1) || (ent.section_num==2) || (ent.section_num==5) ) {
			add_func(sections[ent.section_num]+ent.section_off, BADADDR);
		}
		set_name( sections[ent.section_num]+ent.section_off, nom, SN_PUBLIC);
	}

	qfclose(fp);

	/* FIXUP CODE SEGMENTS TO FUNCTIONS */
	fixup_functions();

	/* HIDE THAT DARNED WAIT BOX BECAUSE WE FINISHED */
	hide_wait_box();
}

//-----------------------------------------------------------------
char comment[] = "This is gets segments and naming from a SEL file.";
char help[]	= "Import SEL file\n\n";
char wanted_name[]   = "SEL Loader";
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

