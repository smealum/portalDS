#include "game/game_main.h"

bigButton_struct bigButton[NUMBIGBUTTONS];
md2Model_struct bigButtonModel;
room_struct* currentRoom;

void initBigButtons(room_struct* r)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		bigButton[i].used=false;
	}
	
	currentRoom=r;
	loadMd2Model("button1.md2","button1.pcx",&bigButtonModel);
}

void initBigButton(bigButton_struct* bb, room_struct* r, vect3D position)
{
	if(!bb || !r)return;
	
	position.y=getHeightValue(r,position,true);
	convertVect(position);
	bb->position=addVect(r->position, position); //TEMP
	
	bb->used=true;
}

bigButton_struct* createBigButton(vect3D position)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(!bigButton[i].used)
		{
			initBigButton(&bigButton[i], currentRoom, position);
			return &bigButton[i];
		}
	}
	return NULL;
}

void drawBigButton(bigButton_struct* bb)
{
	if(!bb || !bb->used)return;
	
	iprintf("%d, %d, %d",bb->position.x,bb->position.y,bb->position.z);
	
	glPushMatrix();
		glTranslate3f32(bb->position.x,bb->position.y,bb->position.z);
		renderModelFrameInterp(0, 0, 0, &bigButtonModel, POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_FORMAT_LIGHT0, false);
	glPopMatrix(1);
}

void drawBigButtons(void)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(bigButton[i].used)
		{
			drawBigButton(&bigButton[i]);
		}
	}
}
