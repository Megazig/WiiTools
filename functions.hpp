#ifndef __FUNCTIONS_H
#define __FUNCTIONS_H

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string.h>

#include "types.hpp"
#include "endian.h"
#include "common.hpp"
#include "dol.hpp"

using namespace std;

// SigCompare defines
#define CMP_MATCHING 0
#define CMP_BAD_CODE -1
#define CMP_BAD_NAME -2
#define CMP_BAD_REFNAMES -3
#define CMP_BAD_REFOFFS -4
#define CMP_BAD_SIG -99

vector<u32> GetU32Vector( string code );
void* FindFunction(char* buffer, u32 length, vector<u32> findme);
char* FindFunction( char* start , char* end , const u32* binary , u32 length );
char* FindFunction(char* buffer, u32 length, const u32* findme, u32 findme_length);
char* CheckFunction(char* buffer, u32 length, const u32* findme, u32 findme_length);
void FindSig( char* buffer, u32 length, string sig, bool dol );
int CompareSigs( string sig1, string sig2 );
void ShowSigCodeDiff(string sig1, string sig2, bool stop=false);
void DumpSigInfo( string sig );
string GetSigName( string sig );
bool FindSigByName( string sig, string sigName );
char* FindBinary( char* start , u32 buffer_len , char* binary , u32 length );
char* FindBinary( char* start , u32 buffer_len , const u32* binary , u32 length );
void CreateIDC( char* buffer, u32 length, string sig, bool dol );

#endif
