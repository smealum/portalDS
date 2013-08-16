#include "editor/editor_main.h"

blockFace_struct* blockFacePool=NULL;
blockFace_struct* blockFacePoolHead=NULL;

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
u8 oppositeDirection[]={1,0,3,2,5,4};

mtlImg_struct *wallTexture, *floorTexture, *unportalableTexture;

void initBlockFacePool(void)
{
	blockFacePoolHead=blockFacePool=malloc(sizeof(blockFace_struct)*BLOCKFACEPOOLSIZE);
	if(!blockFacePool)return;
	int i;
	for(i=0;i<BLOCKFACEPOOLSIZE-1;i++)
	{
		blockFacePool[i].next=&blockFacePool[i+1];
	}
	blockFacePool[i].next=NULL;
}

void freeBlockFacePool(void)
{
	if(blockFacePoolHead)free(blockFacePoolHead);
	blockFacePoolHead=NULL;
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
	floorTexture=createTexture("floor3.pcx", "textures");
	unportalableTexture=createTexture("floor7.pcx", "textures");
}

BLOCK_TYPE getBlock(BLOCK_TYPE* ba, s8 x, s8 y, s8 z)
{
	if(!ba || x<0 || y<0 || z<0 || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return 1; //not empty
	return ba[x+y*ROOMARRAYSIZEX+z*ROOMARRAYSIZEX*ROOMARRAYSIZEY];
}

void setBlock(BLOCK_TYPE* ba, u8 x, u8 y, u8 z, BLOCK_TYPE v)
{
	if(!ba || x<0 || y<0 || z<0 || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return;
	ba[x+y*ROOMARRAYSIZEX+z*ROOMARRAYSIZEX*ROOMARRAYSIZEY]=v;
}

void fixOriginSize(vect3D* o, vect3D* s)
{
	if(!o || !s)return;
	if(s->x<0){o->x-=s->x=-s->x;}
	if(s->y<0){o->y-=s->y=-s->y;}
	if(s->z<0){o->z-=s->z=-s->z;}
}

void searchAndDestroyBlockFacesRange(blockFace_struct** l, vect3D o, vect3D s)
{
	if(!l || !*l)return;

	blockFace_struct** bf=l;
	blockFace_struct* nbf=*bf;
	vect3D e=addVect(o, s);
	while(bf && *bf && nbf)
	{
		if((nbf->x>=o.x && nbf->x<e.x && nbf->y>=o.y && nbf->y<e.y && nbf->z>=o.z && nbf->z<e.z)
		|| (nbf->direction==0 && nbf->x==o.x-1 && nbf->y>=o.y && nbf->y<e.y && nbf->z>=o.z && nbf->z<e.z)
		|| (nbf->direction==1 && nbf->x==e.x   && nbf->y>=o.y && nbf->y<e.y && nbf->z>=o.z && nbf->z<e.z)
		|| (nbf->direction==2 && nbf->y==o.y-1 && nbf->x>=o.x && nbf->x<e.x && nbf->z>=o.z && nbf->z<e.z)
		|| (nbf->direction==3 && nbf->y==e.y   && nbf->x>=o.x && nbf->x<e.x && nbf->z>=o.z && nbf->z<e.z)
		|| (nbf->direction==4 && nbf->z==o.z-1 && nbf->x>=o.x && nbf->x<e.x && nbf->y>=o.y && nbf->y<e.y)
		|| (nbf->direction==5 && nbf->z==e.z   && nbf->x>=o.x && nbf->x<e.x && nbf->y>=o.y && nbf->y<e.y))
		{
			freeBlockFace(popBlockFace(bf));
		}else{
			bf=&((*bf)->next);
		}
		nbf=*bf;
	}
}

void changePortalableBlockDirection(BLOCK_TYPE* ba, u8 x, u8 y, u8 z, u16 u, bool portalable)
{
	if(!ba)return;
	BLOCK_TYPE v=getBlock(ba, x, y, z);

	v|=u;
	if(portalable)v^=u;

	setBlock(ba, x, y, z, v);
}

void changeSludgeBlock(BLOCK_TYPE* ba, u8 x, u8 y, u8 z, u16 u, bool nosludge)
{
	if(!ba)return;
	BLOCK_TYPE v=getBlock(ba, x, y, z);

	v|=BLOCK_SLUDGE;
	if(nosludge)v^=BLOCK_SLUDGE;

	setBlock(ba, x, y, z, v);
}

void changeAttributeBlockArrayRange(BLOCK_TYPE* ba, blockAttributeFunction f, blockFace_struct* l, vect3D o, vect3D s, bool unset)
{
	if(!ba || !f)return;

	fixOriginSize(&o, &s);
	vect3D e=addVect(o, s);

	while(l)
	{
		if((l->x>=o.x && l->x<e.x && l->y>=o.y && l->y<e.y && l->z>=o.z && l->z<e.z)
		|| (l->direction==0 && l->x==o.x-1 && l->y>=o.y && l->y<e.y && l->z>=o.z && l->z<e.z)
		|| (l->direction==1 && l->x==e.x   && l->y>=o.y && l->y<e.y && l->z>=o.z && l->z<e.z)
		|| (l->direction==2 && l->y==o.y-1 && l->x>=o.x && l->x<e.x && l->z>=o.z && l->z<e.z)
		|| (l->direction==3 && l->y==e.y   && l->x>=o.x && l->x<e.x && l->z>=o.z && l->z<e.z)
		|| (l->direction==4 && l->z==o.z-1 && l->x>=o.x && l->x<e.x && l->y>=o.y && l->y<e.y)
		|| (l->direction==5 && l->z==e.z   && l->x>=o.x && l->x<e.x && l->y>=o.y && l->y<e.y))
		{
			f(ba, l->x, l->y, l->z, 1<<(l->direction+1), unset);
		}
		l=l->next;
	}
}

void changeAttributeBlockArrayRangeDirection(BLOCK_TYPE* ba, blockAttributeFunction f, blockFace_struct* l, vect3D o, vect3D s, u8 dir, bool unset)
{
	if(!ba || !f)return;

	fixOriginSize(&o, &s);
	vect3D e=addVect(o, s);

	while(l)
	{
		if(l->direction==dir && ((l->x>=o.x && l->x<e.x && l->y>=o.y && l->y<e.y && l->z>=o.z && l->z<e.z)
		|| (l->direction==0 && l->x==o.x-1 && l->y>=o.y && l->y<e.y && l->z>=o.z && l->z<e.z)
		|| (l->direction==1 && l->x==e.x   && l->y>=o.y && l->y<e.y && l->z>=o.z && l->z<e.z)
		|| (l->direction==2 && l->y==o.y-1 && l->x>=o.x && l->x<e.x && l->z>=o.z && l->z<e.z)
		|| (l->direction==3 && l->y==e.y   && l->x>=o.x && l->x<e.x && l->z>=o.z && l->z<e.z)
		|| (l->direction==4 && l->z==o.z-1 && l->x>=o.x && l->x<e.x && l->y>=o.y && l->y<e.y)
		|| (l->direction==5 && l->z==e.z   && l->x>=o.x && l->x<e.x && l->y>=o.y && l->y<e.y)))
		{
			f(ba, l->x, l->y, l->z, 1<<(l->direction+1), unset);
		}
		l=l->next;
	}
}

void fillBlockArrayRange(BLOCK_TYPE* ba, blockFace_struct** l, vect3D o, vect3D s)
{
	if(!ba)return;

	fixOriginSize(&o, &s);

	int i, j, k;
	for(i=o.x;i<o.x+s.x;i++)
	{
		for(j=o.y;j<o.y+s.y;j++)
		{
			for(k=o.z;k<o.z+s.z;k++)
			{
				setBlock(ba, i, j, k, 1);
			}
		}
	}

	searchAndDestroyBlockFacesRange(l, o, s);
	generateBlockFacesRange(ba, l, o, s, false);
}

void emptyBlockArrayRange(BLOCK_TYPE* ba, blockFace_struct** l, vect3D o, vect3D s)
{
	if(!ba)return;

	fixOriginSize(&o, &s);

	int i, j, k;
	for(i=o.x;i<o.x+s.x;i++)
	{
		for(j=o.y;j<o.y+s.y;j++)
		{
			for(k=o.z;k<o.z+s.z;k++)
			{
				setBlock(ba, i, j, k, 0);
			}
		}
	}

	searchAndDestroyBlockFacesRange(l, o, s);
	generateBlockFacesRange(ba, l, o, s, true);
}

vect3D adjustVectForNormal(u8 dir, vect3D v)
{
	switch(dir)
	{
		case 0: v.x++; break;
		case 1: v.x--; break;
		case 2: v.y++; break;
		case 3: v.y--; break;
		case 4: v.z++; break;
		case 5: v.z--; break;
	}
	return v;
}

void initBlockArray(BLOCK_TYPE* ba)
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
				// if(i<28 || j<1 || k<28 || i>=36 || j>=63 || k>=36)setBlock(ba,i,j,k,1); //TEMP
				else setBlock(ba,i,j,k,0);
			}
		}
	}
}

