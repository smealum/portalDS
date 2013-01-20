#include "game/game_main.h"

bigButton_struct bigButton[NUMBIGBUTTONS];
md2Model_struct bigButtonModel;
u32* bigButtonBrightPalette;

void initBigButtons(void)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		bigButton[i].used=false;
		bigButton[i].room=NULL;
	}
	
	loadMd2Model("models/button1.md2","button1.pcx",&bigButtonModel);
	bigButtonBrightPalette=loadPalettePCX("button1b.pcx","textures");
}

void initBigButton(bigButton_struct* bb, room_struct* r, vect3D pos)
{
	if(!bb || !r)return;
	
	bb->room=r;
	
	pos.y=getHeightValue(r,pos,true);
	
	{//for collisions
		rectangle_struct rec;
		rectangle_struct* recp;
		rec.material=NULL;
		
		rec.position=addVect(pos,vect(-1,1,-1));rec.size=vect(2,0,2);rec.normal=vect(0,inttof32(1),0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp){recp->hide=true;bb->surface=recp;}
		rec.position=addVect(pos,vect(-1,0,-1));rec.size=vect(2,1,0);rec.normal=vect(0,0,-inttof32(1));recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,1,1));rec.size=vect(2,-1,0);rec.normal=vect(0,0,inttof32(1));recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,0,-1));rec.size=vect(0,1,2);rec.normal=vect(-inttof32(1),0,0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(1,1,-1));rec.size=vect(0,-1,2);rec.normal=vect(inttof32(1),0,0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
	}
	
	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	bb->position=convertVect(pos);
	
	initActivator(&bb->activator);
	bb->active=false;
	
	bb->used=true;
}

bigButton_struct* createBigButton(room_struct* r, vect3D position)
{
	if(!r)r=getPlayer()->currentRoom;
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(!bigButton[i].used)
		{
			initBigButton(&bigButton[i], r, position);
			return &bigButton[i];
		}
	}
	return NULL;
}

void drawBigButton(bigButton_struct* bb)
{
	if(!bb || !bb->used)return;
	
	glPushMatrix();
		glTranslate3f32(bb->position.x,bb->position.y,bb->position.z);
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT;
		// setupObjectLighting(NULL, bb->position, &params);
		renderModelFrameInterp(0, 0, 0, &bigButtonModel, params, false, bb->active?(bigButtonBrightPalette):(NULL));
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

void updateBigButton(bigButton_struct* bb)
{
	if(!bb || !bb->used)return;
	
	bb->active=bb->surface->touched;
	if(bb->active)useActivator(&bb->activator);
	else unuseActivator(&bb->activator);
}

void updateBigButtons(void)
{
	int i;
	for(i=0;i<NUMBIGBUTTONS;i++)
	{
		if(bigButton[i].used)
		{
			updateBigButton(&bigButton[i]);
		}
	}
}
