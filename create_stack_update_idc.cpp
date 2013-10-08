#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>

#include "types.hpp"
#include "endian.hpp"
#include "dol.hpp"
#include "functions.hpp"
#include "wii.hpp"

using namespace std;

int main(int argc, char **argv)
{
	bool isDol = false;
	std::vector<bool> arguments(argc, false);

	if ( argc < 2 )
	{
		cout << "Usage: " << argv[0] <<
			" <dump or dol> [options]" << endl;
		cout << "options:" << endl;
		cout << "\t--dol     using a dol" << endl;
		return EXIT_FAILURE;
	}

	if ( argc >= 3 )
	{
		for(int n = 1; n < argc; n++)
		{
			if ( !strncmp(argv[n], "--dol", 5) )
			{
				isDol = true;
				arguments[n] = true;
			}
		}
	}

	int dol_index = 0;
	for(int ii = 0; ii < argc; ii++)
	{
		if(arguments[ii] == false)
		{
			if(!dol_index)
				dol_index = ii;
		}
	}

	ifstream memDump(argv[dol_index], ios::in);
	if ( !memDump )
	{
		cout << "File ";
		cout << argv[dol_index] << "could not be opened"
			<< endl;
		return EXIT_FAILURE;
	}
	memDump.seekg(0, ifstream::end);
	u32 memDumpSize = memDump.tellg();
	memDump.seekg(0);

	char * buffer = new char[memDumpSize];
	memDump.read(buffer, memDumpSize);
	memDump.close();

	// DO SOMETHING COOL //
	cout << "#include <idc.idc>" << endl;
	cout << "static main() {" << endl;

	u32 size = memDumpSize;
	for(char* tmp = buffer; tmp < buffer+memDumpSize; tmp+=4, size-=4)
	{
		char* stwu = FindStackUpdate(tmp, size);
		if(!stwu)
			break;

		u32 delta = (u32)(stwu - buffer);
		u32 addr = 0;
		if(isDol)
			addr = GetMemoryAddressDol(buffer, delta);
		else
			addr = 0x80000000 | delta;

		cout << "\tMakeFunction(0x" << hex << addr << ", BADADDR);" << endl;

		size = memDumpSize - delta;
		tmp = stwu;
	}

	cout << "}" << endl;

	delete [] buffer;
	return EXIT_SUCCESS;
}