char editorMapFilePath[2048];

void setEditorMapFilePath(char* str)
{
	if(!str)return;

	strcpy(editorMapFilePath,str);
}

char* getEditorMapFilePath(void)
{
	return editorMapFilePath;
}

void initEditorRoom(editorRoom_struct* er)
{
	if(!er)return;
	er->blockFaceList=NULL;
	er->blockArray=malloc(sizeof(BLOCK_TYPE)*ROOMARRAYSIZEX*ROOMARRAYSIZEY*ROOMARRAYSIZEZ);
	if(!er->blockArray)return;

	// initBlockArray(er->blockArray);
	// generateBlockFacesRange(er->blockArray, &er->blockFaceList, vect(0,0,0), vect(ROOMARRAYSIZEX,ROOMARRAYSIZEY,ROOMARRAYSIZEZ), false);

	// loadMapEditor(er,"maps/default.map");
	if(!loadMapEditor(er,editorMapFilePath))loadMapEditor(er,"maps/default.map");
}

void freeBlockFace(blockFace_struct* bf)
{
	if(!bf)return;
	
	bf->next=blockFacePool;
	blockFacePool=bf;
}

void freeBlockFaceList(blockFace_struct** l)
{
	if(!l)return;
	while(*l)freeBlockFace(popBlockFace(l));
}

