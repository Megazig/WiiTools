#include "functions.hpp"

vector<u32> GetU32Vector( string code )
{
	vector<u32> binary;
	for(u32 ii = 0; ii < code.length(); ii+=8)
	{
		string sub = code.substr(ii, 8);
#ifdef DEBUG_BINARY
		cout << sub << endl;
#endif
		const char * codeptr = sub.c_str();
		u32 dword = strtoul(codeptr, NULL, 16);
#ifdef DEBUG_BINARY
		cout << "0x" << hex << dword << endl;
#endif
		binary.push_back( dword );
	}
#ifdef DEBUG_BINARY
	cout << endl;
#endif
	return binary;
}

void* FindFunction(char* buffer, u32 length, vector<u32> findme)
{
	u32 findme_length = findme.size();
	for (u32* location = (u32*)buffer; (u8*)location < (u8*)buffer + length - findme_length * 4; location++)
	{
		u32 i = 0;
		for (u32* check = location; check < location + findme_length; check++, i++) {
			if ((findme[i] > 0x10) && (be32(*check) != findme[i]))
				break;
		}
		if (i == findme_length)
			return (void*)location;
	}
	return NULL;
}

char * FindFunction( char * start , char * end , const u32 * binary , u32 length )
{
	if( length > (u32)( end - start ) ) return NULL;
	for( char * search = start ; search < ( end - length ) ; search++ )
	{
		u32 * check = (u32*)search;
		u32 i = 0;
		for( i = 0 ; i < (length / 4) ; i++ )
		{
			if(*(binary+i) < 0x10)
				continue;
			if( Big32(binary+i) != check[i] )
				break;
		}
		if ( i == (length / 4) )
			return search;
	}
	return NULL;
}

char* FindFunction(char* buffer, u32 length, const u32* findme, u32 findme_length)
{
	for (u32* location = (u32*)buffer; (u8*)location < (u8*)buffer + length - findme_length * 4; location++)
	{
		u32 i = 0;
		for (u32* check = location; check < location + findme_length; check++, i++) {
			if((findme[i]>0x10) && (be32(*check) != findme[i]))
				break;
		}
		if (i == findme_length)
			return (char*)location;
	}
	return NULL;
}

char* CheckFunction(char* buffer, u32 length, const u32* findme, u32 findme_length)
{
	u32 i = 0;
	for(u32* check = (u32*)buffer; check < (u32*)buffer + findme_length; check++, i++)
	{
		cout << hex << be32(*check)
			<< "\t" << hex << findme[i];
		if ((findme[i] > 0x10) && (be32(*check) != findme[i]))
			cout << " NOT MATCHING!!!";
		cout << endl;
	}
	if (i == findme_length)
		return (char*)buffer;
	else
		cout << "missed at insn " << i << endl;
	return NULL;
}

void FindSig( char* buffer, u32 length, string sig, bool dol )
{
	if ( sig.length() < 5 )
		return;
	//FIXME
	if ( sig == "---" )
		return;

	stringstream ss(sig);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	if ( FUNCTION_NAME_LIMIT < funcName.length() )
		return;
	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
#ifdef DEBUG
	cout << "Size of refs: " << dec << refs.size() << endl;
	cout << funcName << endl;
#endif
	vector<u32> binary = GetU32Vector( code );

	void * func = NULL;
	func = FindFunction( buffer , length , binary );
	if ( func )
	{
		u32 offs = 0;
		u32 file_offs = (u32)((char*)func - buffer);
		if ( dol )
			offs = GetMemoryAddressDol(buffer, file_offs);
		else
			offs = file_offs + 0x80000000;
		cout << funcName;
		cout << " at ";
		cout << "0x" << hex << offs;
		cout << endl;
		for(u32 ii=0; ii < refs.size(); ii++)
		{
			cout << dec << refs[ii].first << "\t"
				<< refs[ii].second;

			char* ref_offs = (char*)func + refs[ii].first;
			u32 insn = Big32(ref_offs);
			u32 b_amt = insn ^ 0x48000000;
			if ( b_amt & 0x2000000 )
				b_amt = b_amt | 0xfd000000;
			b_amt &= 0xfffffffe;
			u32 ref_address = offs + refs[ii].first;
			ref_address += b_amt;
			if ( ( insn & 0x48000000 ) == 0x48000000 )
				cout << "\t" << hex << ref_address;

			//u32 val = GetFileOffsetDol(buffer, address);
			cout << endl;
		}
		/* show xrefs */
	}
#ifdef QUITME
	exit(1);
#endif
}

