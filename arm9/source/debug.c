#include "common/general.h"

void DS_Debug(char* string, ...)
{
	//va_list varg;
	//NOGBA(string);
	// iprintf(string);
}

void DS_DebugPause(void)
{
	DS_Debug("\n..Touch the screen to continue..\n");
	scanKeys();
	while(!(keysDown() & KEY_TOUCH))scanKeys();
}

size_t DS_UsedMem(void)
{
	return getMemUsed();
}

size_t DS_FreeMem(void)
{
	return getMemFree();
}