void freeEditorRoom(editorRoom_struct* er)
{
	if(!er)return;
	
	if(er->blockArray)free(er->blockArray);
	freeBlockFaceList(&er->blockFaceList);
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

void generateBlockFace(BLOCK_TYPE* ba, blockFace_struct** l, u8 x, u8 y, u8 z, u8 dir)
{
	if(!ba || !l)return;
	if(!getBlock(ba,x,y,z))return;
	bool add=false;
	switch(dir)
	{
		case 0: add=!(getBlock(ba,x+1,y,z)&1); break;
		case 1: add=!(getBlock(ba,x-1,y,z)&1); break;
		case 2: add=!(getBlock(ba,x,y+1,z)&1); break;
		case 3: add=!(getBlock(ba,x,y-1,z)&1); break;
		case 4: add=!(getBlock(ba,x,y,z+1)&1); break;
		case 5: add=!(getBlock(ba,x,y,z-1)&1); break;
	}
	if(add)addBlockFace(l, createBlockFace(x,y,z,dir));
}

void generateBlockFaces(BLOCK_TYPE* ba, blockFace_struct** l, u8 x, u8 y, u8 z)
{
	if(!ba || !l || x>=ROOMARRAYSIZEX || y>=ROOMARRAYSIZEY || z>=ROOMARRAYSIZEZ)return;
	
	BLOCK_TYPE v=getBlock(ba,x,y,z);
	if(!v)return;
	
	generateBlockFace(ba, l, x, y, z, 0);
	generateBlockFace(ba, l, x, y, z, 1);
	generateBlockFace(ba, l, x, y, z, 2);
	generateBlockFace(ba, l, x, y, z, 3);
	generateBlockFace(ba, l, x, y, z, 4);
	generateBlockFace(ba, l, x, y, z, 5);
}

blockFace_struct* findBlockFace(blockFace_struct* l, u8 x, u8 y, u8 z, u8 direction)
{
	while(l)
	{
		if(l->x==x && l->y==y && l->z==z && l->direction==direction)return l;
		l=l->next;
	}
	return NULL;
}

void generateBlockFacesRange(BLOCK_TYPE* ba, blockFace_struct** l, vect3D o, vect3D s, bool outskirts)
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

	if(outskirts)
	{
		for(i=o.x;i<o.x+s.x;i++)
		{
			for(j=o.y;j<o.y+s.y;j++)
			{
				generateBlockFace(ba, l,i,j,o.z-1,4);
				generateBlockFace(ba, l,i,j,o.z+s.z,5);
			}
		}

		for(k=o.z;k<o.z+s.z;k++)
		{
			for(j=o.y;j<o.y+s.y;j++)
			{
				generateBlockFace(ba, l,o.x-1,j,k,0);
				generateBlockFace(ba, l,o.x+s.x,j,k,1);
			}
		}

		for(k=o.z;k<o.z+s.z;k++)
		{
			for(i=o.x;i<o.x+s.x;i++)
			{
				generateBlockFace(ba, l,i,o.y-1,k,2);
				generateBlockFace(ba, l,i,o.y+s.y,k,3);
			}
		}
	}
}

