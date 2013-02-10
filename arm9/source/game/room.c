#include "game/game_main.h"

void drawRoomsGame(u8 mode, u16 color)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	drawRoom(&gameRoom,((1)<<3)|((gameRoom.lmSlot!=0)<<2)|(1)|(mode), color);
}

//READ AREA

extern char* basePath;

void readRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;
	
	readVect(&rec->position,f);
	readVect(&rec->size,f);
	readVect(&rec->lmSize,f);
	readVect(&rec->lmPos,f);
	readVect(&rec->normal,f);
	
	u16 mid=0;
	fread(&mid,sizeof(u16),1,f);
	rec->material=getMaterial(mid);
	
	fread(&rec->rot,sizeof(bool),1,f);
}

void readRectangles(room_struct* r, FILE* f)
{
	if(!r || !f)return;
	int i;
	int k=r->rectangles.num;
	r->rectangles.num=0;
	for(i=0;i<k;i++)
	{
		rectangle_struct rec;
		readRectangle(&rec, f);
		addRoomRectangle(r, rec, rec.material, true);
	}
}

void convertBackMaterialArray(room_struct* r, u16* mat)
{
	int i, j;
	for(i=0;i<r->width;i++)
	{
		for(j=0;j<r->height;j++)
		{
			r->materials[i+j*r->width]=getMaterial(mat[i+j*r->width]);
		}
	}
}

void readRoom(FILE* f, room_struct* r)
{
	if(!f)return;
	if(!r)r=&gameRoom;

	fseek(f, sizeof(s16), SEEK_CUR);
	
	readVect(&r->position,f);
	fread(&r->width, sizeof(int32), 1, f);
	fread(&r->height, sizeof(int32), 1, f);
	fseek(f, sizeof(int32), SEEK_CUR);

	initRoom(r, r->width, r->height, r->position);

	readVect(&r->lmSize,f);
		
	fread(&r->rectangles.num, sizeof(int), 1, f);
	fseek(f, sizeof(u16), SEEK_CUR);
	
	u16* mat=malloc(sizeof(u16)*r->width*r->height);
	
	fseek(f,sizeof(u8)*r->width*r->height, SEEK_CUR);
	fseek(f,sizeof(u8)*r->width*r->height, SEEK_CUR);

	fread(mat,sizeof(u16),r->width*r->height,f);
	
	convertBackMaterialArray(r,mat);
	
	free(mat);
	
	r->lightMapBuffer=malloc(sizeof(u8)*r->lmSize.x*r->lmSize.y);
	fread(r->lightMapBuffer,sizeof(u8),r->lmSize.x*r->lmSize.y,f);

	{
		int i;
		u16 palette[8];
		for(i=0;i<8;i++){u8 v=(i*31)/7;palette[i]=RGB15(v,v,v);}
		r->lightMap=createReservedTextureBufferA5I3(NULL,palette,r->lmSize.x,r->lmSize.y,(void*)(0x6800000+0x0020000));
	} //TEMP
	
	readRectangles(r, f);
}

void readMap(char* filename, room_struct* r)
{
	if(!r)r=&gameRoom;
	char fn[1024];
	FILE* f=NULL;
	if(fsMode==1||fsMode==2)
	{
		if(fsMode==2)
		{
			sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
			NOGBA("lala : %s",fn);
			f=fopen(fn,"rb");
		}
		if(!f)
		{
			sprintf(fn,"nitro:/fpsm/maps/%s",filename);
			NOGBA("lala : %s",fn);
			f=fopen(fn,"rb");
		}
	}else if(fsMode==3)
	{
		sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
		NOGBA("lala : %s",fn);
		f=fopen(fn,"rb");
	}
	if(!f)return;

	fseek(f,sizeof(int),SEEK_CUR);

	readRoom(f, r);
	
	fclose(f);
}

void newReadMap(char* filename, room_struct* r)
{
	if(!r)r=&gameRoom;
	char fn[1024];
	FILE* f=NULL;
	if(fsMode==1||fsMode==2)
	{
		if(fsMode==2)
		{
			sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
			NOGBA("lala : %s",fn);
			f=fopen(fn,"rb");
		}
		if(!f)
		{
			sprintf(fn,"nitro:/fpsm/maps/%s",filename);
			NOGBA("lala : %s",fn);
			f=fopen(fn,"rb");
		}
	}else if(fsMode==3)
	{
		sprintf(fn,"%sfpsm/maps/%s",basePath,filename);
		NOGBA("lala : %s",fn);
		f=fopen(fn,"rb");
	}
	if(!f)return;

	initRoom(r, 64, 64, vect(-32,-32,0));
	fread(&r->rectangles.num,sizeof(int),1,f);
	NOGBA("%d rectangles", r->rectangles.num);

	readRectangles(r, f);

	readVect(&r->lmSize,f);
	NOGBA("%dx%d lightmap",r->lmSize.x,r->lmSize.y);
	r->lightMapBuffer=malloc(sizeof(u8)*r->lmSize.x*r->lmSize.y);
	fread(r->lightMapBuffer,sizeof(u8),r->lmSize.x*r->lmSize.y,f);

	{
		int i;
		u16 palette[8];
		for(i=0;i<8;i++){u8 v=(i*31)/7;palette[i]=RGB15(v,v,v);}
		r->lightMap=createReservedTextureBufferA5I3(NULL,palette,r->lmSize.x,r->lmSize.y,(void*)(0x6800000+0x0020000));
	} //TEMP
	
	fclose(f);
}
