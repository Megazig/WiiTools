#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.hpp"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

//#define DEBUG

void showOptions(char * app_name)
{
	printf("Usage: %s <options> <jumpfrom> <jumpto>\n",
			app_name);
	printf("possible options\n");
	printf("    --help     show help\n");
	printf("    --version  show program version\n");
	printf("    --riiv     make riivolution memory patch\n");
	printf("    --bl       make jump a bl instead of b\n");
	exit(EXIT_FAILURE);
	return;
}

int main(int argc, char **argv)
{
	bool used_args[10] = {false,false,false,false,
		false,false,false,false,false,false};
	bool bl = false;
	bool riiv = false;
	bool b_from = false;
	bool b_to = false;
	u32 jumpfrom = 0x801d5940;
	u32 jumpto = 0x8015f730;

	if(argc == 1)
		showOptions(argv[0]);

	for(int ii=1; ii<argc; ii++)
	{
		// 10 args in used list
		if(ii == 10) break;

		if(!strncmp(argv[ii], "--help", 6)) {
			used_args[ii] = true;
			showOptions(argv[0]);
		}
		else if(!strncmp(argv[ii], "--riiv", 6)) {
			riiv = true;
			used_args[ii] = true;
#ifdef DEBUG
			printf("--riiv at position %d\n", ii);
#endif
		}
		else if(!strncmp(argv[ii], "--bl", 4)) {
			bl = true;
			used_args[ii] = true;
#ifdef DEBUG
			printf("--bl at position %d\n", ii);
#endif
		}
		else if(!strncmp(argv[ii], "--version", 9)) {
			used_args[ii] = true;
			printf("%s v%d.%d\n",
					argv[0], VERSION_MAJOR, VERSION_MINOR);
			exit(EXIT_FAILURE);
		}
	}
	for(int x = 1; x < argc; x++)
	{
		//FIXME
		if(x == 10) {
			printf("Too many args\n");
			exit(EXIT_FAILURE);
		}

		if(!used_args[x])
		{
#ifdef DEBUG
			printf("value found at position %d\n", x);
#endif
			if(!b_from) {
				jumpfrom = strtoul(argv[x], NULL, 16);
				b_from = true;
			} else if(!b_to) {
				jumpto =   strtoul(argv[x], NULL, 16);
				b_to = true;
			}
		}
	}

	u32 delta = jumpto - jumpfrom;
	u32 jumpvalue = 0x48 << 24 | ( delta & 0x3ffffff);

	if ( bl )
		jumpvalue |= 1;

	if(riiv) {
		printf("<memory offset=\"0x%08x\" value-\"%08x\" />\n",
				jumpfrom, jumpvalue);
	} else {
		printf("Jump from %08x to %08x using %08x\n",
				jumpfrom, jumpto, jumpvalue);
	}

	return EXIT_SUCCESS;
}
