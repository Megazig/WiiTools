#ifndef __ENDIAN_H
#define __ENDIAN_H

#include "types.hpp"

#define be16(x)        ((x>>8)|(x<<8))
#define be32(x)        ((x>>24)|((x<<8)&0x00FF0000)|((x>>8)&0x0000FF00)|(x<<24))
#define be64(x)        ((x>>56)|((x<<40)&(u64)0x00FF000000000000)|((x<<24)&(u64)0x0000FF0000000000)|((x<<8)&(u64)0x000000FF00000000)|((x>>8)&(u64)0x00000000FF000000)|((x>>24)&(u64)0x0000000000FF0000)|((x<<40)&(u64)0x000000000000FF00)|(x<<56))

u16 Big16( char* pointer );
u32 Big32( char* pointer );
u32 Big32( const u32* pointer );
u16 Read16( char* pointer );
u32 Read32( char* pointer );

#endif