int CompareSigs( string sig1, string sig2 )
{
	//FIXME
	if ( sig1 == "---" )
		return CMP_BAD_SIG;
	if ( sig2 == "---" )
		return CMP_BAD_SIG;

	// SIG1
	stringstream ss(sig1);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
	
	// SIG2
	stringstream ss2(sig2);
	string code2 , unk12 , funcName2;
	getline( ss2 , code2 , space );
	getline( ss2 , unk12 , space );
	getline( ss2 , funcName2 , space );
	stripCarriageReturns( funcName2 );
	vector< pair<int, string> > refs2;
	while( !ss2.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss2 , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss2 , tempString , space );
		stripCarriageReturns( tempString );
		refs2.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}

	if(code != code2)
		return CMP_BAD_CODE;

	for(u32 ii=0; ii < refs.size(); ii++)
	{
		if(funcName != funcName2)
			return CMP_BAD_NAME;
		if(refs[ii].first != refs2[ii].first)
			return CMP_BAD_REFNAMES;
		if(refs[ii].second != refs2[ii].second)
			return CMP_BAD_REFOFFS;
	}

	return CMP_MATCHING;
}

void ShowSigCodeDiff(string sig1, string sig2, bool stop)
{
	//FIXME
	if ( sig1 == "---" )
		return;
	if ( sig2 == "---" )
		return;

	// SIG1
	stringstream ss(sig1);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
	
	// SIG2
	stringstream ss2(sig2);
	string code2 , unk12 , funcName2;
	getline( ss2 , code2 , space );
	getline( ss2 , unk12 , space );
	getline( ss2 , funcName2 , space );
	stripCarriageReturns( funcName2 );
	vector< pair<int, string> > refs2;
	while( !ss2.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss2 , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss2 , tempString , space );
		stripCarriageReturns( tempString );
		refs2.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}

	vector<u32> vec1 = GetU32Vector( code );
	vector<u32> vec2 = GetU32Vector( code2 );

#if 0
	cout << "\t\tin:" << endl;
#endif
	for(u32 ii = 0; ii < vec1.size(); ii++)
	{
#if 0
		cout << "\t\t\tchecked" << hex << vec1[ii] <<
			"\t" << vec2[ii] << endl;
#endif
		if(vec1[ii] != vec2[ii])
		{
			cout << "\t" << hex << ii <<
				" doesn't match " << 
				vec1[ii] << "\t" << vec2[ii] << endl;
			if(stop)
				return;
		}
	}
	return;
}

void DumpSigInfo( string sig )
{
	//FIXME
	if ( sig == "---" )
		return;

	stringstream ss(sig);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	if ( FUNCTION_NAME_LIMIT < funcName.length() )
		return;
	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
#ifdef DEBUG
	cout << "Size of refs: " << dec << refs.size() << endl;
#endif
	cout << funcName << endl;
	for(u32 ii=0; ii < refs.size(); ii++)
	{
		cout << dec << refs[ii].first << "\t"
			<< refs[ii].second << endl;;
	}
}

string GetSigName( string sig )
{
	if ( sig == "---" )
		return "---";

	stringstream ss(sig);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );

	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
	return funcName;
}

bool FindSigByName( string sig, string sigName )
{
	if ( sig == "---" )
		return false;

	stringstream ss(sig);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );

	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
	if(sigName == funcName)
		return true;
	else
		return false;
}

char * FindBinary( char * start , u32 buffer_len , char * binary , u32 length )
{
	for ( u32 i = 0 ; i < ( buffer_len - length ) ; i++ )
		if ( !memcmp( start + i , binary , length ) )
			return start + i;
	return NULL;
}

char * FindBinary( char * start , u32 buffer_len , const u32 * binary , u32 length )
{
	for ( u32 i = 0 ; i < ( buffer_len - length ) ; i++ )
	{
		u32 * check = (u32*)(start + i);
		u32 j = 0;
		for ( j = 0 ; j < length / 4 ; j++ )
			if ( check[j] != Big32(binary + j) )
				break;
		if ( j == length / 4 )
			return (start + i);
	}
	return NULL;
}

