#include "game/game_main.h"

#define DOORFRAMELENGTH (8)

wallDoor_struct entryWallDoor;
md2Model_struct wallDoorModel;

vect3D doorFrameData[]={{-TILESIZE*2,HEIGHTUNIT*8,0},{-TILESIZE*2,HEIGHTUNIT*5,0},{-TILESIZE,HEIGHTUNIT*8,0},{-TILESIZE,HEIGHTUNIT*7,0},{TILESIZE,HEIGHTUNIT*8,0},{TILESIZE,HEIGHTUNIT*7,0},{TILESIZE*2,HEIGHTUNIT*8,0},{TILESIZE*2,HEIGHTUNIT*5,0}};

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
	wd->position=convertVect(vect(position.x+r->position.x, position.y-4, position.z+r->position.y));
	wd->orientation=orientation;
	wd->frameMaterial=getMaterial(1);

	//TODO : take orientation into account
	rectangle_struct rec;
	rec.material=NULL;
	rec.position=addVect(position,vect(-1,-4,0));rec.size=vect(2,8,0);rec.normal=vect(0,0,inttof32(1));
	wd->rectangle=addRoomRectangle(r, rec, NULL, false);
	// if(wd->rectangle){wd->rectangle->hide=true;bb->surface=wd->rectangle;}
}

void drawWallDoor(wallDoor_struct* wd)
{
	if(!wd || !wd->used)return;

	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(30)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, wd->position, &params);

		glTranslate3f32(wd->position.x,wd->position.y,wd->position.z);
		
		if(wd->orientation)glRotateYi(8192);
		
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
}

void drawWallDoors(void)
{
	drawWallDoor(&entryWallDoor);
}
