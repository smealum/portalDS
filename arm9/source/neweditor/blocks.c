#include "neweditor/editor.h"

blockFace_struct* blockFacePool=NULL;

vect3D faceNormals[]={vect(inttof32(1),0,0),vect(-inttof32(1),0,0),vect(0,inttof32(1),0),vect(0,-inttof32(1),0),vect(0,0,inttof32(1)),vect(0,0,-inttof32(1))};
u16 faceColors[]={RGB15(31,31,31),RGB15(25,25,25),RGB15(20,20,20),RGB15(15,15,15),RGB15(10,10,10),RGB15(5,5,5)}; //TEMP
u32 packedVertex[6][4]={{NORMAL_PACK((1<<5),-(1<<5),-(1<<5)), NORMAL_PACK((1<<5),(1<<5),-(1<<5)), NORMAL_PACK((1<<5),(1<<5),(1<<5)), NORMAL_PACK((1<<5),-(1<<5),(1<<5))},
						{NORMAL_PACK(-(1<<5),-(1<<5),-(1<<5)), NORMAL_PACK(-(1<<5),-(1<<5),(1<<5)), NORMAL_PACK(-(1<<5),(1<<5),(1<<5)), NORMAL_PACK(-(1<<5),(1<<5),-(1<<5))},
						{NORMAL_PACK(-(1<<5),(1<<5),-(1<<5)), NORMAL_PACK(-(1<<5),(1<<5),(1<<5)), NORMAL_PACK((1<<5),(1<<5),(1<<5)), NORMAL_PACK((1<<5),(1<<5),-(1<<5))},
						{NORMAL_PACK(-(1<<5),-(1<<5),-(1<<5)), NORMAL_PACK((1<<5),-(1<<5),-(1<<5)), NORMAL_PACK((1<<5),-(1<<5),(1<<5)), NORMAL_PACK(-(1<<5),-(1<<5),(1<<5))},
						{NORMAL_PACK(-(1<<5),-(1<<5),(1<<5)), NORMAL_PACK((1<<5),-(1<<5),(1<<5)), NORMAL_PACK((1<<5),(1<<5),(1<<5)), NORMAL_PACK(-(1<<5),(1<<5),(1<<5))},
						{NORMAL_PACK(-(1<<5),-(1<<5),-(1<<5)), NORMAL_PACK(-(1<<5),(1<<5),-(1<<5)), NORMAL_PACK((1<<5),(1<<5),-(1<<5)), NORMAL_PACK((1<<5),-(1<<5),-(1<<5))}};
vect3D faceOrigin[]={vect(inttof32(1),-inttof32(1),-inttof32(1)), vect(-inttof32(1),-inttof32(1),-inttof32(1)),
					vect(-inttof32(1),inttof32(1),-inttof32(1)), vect(-inttof32(1),-inttof32(1),-inttof32(1)),
					vect(-inttof32(1),-inttof32(1),inttof32(1)), vect(-inttof32(1),-inttof32(1),-inttof32(1))};
vect3D faceSize[]={vect(0,inttof32(1),inttof32(1)), vect(0,inttof32(1),inttof32(1)),
					vect(inttof32(1),0,inttof32(1)), vect(inttof32(1),0,inttof32(1)),
					vect(inttof32(1),inttof32(1),0), vect(inttof32(1),inttof32(1),0)};

mtlImg_struct* wallTexture;

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
	
	int i;
	for(i=0;i<6;i++)
	{
		faceOrigin[i]=vect(mulf32(faceOrigin[i].x,BLOCKSIZEX)/2,mulf32(faceOrigin[i].y,BLOCKSIZEY)/2,mulf32(faceOrigin[i].z,BLOCKSIZEZ)/2);
		faceSize[i]=vect(mulf32(faceSize[i].x,BLOCKSIZEX),mulf32(faceSize[i].y,BLOCKSIZEY),mulf32(faceSize[i].z,BLOCKSIZEZ));
	}
	
	wallTexture=createTexture("floor6.pcx", "textures");
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
				if(i<28 || j<30 || k<28 || i>=36 || j>=34 || k>=36 || (k==31 && i==31))setBlock(ba,i,j,k,1);
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
	bf->draw=true;
	bf->next=NULL;
	return bf;
}

