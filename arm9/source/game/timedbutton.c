#include "game/game_main.h"

timedButton_struct timedButton[NUMTIMEDBUTTONS];
md2Model_struct timedButtonModel;
u32* timedButtonBrightPalette;

void initTimedButtons(void)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		timedButton[i].used=false;
		timedButton[i].room=NULL;
		timedButton[i].id=i;
	}
	
	loadMd2Model("models/button2.md2","button2.pcx",&timedButtonModel);generateModelDisplayLists(&timedButtonModel, false, 1);
	timedButtonBrightPalette=loadPalettePCX("button1b.pcx","textures");
}

void initTimedButton(timedButton_struct* bb, room_struct* r, vect3D pos, u16 angle)
{
	if(!bb || !r)return;
	
	bb->room=r;
	
	initModelInstance(&bb->modelInstance,&timedButtonModel);
	
	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	bb->position=convertVect(pos);

	initActivator(&bb->activator);
	bb->active=false;
	bb->angle=angle;
	
	bb->used=true;
}

timedButton_struct* createTimedButton(room_struct* r, vect3D position, u16 angle)
{
	if(!r)r=getPlayer()->currentRoom;
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(!timedButton[i].used)
		{
			initTimedButton(&timedButton[i], r, position, angle);
			return &timedButton[i];
		}
	}
	return NULL;
}

void drawTimedButton(timedButton_struct* bb)
{
	if(!bb || !bb->used)return;
	
	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(10+bb->id)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, bb->position, &params);
		
		glTranslate3f32(bb->position.x,bb->position.y,bb->position.z);
		glRotateYi(bb->angle);
		renderModelFrameInterp(bb->modelInstance.currentFrame, bb->modelInstance.nextFrame, bb->modelInstance.interpCounter, bb->modelInstance.model, params, false, bb->active?(timedButtonBrightPalette):(NULL), RGB15(31,31,31));
	glPopMatrix(1);
}

void drawTimedButtons(void)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(timedButton[i].used)
		{
			drawTimedButton(&timedButton[i]);
		}
	}
}

void updateTimedButton(timedButton_struct* bb)
{
	if(!bb || !bb->used)return;
	
	bb->active=false;

	if(bb->active)
	{
		changeAnimation(&bb->modelInstance,1,false);
		useActivator(&bb->activator);
	}else{
		changeAnimation(&bb->modelInstance,0,false);
		unuseActivator(&bb->activator);
	}
	
	updateAnimation(&bb->modelInstance);
}

void updateTimedButtons(void)
{
	int i;
	for(i=0;i<NUMTIMEDBUTTONS;i++)
	{
		if(timedButton[i].used)
		{
			updateTimedButton(&timedButton[i]);
		}
	}
}
