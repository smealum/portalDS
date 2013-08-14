#include "game/game_main.h"

#define ENERGYBALLSIZE (128)

u32* activeDevicePalette;
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
	loadMd2Model("models/balllauncher.md2","balllauncher.pcx",&energyLauncherModel);
	loadMd2Model("models/energyball.md2","energyball.pcx",&energyBallModel);
	activeDevicePalette=loadPalettePCX("balllauncheractive.pcx","textures");
	generateModelDisplayLists(&energyCatcherModel, false, 0);
	generateModelDisplayLists(&energyLauncherModel, false, 0);
	generateModelDisplayLists(&energyBallModel, true, 0);
}

void freeEnergyBalls(void)
{
	freeMd2Model(&energyCatcherModel);
	freeMd2Model(&energyLauncherModel);
	freeMd2Model(&energyBallModel);
}

void initEnergyDevice(room_struct* r, energyDevice_struct* ed, vect3D pos, deviceOrientation_type or, bool type)
{
	if(!ed)return;
	
	initActivator(&ed->activator);
	initModelInstance(&ed->modelInstance, type?(&energyLauncherModel):(&energyCatcherModel));
	ed->orientation=or;
	
	ed->surface=NULL;
	
	{//for collisions
		rectangle_struct rec;
		rectangle_struct* recp;
		rec.material=NULL;
		
			if(or!=mY)
			{
				rec.position=addVect(pos,vect(-1,4,-1));rec.size=vect(2,0,2);rec.normal=vect(0,inttof32(1),0);recp=addRoomRectangle(r, rec, NULL, false);
				if(recp)recp->hide=true;
				if(or==pY)ed->surface=recp;
			}
			if(or!=pY)
			{
				rec.position=addVect(pos,vect(1,-4,1));rec.size=vect(-2,0,-2);rec.normal=vect(0,inttof32(1),0);recp=addRoomRectangle(r, rec, NULL, false);
				if(recp)recp->hide=true;
				if(or==mY)ed->surface=recp;
			}
			if(or!=pZ)
			{
				rec.position=addVect(pos,vect(-1,-4,-1));rec.size=vect(2,8,0);rec.normal=vect(0,0,-inttof32(1));recp=addRoomRectangle(r, rec, NULL, false);
				if(recp)recp->hide=true;
				if(or==mZ)ed->surface=recp;
			}
			if(or!=mZ)
			{
				rec.position=addVect(pos,vect(-1,4,1));rec.size=vect(2,-8,0);rec.normal=vect(0,0,inttof32(1));recp=addRoomRectangle(r, rec, NULL, false);
				if(recp)recp->hide=true;
				if(or==pZ)ed->surface=recp;
			}
			if(or!=pX)
			{
				rec.position=addVect(pos,vect(-1,-4,-1));rec.size=vect(0,8,2);rec.normal=vect(-inttof32(1),0,0);recp=addRoomRectangle(r, rec, NULL, false);
				if(recp)recp->hide=true;
				if(or==mX)ed->surface=recp;
			}
			if(or!=mX)
			{
				rec.position=addVect(pos,vect(1,4,-1));rec.size=vect(0,-8,2);rec.normal=vect(inttof32(1),0,0);recp=addRoomRectangle(r, rec, NULL, false);
				if(recp)recp->hide=true;
				if(or==pX)ed->surface=recp;
			}
	}
	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	ed->position=convertVect(pos);
	
	ed->active=type;
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
		u32 params=POLY_ALPHA(31)|POLY_CULL_NONE|POLY_ID(30+ed->id)|POLY_TOON_HIGHLIGHT|POLY_FOG;
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
				glRotateXi(8192);
				break;
			case mZ:
				glRotateXi(-8192);
				break;
			default:
				break;
		}
		
		renderModelFrameInterp(ed->modelInstance.currentFrame,ed->modelInstance.nextFrame,ed->modelInstance.interpCounter,ed->modelInstance.model,params,false,ed->modelInstance.palette,RGB15(31,31,31));
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
	
	if(ed->type)
	{
		//launcher
		if(ed->active)
		{
			changeAnimation(&ed->modelInstance,1,true);
			createEnergyBall(ed,addVect(ed->position,vectDivInt(energyDeviceDirection[ed->orientation],8)),energyDeviceDirection[ed->orientation], 300);
			ed->active=false;
		}
	}else{
		//receiver
		if(ed->active)
		{
			ed->modelInstance.palette=activeDevicePalette;
			useActivator(&ed->activator);
		}else ed->modelInstance.palette=NULL;
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

void initEnergyBall(energyBall_struct* eb, energyDevice_struct* ed, vect3D pos, vect3D dir, u16 life)
{
	if(!eb)return;
	
	initModelInstance(&eb->modelInstance, &energyBallModel);
	
	eb->maxLife=life;
	eb->life=life;
	eb->launcher=ed;
	eb->position=pos;
	eb->direction=dir;
	eb->speed=128;
	
	eb->used=true;
}

energyBall_struct* createEnergyBall(energyDevice_struct* launcher, vect3D pos, vect3D dir, u16 life)
{
	int i;
	for(i=0;i<NUMENERGYBALLS;i++)
	{
		if(!energyBall[i].used)
		{
			initEnergyBall(&energyBall[i],launcher,pos,dir,life);
			return &energyBall[i];
		}
	}
	return NULL;
}

void drawEnergyBall(energyBall_struct* eb)
{
	if(!eb)return;
	
	u8 alpha=8+(22*eb->life)/eb->maxLife;
	
	glPushMatrix();
		u32 angle=(eb->modelInstance.currentFrame*4+eb->modelInstance.interpCounter);
		glTranslate3f32(eb->position.x,eb->position.y,eb->position.z);
		glPushMatrix();
			glRotateZi(angle*1024);glRotateXi(angle*512);glRotateYi(angle*256);
			renderModelFrameInterp(eb->modelInstance.currentFrame,eb->modelInstance.nextFrame,0,eb->modelInstance.model,POLY_ALPHA(alpha)|POLY_ID(8)|POLY_CULL_NONE|POLY_FOG,false,eb->modelInstance.palette,RGB15(31,31,31));
		glPopMatrix(1);
		glRotateXi(angle*1024);glRotateYi(angle*512);glRotateZi(angle*256);
		renderModelFrameInterp(eb->modelInstance.currentFrame,eb->modelInstance.nextFrame,0,eb->modelInstance.model,POLY_ALPHA(alpha)|POLY_ID(16)|POLY_CULL_NONE|POLY_FOG,false,eb->modelInstance.palette,RGB15(31,31,31));
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

void warpEnergyBall(portal_struct* p, energyBall_struct* eb)
{
	if(!p->targetPortal)return;
	eb->position=addVect(warpVector(p,vectDifference(eb->position,p->position)),p->targetPortal->position);
	eb->direction=warpVector(p,eb->direction);
}

void killEnergyBall(energyBall_struct* eb)
{
	if(!eb)return;
	
	if(eb->launcher)eb->launcher->active=true;
	eb->used=false;
}

energyDevice_struct* isEnergyCatcherSurface(rectangle_struct* rec)
{
	if(!rec)return NULL;
	
	int i;
	for(i=0;i<NUMENERGYDEVICES;i++)
	{
		if(energyDevice[i].used && !energyDevice[i].type && energyDevice[i].surface==rec)return &energyDevice[i];
	}
	return NULL;
}

void updateEnergyBall(energyBall_struct* eb)
{
	if(!eb)return;
	
	player_struct* p=getPlayer();
	
	vect3D l=vectDifference(eb->position,convertSize(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)));
	
	vect3D ip=l, normal;
	rectangle_struct* col=collideGridCell(getCurrentCell(p->currentRoom,eb->position), eb->launcher?eb->launcher->surface:NULL, l, eb->direction, eb->speed, &ip, &normal);
	if(!col)col=collideGridCell(getCurrentCell(p->currentRoom,addVect(eb->position,vectMult(eb->direction,eb->speed))), eb->launcher?eb->launcher->surface:NULL, l, eb->direction, eb->speed, &ip, &normal);
	if(col)
	{
		// NOGBA("COL COL COL %d",col->collides);
		ip=addVect(convertSize(vect(p->currentRoom->position.x,0,p->currentRoom->position.y)),ip);
		energyDevice_struct* ed=isEnergyCatcherSurface(col);
		if(ed && !ed->active)
		{
			//caught
			changeAnimation(&ed->modelInstance,2,false);
			changeAnimation(&ed->modelInstance,1,true);
			killEnergyBall(eb);
			if(eb->launcher)eb->launcher->active=false;
			ed->active=true;
			return;
		}
		vect3D v=vect(0,0,0);
		int32 x, y, z;
		portal_struct* portal=NULL;
		if(isPointInPortal(&portal1,ip,&v,&x,&y,&z))portal=&portal1;
		if(abs(z)>=32)portal=NULL;
		if(!portal)
		{
			if(isPointInPortal(&portal2,ip,&v,&x,&y,&z))portal=&portal2;
			if(abs(z)>=32)portal=NULL;
		}
		if(!portal)
		{
			eb->position=ip;
			eb->direction=vectDifference(eb->direction,vectMult(normal,2*dotProduct(eb->direction,normal)));
			eb->position=addVect(eb->position,vectMult(eb->direction,ENERGYBALLSIZE));
		}else{
			eb->position=addVect(eb->position,vectMult(eb->direction,eb->speed));
			warpEnergyBall(portal,eb);
			eb->position=addVect(eb->position,vectMult(eb->direction,eb->speed));
		}
	}else{
		eb->position=addVect(eb->position,vectMult(eb->direction,eb->speed));
	}
	
	updateAnimation(&eb->modelInstance);
	
	if(!eb->life)killEnergyBall(eb);
	else eb->life--;
}

void updateEnergyBalls(void)
{
	int i;
	for(i=0;i<NUMENERGYBALLS;i++)
	{
		if(energyBall[i].used)updateEnergyBall(&energyBall[i]);
	}
}
