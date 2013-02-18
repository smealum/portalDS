#include "game/game_main.h"
#include "editor/io.h"

void drawRoomsGame(u8 mode, u16 color)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	drawRoom(&gameRoom,((1)<<3)|((gameRoom.lmSlot!=0)<<2)|(1)|(mode), color);
}

//READ AREA

extern char* basePath;

s16 entityTargetArray[NUMENTITIES];
activator_struct* entityActivatorArray[NUMENTITIES];

void readRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;
	
	readVect(&rec->position,f);
	readVect(&rec->size,f);
	readVect(&rec->lmSize,f);
	readVect(&rec->lmPos,f);
	readVect(&rec->normal,f);

	fread(&rec->portalable,sizeof(bool),1,f);
	
	u16 mid=0; fread(&mid,sizeof(u16),1,f);

	//TEMP ?
	if(rec->portalable)rec->material=getMaterial(1);
	else rec->material=getMaterial(2);
	
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
		addRoomRectangle(r, rec, rec.material, rec.portalable);
	}
}

//THIS SYSTEM DOES NOT WORK
//FIX THIS YOU IDIOT
void addEntityTarget(u8 k, void* target, activatorTarget_type type)
{
	if(!target)return;
	int i;
	for(i=0;i<k;i++)
	{
		if(entityTargetArray[i]==k && entityActivatorArray[i])
		{
			addActivatorTarget(entityActivatorArray[i], target, type);
			NOGBA("TARGET");
		}
	}
}

void readEntity(u8 i, FILE* f)
{
	if(!f)return;
	u8 type=0, dir=0; vect3D v;
	fread(&type, sizeof(u8), 1, f);
	readVect(&v, f);
	fread(&dir, sizeof(u8), 1, f);
	entityTargetArray[i]=-1;
	entityActivatorArray[i]=NULL;
	switch(type)
	{
		case 0:
			//energy ball catcher
			{
				vect3D p; readVect(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				energyDevice_struct* e=createEnergyDevice(NULL, p, dir, type);
				if(e)entityActivatorArray[i]=&e->activator;
				entityTargetArray[i]=target;
			}
			break;
		case 1:
			//energy ball launcher
			{
				vect3D p; readVect(&p,f);
				createEnergyDevice(NULL, p, dir, type);
			}
			break;
		case 3:
			//pressure button
			{
				vect3D p; readVect(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				bigButton_struct* e=createBigButton(NULL, p);
				if(e)entityActivatorArray[i]=&e->activator;
				entityTargetArray[i]=target;
			}
			break;
		case 4:
			//turret
			{
				vect3D p; readVect(&p,f);
				createTurret(NULL, p);
			}
			break;
		case 5: case 6:
			//cubes
			{
				vect3D p; readVect(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
			}
			break;
		case 7:
			//dispenser
			{
				vect3D p; readVect(&p,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				cubeDispenser_struct* e=createCubeDispenser(NULL, p, true);
				if(e)addEntityTarget(i, (void*)e, DISPENSER_TARGET);
			}
			break;
		case 8:
			//emancipation grid
			{
				vect3D p; readVect(&p,f);
				createEmancipationGrid(NULL, p, (dir%2)?(TILESIZE*2):(TILESIZE*2), !(dir<=1)); //TEMP
			}
			break;
		case 9:
			//platform
			{
				vect3D p1, p2;
				readVect(&p1,f);
				readVect(&p2,f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				platform_struct* e=createPlatform(NULL, p1, p2, true);
				if(e)addEntityTarget(i, (void*)e, PLATFORM_TARGET);
			}
			return;
		case 10:
			//door
			{
				vect3D p; readVect(&p,f);
				bool orientation; fread(&orientation, sizeof(bool), 1, f);
				door_struct* e=createDoor(NULL, p, orientation);
				NOGBA("DOOR");
				if(e)addEntityTarget(i, (void*)e, DOOR_TARGET);
			}
			break;
		case 11:
			//light
			{
				vect3D p; readVect(&p,f);
				createLight(p, TILESIZE*2*16);
			}
			break;
		case 12:
			//platform target
			{
				s16 target=-1;
				fread(&target, sizeof(s16), 1, f);
			}
			return;
		default:
			break;
	}
}

void readEntities(FILE* f)
{
	if(!f)return;

	u16 cnt; fread(&cnt,sizeof(u16),1,f);
	int i; for(i=0;i<cnt;i++)readEntity(i,f);
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

	mapHeader_struct h;
	readHeader(&h, f);

	//room data
	initRoom(r, 64*2, 64*2, vect(-32*2,-32*2,0));

	fseek(f, h.rectanglesPosition, SEEK_SET);
		fread(&r->rectangles.num,sizeof(int),1,f);
		readRectangles(r, f);

	//lightmap stuff
	fseek(f, h.lightPosition, SEEK_SET);
		readVect(&r->lmSize,f);
		NOGBA("%dx%d lightmap",r->lmSize.x,r->lmSize.y);
		r->lightMapBuffer=malloc(sizeof(u8)*r->lmSize.x*r->lmSize.y);
		fread(r->lightMapBuffer,sizeof(u8),r->lmSize.x*r->lmSize.y,f);
		{
			int i;
			u16 palette[8];
			for(i=0;i<8;i++){u8 v=(i*31)/7;palette[i]=RGB15(v,v,v);}
			r->lightMap=createReservedTextureBufferA5I3(NULL,palette,r->lmSize.x,r->lmSize.y,(void*)(0x6800000+0x0020000));
		}

	//entities
	fseek(f, h.entityPosition, SEEK_SET);
		readEntities(f);
	
	fclose(f);
}