void addBlockFace(blockFace_struct** l, blockFace_struct* bf)
{
	if(!l || !bf)return;	
	bf->next=*l;
	*l=bf;
}

void generateBlockFaces(u8* ba, blockFace_struct** l, u8 x, u8 y, u8 z)
{
	if(!ba || !l || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return;
	
	u8 v=getBlock(ba,x,y,z);
	if(!v)return;
	
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

vect3D getBlockPosition(u8 x, u8 y, u8 z)
{
	return vect((x-ROOMARRAYSIZEX/2)*BLOCKSIZEX,(y-ROOMARRAYSIZEY/2)*BLOCKSIZEY,(z-ROOMARRAYSIZEZ/2)*BLOCKSIZEZ);
}

bool collideLineBlockFace(blockFace_struct* bf, vect3D o, vect3D v, int32 d, int32* dist)
{
	vect3D n=faceNormals[bf->direction];
	int32 p1=dotProduct(v,n);
	if(!equals(p1,0) && p1<0)
	{
		vect3D p=addVect(faceOrigin[bf->direction],getBlockPosition(bf->x,bf->y,bf->z));
		vect3D s=faceSize[bf->direction];
		
		int32 p2=dotProduct(vectDifference(p,o),n);
		int32 k=divf32(p2,p1);
		if(k>d){return false;}
		vect3D i=addVect(o,vectMult(v,k));
		i=vectDifference(i,p);
		
		bool r=true;
		if(s.x)
		{
			if(s.x>0)r=r&&i.x<s.x&&i.x>=0;
			else r=r&&i.x>s.x&&i.x<=0;
		}
		if(s.y)
		{
			if(s.y>0)r=r&&i.y<s.y&&i.y>=0;
			else r=r&&i.y>s.y&&i.y<=0;
		}
		if(s.z)
		{
			if(s.z>0)r=r&&i.z<s.z&&i.z>=0;
			else r=r&&i.z>s.z&&i.z<=0;
		}
		if(r && dist)*dist=k;
		return r;
	}
	return false;
}

blockFace_struct* collideLineBlockFaceListClosest(blockFace_struct* l, vect3D o, vect3D v)
{
	if(!l)return NULL;
	
	int32 closestDist=1<<26;
	blockFace_struct* bf=NULL;
		
	while(l)
	{
		if(collideLineBlockFace(l, o, v, closestDist, &closestDist))
		{
			bf=l;
		}
		l=l->next;
	}
	return bf;
}

void drawBlockFace(blockFace_struct* bf)
{
	if(!bf || !bf->draw)return;
	
	u32* vtxPtr=packedVertex[bf->direction];
	
	applyMTL(wallTexture);
	
	glPushMatrix();
	
		glTranslate3f32(inttof32(bf->x),inttof32(bf->y),inttof32(bf->z));
	
		GFX_COLOR=faceColors[bf->direction];
	
		GFX_BEGIN=GL_QUADS;
		
		GFX_TEX_COORD=TEXTURE_PACK(0*16, 0*16);
		GFX_VERTEX10=*vtxPtr++;
		GFX_TEX_COORD=TEXTURE_PACK(64*16, 0*16);
		GFX_VERTEX10=*vtxPtr++;
		GFX_TEX_COORD=TEXTURE_PACK(64*16, 64*16);
		GFX_VERTEX10=*vtxPtr++;
		GFX_TEX_COORD=TEXTURE_PACK(0*16, 64*16);
		GFX_VERTEX10=*vtxPtr++;
	
	glPopMatrix(1);
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
		
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_ID(1));
	GFX_COLOR=RGB15(31,31,31);
	
	glPushMatrix();
		glScalef32((BLOCKSIZEX),(BLOCKSIZEY),(BLOCKSIZEZ));
		glTranslate3f32(inttof32(-ROOMARRAYSIZEX/2),inttof32(-ROOMARRAYSIZEY/2),inttof32(-ROOMARRAYSIZEZ/2));		
		drawBlockFaceList(er->blockFaceList);
	glPopMatrix(1);
}
