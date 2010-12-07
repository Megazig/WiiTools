#include "wii.hpp"

bool in_mem1( u32 value )
{
	return (value<0x80000000 || value>0x817fffff) ? false : true;
}

bool in_mem2( u32 value )
{
	return (value<0x90000000 || value>0x93ffffff) ? false : true;
}

char* FindStackUpdate(char* buffer, u32 length)
{
	for(char* temp = buffer; temp < buffer+length; temp+=4)
	{
		if((Big32(temp) & 0xffff0000) == 0x94210000)
			return (char*)temp;
	}
	return NULL;
}

char* FindStackUpdateReverse(char* buffer, u32 length)
{
	for(char* temp = buffer; temp > buffer-length; temp-=4)
	{
		if((Big32(temp) & 0xffff0000) == 0x94210000)
			return (char*)temp;
	}
	return NULL;
}

char* FindBlr(char* buffer, u32 length)
{
	for(char* temp = buffer; temp < buffer+length; temp+=4)
	{
		if(Big32(temp) == 0x4e800020)
			return (char*)temp;
	}
	return NULL;
}

char* FindBlrReverse(char* buffer, u32 length)
{
	for(char* temp = buffer; temp > buffer-length; temp-=4)
	{
		if(Big32(temp) == 0x4e800020)
			return (char*)temp;
	}
	return NULL;
}

