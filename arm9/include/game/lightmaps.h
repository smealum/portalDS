#ifndef __LIGHTMAPS9__
#define __LIGHTMAPS9__

#define LIGHTMAPSLOTS 1

typedef struct
{
	mtlImg_struct* mtl;
	void* r;
	bool used;
}lightMapSlots_struct;

void initLightMaps(void);

#endif
