#include <nds.h>
#include "game/game_main.h"

turret_struct turrets[NUMTURRETS];
md2Model_struct turretModel;

void initTurrets(void)
{
	int i;
	for(i=0;i<NUMTURRETS;i++)
	{
		turrets[i].used=false;
		turrets[i].OBB=NULL;
	}
	
	loadMd2Model("turret.md2","turret.pcx",&turretModel);
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
			changeAnimation(&turrets[i].OBB->modelInstance,3,false); //TEMP
			return &turrets[i];
		}
	}
	return NULL;
}

void updateTurret(turret_struct* t)
{
	if(!t || !t->used)return;
	
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
