#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <cstdlib>

#include <fstream>

#include <string.h>

//#define DEBUG 1
//#define DEBUG_BINARY 1
//#define QUITME 1
//#define FUNCTION_NAME_LIMIT 20

#include "types.hpp"
#include "endian.hpp"
#include "dol.hpp"
#include "functions.hpp"
#include "wii.hpp"

using namespace std;

#define FWRITEKNOWN 0x8045ac38
#define IOSKNOWN 0x804bc930
#define OSKNOWN 0x804cd620

char* FindIosIoctlShort(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0x39610030,0x00000000,0x34010008,0x7c791b78,0x7c9a2378,
	};
	u32 findme_length = 8;

	return FindFunction(buffer, length, findme, findme_length);
}


char* FindIosIoctl(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0x39610030,0x00000000,0x34010008,0x7c791b78,0x7c9a2378,
		0x7cbb2b78,0x7cdc3378,0x7cfd3b78,0x7d1e4378,0x3be00000,0x4082000c,0x3be0fffc,0x4800004c,
		0x00000001,0x38800040,0x38a00020,0x00000000,0x2c030000,0x90610008,0x4082000c,0x3be0ffea,
		0x48000028,0x38a00000,0x38000006,0x90a30020,0x80810008,0x90a40024,0x80810008,0x90a40028,
		0x90030000,0x93230008,0x2c1f0000,0x40820088,0x80a10008,0x3be00000,0x2c050000,0x4082000c,
		0x3be0fffc,0x48000058,0x2c1d0000,0x9345000c,0x4182000c,0x3c1d8000,0x48000008,0x38000000,
		0x90050018,0x2c1b0000,0x93c5001c,0x4182000c,0x3c1b8000,0x48000008,0x38000000,0x90050010,
		0x7f63db78,0x7f84e378,0x93850014,0x00000000,0x7fa3eb78,0x7fc4f378,0x00000000,0x2c1f0000,
		0x40820014,0x80610008,0x38800000,0x00000000,0x7c7f1b78,0x39610030,0x7fe3fb78,0x00000000,
		0x80010034,0x7c0803a6,0x38210030,0x4e800020
	};
	u32 findme_length = 76;

	return FindFunction(buffer, length, findme, findme_length);
}

char* FindFwriteShort(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x7fdbc9d7,0x4182001c,0x881c000a,0x2c000000,0x40820010,0x801c0004,0x5400577f,0x4082000c,
		0x38600000,0x00000008,0x28000002,0x40820008
	};
	u32 findme_length = 12;
	
	return FindFunction(buffer, length, findme, findme_length);
}


