#include "game/game_main.h"

#define DOORFRAMELENGTH (8)
#define WALLDOORINTERVAL (32)


wallDoor_struct entryWallDoor;
wallDoor_struct exitWallDoor;

static md2Model_struct wallDoorModel;
static room_struct elevatorRoom;


static vect3D doorFrameData[]={{-TILESIZE*2,HEIGHTUNIT*8,0},{-TILESIZE*2,HEIGHTUNIT*5,0},{-TILESIZE,HEIGHTUNIT*8,0},{-TILESIZE,HEIGHTUNIT*7,0},{TILESIZE,HEIGHTUNIT*8,0},{TILESIZE,HEIGHTUNIT*7,0},{TILESIZE*2,HEIGHTUNIT*8,0},{TILESIZE*2,HEIGHTUNIT*5,0}};

static vect3D wallDoorV1[]={{0,0,1},
					{0,0,-1},
					{0,0,0},
					{0,0,0},
					{1,0,0},
					{-1,0,0}};

static vect3D wallDoorV2[]={{-1,0,0},
					{1,0,0},
					{0,0,0},
					{0,0,0},
					{0,0,-1},
					{0,0,1}};

static vect3D elevatorRoomSize;

void initWallDoor(wallDoor_struct* wd)
{
	if(!wd)return;

	wd->used=false;
	wd->rectangle=NULL;
	wd->override=false;

	initModelInstance(&wd->modelInstance, &wallDoorModel);
}

void initWallDoors(void)
{
	initWallDoor(&entryWallDoor);
	initWallDoor(&exitWallDoor);

	//TEMP ?
	loadMd2Model("models/door.md2", "door.pcx", &wallDoorModel);
	generateModelDisplayLists(&wallDoorModel, false, true);

	newReadMap("maps/elevatorroom.map", &elevatorRoom, 1);
	roomResetOrigin(&elevatorRoom);
	roomOriginSize(&elevatorRoom, NULL, &elevatorRoomSize);
	elevatorRoom.displayList=generateRoomDisplayList(&elevatorRoom, vect(0,0,0), vect(0,0,0), false);
}

void freeWallDoors(void)
{
	freeMd2Model(&wallDoorModel);
	freeRoom(&elevatorRoom);
}

void setupWallDoor(room_struct* r, wallDoor_struct* wd, vect3D position, u8 orientation)
{
	if(!wd || wd->used)return;
	if(!r)r=&gameRoom;

	wd->used=true;
	wd->gridPosition=vect(position.x, position.y-4, position.z);
	wd->position=convertVect(vect(position.x+r->position.x, position.y-4, position.z+r->position.y));
	wd->orientation=orientation;
	wd->frameMaterial=getMaterial(1);

	NOGBA("ORIENTATION %d",orientation);

	rectangle_struct rec;

	//door wall
	rec.material=NULL;
	rec.position=addVect(position,vectDifference(vect(0,-4,0),wallDoorV1[orientation]));rec.size=addVect(vect(0,8,0),vectMultInt(wallDoorV1[orientation],2));rec.normal=vectMultInt(wallDoorV2[orientation],inttof32(1));
	wd->rectangle=addRoomRectangle(r, rec, NULL, false);
	if(wd->rectangle){wd->rectangle->hide=true;}

	//elevator
	initElevator(&wd->elevator, r, addVect(position,vectMultInt(wallDoorV2[wd->orientation],7)), orientation, true);
	setElevatorArriving(&wd->elevator,2048);

	//elevator room
	insertRoom(r,&elevatorRoom,position,orientation);
}

bool pointInWallDoorRoom(wallDoor_struct* wd, vect3D p)
{
	if(!wd)return false;
	int32 v1=dotProduct(vectDifference(p,wd->gridPosition),vectMultInt(wallDoorV1[wd->orientation],inttof32(1)));
	int32 v2=dotProduct(vectDifference(p,wd->gridPosition),vectMultInt(wallDoorV2[wd->orientation],inttof32(1)));
	return (v1<=5)&&(wd->gridPosition.y-2<=p.y)&&(v2<=8)
		&& (v1>=-5)&&(wd->gridPosition.y+22>=p.y)&&(v2>0);
}