vect3D vectBlockToRectangle(vect3D v){return (vect3D){v.x*BLOCKMULTX,v.y*BLOCKMULTY,v.z*BLOCKMULTZ};}

static inline bool isWall(u8 v1, u8 v2){return ((v1&1) && !(v2&1)) && !(v1&BLOCK_NOWALLS) && !(v2&BLOCK_NOWALLS);}

rectangleList_struct generateOptimizedRectangles(BLOCK_TYPE* ba, rectangleList_struct* sludgeList)
{
	rectangleList_struct rl;
	initRectangleList(&rl);
	if(!ba)return rl;
	if(sludgeList)initRectangleList(sludgeList);
	int i, j, k;
	u8 *data1, *data2;
	s16 cnt1, cnt2;
	s16 cnt12, cnt22;
	u16 maxSize=max(ROOMARRAYSIZEX, max(ROOMARRAYSIZEY, ROOMARRAYSIZEZ));
	data1=malloc(maxSize*maxSize*sizeof(u8));if(!data1)return rl;
	data2=malloc(maxSize*maxSize*sizeof(u8));if(!data2){free(data1);return rl;}

	cnt1=0; cnt2=0;
	cnt12=0; cnt22=0;
	for(i=0;i<ROOMARRAYSIZEX;i++)
	{
		u8 *d1=data1, *d2=data2;
		for(k=0;k<ROOMARRAYSIZEZ;k++)
		{
			for(j=0;j<ROOMARRAYSIZEY;j++)
			{
				const BLOCK_TYPE v=getBlock(ba,i,j,k);
				*d1=isWall(v,getBlock(ba,i+1,j,k));
				*d2=isWall(v,getBlock(ba,i-1,j,k));

				if(*d1 && (v>>1)&1)*d1=2;
				if(*d2 && (v>>2)&1)*d2=2;

				if(*d1==1)cnt1++;
				else if(*d1==2)cnt12++;
				if(*d2==1)cnt2++;
				else if(*d2==2)cnt22++;
				d1++;d2++;
			}
		}

		//portalable
		while(cnt1>0)
		{
			vect2D p, s;
			getMaxRectangle(data1, 1, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data1, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(i+1,p.x+s.x,p.y)),vectBlockToRectangle(vect(0,-s.x,s.y)),true),&rl);
			cnt1-=s.x*s.y;
		}
		while(cnt2>0)
		{
			vect2D p, s;
			getMaxRectangle(data2, 1, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data2, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(i,p.x,p.y)),vectBlockToRectangle(vect(0,s.x,s.y)),true),&rl);
			cnt2-=s.x*s.y;
		}

		//unportalable
		while(cnt12>0)
		{
			vect2D p, s;
			getMaxRectangle(data1, 2, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data1, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(i+1,p.x+s.x,p.y)),vectBlockToRectangle(vect(0,-s.x,s.y)),false),&rl);
			cnt12-=s.x*s.y;
		}
		while(cnt22>0)
		{
			vect2D p, s;
			getMaxRectangle(data2, 2, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data2, ROOMARRAYSIZEY, ROOMARRAYSIZEZ, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(i,p.x,p.y)),vectBlockToRectangle(vect(0,s.x,s.y)),false),&rl);
			cnt22-=s.x*s.y;
		}
	}

	cnt1=0; cnt2=0;
	cnt12=0; cnt22=0;
	s16 cnt3=0;
	for(j=0;j<ROOMARRAYSIZEY;j++)
	{
		u8 *d1=data1, *d2=data2;
		for(k=0;k<ROOMARRAYSIZEZ;k++)
		{
			for(i=0;i<ROOMARRAYSIZEX;i++)
			{
				const BLOCK_TYPE v=getBlock(ba,i,j,k);
				*d1=isWall(v,getBlock(ba,i,j+1,k));
				*d2=isWall(v,getBlock(ba,i,j-1,k));

				if(*d1 && (v>>3)&1)*d1=2;
				if(*d2 && (v>>4)&1)*d2=2;

				if(*d1==1)cnt1++;
				else if(*d1==2)cnt12++;
				if(*d2==1)cnt2++;
				else if(*d2==2)cnt22++;
				if(*d1 && v&BLOCK_SLUDGE){*d1|=4;cnt3++;}

				d1++;d2++;
			}
		}

		//portalable
		while(cnt1>0)
		{
			vect2D p, s;
			getMaxRectangle(data1, 1, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data1, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s, 3);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x,j+1,p.y)),vectBlockToRectangle(vect(s.x,0,s.y)),true),&rl);
			cnt1-=s.x*s.y;
		}
		while(cnt2>0)
		{
			vect2D p, s;
			getMaxRectangle(data2, 1, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data2, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x+s.x,j,p.y)),vectBlockToRectangle(vect(-s.x,0,s.y)),true),&rl);
			cnt2-=s.x*s.y;
		}

		//unportalable
		while(cnt12>0)
		{
			vect2D p, s;
			getMaxRectangle(data1, 2, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data1, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s, 3);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x,j+1,p.y)),vectBlockToRectangle(vect(s.x,0,s.y)),false),&rl);
			cnt12-=s.x*s.y;
		}
		while(cnt22>0)
		{
			vect2D p, s;
			getMaxRectangle(data2, 2, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s);
			fillRectangle(data2, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x+s.x,j,p.y)),vectBlockToRectangle(vect(-s.x,0,s.y)),false),&rl);
			cnt22-=s.x*s.y;
		}

		//sludge
		if(sludgeList)
		{
			while(cnt3>0)
			{
				vect2D p, s;
				getMaxRectangle(data1, 4, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s);
				fillRectangle(data1, ROOMARRAYSIZEX, ROOMARRAYSIZEZ, &p, &s, 255);
				addRectangle(createRectangle(vectBlockToRectangle(vect(p.x,j+2,p.y)),vectBlockToRectangle(vect(s.x,0,s.y)),true),sludgeList);
				cnt3-=s.x*s.y;
			}
		}
	}

	cnt1=0; cnt2=0;
	cnt12=0; cnt22=0;
	for(k=0;k<ROOMARRAYSIZEZ;k++)
	{
		u8 *d1=data1, *d2=data2;
		for(j=0;j<ROOMARRAYSIZEY;j++)
		{
			for(i=0;i<ROOMARRAYSIZEX;i++)
			{
				const BLOCK_TYPE v=getBlock(ba,i,j,k);
				*d1=isWall(v,getBlock(ba,i,j,k+1));
				*d2=isWall(v,getBlock(ba,i,j,k-1));

				if(*d1 && (v>>5)&1)*d1=2;
				if(*d2 && (v>>6)&1)*d2=2;

				if(*d1==1)cnt1++;
				else if(*d1==2)cnt12++;
				if(*d2==1)cnt2++;
				else if(*d2==2)cnt22++;
				d1++;d2++;
			}
		}

		//portalable
		while(cnt1>0)
		{
			vect2D p, s;
			getMaxRectangle(data1, 1, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s);
			fillRectangle(data1, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x+s.x,p.y,k+1)),vectBlockToRectangle(vect(-s.x,s.y,0)),true),&rl);
			cnt1-=s.x*s.y;
		}
		while(cnt2>0)
		{
			vect2D p, s;
			getMaxRectangle(data2, 1, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s);
			fillRectangle(data2, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x,p.y,k)),vectBlockToRectangle(vect(s.x,s.y,0)),true),&rl);
			cnt2-=s.x*s.y;
		}

		//unportalable
		while(cnt12>0)
		{
			vect2D p, s;
			getMaxRectangle(data1, 2, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s);
			fillRectangle(data1, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x+s.x,p.y,k+1)),vectBlockToRectangle(vect(-s.x,s.y,0)),false),&rl);
			cnt12-=s.x*s.y;
		}
		while(cnt22>0)
		{
			vect2D p, s;
			getMaxRectangle(data2, 2, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s);
			fillRectangle(data2, ROOMARRAYSIZEX, ROOMARRAYSIZEY, &p, &s, 255);
			addRectangle(createRectangle(vectBlockToRectangle(vect(p.x,p.y,k)),vectBlockToRectangle(vect(s.x,s.y,0)),false),&rl);
			cnt22-=s.x*s.y;
		}
	}

	if(data1)free(data1);
	if(data2)free(data2);

	return rl;
}

