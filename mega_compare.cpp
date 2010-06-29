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
	bool isExpanded = false;

	if ( argc < 3 )
	{
		cout << "Usage: " << argv[0] <<
			" <mega file 1> <mega file 2> [options]" << endl;
		cout << "options:" << endl;
		cout << "\t--expanded     show expanded diffs" << endl;
		return EXIT_FAILURE;
	}

	if ( argc >= 4 )
	{
		for(int n = 3; n < argc; n++)
		{
			if ( !strncmp(argv[n], "--expanded", 10) )
			{
#if 1
				cout << "Expanded" << endl;
#endif
				isExpanded = true;
			}
		}
	}

	// FILE1
	ifstream myInputFile(argv[1], ios::in);
	if ( !myInputFile )
	{
		cout << "File ";
		cout << argv[1] << "could not be opened" << endl;
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

	// FILE2
	ifstream myInputFile2(argv[2], ios::in);
	if ( !myInputFile2 )
	{
		cout << "File ";
		cout << argv[2] << "could not be opened" << endl;
		return EXIT_FAILURE;
	}

	vector<string> sigs2;
	string sLine2;
	while( getline(myInputFile2, sLine2) )
	{
		if ( !sLine2.empty() )
			sigs2.push_back( sLine2 );
	}
	myInputFile2.close();


	// DO SOMETHING COOL //
	for(u32 stri = 0; stri < sigs.size(); stri++)
	{
		string sig1Name = GetSigName( sigs[stri] );
		bool match = false;
		for(u32 stri2 = 0; stri2 < sigs2.size(); stri2++)
		{
			match = FindSigByName(sigs2[stri2], sig1Name);
			if(match)
			{
				int cmp= CompareSigs(sigs[stri], sigs2[stri2]);
				if(cmp  == CMP_MATCHING) 
				{
					cout << sig1Name << " MATCHES!!" << endl;
				}else if(cmp == CMP_BAD_CODE){
					cout << sig1Name <<
						" CODE DOESN'T MATCH" << endl;
					if(isExpanded)
					{
#if 0
						cout << "\texpand:" << endl;
#endif
						ShowSigCodeDiff(
								sigs[stri],
								sigs2[stri2] );
					}
				}else if(cmp == CMP_BAD_NAME){
					cout << sig1Name <<
						" NAME DOESN'T MATCH" << endl;
				}else if(cmp == CMP_BAD_REFNAMES){
					cout << sig1Name <<
						" REFNAME DOESN'T MATCH" << endl;
				}else if(cmp == CMP_BAD_REFOFFS){
					cout << sig1Name <<
						" REFOFF DOESN'T MATCH" << endl;
				}
				break;
			}
		}
		if(!match)
			cout << sig1Name << " NOT FOUND!!!" << endl;
	}

	return EXIT_SUCCESS;
}
