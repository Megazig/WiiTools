#include "endian.hpp"

u16 Big16( char * pointer )
{
	return be16(*(u16*)(pointer));
}

u32 Big32( char * pointer )
{
	return be32(*(u32*)(pointer));
}

u32 Big32( const u32 * pointer )
{
	return be32(*pointer);
}

u16 Read16( char * pointer )
{
	return *(u16*)pointer;
}

u32 Read32( char * pointer )
{
	return *(u32*)pointer;
}