char* FindFwrite(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0xbf210014,0x7c9b2378,0x7cdc3378,0x7c7a1b78,0x7cb92b78,
		0x38800000,0x7f83e378,0x00000000,0x2c030000,0x40820010,0x7f83e378,0x00000000,0x00000000,
		0x00000000,0x4182001c,0x881c000a,0x2c000000,0x40820010,0x801c0004,0x00000000,0x4082000c,
		0x38600000,0x48000290,0x28000002,0x40820008,0x00000000,0x807c0004,0x00000000,0x38800000,
		0x00000000,0x41820010,0x54603fbe,0x28000002,0x40820008,0x00000000,0x2c040000,0x40820018,
		0x801c0004,0x54003fbe,0x00000000,0x41820008,0x3be00000,0x801c0008,0x00000000,0x40820054,
		0x807c0004,0x00000000,0x54602f7e,0x41820044,0x00000000,0x41820024,0x7f83e378,0x38800000,
		0x38a00002,0x00000000,0x2c030000,0x4182000c,0x38600000,0x48000200,0x801c0008,0x00000000,
		0x5060e804,0x901c0008,0x7f83e378,0x00000000,0x801c0008,0x54001f7e,0x00000000,0x4182001c,
		0x00000000,0x38000000,0x987c000a,0x38600000,0x901c0028,0x480001c0,0x2c1e0000,0x3ba00000,
		0x41820128,0x807c001c,0x809c0024,0x7c041840,0x4082000c,0x2c1f0000,0x00000008,0x801c0020,
		0x7c632050,0x7c030050,0x901c0028,0x80bc0028,0x3b200000,0x7c05f040,0x90a10008,0x4081000c,
		0x7fc5f378,0x93c10008,0x801c0004,0x54003fbe,0x00000000,0x40820030,0x2c050000,0x41820028,
		0x7f43d378,0x3880000a,0x00000000,0x2c030000,0x7c791b78,0x41820010,0x00000000,0x7cba0050,
		0x90a10008,0x80a10008,0x2c050000,0x41820038,0x807c0024,0x7f44d378,0x00000000,0x80810008,
		0x807c0024,0x801c0028,0x7f5a2214,0x7c632214,0x7fc4f050,0x907c0024,0x80610008,0x7c030050,
		0x901c0028,0x801c0028,0x2c000000,0x41820018,0x2c190000,0x40820010,0x801c0004,0x00000000,
		0x40820030,0x7f83e378,0x38800000,0x00000000,0x2c030000,0x4182001c,0x00000000,0x38000000,
		0x987c000a,0x3bc00000,0x901c0028,0x4800001c,0x80010008,0x2c1e0000,0x7fbd0214,0x4182000c,
		0x2c1f0000,0x4082ff08,0x2c1e0000,0x4182006c,0x2c1f0000,0x40820064,0x833c001c,0x7c1af214,
		0x83fc0020,0x7f83e378,0x935c001c,0x38810008,0x93dc0020,0x901c0024,0x00000000,0x2c030000,
		0x41820018,0x00000000,0x38000000,0x987c000a,0x901c0028,0x4800000c,0x80010008,0x7fbd0214,
		0x933c001c,0x7f83e378,0x93fc0020,0x00000000,0x38000000,0x901c0028,0x801c0004,0x54003fbe,
		0x28000002,0x4182000c,0x38000000,0x901c0028,0x7c7ddb96,0xbb210014,0x80010034,0x7c0803a6,
		0x38210030,0x4e800020
	};
	u32 findme_length = 194;
	
	return FindFunction(buffer, length, findme, findme_length);
}

char* FindOsReport(char* buffer, u32 length)
{
	static const u32 findme[] = {
	0x9421ff80,0x7c0802a6,0x90010084,0x40860024,0xd8210028,0xd8410030,0xd8610038,0xd8810040,
	};
	u32 findme_length = 8;

	return FindFunction(buffer, length, findme, findme_length);
}

