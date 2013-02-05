#include "neweditor/editor.h"

blockFace_struct* blockFacePool=NULL;

void initBlocks(void)
{
	blockFacePool=NULL;
}

blockFace_struct* popBlockFace(blockFace_struct** l)
{
	if(!l || !*l)return NULL;
	blockFace_struct* bf=*l;
	*l=(*l)->next;
	bf->next=NULL;
	return bf;
}

blockFace_struct* newBlockFace(void)
{
	blockFace_struct* bf=popBlockFace(&blockFacePool);
	//if(!bf) ...add stuff to the pool...?
	return bf;
}

blockFace_struct* createBlockFace(u8 x, u8 y, u8 z, u8 dir)
{
	blockFace_struct* bf=newBlockFace();
	if(!bf)return bf;
	bf->x=x;bf->y=y;bf->z=z;
	bf->direction=dir;
	bf->next=NULL;
	return bf;
}

void addBlockFace(blockFace_struct** l, blockFace_struct* bf)
{
	if(!l || !*l || !bf)return;
	bf->next=*l;
	*l=bf;
}


