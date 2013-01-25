#include "game/game_main.h"

#define ENERGYBALLSIZE (128)

energyDevice_struct energyDevice[NUMENERGYDEVICES];
energyBall_struct energyBall[NUMENERGYBALLS];

md2Model_struct energyCatcherModel, energyLauncherModel, energyBallModel;

const vect3D energyDeviceDirection[6]={(vect3D){inttof32(1),0,0},(vect3D){-inttof32(1),0,0},(vect3D){0,inttof32(1),0},(vect3D){0,-inttof32(1),0},(vect3D){0,0,inttof32(1)},(vect3D){0,0,-inttof32(1)}};

void initEnergyBalls(void)
{
	int i;
	for(i=0;i<NUMENERGYDEVICES;i++)
	{
		energyDevice[i].used=false;
		energyDevice[i].id=i;
	}
	for(i=0;i<NUMENERGYBALLS;i++)
	{
		energyBall[i].used=false;
		energyBall[i].id=i;
	}
	
	loadMd2Model("models/ballcatcher.md2","balllauncher.pcx",&energyCatcherModel);
	loadMd2Model("models/ballcatcher.md2","balllauncher.pcx",&energyLauncherModel);
	loadMd2Model("models/energyball.md2","energyball.pcx",&energyBallModel);
	generateModelDisplayLists(&energyBallModel);
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
		u32 params=POLY_ALPHA(31)|POLY_CULL_NONE|POLY_ID(30+ed->id)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, ed->position, &params);
		
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
		
		renderModelFrameInterp(ed->modelInstance.currentFrame,ed->modelInstance.nextFrame,ed->modelInstance.interpCounter,ed->modelInstance.model,params,false,ed->modelInstance.palette);
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
	
	if(ed->type) //TEMP
	{
		createEnergyBall(addVect(ed->position,vectDivInt(energyDeviceDirection[ed->orientation],8)),energyDeviceDirection[ed->orientation]);
		ed->type=false;
	}
	
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

void initEnergyBall(energyBall_struct* eb, vect3D pos, vect3D dir)
{
	if(!eb)return;
	
	initModelInstance(&eb->modelInstance, &energyBallModel);
	
	eb->position=pos;
	eb->direction=dir;
	eb->speed=128;
	
	eb->used=true;
}

energyBall_struct* createEnergyBall(vect3D pos, vect3D dir)
{
	int i;
	for(i=0;i<NUMENERGYBALLS;i++)
	{
		if(!energyBall[i].used)
		{
			initEnergyBall(&energyBall[i],pos,dir);
			return &energyBall[i];
		}
	}
	return NULL;
}

void drawEnergyBall(energyBall_struct* eb)
{
	if(!eb)return;
	
	glPushMatrix();
		u32 angle=(eb->modelInstance.currentFrame*4+eb->modelInstance.interpCounter);
		glTranslate3f32(eb->position.x,eb->position.y,eb->position.z);
		glPushMatrix();
			glRotateZi(angle*1024);glRotateXi(angle*512);glRotateYi(angle*256);
			renderModelFrameInterp(eb->modelInstance.currentFrame,eb->modelInstance.nextFrame,0,eb->modelInstance.model,POLY_ALPHA(27)|POLY_ID(8)|POLY_CULL_NONE,false,eb->modelInstance.palette);
		glPopMatrix(1);
		glRotateXi(angle*1024);glRotateYi(angle*512);glRotateZi(angle*256);
		renderModelFrameInterp(eb->modelInstance.currentFrame,eb->modelInstance.nextFrame,0,eb->modelInstance.model,POLY_ALPHA(29)|POLY_ID(16)|POLY_CULL_NONE,false,eb->modelInstance.palette);
	glPopMatrix(1);
}

void drawEnergyBalls(void)
{
	int i;
	for(i=0;i<NUMENERGYBALLS;i++)
	{
		if(energyBall[i].used)drawEnergyBall(&energyBall[i]);
	}
}

void updateEnergyBall(energyBall_struct* eb)
{
	if(!eb)return;
	
	player_struct* p=getPlayer();
	
	vect3D l=vectDifference(eb->position,convertVect(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)));
	
	vect3D ip=l;
	bool col=collideLineMap(p->currentRoom, NULL, l, eb->direction, eb->speed, &ip); //ADD CULLING
	if(col)
	{
		ip=addVect(convertVect(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)),ip);
		// vect3D v=vect(0,0,0);
		// int32 z;
		// bool portal=isPointInPortal(&portal1,ip,&v,&z,&z,&z);
		eb->position=ip;
		eb->direction=vectMultInt(eb->direction,-1); //TEMP, improve (mirror depending on rec normal)
		eb->position=addVect(eb->position,vectMult(eb->direction,ENERGYBALLSIZE));
	}else{
		eb->position=addVect(eb->position,vectMult(eb->direction,eb->speed));
	}
	
	updateAnimation(&eb->modelInstance);
}

void updateEnergyBalls(void)
{
	int i;
	for(i=0;i<NUMENERGYBALLS;i++)
	{
		if(energyBall[i].used)updateEnergyBall(&energyBall[i]);
	}
}