void CreateIDC( char* buffer, u32 length, string sig, bool dol )
{
	if ( sig.length() < 5 )
		return;
	//FIXME
	if ( sig == "---" )
		return;

	stringstream ss(sig);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	if ( FUNCTION_NAME_LIMIT < funcName.length() )
		return;
	vector< pair<int, string> > refs;
	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
#ifdef DEBUG
	cout << "Size of refs: " << dec << refs.size() << endl;
	cout << funcName << endl;
#endif
	vector<u32> binary = GetU32Vector( code );

	void * func = NULL;
	func = FindFunction( buffer , length , binary );
	if ( func )
	{
		u32 offs = 0;
		u32 file_offs = (u32)((char*)func - buffer);
		if ( dol )
			offs = GetMemoryAddressDol(buffer, file_offs);
		else
			offs = file_offs + 0x80000000;
		cout << "MakeFunction(0x" << hex << offs <<
			", BADADDR); MakeName(0x" << hex << offs <<
			", \"" << funcName << "\");" << endl;
		for(u32 ii=0; ii < refs.size(); ii++)
		{
			/*
			cout << dec << refs[ii].first << "\t"
				<< refs[ii].second;
			*/

			char* ref_offs = (char*)func + refs[ii].first;
			u32 insn = Big32(ref_offs);
			u32 b_amt = insn ^ 0x48000000;
			if ( b_amt & 0x2000000 )
				b_amt = b_amt | 0xfd000000;
			b_amt &= 0xfffffffe;
			u32 ref_address = offs + refs[ii].first;
			ref_address += b_amt;
			if ( ( insn & 0x48000000 ) == 0x48000000 )
			{
				//cout << "\t" << hex << ref_address;
				cout << "MakeFunction(0x" << hex <<
					ref_address << ", BADADDR); MakeName(0x" <<
					hex << ref_address << ", \"" <<
					refs[ii].second << "\");" << endl;
			}

			//u32 val = GetFileOffsetDol(buffer, address);
			//cout << endl;
		}
		/* show xrefs */
	}
#ifdef QUITME
	exit(1);
#endif
}

m_sig ParseMegaLine(char* buffer, u32 length, string sig, bool dol)
{
	m_sig msig;
	msig.code = "";
	msig.unk1 = "0000:";
	msig.funcName = "";
	vector< pair<int, string> > refs;
	msig.refs = refs;
	if ( sig.length() < 5 )
		return msig;
	//FIXME
	if ( sig == "---" )
		return msig;

	stringstream ss(sig);
	string code , unk1 , funcName;
	char space = ' ';
	getline( ss , code , space );
	getline( ss , unk1 , space );
	getline( ss , funcName , space );
	stripCarriageReturns( funcName );
	msig.code = code;
	msig.unk1 = unk1;
	msig.funcName = funcName;

	while( !ss.eof() )
	{
		//FIXME
		string tempNum, tempString;
		getline( ss , tempNum , space );
		u32 num = strtoul(tempNum.c_str() + 1, NULL, 16);
		getline( ss , tempString , space );
		stripCarriageReturns( tempString );
		refs.push_back( pair<int, string>(num, tempString) );
		//refs.push_back( tempString );
	}
	msig.refs = refs;
	return msig;
}

function_instance FindMSig(char* buffer, u32 length, u32 offset, m_sig sig, bool dol)
{
#if 0
	cout << "buffer: " << hex << (u32)buffer << " [DEBUG]" << endl
		<< "length: " << hex << length << " [DEBUG]" << endl
		<< "offset: " << hex << offset << " [DEBUG]" << endl
		<< "code: " << sig.code << " [DEBUG]" << endl;
#endif
	function_instance instance;
	instance.sig = sig;
	instance.buffer_location = NULL;
	instance.memory_address = 0;
	vector<u32> binary = GetU32Vector( sig.code );

	void * func = NULL;
	func = FindFunction( buffer , length , binary );
#if 0
	cout << sig.funcName << ": " << hex << (u32)func
		<< " [DEBUG]" << endl;
#endif
	if ( func )
	{
		instance.buffer_location = (char*)func - offset;
		u32 offs = 0;
		u32 file_offs = (u32)((char*)func - buffer) + offset;
		if ( dol )
			offs=GetMemoryAddressDol(buffer-offset, file_offs);
		else
			offs = file_offs + 0x80000000;
		instance.memory_address = offs;
	}
	return instance;
}

