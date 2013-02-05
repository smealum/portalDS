#include "neweditor/editor.h"

blockFace_struct* blockFacePool=NULL;

vect3D normals[]={vect(inttof32(1),0,0),vect(-inttof32(1),0,0),vect(0,inttof32(1),0),vect(0,-inttof32(1),0),vect(0,0,inttof32(1)),vect(0,0,-inttof32(1))};
u32 packedVertex[6][4]={{NORMAL_PACK((1<<6),-(1<<6),-(1<<6)), NORMAL_PACK((1<<6),-(1<<6),(1<<6)), NORMAL_PACK((1<<6),(1<<6),(1<<6)), NORMAL_PACK((1<<6),(1<<6),-(1<<6))},
						{NORMAL_PACK(-(1<<6),-(1<<6),-(1<<6)), NORMAL_PACK(-(1<<6),(1<<6),-(1<<6)), NORMAL_PACK(-(1<<6),(1<<6),(1<<6)), NORMAL_PACK(-(1<<6),-(1<<6),(1<<6))},
						{NORMAL_PACK(-(1<<6),(1<<6),-(1<<6)), NORMAL_PACK(-(1<<6),(1<<6),(1<<6)), NORMAL_PACK((1<<6),(1<<6),(1<<6)), NORMAL_PACK((1<<6),(1<<6),-(1<<6))},
						{NORMAL_PACK(-(1<<6),-(1<<6),-(1<<6)), NORMAL_PACK((1<<6),-(1<<6),-(1<<6)), NORMAL_PACK((1<<6),-(1<<6),(1<<6)), NORMAL_PACK(-(1<<6),-(1<<6),(1<<6))},
						{NORMAL_PACK(-(1<<6),-(1<<6),(1<<6)), NORMAL_PACK(-(1<<6),(1<<6),(1<<6)), NORMAL_PACK((1<<6),(1<<6),(1<<6)), NORMAL_PACK((1<<6),-(1<<6),(1<<6))},
						{NORMAL_PACK(-(1<<6),-(1<<6),-(1<<6)), NORMAL_PACK((1<<6),-(1<<6),-(1<<6)), NORMAL_PACK((1<<6),(1<<6),-(1<<6)), NORMAL_PACK(-(1<<6),(1<<6),-(1<<6))}};

void initBlockFacePool(void)
{
	blockFacePool=malloc(sizeof(blockFace_struct)*BLOCKFACEPOOLSIZE);
	if(!blockFacePool)return;
	int i;
	for(i=0;i<BLOCKFACEPOOLSIZE-1;i++)
	{
		blockFacePool[i].next=&blockFacePool[i+1];
	}
	blockFacePool[i].next=NULL;
}

void initBlocks(void)
{
	blockFacePool=NULL;
	initBlockFacePool();
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
	if(!er->blockArray)return;
	initBlockArray(er->blockArray);
	generateBlockFacesRange(er->blockArray, &er->blockFaceList, vect(0,0,0), vect(ROOMARRAYSIZEX,ROOMARRAYSIZEY,ROOMARRAYSIZEZ));
}

void freeBlockFace(blockFace_struct* bf)
{
	if(!bf)return;
	
	bf->next=blockFacePool;
	blockFacePool=bf;
}

void freeEditorRoom(editorRoom_struct* er)
{
	if(!er)return;
	
	if(er->blockArray)free(er->blockArray);
	while(er->blockFaceList)freeBlockFace(popBlockFace(&er->blockFaceList));
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
	
	if(!getBlock(ba,x+1,y,z))addBlockFace(l, createBlockFace(x,y,z,0));
	if(!getBlock(ba,x-1,y,z))addBlockFace(l, createBlockFace(x,y,z,1));
	if(!getBlock(ba,x,y+1,z))addBlockFace(l, createBlockFace(x,y,z,2));
	if(!getBlock(ba,x,y-1,z))addBlockFace(l, createBlockFace(x,y,z,3));
	if(!getBlock(ba,x,y,z+1))addBlockFace(l, createBlockFace(x,y,z,4));
	if(!getBlock(ba,x,y,z-1))addBlockFace(l, createBlockFace(x,y,z,5));
}

void generateBlockFacesRange(u8* ba, blockFace_struct** l, vect3D o, vect3D s)
{
	if(!ba)return;
	
	int i, j, k;
	for(i=o.x;i<o.x+s.x;i++)
	{
		for(j=o.y;j<o.y+s.y;j++)
		{
			for(k=o.z;k<o.z+s.z;k++)
			{
				generateBlockFaces(ba, l, i, j, k);
			}
		}
	}	
}

void drawBlockFace(blockFace_struct* bf)
{
	if(!bf)return;
	
	u32* vtxPtr=packedVertex[bf->direction];
	
	GFX_BEGIN=GL_QUADS;
	
	GFX_VERTEX10=*vtxPtr++;
	GFX_VERTEX10=*vtxPtr++;
	GFX_VERTEX10=*vtxPtr++;
	GFX_VERTEX10=*vtxPtr++;
}

void drawBlockFaceList(blockFace_struct* l)
{
	if(!l)return;
	
	while(l)
	{
		drawBlockFace(l);
		l=l->next;
	}
}

void drawEditorRoom(editorRoom_struct* er)
{
	if(!er)return;
	
	drawBlockFaceList(er->blockFaceList);
}
