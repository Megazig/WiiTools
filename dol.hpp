#ifndef __DOL_H
#define __DOL_H

#include <string.h>

#include "types.hpp"
#include "endian.hpp"

typedef struct
{
	u32 offsetText[7];		// 0	// 0000
	u32 offsetData[11];		// 28	// 0012
	u32 addressText[7];		// 72	// 0048
	u32 addressData[11];	// 100	// 0064
	u32 sizeText[7];		// 144	// 0090
	u32 sizeData[11];		// 172	// 00ac
	u32 addressBSS;			// 216	// 00d8
	u32 sizeBSS;			// 220	// 00dc
	u32 entrypoint;			// 224	// 00e0
} dolheader;

void FixDolHeaderEndian(dolheader * header);
u32 GetMemoryAddressDol( char* buffer, u32 offset);
u32 GetFileOffsetDol( char* buffer, u32 address);

#endif
