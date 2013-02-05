#include "neweditor/editor.h"

blockFace_struct* blockFacePool=NULL;

void initBlocks(void)
{
	blockFacePool=NULL;
}

u8 getBlock(u8* ba, s8 x, s8 y, s8 z)
{
	if(!ba || x<0 || y<0 || z<0 || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return 1; //not empty
	return ba[x+y*ROOMARRAYSIZEX+z*ROOMARRAYSIZEX*ROOMARRAYSIZEY];
}

void setBlock(u8* ba, u8 x, u8 y, u8 z, u8 v)
{
	if(!ba || x<0 || y<0 || z<0 || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return;
	ba[x+y*ROOMARRAYSIZEX+z*ROOMARRAYSIZEX*ROOMARRAYSIZEY]=v;
}

void initBlockArray(u8* ba)
{
	if(!ba)return;
	int i, j, k;
	for(i=0;i<ROOMARRAYSIZEX;i++)
	{
		for(j=0;j<ROOMARRAYSIZEY;j++)
		{
			for(k=0;k<ROOMARRAYSIZEZ;k++)
			{
				if(i<16 || j<16 || k<16 || i>=48 || j>=48 || k>=48)setBlock(ba,i,j,k,1);
				else setBlock(ba,i,j,k,0);
			}
		}
	}
}

void initEditorRoom(editorRoom_struct* er)
{
	if(!er)return;
	er->blockFaceList=NULL;
	er->blockArray=malloc(sizeof(u8)*ROOMARRAYSIZEX*ROOMARRAYSIZEY*ROOMARRAYSIZEZ);
	initBlockArray(er->blockArray);
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

void generateBlockFaces(u8* ba, blockFace_struct** l, u8 x, u8 y, u8 z)
{
	if(!ba || !l || !*l || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return;
	
	if()addBlockFace(l, createBlockFace(x,y,z,));
}
