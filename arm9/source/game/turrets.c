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
	generateModelDisplayLists(&turretModel);
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
	GFX_COLOR=RGB15(31,0,0);
	glBegin(GL_TRIANGLES);	
		glVertex3v16(orig.x, orig.y, orig.z);
		glVertex3v16(target.x, target.y, target.z);
		glVertex3v16(target.x, target.y, target.z);
}

void drawTurretStuff(turret_struct* t)
{
	if(!t || !t->used)return;
	
	int32* m=t->OBB->transformationMatrix;
	
	vect3D pos=addVect(vectDivInt(t->OBB->position,4),evalVectMatrix33(m,laserOrigin));
	drawLaser(pos,addVect(pos,vect(m[2],m[5],m[8])));
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

void updateTurret(turret_struct* t)
{
	if(!t || !t->used)return;
	
	t->counter+=2;t->counter%=63; //TEMP
	editPalette((u16*)t->OBB->modelInstance.palette,0,RGB15(abs(31-t->counter),0,0)); //TEMP
	
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
