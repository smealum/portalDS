#include "game/game_main.h"
#include "editor/io.h"

void drawRoomsGame(u8 mode, u16 color)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	drawRoom(&gameRoom,((1)<<3)|(1<<2)|(1)|(mode), color);
}

//READ AREA

extern char* basePath;

s16 entityTargetArray[NUMENTITIES];
activator_struct* entityActivatorArray[NUMENTITIES];
void* entityEntityArray[NUMENTITIES];
activatorTarget_type entityTargetTypeArray[NUMENTITIES];

void readRectangle(rectangle_struct* rec, FILE* f)
{
	if(!rec || !f)return;
	
	readVect(&rec->position,f);
	readVect(&rec->size,f);
	readVect(&rec->normal,f);

	fread(&rec->portalable,sizeof(bool),1,f);
	
	u16 mid=0; fread(&mid,sizeof(u16),1,f);

	rec->lightData.lightMap=NULL;

	//TEMP ?
	if(rec->portalable)rec->material=getMaterial(1);
	else rec->material=getMaterial(2);
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

void addEntityTarget(u8 k, u8 n, void* target, activatorTarget_type type)
{
	if(!target)return;
	int i;
	for(i=0;i<n;i++)
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
	entityEntityArray[i]=NULL;
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
				entityEntityArray[i]=(void*)e;
				entityTargetTypeArray[i]=DISPENSER_TARGET;
			}
			break;
		case 8:
			//emancipation grid
			{
				int32 l; fread(&l,sizeof(int32),1,f);
				vect3D p; readVect(&p,f);
				createEmancipationGrid(NULL, p, (dir%2)?(-l):(l), !(dir<=1)); //TEMP ?
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
				entityEntityArray[i]=(void*)e;
				entityTargetTypeArray[i]=PLATFORM_TARGET;
			}
			return;
		case 10:
			//door
			{
				vect3D p; readVect(&p,f);
				bool orientation; fread(&orientation, sizeof(bool), 1, f);
				door_struct* e=createDoor(NULL, p, orientation);
				entityEntityArray[i]=(void*)e;
				entityTargetTypeArray[i]=DOOR_TARGET;
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
		case 13:
			//wall door (start)
			{
				vect3D p; readVect(&p,f);
				u8 o; fread(&o,sizeof(u8),1,f);
				setupWallDoor(NULL, &entryWallDoor, p, o);
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
	for(i=0;i<cnt;i++)addEntityTarget(i,cnt,entityEntityArray[i],entityTargetTypeArray[i]);
}

void readVertexLightingData(vertexLightingData_struct* vld, FILE* f)
{
	if(!vld || !f)return;

	fread(&vld->width, sizeof(u8), 1, f);
	fread(&vld->height, sizeof(u8), 1, f);

	vld->values=malloc(sizeof(u8)*vld->width*vld->height);
	fread(vld->values, sizeof(u8), vld->width*vld->height, f);
}

void readLightingData(room_struct* r, lightingData_struct* ld, FILE* f)
{
	if(!r || !ld || !f)return;

	ld->type=VERTEXLIGHT_DATA; //TEMP

	switch(ld->type)
	{
		case LIGHTMAP_DATA:
			initLightDataLM(ld, r->rectangles.num);

			readVect(&ld->data.lightMap.lmSize,f);

			ld->data.lightMap.buffer=malloc(sizeof(u8)*ld->data.lightMap.lmSize.x*ld->data.lightMap.lmSize.y);
			if(!ld->data.lightMap.buffer)return;

			fread(ld->data.lightMap.buffer,sizeof(u8),ld->data.lightMap.lmSize.x*ld->data.lightMap.lmSize.y,f);
			fread(ld->data.lightMap.coords, sizeof(lightMapCoordinates_struct), ld->size, f);

			{
				int i;
				u16 palette[8];
				for(i=0;i<8;i++){u8 v=(i*31)/7;palette[i]=RGB15(v,v,v);}
				// for(i=0;i<ld->data.lightMap.lmSize.x*ld->data.lightMap.lmSize.y;i++){ld->data.lightMap.buffer[i]=(ld->data.lightMap.buffer[i]<<3);}
				ld->data.lightMap.texture=createReservedTextureBufferA5I3(ld->data.lightMap.buffer,palette,ld->data.lightMap.lmSize.x,ld->data.lightMap.lmSize.y,(void*)(0x6800000+0x0020000));
			}

			int i=r->rectangles.num-1;
			listCell_struct* lc=r->rectangles.first;
			while(lc)
			{
				lc->data.lightData.lightMap=&ld->data.lightMap.coords[i--]; //stacking reverses the order...
				lc=lc->next;
			}

			break;
		default:
			initLightDataVL(ld, r->rectangles.num);
			{
				int i;
				for(i=0;i<ld->size;i++)readVertexLightingData(&ld->data.vertexLighting[i],f);
			}
			{
				int i=r->rectangles.num-1;
				listCell_struct* lc=r->rectangles.first;
				while(lc)
				{
					lc->data.lightData.vertex=&ld->data.vertexLighting[i--]; //stacking reverses the order...
					lc=lc->next;
				}
			}
			break;
	}
}

void newReadMap(char* filename, room_struct* r)
{
	if(!r)r=&gameRoom;
	char fn[1024];
	FILE* f=NULL;

	//DEBUG DEBUG DEBUG
	f=fopen("fat:/test.map","rb");

	if(!f)
	{
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
	}
	if(!f)return;

	mapHeader_struct h;
	readHeader(&h, f);

	//room data
	initRoom(r, 64*2, 64*2, vect(-32*2,-32*2,0));

	fseek(f, h.rectanglesPosition, SEEK_SET);
		fread(&r->rectangles.num,sizeof(int),1,f);
		readRectangles(r, f);

	//lighting stuff
	fseek(f, h.lightPosition, SEEK_SET);
		readLightingData(r, &r->lightingData, f);

	//entities
	fseek(f, h.entityPosition, SEEK_SET);
		readEntities(f);
	
	fclose(f);
}
