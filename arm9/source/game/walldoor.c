#include "game/game_main.h"

#define DOORFRAMELENGTH (8)

wallDoor_struct entryWallDoor;
md2Model_struct wallDoorModel;

vect3D doorFrameData[]={{-TILESIZE*2,HEIGHTUNIT*8,0},{-TILESIZE*2,HEIGHTUNIT*5,0},{-TILESIZE,HEIGHTUNIT*8,0},{-TILESIZE,HEIGHTUNIT*7,0},{TILESIZE,HEIGHTUNIT*8,0},{TILESIZE,HEIGHTUNIT*7,0},{TILESIZE*2,HEIGHTUNIT*8,0},{TILESIZE*2,HEIGHTUNIT*5,0}};

vect3D wallDoorV1[]={{0,0,1},
					{0,0,-1},
					{0,0,0},
					{0,0,0},
					{1,0,0},
					{-1,0,0}};

vect3D wallDoorV2[]={{-1,0,0},
					{1,0,0},
					{0,0,0},
					{0,0,0},
					{0,0,-1},
					{0,0,1}};

void initWallDoor(wallDoor_struct* wd)
{
	if(!wd)return;

	wd->used=false;
	wd->rectangle=NULL;
	
	initModelInstance(&wd->modelInstance, &wallDoorModel);
}

void initWallDoors(void)
{
	initWallDoor(&entryWallDoor);

	//TEMP ?
	loadMd2Model("models/door.md2", "door.pcx", &wallDoorModel);
	generateModelDisplayLists(&wallDoorModel, false, true);
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

	rectangle_struct rec;
	rectangle_struct* recp;
	//door wall
	rec.material=NULL;
	rec.position=addVect(position,vectDifference(vect(0,-4,0),wallDoorV1[orientation]));rec.size=addVect(vect(0,8,0),vectMultInt(wallDoorV1[orientation],2));rec.normal=vectMultInt(wallDoorV2[orientation],inttof32(1));
	wd->rectangle=addRoomRectangle(r, rec, NULL, false);
	if(wd->rectangle){wd->rectangle->hide=true;}

	//room walls
	rec.position=addVect(position,vectDifference(vect(0,-4,0),vectMultInt(wallDoorV1[orientation],3)));rec.size=addVect(vectMultInt(wallDoorV1[orientation],6),vectMultInt(wallDoorV2[orientation],6));rec.normal=vect(0,inttof32(1),0);
	recp=addRoomRectangle(r, rec, NULL, false);
	if(recp){recp->hide=true;}

	//elevator
	initElevator(&wd->elevator, r, addVect(position,vectMultInt(wallDoorV2[wd->orientation],5)), orientation, true);
	setElevatorArriving(&wd->elevator,4096);
}

bool pointInWallDoorRoom(wallDoor_struct* wd, vect3D p)
{
	if(!wd)return false;
	int32 v1=dotProduct(vectDifference(p,wd->gridPosition),vectMultInt(wallDoorV1[wd->orientation],inttof32(1)));
	int32 v2=dotProduct(vectDifference(p,wd->gridPosition),vectMultInt(wallDoorV2[wd->orientation],inttof32(1)));
	return (v1<=3)&&(wd->gridPosition.y-12<=p.y)&&(v2<=6)
		&& (v1>=-3)&&(wd->gridPosition.y+12>=p.y)&&(v2>=0);
}

void updateWallDoor(wallDoor_struct* wd)
{
	if(!wd)return;

	bool pin=pointInWallDoorRoom(wd,getPlayer()->relativePosition);

	if(pin)
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
}

void drawWallDoor(wallDoor_struct* wd)
{
	if(!wd || !wd->used)return;

	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(30)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, wd->position, &params);

		glTranslate3f32(wd->position.x,wd->position.y,wd->position.z);
		
		if(wd->orientation<=1)glRotateYi(8192);
		
		renderModelFrameInterp(wd->modelInstance.currentFrame,wd->modelInstance.nextFrame,wd->modelInstance.interpCounter,wd->modelInstance.model,params,false,wd->modelInstance.palette,RGB15(31,31,31));
	
		glPolyFmt(POLY_ALPHA(31)|POLY_CULL_NONE|POLY_ID(31));
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

	glPopMatrix(1);

	drawElevator(&wd->elevator);
}

void drawWallDoors(void)
{
	drawWallDoor(&entryWallDoor);
}