void updateWallDoor(wallDoor_struct* wd)
{
	if(!wd)return;

	bool pin=pointInWallDoorRoom(wd,getPlayer()->relativePosition);

	if(pin || wd->override)
	{
		if(wd->modelInstance.currentAnim==0)
		{
			changeAnimation(&wd->modelInstance, 2, false);
			changeAnimation(&wd->modelInstance, 1, true);
		}
		updateElevator(&wd->elevator);
	}else{
		if(wd->modelInstance.currentAnim==2)
		{
			changeAnimation(&wd->modelInstance, 0, false);
			changeAnimation(&wd->modelInstance, 3, true);
		}
	}

	if(wd->rectangle)
	{
		if(wd->modelInstance.currentAnim==2)wd->rectangle->collides=false;
		else wd->rectangle->collides=true;
	}

	updateAnimation(&wd->modelInstance);
}

void updateWallDoors(void)
{
	updateWallDoor(&entryWallDoor);
	updateWallDoor(&exitWallDoor);

	if(exitWallDoor.elevator.state==ELEVATOR_LEAVING)endGame();
}

void drawWallDoor(wallDoor_struct* wd, portal_struct* p)
{
	if(!wd || !wd->used)return;

	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(30)|POLY_TOON_HIGHLIGHT|POLY_FOG;
		setupObjectLighting(NULL, wd->position, &params);

		glTranslate3f32(wd->position.x,wd->position.y,wd->position.z);

		glPushMatrix();
			if(wd->orientation<=1)glRotateYi(8192);

			renderModelFrameInterp(wd->modelInstance.currentFrame,wd->modelInstance.nextFrame,wd->modelInstance.interpCounter,wd->modelInstance.model,params,false,wd->modelInstance.palette,RGB15(31,31,31));

			glPolyFmt(POLY_ALPHA(31)|POLY_CULL_BACK|POLY_ID(31)|POLY_FOG);
			GFX_COLOR=RGB15(31,31,31);
			vect3D v[4];
			bindMaterial(wd->frameMaterial, wd->rectangle, NULL, v, false);
			GFX_BEGIN=GL_QUAD_STRIP;

				int i;
				for(i=0;i<DOORFRAMELENGTH;i++)
				{
					int32 tx=v[0].x+((doorFrameData[i].x-doorFrameData[0].x)*(v[2].x-v[0].x))/(TILESIZE*4);
					int32 ty=v[0].y+((doorFrameData[i].y-doorFrameData[0].y)*(v[2].y-v[0].y))/(HEIGHTUNIT*8);
					GFX_TEX_COORD=TEXTURE_PACK((tx), (ty));
					glVertex3v16(doorFrameData[i].x,doorFrameData[i].y,doorFrameData[i].z);
				}

			glPolyFmt(POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(31)|POLY_FOG);
			GFX_BEGIN=GL_QUAD_STRIP;

				for(i=0;i<DOORFRAMELENGTH;i++)
				{
					int32 tx=v[0].x+((doorFrameData[i].x-doorFrameData[0].x)*(v[2].x-v[0].x))/(TILESIZE*4);
					int32 ty=v[0].y+((doorFrameData[i].y-doorFrameData[0].y)*(v[2].y-v[0].y))/(HEIGHTUNIT*8);
					GFX_TEX_COORD=TEXTURE_PACK((tx), (ty));
					glVertex3v16(doorFrameData[i].x,doorFrameData[i].y,doorFrameData[i].z-WALLDOORINTERVAL);
				}
		glPopMatrix(1);

		if(wd->modelInstance.currentAnim==0)
		{
			glPopMatrix(1);
			return;
		}

		if(!p || (dotProduct(vectDifference(p->position,wd->position), p->normal)>0))
		{
			//TEMP TEST
			glPushMatrix();
				switch(wd->orientation)
				{
					case 4:
						glRotateYi(8192*2);
						break;
					case 1:
						glRotateYi(8192);
						break;
					case 0:
						glRotateYi(-8192);
						break;
				}
				glTranslate3f32(-elevatorRoomSize.x*TILESIZE+TILESIZE,0,TILESIZE+WALLDOORINTERVAL); //WALLDOORINTERVAL is arbitrary, just to move it away from the wall
				drawRoom(&elevatorRoom,((1)<<3)|(1<<2)|(1), getCurrentPortalColor(getPlayer()->object->position));
			glPopMatrix(1);
		}

	glPopMatrix(1);

	drawElevator(&wd->elevator);
}

void drawWallDoors(portal_struct* p)
{
	drawWallDoor(&entryWallDoor, p);
	drawWallDoor(&exitWallDoor, p);
}