char* CheckFwrite(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0xbf210014,0x7c9b2378,0x7cdc3378,0x7c7a1b78,0x7cb92b78,
		0x38800000,0x7f83e378,0x00000000,0x2c030000,0x40820010,0x7f83e378,0x00000000,0x00000000,
		0x00000000,0x4182001c,0x881c000a,0x2c000000,0x40820010,0x801c0004,0x00000000,0x4082000c,
		0x38600000,0x48000290,0x28000002,0x40820008,0x00000000,0x807c0004,0x00000000,0x38800000,
		0x00000000,0x41820010,0x54603fbe,0x28000002,0x40820008,0x00000000,0x2c040000,0x40820018,
		0x801c0004,0x54003fbe,0x00000000,0x41820008,0x3be00000,0x801c0008,0x00000000,0x40820054,
		0x807c0004,0x00000000,0x54602f7e,0x41820044,0x00000000,0x41820024,0x7f83e378,0x38800000,
		0x38a00002,0x00000000,0x2c030000,0x4182000c,0x38600000,0x48000200,0x801c0008,0x00000000,
		0x5060e804,0x901c0008,0x7f83e378,0x00000000,0x801c0008,0x54001f7e,0x00000000,0x4182001c,
		0x00000000,0x38000000,0x987c000a,0x38600000,0x901c0028,0x480001c0,0x2c1e0000,0x3ba00000,
		0x41820128,0x807c001c,0x809c0024,0x7c041840,0x4082000c,0x2c1f0000,0x41820110,0x801c0020,
		0x7c632050,0x7c030050,0x901c0028,0x80bc0028,0x3b200000,0x00000009,0x00000009,0x4081000c,
		0x7fc5f378,0x93c10008,0x801c0004,0x54003fbe,0x00000000,0x40820030,0x2c050000,0x41820028,
		0x7f43d378,0x3880000a,0x00000000,0x2c030000,0x7c791b78,0x41820010,0x00000000,0x7cba0050,
		0x90a10008,0x80a10008,0x2c050000,0x41820038,0x807c0024,0x7f44d378,0x00000000,0x80810008,
		0x807c0024,0x801c0028,0x7f5a2214,0x7c632214,0x00000009,0x00000009,0x80610008,0x7c030050,
		0x901c0028,0x801c0028,0x2c000000,0x41820018,0x2c190000,0x40820010,0x801c0004,0x00000000,
		0x40820030,0x7f83e378,0x38800000,0x00000000,0x2c030000,0x4182001c,0x00000000,0x38000000,
		0x987c000a,0x3bc00000,0x901c0028,0x4800001c,0x80010008,0x2c1e0000,0x7fbd0214,0x4182000c,
		0x2c1f0000,0x4082ff08,0x2c1e0000,0x4182006c,0x2c1f0000,0x40820064,0x833c001c,0x7c1af214,
		0x83fc0020,0x7f83e378,0x935c001c,0x38810008,0x93dc0020,0x901c0024,0x00000000,0x2c030000,
		0x41820018,0x00000000,0x38000000,0x987c000a,0x901c0028,0x4800000c,0x80010008,0x7fbd0214,
		0x933c001c,0x7f83e378,0x93fc0020,0x00000000,0x38000000,0x901c0028,0x801c0004,0x54003fbe,
		0x28000002,0x4182000c,0x38000000,0x901c0028,0x7c7ddb96,0xbb210014,0x80010034,0x7c0803a6,
		0x38210030,0x4e800020
	};
	u32 findme_length = 194;
	
	return CheckFunction(buffer,length, findme, findme_length);
}

char* CheckIosIoctl(char* buffer, u32 length)
{
	static const u32 findme[] = {
		0x9421ffd0,0x7c0802a6,0x90010034,0x39610030,0x00000000,0x34010008,0x7c791b78,0x7c9a2378,
		0x7cbb2b78,0x7cdc3378,0x7cfd3b78,0x7d1e4378,0x3be00000,0x4082000c,0x3be0fffc,0x4800004c,
		0x00000001,0x38800040,0x38a00020,0x00000000,0x2c030000,0x90610008,0x4082000c,0x3be0ffea,
		0x48000028,0x38a00000,0x00000009,0x00000009,0x80810008,0x90a40024,0x80810008,0x90a40028,
		0x90030000,0x93230008,0x2c1f0000,0x40820088,0x80a10008,0x3be00000,0x2c050000,0x4082000c,
		0x3be0fffc,0x48000058,0x2c1d0000,0x9345000c,0x4182000c,0x3c1d8000,0x48000008,0x38000000,
		0x90050018,0x2c1b0000,0x93c5001c,0x4182000c,0x3c1b8000,0x48000008,0x38000000,0x90050010,
		0x7f63db78,0x7f84e378,0x93850014,0x00000000,0x7fa3eb78,0x7fc4f378,0x00000000,0x2c1f0000,
		0x40820014,0x80610008,0x38800000,0x00000000,0x7c7f1b78,0x39610030,0x7fe3fb78,0x00000000,
		0x80010034,0x7c0803a6,0x38210030,0x4e800020
	};
	u32 findme_length = 76;

	return CheckFunction(buffer,length, findme, findme_length);
}

char* CheckOsReport(char* buffer, u32 length)
{
	static const u32 findme[] = {
	0x9421ff80,0x7c0802a6,0x90010084,0x93e1007c,0x40860024,0xd8210028,0xd8410030,0xd8610038,
	0xd8810040,0xd8a10048,0xd8c10050,0xd8e10058,0xd9010060,0x39610088,0x38010008,0x3d800100,
	0x90610008,0x3be10068,0x9081000c,0x7fe4fb78,0x90a10010,0x90c10014,0x90e10018,0x9101001c,
	0x91210020,0x91410024,0x91810068,0x9161006c,0x90010070,0x00000000,0x80010084,0x83e1007c,
	0x7c0803a6,0x38210080,0x4e800020
	};
	u32 findme_length = 35;

	return CheckFunction(buffer,length, findme, findme_length);
}

