#include <nds.h>
#include "game/game_main.h"

energyDevice_struct energyDevice[NUMENERGYDEVICES];

md2Model_struct energyCatcherModel, energyLauncherModel;

void initEnergyBalls(void)
{
	int i;
	for(i=0;i<NUMENERGYDEVICES;i++)
	{
		energyDevice[i].used=false;
	}
	
	loadMd2Model("models/ballcatcher.md2","balllauncher.pcx",&energyCatcherModel);
	loadMd2Model("models/ballcatcher.md2","balllauncher.pcx",&energyLauncherModel);
}

void initEnergyDevice(room_struct* r, energyDevice_struct* ed, vect3D pos, deviceOrientation_type or, bool type)
{
	if(!ed)return;
	
	initModelInstance(&ed->modelInstance, ed->type?(&energyLauncherModel):(&energyCatcherModel));
	ed->orientation=or;
	
	if(or==pY)pos.y=getHeightValue(r,pos,true);
	else if(or==mY)pos.y=getHeightValue(r,pos,false);
	{//for collisions
		rectangle_struct rec;
		rectangle_struct* recp;
		rec.material=NULL;
		
			if(or!=pY)
			{
				rec.position=addVect(pos,vect(-1,4,-1));rec.size=vect(2,0,2);rec.normal=vect(0,inttof32(1),0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
				if(recp)recp->hide=true;
			}
			if(or!=mY)
			{
				rec.position=addVect(pos,vect(1,-4,1));rec.size=vect(-2,0,-2);rec.normal=vect(0,inttof32(1),0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
				if(recp)recp->hide=true;
			}
			if(or!=mZ)
			{
				rec.position=addVect(pos,vect(-1,-4,-1));rec.size=vect(2,8,0);rec.normal=vect(0,0,-inttof32(1));recp=addRoomRectangle(r, NULL, rec, NULL, false);
				if(recp)recp->hide=true;
			}
			if(or!=pZ)
			{
				rec.position=addVect(pos,vect(-1,4,1));rec.size=vect(2,-8,0);rec.normal=vect(0,0,inttof32(1));recp=addRoomRectangle(r, NULL, rec, NULL, false);
				if(recp)recp->hide=true;
			}
			if(or!=pX)
			{
				rec.position=addVect(pos,vect(-1,-4,-1));rec.size=vect(0,8,2);rec.normal=vect(-inttof32(1),0,0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
				if(recp)recp->hide=true;
			}
			if(or!=mX)
			{
				rec.position=addVect(pos,vect(1,4,-1));rec.size=vect(0,-8,2);rec.normal=vect(inttof32(1),0,0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
				if(recp)recp->hide=true;
			}
	}
	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	ed->position=convertVect(pos);
	
	ed->type=type;
	ed->used=true;
}

energyDevice_struct* createEnergyDevice(room_struct* r, vect3D pos, deviceOrientation_type or, bool type)
{
	if(!r)r=getPlayer()->currentRoom;
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMENERGYDEVICES;i++)
	{
		if(!energyDevice[i].used)
		{
			initEnergyDevice(r, &energyDevice[i],pos,or,type);
			return &energyDevice[i];
		}
	}
	return NULL;
}

void drawEnergyDevice(energyDevice_struct* ed)
{
	if(!ed)return;
	
	glPushMatrix();
		glTranslate3f32(ed->position.x,ed->position.y,ed->position.z);
		
		switch(ed->orientation)
		{
			case mY:
				glRotateXi(16384);
				break;
			case pX:
				glRotateZi(-8192);
				break;
			case mX:
				glRotateZi(8192);
				break;
			case pZ:
				glRotateXi(-8192);
				break;
			case mZ:
				glRotateXi(8192);
				break;
			default:
				break;
		}
		
		renderModelFrameInterp(ed->modelInstance.currentFrame,ed->modelInstance.nextFrame,ed->modelInstance.interpCounter,ed->modelInstance.model,POLY_ALPHA(31)|POLY_CULL_NONE,false,ed->modelInstance.palette);
	glPopMatrix(1);
}

void drawEnergyDevices(void)
{
	int i;
	for(i=0;i<NUMENERGYDEVICES;i++)
	{
		if(energyDevice[i].used)drawEnergyDevice(&energyDevice[i]);
	}
}

void updateEnergyDevice(energyDevice_struct* ed)
{
	if(!ed)return;
	
	updateAnimation(&ed->modelInstance);
}

void updateEnergyDevices(void)
{
	int i;
	for(i=0;i<NUMENERGYDEVICES;i++)
	{
		if(energyDevice[i].used)updateEnergyDevice(&energyDevice[i]);
	}
}
