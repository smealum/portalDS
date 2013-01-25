#include "game/game_main.h"

cubeDispenser_struct cubeDispenser[NUMCUBEDISPENSERS];
md2Model_struct storageCubeModel, companionCubeModel, cubeDispenserModel;

void initCubes(void)
{
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		cubeDispenser[i].used=false;
		cubeDispenser[i].id=i;
	}
	
	loadMd2Model("models/cube.md2","storagecube.pcx",&storageCubeModel);generateModelDisplayLists(&storageCubeModel);
	loadMd2Model("models/cube.md2","companion.pcx",&companionCubeModel);generateModelDisplayLists(&storageCubeModel);
	loadMd2Model("models/cubedispenser.md2","cubedispenser.pcx",&cubeDispenserModel);generateModelDisplayLists(&cubeDispenserModel);
}

void initCubeDispenser(room_struct* r, cubeDispenser_struct* cd, vect3D pos, bool companion)
{
	if(!cd || !r)return;
	
	initModelInstance(&cd->modelInstance,&cubeDispenserModel);
	
	pos.y=getHeightValue(r,pos,false);
	
	{//for collisions
		rectangle_struct rec;
		rectangle_struct* recp;
		rec.material=NULL;
		
		rec.position=addVect(pos,vect(-1,-8,1));rec.size=vect(2,0,-2);rec.normal=vect(0,-inttof32(1),0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,-8,-1));rec.size=vect(2,8,0);rec.normal=vect(0,0,-inttof32(1));recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,0,1));rec.size=vect(2,-8,0);rec.normal=vect(0,0,inttof32(1));recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(-1,-8,-1));rec.size=vect(0,8,2);rec.normal=vect(-inttof32(1),0,0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
		rec.position=addVect(pos,vect(1,0,-1));rec.size=vect(0,-8,2);rec.normal=vect(inttof32(1),0,0);recp=addRoomRectangle(r, NULL, rec, NULL, false);
		if(recp)recp->hide=true;
	}	
	
	pos=vect(pos.x+r->position.x, pos.y, pos.z+r->position.y);
	cd->position=convertVect(pos);
	
	cd->companion=companion;
	cd->currentCube=NULL;
	
	changeAnimation(&cd->modelInstance,0,false);
	
	cd->used=true;
}

cubeDispenser_struct* createCubeDispenser(room_struct* r, vect3D pos, bool companion)
{
	if(!r)r=getPlayer()->currentRoom;
	if(!r)return NULL;
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		if(!cubeDispenser[i].used)
		{
			initCubeDispenser(r,&cubeDispenser[i],pos, companion);
			return &cubeDispenser[i];
		}
	}
	return NULL;
}

void drawCubeDispenser(cubeDispenser_struct* cd)
{
	if(!cd)return;
	
	glPushMatrix();
		u32 params=POLY_ALPHA(31)|POLY_CULL_NONE|POLY_ID(20+cd->id)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, cd->position, &params);
		glTranslate3f32(cd->position.x,cd->position.y,cd->position.z);
		renderModelFrameInterp(cd->modelInstance.currentFrame,cd->modelInstance.nextFrame,cd->modelInstance.interpCounter,cd->modelInstance.model,params,false,cd->modelInstance.palette);
	glPopMatrix(1);
}

void drawCubeDispensers(void)
{
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		if(cubeDispenser[i].used)drawCubeDispenser(&cubeDispenser[i]);
	}
}

void updateCubeDispenser(cubeDispenser_struct* cd)
{
	if(!cd)return;
	
	if(cd->active && !cd->oldActive)
	{
		if(!cd->currentCube)cd->currentCube=createBox(vectMultInt(cd->position,4),inttof32(1),(cd->companion)?(&companionCubeModel):(&storageCubeModel));
		else resetBox(cd->currentCube, vectMultInt(cd->position,4));
		changeAnimation(&cd->modelInstance,1,true);
	}
	
	updateAnimation(&cd->modelInstance);
	cd->oldActive=cd->active;
}

void updateCubeDispensers(void)
{
	int i;
	for(i=0;i<NUMCUBEDISPENSERS;i++)
	{
		if(cubeDispenser[i].used)updateCubeDispenser(&cubeDispenser[i]);
	}
}