int main(int argc, char **argv)
{
	if ( argc < 2 )
	{
		cout << "Usage: " << argv[0] <<
			" <dump or dol> [options]" << endl;
		cout << "options:" << endl;
		cout << "\t--check      check functions" << endl;
		cout << "\t--dol        using a dol" << endl;
		cout << "\t--patch      create a patch" << endl;
		cout << "\t--osreport   do osreport" << endl;
		return EXIT_FAILURE;
	}

	bool isDol = false;
	bool checkFunctions = false;
	bool createPatch = false;
	bool findOSReport = false;

	bool arguments[argc];
	for(int ii=0; ii<argc; ii++)
		arguments[ii] = false;

	if ( argc >= 3 )
	{
		for(int n=1; n < argc; n++)
		{
			if (!strncmp(argv[n], "--check", 7))
			{
				checkFunctions = true;
				arguments[n] = true;
			}
			if (!strncmp(argv[n], "--dol", 5))
			{
				isDol = true;
				arguments[n] = true;
			}
			if (!strncmp(argv[n], "--patch", 7))
			{
				createPatch = true;
				arguments[n] = true;
			}
			if (!strncmp(argv[n], "--osreport", 10))
			{
				findOSReport = true;
				arguments[n] = true;
			}
		}
	}

	int dol_index = 0;
	for(int ii=1; ii<argc; ii++)
	{
		if(!arguments[ii])
			if(!dol_index)
				dol_index = ii;
	}

	ifstream memDump(argv[dol_index], ios::in);
	if ( !memDump )
	{
		cout << "File ";
		cout << argv[dol_index] << " could not be opened"
			<< endl;
		return EXIT_FAILURE;
	}
	memDump.seekg(0, ifstream::end);
	unsigned int memDumpSize = memDump.tellg();
	memDump.seekg(0);

	char * buffer = new char[memDumpSize];
	memDump.read(buffer, memDumpSize);
	memDump.close();

	dolheader header;
	if ( isDol )
	{
		memcpy(&header, buffer, sizeof(header));
		FixDolHeaderEndian(&header);
	}

	u32 mem_fwrite = 0;
	u32 mem_ios_ioctl = 0;

	char* f_fwrite = FindFwrite(buffer, memDumpSize);
	if(!f_fwrite)
		cout << "can't find __fwrite" << endl;
	f_fwrite = NULL;
	f_fwrite = FindFwriteShort(buffer, memDumpSize);
	if(!f_fwrite)
		cout << "can't find __fwrite short" << endl;
	else
	{
		u32 delta = (u32)(f_fwrite - buffer);
		char* start = FindStackUpdateReverse(f_fwrite, delta);
		cout << "Should be 0x9421.... : 0x" << hex << Big32(start) << endl;
		if(!start)
			start = f_fwrite;
		u32 mem = (u32)(start - buffer);
		cout << "__fwrite short found at file offset: 0x"
			<< hex << mem << endl;
		if ( !isDol )
		{
			cout << "Memory address: 0x"
				<< hex << (0x80000000 | mem)  << endl;
			mem_fwrite = 0x80000000 | mem;
		}
		else
		{
			mem = GetMemoryAddressDol(buffer, mem);
			cout << "Memory address: 0x"
				<< hex << (0x80000000 | mem)  << endl;
			mem_fwrite = mem;
		}
	}
	if ( checkFunctions )
	{
		f_fwrite = NULL;
		u32 ofs = GetFileOffsetDol(buffer, FWRITEKNOWN);
		cout << "checking file offset: " << hex << ofs << endl;
		f_fwrite = CheckFwrite(buffer+ofs,memDumpSize);
		if(!f_fwrite)
			cout << "__fwrite didn't match" << endl;
		else
			cout << "__fwrite found" << endl;
	}
	char* f_ios_ioctl = FindIosIoctl(buffer, memDumpSize);
	if(!f_ios_ioctl)
		cout << "can't find IOS_Ioctl" << endl;
	else
		cout << "IOS_Ioctl found" << endl;
	f_ios_ioctl = NULL;
	f_ios_ioctl = FindIosIoctlShort(buffer, memDumpSize);
	if(!f_ios_ioctl)
		cout << "can't find IOS_Ioctl short" << endl;
	else
	{
		u32 mem = (u32)(f_ios_ioctl - buffer);
		cout << "IOS_Ioctl short found at file offset: 0x"
			<< hex << (u32)(f_ios_ioctl - buffer) << endl;
		if ( !isDol )
		{
			cout << "Memory address: 0x"
				<< hex << (0x80000000 | mem)  << endl;
			mem_ios_ioctl = 0x80000000 | mem;
		}
		else
		{
			mem = GetMemoryAddressDol(buffer, mem);
			cout << "Memory address: 0x"
				<< hex << (0x80000000 | mem)  << endl;
			mem_ios_ioctl = mem;
		}
	}
	if ( checkFunctions )
	{
		f_ios_ioctl = NULL;
		u32 ofs = GetFileOffsetDol(buffer, IOSKNOWN);
		cout << "checking file offset: " << hex << ofs << endl;
		f_ios_ioctl =CheckIosIoctl(buffer+ofs,memDumpSize);
		if(!f_ios_ioctl)
			cout << "can't find IOS_Ioctl" << endl;
		else
			cout << "IOS_Ioctl found" << endl;
	}
	if ( findOSReport )
	{
		char * f_osreport = FindOsReport(buffer, memDumpSize);
		if(!f_osreport)
		{
			cout << "can't find OSReport" << endl;
		} else {
			u32 mem = (u32)(f_osreport - buffer);
			cout << "OSReport found at file offset: 0x"
				<< hex << mem << endl;
			if(isDol)
			{
				mem = GetMemoryAddressDol(buffer, mem);
				cout << "Memory address: 0x"
					<< hex << (0x80000000 | mem)  << endl;
			} else {
				cout << "Memory address: 0x"
					<< hex << (0x80000000 | mem)  << endl;
			}
		}
		if(checkFunctions)
		{
			f_osreport = NULL;
			u32 ofs = GetFileOffsetDol(buffer, 0x804cd620);
			cout << "checking file offset: " << hex << ofs << endl;
			f_osreport =CheckOsReport(buffer+ofs,memDumpSize);
			if(!f_osreport)
				cout << "can't find OSReport" << endl;
			else
				cout << "OSReport found" << endl;
		}
	}
	if ( createPatch )
	{
		/* create patch for Riivolution */
		u32 patch[] = {
		0x38800061,			// ioctl = 0x61
		0x7CA62B78,			// in = buffer
		0x7C651B78,			// in_len = len
		0x38E00000,			// out = 0
		0x39000000,			// out_len = 0
		0x38600004,			// fd = 4
		0x4BF4702C			// b IOS_Ioctl( fd , ioctl , in , in_len , out , out_len );
		};
		if ((!mem_fwrite) || (!mem_ios_ioctl) )
			cout << "Error creating patch!!" << endl;

		//FIXME
		u32 riiv_fd = 4;
		// change fd
		patch[5] = 0x38600000 | riiv_fd;
		// change branch
		patch[6] = (0x48 << 24) | ((mem_ios_ioctl - (mem_fwrite + 24)) & 0x3ffffff);
#ifdef DEBUG
		cout << "From: 0x" << hex << mem_fwrite + 24
			<< " to 0x" << hex << mem_ios_ioctl
			<< " : 0x" << hex << patch[6] << endl;
#endif

		char outbuffer[100];
		memset(outbuffer, 0, 100);	// only need 96
		sprintf(outbuffer, "<memory offset=\"0x%08X\" value=\"388000617CA62B787C651B7838E0000039000000%08X%08X\" />", mem_fwrite, patch[5], patch[6] );
		cout << outbuffer << endl;
	}



	// DO SOMETHING COOL //
	/*
	for(unsigned int stri = 0; stri < sigs.size(); stri++)
		FindSig( buffer , memDumpSize, sigs[stri] );
	*/

	delete [] buffer;
	return EXIT_SUCCESS;
}
