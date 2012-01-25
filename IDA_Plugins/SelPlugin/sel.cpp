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

#define DEBUG 1

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

// Get a lis/addi pair - using u32 based offsets
unsigned int get_combined_address(unsigned int first, unsigned int second, unsigned int start)
{
	unsigned int ret1 = (get_long( start +  first*4 ) & 0xffff) << 16;
	unsigned int ret2 = (get_long( start + second*4 ) & 0xffff) << 0;
	if(ret2 >= 0x8000)
		ret1 -= (0x10000 - ret2);
	else
		ret1 += ret2;
	return ret1;
}

//0x00000001 == bl
//0x00000002 == mem_off
u32 RsoLink[] = {
	0x9421ffb0, 0x7c0802a6, 0x90010054, 0x39610050,
	0x00000001, 0x8083000c, 0x3a800000, 0x80030010,
	0x00000002, 0x7d441a14, 0x80a30030, 0x7d201a14,
	0x80830038, 0x7d051a14, 0x80030040, 0x7ce41a14,
	0x80a30048, 0x7cc01a14, 0x8083004c, 0x80030054,
	0x7ca51a14, 0x7c841a14, 0x00000002, 0x7c001a14,
	0x00000002, 0x00000002, 0x00000002, 0x00000002,
	0x00000002, 0x00000002, 0x00000002, 0x00000002,
	0x9a830023, 0x7c7f1b78, 0x00000002, 0x9143000c,
	0x00000002, 0x00000002, 0x00000002, 0x91230010,
	0x00000002, 0x00000002, 0x00000002, 0x91030030,
	0x00000002, 0x00000002, 0x00000002, 0x90e30038,
	0x3a200001, 0x39e00004, 0x3a400008, 0x90c30040,
	0x00000002, 0x90a30048, 0x9083004c, 0x90030054,
	0x480000f0, 0x801f000c, 0x2811000d, 0x7e009214,
	0x418100b4, 0x00000002, 0x7c63782e, 0x7c6903a6,
	0x4e800420, 0x92d00000, 0x480000bc, 0x92f00000,
	0x480000b4, 0x80b00004, 0x2c050000, 0x41820014,
	0x7e248b78, 0x00000002, 0x4cc63182, 0x00000001,
	0x92900000, 0x48000090, 0x80b00004, 0x2c050000,
	0x41820014, 0x7e248b78, 0x00000002, 0x4cc63182,
	0x00000001, 0x92900000, 0x4800006c, 0x93100000,
	0x48000064, 0x93300000, 0x4800005c, 0x93500000,
	0x48000054, 0x93700000, 0x4800004c, 0x93900000,
	0x48000044, 0x93b00000, 0x4800003c, 0x93d00000,
	0x48000034, 0x92900000, 0x4800002c, 0x92900000,
	0x48000024, 0x80b00004, 0x2c050000, 0x41820014,
	0x7e248b78, 0x00000002, 0x4cc63182, 0x00000001,
	0x92900000, 0x3a520008, 0x3a310001, 0x39ef0004,
	0x801f0008, 0x7c110040, 0x4180ff0c, 0x3c60aaab,
	0x801f003c, 0x3863aaab, 0x38800000, 0x7c030016,
	0x5405e8ff, 0x418200fc, 0x28050008, 0x38c5fff8,
	0x408100c4, 0x38060007, 0x38600000, 0x5400e8fe,
	0x7c0903a6, 0x28060000, 0x408100ac, 0x80df0038,
	0x38840008, 0x7c06182e, 0x7c00fa14, 0x7c06192e,
	0x801f0038, 0x7cc01a14, 0x8006000c, 0x7c00fa14,
	0x9006000c, 0x801f0038, 0x7cc01a14, 0x80060018,
	0x7c00fa14, 0x90060018, 0x801f0038, 0x7cc01a14,
	0x80060024, 0x7c00fa14, 0x90060024, 0x801f0038,
	0x7cc01a14, 0x80060030, 0x7c00fa14, 0x90060030,
	0x801f0038, 0x7cc01a14, 0x8006003c, 0x7c00fa14,
	0x9006003c, 0x801f0038, 0x7cc01a14, 0x80060048,
	0x7c00fa14, 0x90060048, 0x801f0038, 0x7cc01a14,
	0x38630060, 0x80060054, 0x7c00fa14, 0x90060054,
	0x4200ff5c, 0x7c042850, 0x1cc4000c, 0x7c0903a6,
	0x7c042840, 0x4080001c, 0x807f0038, 0x7c03302e,
	0x7c00fa14, 0x7c03312e, 0x38c6000c, 0x4200ffec,
	0x39610050, 0x38600001, 0x00000001, 0x80010054,
	0x7c0803a6, 0x38210050, 0x4e800020,
};
bool find_RsoLink(unsigned int* sections)
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
		size_t length = end - start;
		if(sizeof(RsoLink) > length)
			continue;
		msg("Size of RsoLink: %08x", sizeof(RsoLink));
		for(u32 ii = start; ii < end - sizeof(RsoLink); ii += 4) {
			for(u32 jj = 0; jj < sizeof(RsoLink); jj+=4) {
				if(RsoLink[jj/4] == 0x00000001)
					continue;
				if(RsoLink[jj/4] == 0x00000002)
					continue;
				if(RsoLink[jj/4] != get_long(ii + jj))
					break;
				if(jj == sizeof(RsoLink) - 4) {
					msg("Found RsoLink: %08x", ii);
					sections[0] = 0;
					//r22
					sections[1] = get_combined_address(31, 46, ii);
					msg("Section1: %08x\n", sections[1]);
					//r23
					sections[2] = get_combined_address(30, 45, ii);
					msg("Section2: %08x\n", sections[2]);
					//ctors - 0
					sections[3] = 0;
					//dtors - 0
					sections[4] = 0;
					//r24
					sections[5] = get_combined_address(29, 44, ii);
					//r25
					sections[6] = get_combined_address(28, 42, ii);
					//r26
					sections[7] = get_combined_address(27, 41, ii);
					//r27
					sections[8] = get_combined_address(26, 40, ii);
					//r29
					sections[9] = get_combined_address(24, 37, ii);
					// - 0
					sections[10] = 0;
					//r28
					sections[11] = get_combined_address(25, 38, ii);
					//r30
					sections[12] = get_combined_address(22, 36, ii);
					// - 0
					sections[13] = 0;
					return true;
				}
			}
		}
	}
	msg("[ERROR] RsoLink not found!!!\n");
	return false;
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
		char* chosen_name = askfile_c(false, "*.sel", "Please choose a .sel file");
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
	if(!find_RsoLink(sections)) {
		qfclose(fp);
		return;
	}
  
	unsigned int off = rhdr.ExportTableOffset;
	unsigned int len = rhdr.ExportTableLength;
	unsigned int nam = rhdr.ExportTableNames;

	/* SHOW A WAIT BOX TO LET EU KNOW WE MEAN BUSINESS */
	show_wait_box("Doing something cool");

	/* loop through symbols */
	/* set naming */
	unsigned int which = 0;
	for(unsigned int ii = off; ii < off+len; ii += sizeof(export_table_entry)) {
		which++;
#if DEBUG
		if (which > 0x1000)
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
		//FIXME - rodata==5
		if( (ent.section_num==1) || (ent.section_num==2) || (ent.section_num==5) ) {
			add_func(sections[ent.section_num]+ent.section_off, BADADDR);
		}
		set_name( sections[ent.section_num]+ent.section_off, nom, SN_PUBLIC);
	}

	qfclose(fp);

	/* TODO uncomment to allow CODE sections to be made into all code/functions */
	/* FIXUP CODE SEGMENTS TO FUNCTIONS */
	//fixup_functions();

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

