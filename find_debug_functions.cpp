
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.hpp"
#include "endian.hpp"
#include "common.hpp"
#include "functions.hpp"

char * FindOSReport(char* buffer, u32 length)
{
	/*
	static const u32 OSReport[] = {
	0x9421ff80,0x7c0802a6,0x90010084,0x93e1007c,0x40860024,0xd8210028,0xd8410030,0xd8610038,
	0xd8810040,0xd8a10048,0xd8c10050,0xd8e10058,0xd9010060,0x39610088,0x38010008,0x3d800100,
	0x90610008,0x3be10068,0x9081000c,0x7fe4fb78,0x90a10010,0x90c10014,0x90e10018,0x9101001c,
	0x91210020,0x91410024,0x91810068,0x9161006c,0x90010070,0x00000000,0x80010084,0x83e1007c,
	0x7c0803a6,0x38210080,0x4e800020
	};
	u32 OSReport_size = 0x0000008c;
	*/
	static const u32 OSReport[] = {
	0x9421ff80,0x7c0802a6,0x90010084,0x40860024,0xd8210028,0xd8410030,0xd8610038,0xd8810040,
	0xd8a10048,0xd8c10050,0xd8e10058,0xd9010060,0x39610088,0x38010008,0x3d800100,0x9081000c,
	0x38810068,0x90610008,0x90a10010,0x90c10014,0x90e10018,0x9101001c,0x91210020,0x91410024,
	0x91810068,0x9161006c,0x90010070,0x00000000,0x80010084,0x7c0803a6,0x38210080,0x4e800020
	};
	u32 OSReport_size = 0x00000080;
	return FindFunction( buffer , buffer+length , OSReport, OSReport_size );
}

char * FindDebugPrint(char* buffer, u32 length)
{
	static const u32 DEBUGPrint[] = {
	0x9421ff90,0x40860024,0xd8210028,0xd8410030,0xd8610038,0xd8810040,0xd8a10048,0xd8c10050,
	0xd8e10058,0xd9010060,0x90610008,0x9081000c,0x90a10010,0x90c10014,0x90e10018,0x9101001c,
	0x91210020,0x91410024,0x38210070,0x4e800020
	};
	u32 DEBUGPrint_size = 0x00000050;
	return FindFunction( buffer , buffer+length , DEBUGPrint, DEBUGPrint_size );
}

int main( int argc , char * argv[] )
{
	bool isDol = false;
	bool riiv = false;

	if(argc < 2)
	{
		printf("Usage: %s <dump or dol>\n", argv[0]);
		printf("options:\n");
		printf("\t--dol          Using a dol\n");
		printf("\t--riivolution  Show Riivolution memory patch\n");
		exit(EXIT_FAILURE);
	}
	if ( argc >= 3 )
	{
		for(int n = 2; n < argc; n++)
		{
			if ( !strncmp(argv[n], "--dol", 5) )
				isDol = true;
			if ( !strncmp(argv[n], "--riivolution", 13) )
				riiv = true;
		}
	}

	FILE * fp = fopen( argv[1] , "rb" );
	if ( fp == NULL )
	{
		printf("Error opening file\n");
		exit(EXIT_FAILURE);
	}
	fseek( fp , 0 , SEEK_END );
	u32 length = ftell( fp );
	fseek( fp , 0 , SEEK_SET );
	char * buffer = new char[length];
	u32 count = fread( buffer , 1 , length , fp );
	if ( count != length )
	{
		printf("Error reading file in\n");
		exit(EXIT_FAILURE);
	}

	u32 os_offs = 0;
	char * osreport = NULL;
	osreport = FindOSReport( buffer , length);
	if(osreport)
	{
		u32 offs = 0;
		u32 file_offs = (u32)osreport - (u32)buffer;
		if(isDol)
			offs = GetMemoryAddressDol(buffer, file_offs);
		else
			offs = file_offs + 0x80000000;
		printf("MakeFunction( 0x%08x , BADADDR );\n", offs );
		printf("MakeName( 0x%08x , \"OSReport\" );\n", offs );
		os_offs = offs;
	} else {
		printf("OSReport not found\n");
		return -1;
	}

	char * function_pointer = buffer;
	char * end_pointer = buffer + length;
	printf("Finding all DEBUGPrint() occurances\n");
	while( true )
	{
		u32 len = (u32)end_pointer - (u32)function_pointer;
		function_pointer = FindDebugPrint( function_pointer , len);
		if(function_pointer)
		{
			u32 offs = 0;
			u32 file_offs= (u32)function_pointer - (u32)buffer;
			if(isDol)
				offs = GetMemoryAddressDol(buffer, file_offs);
			else
				offs = file_offs + 0x80000000;
			u32 func_offs = os_offs - offs;
			//printf("MakeName( 0x%08x , \"DEBUGPrint\" );\n", offs );
			u32 jumpvalue = 0x48 << 24 | (func_offs & 0x3ffffff);
			printf("Jump from %08x to %08x using %08x\n", offs, os_offs, jumpvalue);
			if(riiv)
				printf("<memory offset=\"0x%08x\" value=\"%08x\" />\n", offs, jumpvalue);
			function_pointer += 4;
		} else {
			break;
		}
	}

	delete [] buffer;
	fclose( fp );
	return EXIT_SUCCESS;
}
