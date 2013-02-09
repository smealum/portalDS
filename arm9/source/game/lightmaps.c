#include "game/game_main.h"

lightMapSlots_struct lightMapSlots[LIGHTMAPSLOTS];

void initLightMaps(void)
{
	int i;
	u16 palette[8];
	for(i=0;i<8;i++){u8 v=(i*31)/7;palette[i]=RGB15(v,v,v);}
	for(i=0;i<LIGHTMAPSLOTS;i++)
	{
		lightMapSlots[i].used=false;
		lightMapSlots[i].r=NULL;
		lightMapSlots[i].mtl=createReservedTextureBufferA5I3(NULL,palette,32,32,(void*)(0x6800000+0x0020000*(i+1)));
	}
}

lightMapSlots_struct* getLightMapSlot(void)
{
	int i;
	for(i=0;i<LIGHTMAPSLOTS;i++)
	{
		if(!lightMapSlots[i].used)
		{
			lightMapSlots[i].used=true;
			lightMapSlots[i].r=NULL;
			return &lightMapSlots[i];
		}
	}
	return NULL;
}

void loadLightMap(room_struct* r)
{
	if(!r||!r->lightMapBuffer||(r->lmSlot&&r->lmSlot->used))return;
	NOGBA("here?");
	r->lmSlot=getLightMapSlot();
	if(!r->lmSlot)return;
	r->lmSlot->r=r;
	changeTextureSizeA5I3(r->lmSlot->mtl,r->lmSize.x,r->lmSize.y);
	loadToBank(r->lmSlot->mtl,r->lightMapBuffer);
}

void unloadLightMap(room_struct* r)
{
	if(!r||!r->lightMapBuffer)return;
	if(!r->lmSlot)return;
	r->lmSlot->used=false;
	r->lmSlot=NULL;
}

void unloadLightMaps(room_struct* r, room_struct* r2)
{
	int i;
	for(i=0;i<LIGHTMAPSLOTS;i++)
	{
		if(lightMapSlots[i].used && &lightMapSlots[i]!=r->lmSlot && &lightMapSlots[i]!=r2->lmSlot)
		{
			room_struct* r3=lightMapSlots[i].r;
			if(r3 && r3->lmSlot==&lightMapSlots[i])r3->lmSlot=NULL;
			lightMapSlots[i].r=NULL;
			lightMapSlots[i].used=false;
		}
	}
}