vect3D getBlockPosition(u8 x, u8 y, u8 z)
{
	return vect((x-ROOMARRAYSIZEX/2)*BLOCKSIZEX,(y-ROOMARRAYSIZEY/2)*BLOCKSIZEY,(z-ROOMARRAYSIZEZ/2)*BLOCKSIZEZ);
}

vect3D getMinBlockArray(BLOCK_TYPE* ba)
{
	if(!ba)return vect(0,0,0);
	int i, j, k;
	int mx=ROOMARRAYSIZEX; int my=ROOMARRAYSIZEY; int mz=ROOMARRAYSIZEZ;
	for(i=0;i<mx;i++)
	{
		for(j=0;j<my;j++)
		{
			for(k=0;k<mz;k++)
			{
				if(!ba[i+j*ROOMARRAYSIZEX+k*ROOMARRAYSIZEX*ROOMARRAYSIZEY])
				{
					if(i<mx)mx=i;
					if(j<my)my=j;
					if(k<mz)mz=k;
				}
			}
		}
	}
	return vect(mx,my,mz);
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
		if(k>d || k<0){return false;}
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

blockFace_struct* collideLineBlockFaceListClosest(blockFace_struct* l, vect3D o, vect3D v, int32* d)
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
	if(d)*d=closestDist;
	return bf;
}

