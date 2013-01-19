#include "game/game_main.h"

platform_struct platform[NUMPLATFORMS];

void initPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		platform[i].used=false;
	}
}

void initPlatform(platform_struct* pf, vect3D pos)
{
	if(!pf)return;
	
	pf->used=true;
}

platform_struct* createPlatform(vect3D pos)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(!platform[i].used)
		{
			initPlatform(&platform[i],pos);
			return &platform[i];;
		}
	}
	return NULL;
}

void drawPlatform(platform_struct* pf)
{
	if(!pf)return;
}

void drawPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)drawPlatform(&platform[i]);
	}
}

void updatePlatform(platform_struct* pf)
{
	if(!pf)return;
}

void updatePlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)updatePlatform(&platform[i]);
	}
}
