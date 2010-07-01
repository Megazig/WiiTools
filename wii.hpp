#ifndef __WII_H
#define __WII_H

#include <string>
#include "types.hpp"
#include "endian.hpp"

bool in_mem1( u32 value );
bool in_mem2( u32 value );
char* FindStackUpdate(char* buffer, u32 length);
char* FindStackUpdateReverse(char* buffer, u32 length);
char* FindBlr(char* buffer, u32 length);
char* FindBlrReverse(char* buffer, u32 length);

#endif
