/*
 *  IDA Nintendo GameCube SEL Loader Module
 *  (C) Copyright 2010 Stephen Simpson
 *
 */

#ifndef __SEL_H__
#define __SEL_H__

#define START	0x80900000

/* Header Size = 100h bytes 

		0000	Prev
		0004	Next
		0008	Section Count
		000C	Section Offset
		0010	Path Offset
		0014	Path Length
	0018-002F	Unk[6]
		0030	Internal Table Offset
		0034	Internal Table Length
		0038	External Table Offset
		003C	External Table Length
		0040	Export Table Offset
		0044	Export Table Length
		0048	Export Table Names
		004C	Import Table Offset
		0050	Import Table Length
		0054	Import Table Names
*/

typedef struct {
	unsigned int Prev;
	unsigned int Next;
	unsigned int SectionCount;
	unsigned int SectionOffset;
	unsigned int PathOffset;
	unsigned int PathLength;
	unsigned int Unknown[6];
	unsigned int InternalTableOffset;
	unsigned int InternalTableLength;
	unsigned int ExternalTableOffset;
	unsigned int ExternalTableLength;
	unsigned int ExportTableOffset;
	unsigned int ExportTableLength;
	unsigned int ExportTableNames;
	unsigned int ImportTableOffset;
	unsigned int ImportTableLength;
	unsigned int ImportTableNames;
} selhdr;

typedef struct {
	unsigned int Offset;
	unsigned int Length;
} section_entry;

typedef struct {
	unsigned int name_off;
	unsigned int section_off;
	unsigned int section_num;
	unsigned int elf_hash;
} export_table_entry;

#endif
