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

	u32 addresses[18];
	u32 offsets[18];
	memset(addresses, 0, 18);
	memset(offsets, 0, 18);
	u32 counter = 0;
	for(int ii=0; ii<7; ii++) {
		if(header.addressText[ii] == 0)
			continue;
		offsets[counter] = header.offsetText[ii];
		addresses[counter] = header.addressText[ii];
		counter++;
	}
	for(int ii=0; ii<11; ii++) {
		if(header.addressText[ii] == 0)
			continue;
		offsets[counter] = header.offsetData[ii];
		addresses[counter] = header.addressData[ii];
		counter++;
	}
	counter--;
	
	for(u32 ii=0; ii<counter; ii++) {
		for(u32 jj=1; jj<counter; jj++) {
			if(offsets[jj] < offsets[jj-1]) {
				u32 temp = offsets[jj-1];
				offsets[jj-1] = offsets[jj];
				offsets[jj] = temp;
			}
		}
	}

	u32 mem = 0;
	u32 j = 0;
	for(u32 i=0; i<counter; i++, j++)
	{
#ifdef DEBUG
		cout << "offsetText[" << i << "]: "
			<< offsets[i];
		cout << "\t";
		cout << "addressText[" << i << "]: "
			<< addresses[i] << endl;
#endif
		if((offsets[i] > offset) ||
				(offsets[i] == 0))
			break;
	}
	j--;
	offset += addresses[j];
	offset -= offsets[j];
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

	u32 addresses[18];
	u32 offsets[18];
	memset(addresses, 0, 18);
	memset(offsets, 0, 18);
	u32 counter = 0;
	for(int ii=0; ii<7; ii++) {
		if(header.addressText[ii] == 0)
			continue;
		offsets[counter] = header.offsetText[ii];
		addresses[counter] = header.addressText[ii];
		counter++;
	}
	for(int ii=0; ii<11; ii++) {
		if(header.addressText[ii] == 0)
			continue;
		offsets[counter] = header.offsetData[ii];
		addresses[counter] = header.addressData[ii];
		counter++;
	}
	counter--;
	
	for(u32 ii=0; ii<counter; ii++) {
		for(u32 jj=1; jj<counter; jj++) {
			if(offsets[jj] < offsets[jj-1]) {
				u32 temp = offsets[jj-1];
				offsets[jj-1] = offsets[jj];
				offsets[jj] = temp;
			}
		}
	}

	u32 mem = 0;
	u32 j = 0;
	for(int i=0; i<counter; i++, j++)
	{
#ifdef DEBUG
		cout << "offsetText[" << i << "]: "
			<< offsets[i];
		cout << "\t";
		cout << "addressText[" << i << "]: "
			<< addresses[i] << endl;
#endif
		if((addresses[i] > address) ||
				(addresses[i] == 0))
			break;
	}
	j--;
	address -= addresses[j];
	address += offsets[j];
#ifdef DEBUG
	cout << "File offset: 0x"
		<< hex << address  <<endl;
#endif
	mem = address;
	return mem;
}

