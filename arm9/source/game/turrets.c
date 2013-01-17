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
	
	loadMd2Model("models/turret.md2","turret.pcx",&turretModel);
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

void updateTurret(turret_struct* t)
{
	if(!t || !t->used)return;
	
	editPalette((u16*)t->OBB->modelInstance.palette,0,RGB15(31,0,0)); //TEMP
	
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
