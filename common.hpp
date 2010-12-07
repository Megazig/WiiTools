#ifndef __COMMON_H
#define __COMMON_H

//#define DEBUG 1
//#define DEBUG_BINARY 1
//#define QUITME 1
#define FUNCTION_NAME_LIMIT 20

#include <string>
#include <vector>
#include <iostream>

#include "types.hpp"

/* CRAP ADDED TO BE WEEDED OUT */
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <cstdlib>
#include <fstream>
#include <string.h>
/* END OF CRAP */

using namespace std;

void ascii( u8 character );
void hexdump( char * pointer , u32 length );
void printStringVector( vector<string> lines );
void printCharVector( vector<char> line );
void stripCarriageReturns( string& StringToModify );
u32 ReadFile(const char* filename, char* buffer);
vector<char> readLine1(char * buffer, int len);
#if 0
string readLine2(char * buffer, int len);
#endif

#endif
