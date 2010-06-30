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

using namespace std;

int main(int argc, char **argv)
{
	bool isDol = false;
	bool createIDC = false;
	bool ordered = false;

	//FIXME
	bool arguments[argc];
	for(int ii=0; ii<argc; ii++)
		arguments[ii] = false;

	if ( argc < 3 )
	{
		cout << "Usage: " << argv[0] <<
			" <mega file> <dump or dol> [options]" << endl;
		cout << "options:" << endl;
		cout << "\t--dol     using a dol" << endl;
		cout << "\t--idc     create idc file [not working]"
			<< endl;
		cout << "\t--ordered [not functional yet]" << endl;
		return EXIT_FAILURE;
	}

	if ( argc >= 4 )
	{
		for(int n = 1; n < argc; n++)
		{
			if ( !strncmp(argv[n], "--dol", 5) )
			{
				isDol = true;
				arguments[n] = true;
			}
			if ( !strncmp(argv[n], "--idc", 5) )
			{
				createIDC = true;
				arguments[n] = true;
			}
			if ( !strncmp(argv[n], "--ordered", 9) )
			{
				ordered = true;
				arguments[n] = true;
			}
		}
	}

	int mega_index = 0;
	int dol_index = 0;
	for(int ii = 0; ii < argc; ii++)
	{
		if(arguments[ii] == false)
		{
			if(!mega_index)
				mega_index = ii;
			else if(!dol_index)
				dol_index = ii;
		}
	}

	ifstream myInputFile(argv[mega_index], ios::in);
	if ( !myInputFile )
	{
		cout << "File ";
		cout << argv[mega_index] << "could not be opened"
			<< endl;
		return EXIT_FAILURE;
	}

	vector<string> sigs;
	string sLine;
	while( getline(myInputFile, sLine) )
	{
		if ( !sLine.empty() )
			sigs.push_back( sLine );
	}
	myInputFile.close();

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
	char* tmpBuf = buffer;
	u32 size = memDumpSize;
	u32 offset = 0;
	for(u32 stri = 0; stri < sigs.size(); stri++)
	{
		m_sig sig = ParseMegaLine(tmpBuf, size, sigs[stri], isDol);
		function_instance instance = FindMSig(tmpBuf, size, offset, sig, isDol);
		if(instance.buffer_location)
			cout << instance.sig.funcName << ": " << hex << instance.memory_address << endl;
		if(instance.buffer_location)
		{
			tmpBuf = instance.buffer_location + 4;
			offset = (u32)(instance.buffer_location - buffer) + 4;
			size = memDumpSize - offset;
		}
	}

	delete [] buffer;
	return EXIT_SUCCESS;
}

