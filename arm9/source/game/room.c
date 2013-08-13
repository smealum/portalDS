#include "game/game_main.h"
#include "editor/io.h"

void drawRoomsGame(u8 mode, u16 color)
{
	int i;
	unbindMtl();
	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	drawRoom(&gameRoom,((1)<<3)|(1<<2)|(1)|(mode), color); //TEMP
}

void roomOriginSize(room_struct* r, vect3D* o, vect3D* s)
{
	if(!r || (!o && !s))return;

	vect3D m=vect(8192,8192,8192); vect3D M=vect(0,0,0);

	listCell_struct *lc=r->rectangles.first;

	while(lc)
	{
		m=minVect(lc->data.position,m);
		m=minVect(addVect(lc->data.position,lc->data.size),m);
		M=maxVect(lc->data.position,M);
		M=maxVect(addVect(lc->data.position,lc->data.size),M);
		NOGBA("%d",lc->data.position.y);
		lc=lc->next;
	}

	if(o)*o=m;
	if(s)*s=vectDifference(M,m);
}

void roomResetOrigin(room_struct* r)
{
	if(!r)return;

	vect3D o;

	roomOriginSize(r, &o, NULL);
	r->position=vect(0,0,0);

	listCell_struct *lc=r->rectangles.first;

	while(lc)
	{
		lc->data.position=vectDifference(lc->data.position,o);
		lc=lc->next;
	}
}

vect3D orientVector(vect3D v, u8 k)
{
	vect3D u;

	switch(k)
	{
		case 0:
			u.x=-v.z;
			u.y=v.y;
			u.z=v.x;
			break;
		case 1:
			u.x=v.z;
			u.y=v.y;
			u.z=-v.x;
			break;
		case 4:
			u.x=v.x;
			u.y=v.y;
			u.z=-v.z;
			break;
		default:
			u=v;
			break;
	}

	return u;
}

void invertRectangle(rectangle_struct* rec)
{
	if(!rec)return;

	if(rec->size.x)
	{
		rec->position.x+=rec->size.x;
		rec->size.x=-rec->size.x;
	}else{
		rec->position.z+=rec->size.z;
		rec->size.z=-rec->size.z;
	}
}

void insertRoom(room_struct* r1, room_struct* r2, vect3D v, u8 orientation)
{
	if(!r1 || !r2)return;

	listCell_struct *lc=r2->rectangles.first;

	vect3D o=vect(0,0,0), s=vect(0,0,0);
	roomOriginSize(r2,&o,&s);

	switch(orientation) //TODO : do pre-rotation ?
	{
		case 0:
			v.z-=s.x/2; //not a mistake
			break;
		case 1:
			v.z+=s.x/2; //not a mistake
			break;
		case 4:	case 5:
			v.x-=s.x/2;
			break;
	}
	v.y-=4;

	while(lc)
	{
		rectangle_struct rec=lc->data;
		rec.position=vectDifference(rec.position,o);

		//rotate
		rec.position=orientVector(rec.position,orientation);
		rec.size=orientVector(rec.size,orientation);
		if(!(orientation%2) || orientation==1)invertRectangle(&rec);

		rec.position=addVect(rec.position,v);
		rectangle_struct* recp=addRoomRectangle(r1, rec, rec.material, rec.portalable);	
		if(recp)
		{
			recp->hide=true; //TEMP ?
			recp->collides=!lc->data.portalable;
			recp->lightData.vertex=lc->data.lightData.vertex;
		}
		lc=lc->next;
	}
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

void readSludgeRectangles(FILE* f)
{
	if(!f)return;
	int i, k;

	fread(&k,sizeof(int),1,f);
	for(i=0;i<k;i++)
	{
		rectangle_struct rec;
		readRectangle(&rec, f);
		addSludgeRectangle(&rec);
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
		case 2:
			//timed button
			{
				vect3D p; readVect(&p,f);
				u8 d; fread(&d, sizeof(u8), 1, f);
				s16 target=-1; fread(&target, sizeof(s16), 1, f);
				timedButton_struct* e=createTimedButton(NULL, p, (d+2)*8192);
				if(e)entityActivatorArray[i]=&e->activator;
				entityTargetArray[i]=target;
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
				u8 d; fread(&d, sizeof(u8), 1, f);
				createTurret(NULL, p, d);
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
				u8 orientation; fread(&orientation, sizeof(u8), 1, f);
				door_struct* e=createDoor(NULL, p, orientation%2);
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
				if(entryWallDoor.used)
				{
					getPlayer()->object->position=addVect(entryWallDoor.elevator.realPosition,vect(0,PLAYERRADIUS*5,0));
					switch(o)
					{
						case 0:
							rotateCamera(NULL, vect(0,-8192,0));
							break;
						case 1:
							rotateCamera(NULL, vect(0,8192,0));
							break;
						case 4:
							rotateCamera(NULL, vect(0,8192*2,0));
							break;
					}
				}
				entityEntityArray[i]=(void*)&entryWallDoor;
				entityTargetTypeArray[i]=WALLDOOR_TARGET;
			}
			return;
		case 14:
			//wall door (exit)
			{
				vect3D p; readVect(&p,f);
				u8 o; fread(&o,sizeof(u8),1,f);
				setupWallDoor(NULL, &exitWallDoor, p, o);
				entityEntityArray[i]=(void*)&exitWallDoor;
				entityTargetTypeArray[i]=WALLDOOR_TARGET;
				exitWallDoor.override=true;
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

void newReadMap(char* filename, room_struct* r, u8 flags)
{
	if(!r)r=&gameRoom;
	char fn[1024];
	FILE* f=NULL;

	//DEBUG DEBUG DEBUG
	if(flags&(1<<7))f=fopen("fat:/test.map","rb");

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
	if(flags&(1))
	{
		fseek(f, h.lightPosition, SEEK_SET);
			readLightingData(r, &r->lightingData, f);
	}

	//entities
	if(flags&(1<<1))
	{
		fseek(f, h.entityPosition, SEEK_SET);
			readEntities(f);
	}

	//sludge stuff
	fseek(f, h.sludgePosition, SEEK_SET);
		readSludgeRectangles(f);
	
	fclose(f);
}
