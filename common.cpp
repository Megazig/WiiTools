#include "common.hpp"

void ascii( u8 character )
{
	printf("%c", (character<0x1e || character>0x7f) ? '.' : character);
}

void hexdump( char * pointer , u32 length )
{
	u32 go_to = 0;
#ifdef SHORT_DUMP
	go_to = 10;
#else
	go_to = (length + 0xF) >> 4;
#endif

#ifdef SAFETY
	if ( go_to > 16 )
		go_to = 16;
#endif

	u8 * temp1 = NULL;
	u8 * temp2 = NULL;
	temp1 = (u8*)pointer;
	temp2 = (u8*)pointer;
	for ( u32 j = 0 ; j < go_to ; j++ )
	{
		for ( u32 i = 0 ; i < 0x10 ; i++ )
			printf("%02x ", *temp1++);
		printf("  ");
		for ( u32 i = 0 ; i < 0x10 ; i++ )
			ascii(*temp2++);
		printf("\n");
	}
}

void printStringVector( vector<string> lines )
{
	for(u32 ii = 0; ii < lines.size(); ii++)
		cout << lines[ii] << endl;
}

void printCharVector( vector<char> line )
{
	for(u32 ii = 0; ii < line.size(); ii++)
		cout << line[ii];
	cout << endl;
}

void stripCarriageReturns( string& StringToModify )
{
	if(StringToModify.empty()) return;

	int startIndex = StringToModify.find_first_not_of("\r");
	int endIndex = StringToModify.find_last_not_of("\r");
	string tempString = StringToModify;
	StringToModify.erase();

	StringToModify = tempString.substr(startIndex, (endIndex - startIndex + 1));
}

u32 ReadFile(const char* filename, char* buffer) {
	ifstream save(filename, ios::in);
	if(!save)
	{
		cout << "File ";
		cout << filename << "could not be opened" << endl;
		exit(EXIT_FAILURE);
	}
	save.seekg(0, ifstream::end);
	u32 saveSize = save.tellg();
	save.seekg(0);

	buffer = new char[saveSize];
	save.read(buffer, saveSize);
	save.close();
	return saveSize;
}

vector<char> readLine1(char * buffer, int len)
{
	vector<char> line;
	for(char * spot = buffer; spot < buffer + len; spot++)
		if( *spot != '\n' )
			line.push_back(*spot);
		else
			return line;
	return line;
}

#if 0
string readLine2(const char * buffer, int len)
{
	string line;
	line.empty();
	for(const char* spot = buffer; spot < buffer + len; spot++)
		if( *spot != '\n' )
			line.append(*spot);
		else
			return line;
	return line;
}
#endif
