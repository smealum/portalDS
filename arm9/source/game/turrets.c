#include "game/game_main.h"

turret_struct turrets[NUMTURRETS];
md2Model_struct turretModel;

const vect3D laserOrigin=(vect3D){16,76,140};

void initTurrets(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		turrets[i].used=false;
		turrets[i].OBB=NULL;
	}
	
	loadMd2Model("models/turret.md2","turret.pcx",&turretModel);
	NOGBA("turret1 mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	generateModelDisplayLists(&turretModel, true, 1);
	NOGBA("turret2 mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

turret_struct* createTurret(vect3D position)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		if(!turrets[i].used)
		{
			turrets[i].OBB=createBox(position,TURRETMASS,&turretModel);
			if(!turrets[i].OBB)return NULL;
			turrets[i].used=true;
			turrets[i].OBB->modelInstance.palette=loadPalettePCX("turret.pcx","textures");
			changeAnimation(&turrets[i].OBB->modelInstance,3,false); //TEMP
			return &turrets[i];
		}
	}
	return NULL;
}

void drawLaser(vect3D orig, vect3D target)
{
	unbindMtl();
	glPolyFmt(POLY_ID(63));
	GFX_COLOR=RGB15(31,0,0);
	glBegin(GL_TRIANGLES);	
		glVertex3v16(orig.x, orig.y, orig.z);
		glVertex3v16(target.x, target.y, target.z);
		glVertex3v16(target.x, target.y, target.z);
}

void drawTurretStuff(turret_struct* t)
{
	if(!t || !t->used)return;
	
	drawLaser(t->laserOrigin,t->laserDestination);
	if(t->laserThroughPortal)drawLaser(t->laserOrigin2,t->laserDestination2);
}

void drawTurretsStuff(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		if(turrets[i].used)
		{
			drawTurretStuff(&turrets[i]);
		}
	}
}

void laserProgression(room_struct* r, vect3D* origin, vect3D* destination, vect3D dir)
{
	if(!r || !origin || !destination)return;
	vect3D ip;
	vect3D l=*origin;
	vect3D v=vectDifference(addVect(l,vectMult(dir,32)),convertSize(vect(r->position.x,0,r->position.y)));
	gridCell_struct* gc=getCurrentCell(r,l);
	while(gc && !collideGridCell(gc, NULL, v, dir, inttof32(100), &ip, NULL))
	{
		l=addVect(l,vectMult(dir,CELLSIZE*TILESIZE*2));
		gc=getCurrentCell(r,l);
	}
	if(gc)*destination=addVect(ip,convertSize(vect(r->position.x,0,r->position.y)));
}

void updateTurret(turret_struct* t)
{
	if(!t || !t->used)return;
	
	t->counter+=2;t->counter%=63; //TEMP
	editPalette((u16*)t->OBB->modelInstance.palette,0,RGB15(abs(31-t->counter),0,0)); //TEMP
	
	int32* m=t->OBB->transformationMatrix;
	room_struct* r=getPlayer()->currentRoom;
	if(!r)return;
	
	t->laserOrigin=addVect(vectDivInt(t->OBB->position,4),evalVectMatrix33(m,laserOrigin));
	t->laserDestination=addVect(t->laserOrigin,vect(m[2],m[5],m[8]));
	t->laserThroughPortal=false;
	
	laserProgression(r, &t->laserOrigin, &t->laserDestination, vect(m[2],m[5],m[8]));
	
	int32 x, y, z;
	vect3D v;
	portal_struct* portal=NULL;
	if(isPointInPortal(&portal1, t->laserDestination, &v, &x, &y, &z))portal=&portal1;
	if(abs(z)>=32)portal=NULL;
	if(!portal)
	{
		if(isPointInPortal(&portal2, t->laserDestination, &v, &x, &y, &z))portal=&portal2;
		if(abs(z)>=32)portal=NULL;
	}
	if(portal)
	{
		t->laserThroughPortal=true;
		vect3D dir=warpVector(portal,vect(m[2],m[5],m[8]));
		t->laserOrigin2=addVect(portal->targetPortal->position, warpVector(portal, vectDifference(t->laserDestination, portal->position)));
		t->laserDestination2=addVect(t->laserOrigin2,dir);
		
		laserProgression(r, &t->laserOrigin2, &t->laserDestination2, dir);
		
		//TEST
		t->laserDestination=addVect(t->laserDestination,vectMult(vect(m[2],m[5],m[8]),TILESIZE));
		t->laserOrigin2=addVect(t->laserOrigin2,vectMult(dir,-TILESIZE));
	}
	
	updateAnimation(&t->OBB->modelInstance); //TEMP
}

void updateTurrets(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		if(turrets[i].used)
		{
			updateTurret(&turrets[i]);
		}
	}
}
