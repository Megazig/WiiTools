#ifndef __DOL_H
#define __DOL_H

#include <string.h>

#include "types.hpp"
#include "endian.hpp"

typedef struct
{
	u32 offsetText[7];
	u32 offsetData[11];
	u32 addressText[7];
	u32 addressData[11];
	u32 sizeText[7];
	u32 sizeData[11];
	u32 addressBSS;
	u32 sizeBSS;
	u32 entrypoint;
} dolheader;

void FixDolHeaderEndian(dolheader * header);
u32 GetMemoryAddressDol( char* buffer, u32 offset);
u32 GetFileOffsetDol( char* buffer, u32 address);

#endif
