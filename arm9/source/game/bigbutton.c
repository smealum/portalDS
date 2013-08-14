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
		bigButton[i].id=i;
	}
	
	loadMd2Model("models/button1.md2","button1.pcx",&bigButtonModel);generateModelDisplayLists(&bigButtonModel, false, 1);
	bigButtonBrightPalette=loadPalettePCX("button1b.pcx","textures");
}

void freeBigButtons(void)
{
	freeMd2Model(&bigButtonModel);
}

void initBigButton(bigButton_struct* bb, room_struct* r, vect3D pos)
{
	if(!bb || !r)return;
	
	bb->room=r;
	
	initModelInstance(&bb->modelInstance,&bigButtonModel);
	
	{//for collisions
		rectangle_struct rec;
		rectangle_struct* recp;
		rec.material=NULL;
		
		rec.position=addVect(pos,vect(-1,1,-1));rec.size=vect(2,0,2);rec.normal=vect(0,inttof32(1),0);recp=addRoomRectangle(r, rec, NULL, false);
		if(recp){recp->hide=true;bb->surface=recp;}
		rec.position=addVect(pos,vect(-1,0,-1));rec.size=vect(2,1,0);rec.normal=vect(0,0,-inttof32(1));recp=addRoomRectangle(r, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,1,1));rec.size=vect(2,-1,0);rec.normal=vect(0,0,inttof32(1));recp=addRoomRectangle(r, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,0,-1));rec.size=vect(0,1,2);rec.normal=vect(-inttof32(1),0,0);recp=addRoomRectangle(r, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(1,1,-1));rec.size=vect(0,-1,2);rec.normal=vect(inttof32(1),0,0);recp=addRoomRectangle(r, rec, NULL, false);
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
		u32 params=POLY_ALPHA(31)|POLY_CULL_FRONT|POLY_ID(10+bb->id)|POLY_TOON_HIGHLIGHT|POLY_FOG;
		setupObjectLighting(NULL, bb->position, &params);
		
		glTranslate3f32(bb->position.x,bb->position.y,bb->position.z);
		renderModelFrameInterp(bb->modelInstance.currentFrame, bb->modelInstance.nextFrame, bb->modelInstance.interpCounter, bb->modelInstance.model, params, false, bb->active?(bigButtonBrightPalette):(NULL), RGB15(31,31,31));
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
	
	bb->active=false;
	if(bb->surface->touched)bb->active=true;
	if(bb->surface->AARid>0)
	{
		if(!bb->active)bb->active=aaRectangles[bb->surface->AARid].touched;
		aaRectangles[bb->surface->AARid].touched=false;
	}
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
