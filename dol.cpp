#include "dol.hpp"

void FixDolHeaderEndian(dolheader * header)
{
	for(int ii=0; ii<7; ii++)
	{
		header->offsetText[ii] = be32(header->offsetText[ii]);
		header->addressText[ii] =be32(header->addressText[ii]);
		header->sizeText[ii]   = be32(header->sizeText[ii]);
	}
	for(int ii=0; ii<11; ii++)
	{
		header->offsetData[ii] = be32(header->offsetData[ii]);
		header->addressData[ii] =be32(header->addressData[ii]);
		header->sizeData[ii]   = be32(header->sizeData[ii]);
	}
	header->addressBSS = be32(header->addressBSS);
	header->sizeBSS = be32(header->sizeBSS);
	header->entrypoint = be32(header->entrypoint);
}

u32 GetMemoryAddressDol( char* buffer, u32 offset)
{
	dolheader header;
	memcpy(&header, buffer, sizeof(header));

	FixDolHeaderEndian(&header);

	u32 mem = 0;
	u32 j = 0;
	for(int i=0; i<7; i++, j++)
	{
#ifdef DEBUG
		cout << "offsetText[" << i << "]: "
			<< header.offsetText[i];
		cout << "\t";
		cout << "addressText[" << i << "]: "
			<< header.addressText[i] << endl;
#endif
		if((header.offsetText[i] > offset) ||
				(header.offsetText[i] == 0))
			break;
	}
	j--;
	offset += header.addressText[j];
	offset -= header.offsetText[j];
#ifdef DEBUG
	cout << "Memory address: 0x"
		<< hex << offset  <<endl;
#endif
	mem = offset;
	return mem;
}

u32 GetFileOffsetDol( char* buffer, u32 address)
{
	dolheader header;
	memcpy(&header, buffer, sizeof(header));

	FixDolHeaderEndian(&header);

	u32 mem = 0;
	u32 j = 0;
	for(int i=0; i<7; i++, j++)
	{
#ifdef DEBUG
		cout << "offsetText[" << i << "]: "
			<< header.offsetText[i];
		cout << "\t";
		cout << "addressText[" << i << "]: "
			<< header.addressText[i] << endl;
#endif
		if((header.addressText[i] > address) ||
				(header.addressText[i] == 0))
			break;
	}
	j--;
	address -= header.addressText[j];
	address += header.offsetText[j];
#ifdef DEBUG
	cout << "File offset: 0x"
		<< hex << address  <<endl;
#endif
	mem = address;
	return mem;
}