void drawBlockFace(blockFace_struct* bf, BLOCK_TYPE* ba)
{
	if(!bf || !bf->draw)return;
	
	u32* vtxPtr=packedVertex[bf->direction];
	
	BLOCK_TYPE v=getBlock(ba,bf->x,bf->y,bf->z);

	if(v&(1<<(bf->direction+1)))applyMTL(unportalableTexture);
	else{
		if(bf->direction==2)applyMTL(floorTexture);
		else applyMTL(wallTexture);
	}

	u8 textmult=1; //temp
	if(bf->direction==2)textmult=2;
	
	glPushMatrix();
	
		glTranslate3f32(inttof32(bf->x),inttof32(bf->y),inttof32(bf->z));
	
		GFX_COLOR=faceColors[bf->direction];

		if(bf->direction==2 && v&BLOCK_SLUDGE)GFX_COLOR=RGB15(2,15,2);
	
		GFX_BEGIN=GL_QUADS;
		
		GFX_TEX_COORD=TEXTURE_PACK(0*16, 0*16);
		GFX_VERTEX10=*vtxPtr++;
		GFX_TEX_COORD=TEXTURE_PACK(64*textmult*16, 0*16);
		GFX_VERTEX10=*vtxPtr++;
		GFX_TEX_COORD=TEXTURE_PACK(64*textmult*16, 64*textmult*16);
		GFX_VERTEX10=*vtxPtr++;
		GFX_TEX_COORD=TEXTURE_PACK(0*16, 64*textmult*16);
		GFX_VERTEX10=*vtxPtr++;
	
	glPopMatrix(1);
}

void drawBlockFaceList(blockFace_struct* l, BLOCK_TYPE* ba)
{
	if(!l)return;
		
	while(l)
	{
		drawBlockFace(l, ba);
		l=l->next;
	}
}

void editorRoomTransform(void)
{
	glScalef32((BLOCKSIZEX),(BLOCKSIZEY),(BLOCKSIZEZ));
	glTranslate3f32(inttof32(-ROOMARRAYSIZEX/2),inttof32(-ROOMARRAYSIZEY/2),inttof32(-ROOMARRAYSIZEZ/2));	
}

void drawEditorRoom(editorRoom_struct* er)
{
	if(!er)return;
		
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_ID(1));
	GFX_COLOR=RGB15(31,31,31);
	
	glPushMatrix();
		editorRoomTransform();
		drawBlockFaceList(er->blockFaceList, er->blockArray);
	glPopMatrix(1);
}
