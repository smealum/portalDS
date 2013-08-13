#include "editor/editor_main.h"

mtlImg_struct* pointTexture;

void cubeSpecialInit(entity_struct* e);
void wallDoorSpecialDraw(entity_struct* e);
void cubeSpecialMove(entity_struct* e, vect3D op, u8 od, bool m);
void dispenserSpecialMove(entity_struct* e, vect3D op, u8 od, bool m);
void platformSpecialMove(entity_struct* e, vect3D op, u8 od, bool m);
void wallDoorSpecialMove(entity_struct* e, vect3D op, u8 od, bool m);
bool wallDoorSpecialMoveCheck(entity_struct* e, vect3D p, u8 dir);
bool platformTargetSpecialMoveCheck(entity_struct* e, vect3D p, u8 dir);

entityType_struct entityTypes[]={(entityType_struct){"editor/models/ballcatcher_ed.md2", "balllauncher.pcx", pX_mask | mX_mask | pY_mask | mY_mask | pZ_mask | mZ_mask, ballCatcherButtonArray, 2, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/balllauncher_ed.md2", "balllauncher.pcx", pX_mask | mX_mask | pY_mask | mY_mask | pZ_mask | mZ_mask, ballLauncherButtonArray, 1, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/button2_ed.md2", "button2.pcx", pY_mask, button2ButtonArray, 3, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/button1_ed.md2", "button1.pcx", pY_mask, button1ButtonArray, 2, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/turret_ed.md2", "turret.pcx", pY_mask, turretButtonArray, 2, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/cube_ed.md2", "companion.pcx", pY_mask, cubeButtonArray, 1, cubeSpecialInit, NULL, cubeSpecialMove, NULL, true, true},
								(entityType_struct){"editor/models/cube_ed.md2", "storagecube.pcx", pY_mask, cubeButtonArray, 1, cubeSpecialInit, NULL, cubeSpecialMove, NULL, true, true},
								(entityType_struct){"editor/models/dispenser_ed.md2", "cubedispenser.pcx", mY_mask, cubeButtonArray, 1, NULL, NULL, dispenserSpecialMove, NULL, true, true},
								(entityType_struct){"editor/models/grid_ed.md2", "balllauncher.pcx", pX_mask | mX_mask | pZ_mask | mZ_mask, gridButtonArray, 1, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/platform_ed.md2", "platform.pcx", pX_mask | mX_mask | pY_mask | pZ_mask | mZ_mask, platformButtonArray, 2, NULL, NULL, platformSpecialMove, NULL, true, false},
								(entityType_struct){"editor/models/door_ed.md2", "door.pcx", pY_mask, doorButtonArray, 2, NULL, NULL, NULL, NULL, false, true},
								(entityType_struct){"editor/models/light_ed.md2", "lightbulb.pcx", pX_mask | mX_mask | pY_mask | mY_mask | pZ_mask | mZ_mask, lightButtonArray, 1, NULL, NULL, NULL, NULL, false, false},
								(entityType_struct){"editor/models/platform_ed.md2", "platformtarget.pcx", pX_mask | mX_mask | pY_mask | pZ_mask | mZ_mask, platformButtonArray, 0, NULL, NULL, NULL, platformTargetSpecialMoveCheck, true, false},
								(entityType_struct){"editor/models/walldoor_ed.md2", "door_plain.pcx", pX_mask | mX_mask | pZ_mask | mZ_mask, gridButtonArray, 1, NULL, wallDoorSpecialDraw, wallDoorSpecialMove, wallDoorSpecialMoveCheck, false, true},
								(entityType_struct){"editor/models/walldoor_ed.md2", "door.pcx", pX_mask | mX_mask | pZ_mask | mZ_mask, gridButtonArray, 1, NULL, wallDoorSpecialDraw, wallDoorSpecialMove, wallDoorSpecialMoveCheck, false, true},
							};

entity_struct entity[NUMENTITIES];

void initEntityType(entityType_struct* et)
{
	if(!et)return;

	loadMd2Model((char*)et->modelName,(char*)et->textureName,&et->model);
}

void initEntityTypes(void)
{
	int i;
	for(i=0;i<NUMENTITYTYPES;i++)
	{
		initEntityType(&entityTypes[i]);
		entityTypes[i].id=i;
	}
}

void initEntities(void)
{
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		entity[i].used=false;
		entity[i].blockFace=NULL;
		entity[i].target=NULL;
	}
	initEntityTypes();

	pointTexture=createTexture("points.pcx", "textures");
}

void initEntity(entity_struct* e, entityType_struct* et, vect3D pos, bool placed)
{
	if(!e)return;

	e->position=pos;
	e->type=et;
	e->placed=placed;
	e->blockFace=NULL;
	e->target=NULL;
	e->used=true;
	e->orientation=0;

	if(et && et->specialInit)et->specialInit(e);
}

void changeEntityType(entity_struct* e, u8 type)
{
	if(!e || type>=NUMENTITYTYPES)return;
	e->type=&entityTypes[type];
}

void generateLightsFromEntities(void)
{
	initLights();
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		entity_struct* e=&entity[i];
		if(e->used && e->type==&entityTypes[11])
		{
			createLight(vect(e->position.x*BLOCKMULTX,e->position.y*BLOCKMULTY,e->position.z*BLOCKMULTZ), BLOCKSIZEX*16);
		}
	}
}

entity_struct* createEntity(vect3D pos, u8 type, bool placed)
{
	if(type>=NUMENTITYTYPES)return NULL;
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(!entity[i].used)
		{
			initEntity(&entity[i], &entityTypes[type], pos, placed);
			return &entity[i];
		}
	}
	return NULL;
}

void removeTargetEntities(entity_struct* e)
{
	if(!e)return;
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(entity[i].used && entity[i].target==e)entity[i].target=NULL;
	}
}

void removeEntity(entity_struct* e)
{
	if(!e || !e->used)return;

	if(e->type && e->type->removeTarget && e->target)
	{
		entity_struct* t=e->target;
		e->target=NULL;
		removeEntity(t);
	}
	removeTargetEntities(e);
	e->used=false;
}

void removeEntities(void)
{
	int i;for(i=0;i<NUMENTITIES;i++)removeEntity(&entity[i]);
}

bool collideLineEntity(entity_struct* e, vect3D o, vect3D v, vect3D p1, vect3D p2, int32* d)
{
	if(!e)return false;
	o=vectDifference(o,getBlockPosition(e->position.x,e->position.y,e->position.z));
	int32 dist=dotProduct(o,v);
	if(d)*d=dist;
	o=vectDifference(o,vectMult(v,dist));
	o=vect(dotProduct(o,p1),dotProduct(o,p2),0);
	return (o.x>=-BLOCKSIZEX/2 && o.x<=BLOCKSIZEX/2 && o.y>=-BLOCKSIZEY/2 && o.y<=BLOCKSIZEY/2);
}

entity_struct* collideLineEntities(vect3D o, vect3D v, vect3D p1, vect3D p2, int32* d)
{
	int i;
	int32 distance=1<<29;
	entity_struct* ret=NULL;
	for(i=0;i<NUMENTITIES;i++)
	{
		int32 d;
		if(entity[i].used && collideLineEntity(&entity[i],o,v,p1,p2,&d))
		{
			if(d<distance)
			{
				distance=d;
				ret=&entity[i];
			}
		}
	}
	if(ret && d)*d=distance;
	return ret;
}

bool isEntityPositionValid(entity_struct* e, vect3D p, u8 dir)
{
	if(!e)return false;

	if(e->type && e->type->specialMoveCheck && !e->type->specialMoveCheck(e,p,dir))return false;

	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(&entity[i]!=e && entity[i].used && entity[i].position.x==p.x && entity[i].position.y==p.y && entity[i].position.z==p.z)return false; //TEMP
	}
	return true;
}

bool isEntityBlockFaceValid(entity_struct* e, blockFace_struct* bf)
{
	if(!e || !bf)return false;
	entityType_struct* et=e->type;
	if(!et)return false;

	return (et->possibleDirections & (1<<(bf->direction))) && isEntityPositionValid(e, adjustVectForNormal(bf->direction, vect(bf->x,bf->y,bf->z)), bf->direction);
}

bool moveEntityToBlockFace(entity_struct* e, blockFace_struct* bf)
{
	if(!e || !bf)return false;

	if(!isEntityBlockFaceValid(e,bf)){return false;}

	vect3D op=e->position;
	u8 od=e->direction;

	e->position=adjustVectForNormal(bf->direction, vect(bf->x,bf->y,bf->z));
	e->direction=bf->direction;
	e->blockFace=bf;
	e->placed=true;

	if(e->type && e->type->specialMove)e->type->specialMove(e, op, od, op.x==e->position.x && op.y==e->position.y && op.z==e->position.z && od==e->direction);

	return true;
}

blockFace_struct* getEntityBlockFace(entity_struct* e, blockFace_struct* l)
{
	if(!e || !e->used || !l)return NULL;

	vect3D o=adjustVectForNormal(oppositeDirection[e->direction], e->position);

	while(l)
	{
		if(l->x==o.x && l->y==o.y && l->z==o.z && l->direction==e->direction)return l;
		l=l->next;
	}
	return NULL;
}

void getEntityBlockFaces(blockFace_struct* l)
{
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(entity[i].used){entity[i].blockFace=getEntityBlockFace(&entity[i],l);}
	}
}

void getEntityBlockFacesRange(blockFace_struct* l, vect3D o, vect3D s, bool delete)
{
	int i;
	vect3D v=addVect(o, s);
	for(i=0;i<NUMENTITIES;i++)
	{
		entity_struct* e=&entity[i];
		if(e->used && e->position.x>=o.x && e->position.x<v.x && e->position.y>=o.y && e->position.y<v.y && e->position.z>=o.z && e->position.z<v.z)
		{
			e->blockFace=getEntityBlockFace(e,l);
			if(delete && !e->blockFace)removeEntity(e);
		}
	}
}

void moveEntitiesRange(vect3D o, vect3D s, vect3D u)
{
	int i;
	vect3D v=addVect(o, s);
	for(i=0;i<NUMENTITIES;i++)
	{
		entity_struct* e=&entity[i];
		if(e->used && e->position.x>=o.x && e->position.x<v.x && e->position.y>=o.y && e->position.y<v.y && e->position.z>=o.z && e->position.z<v.z)
		{
			e->position=addVect(e->position, u);
			e->blockFace=NULL;
		}
	}
}

extern camera_struct editorCamera;

void drawEntity(entity_struct* e)
{
	if(!e || !e->placed)return;
	entityType_struct* et=e->type;
	if(!et)et=&entityTypes[0];

	glPushMatrix();
		editorRoomTransform();
		glTranslate3f32(inttof32(e->position.x),inttof32(e->position.y),inttof32(e->position.z));

		if(e->blockFace && et->rotate)
		{
			if(e->direction<=1)glRotateZi(-8192);
			else if(e->direction>=4){glRotateXi(8192);glRotateYi(-8192);}
			if(e->direction%2)glRotateXi(16384);
		}
		glRotateYi(8192*e->orientation);
		glTranslate3f32(0,-inttof32(1)/2,0);
		renderModelFrameInterp(0, 0, 0, &et->model, POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT, false, NULL, RGB15(31,31,31));
		if(e->type && e->type->specialDraw)e->type->specialDraw(e);
	glPopMatrix(1);
}

void drawEntities(void)
{
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(entity[i].used)drawEntity(&entity[i]);
	}
}

//ENTITY CONTEXT BUTTONS

void deleteEntityButton(sguiButton_struct* b)
{
	selection_struct* s=&editorSelection;
	if(!s->entity)return;

	removeEntity(s->entity);
	undoSelection(s);
}

void rotateEntityButton(sguiButton_struct* b)
{
	selection_struct* s=&editorSelection;
	if(!s->entity)return;

	s->entity->orientation++;
	s->entity->orientation%=4;
}

void selectTargetButton(sguiButton_struct* b)
{
	cleanUpContextButtons();
	editorSelection.selectingTarget=true;
}

void setPlatformTargetButton(sguiButton_struct* b)
{
	if(!editorSelection.entity)return;
	entity_struct* e=createEntity(vect(32,32,32), 12, false);
	if(!e)return;
	e->target=editorSelection.entity;
	editorSelection.entity->target=e;

	editorSelection.selectingTarget=false;
	editorSelection.entity=e;
	cleanUpContextButtons();
}

contextButton_struct ballLauncherButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}};
contextButton_struct ballCatcherButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}, (contextButton_struct){"target", (buttonTargetFunction)selectTargetButton}};
contextButton_struct button1ButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}, (contextButton_struct){"target", (buttonTargetFunction)selectTargetButton}};
contextButton_struct button2ButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}, (contextButton_struct){"target", (buttonTargetFunction)selectTargetButton}, (contextButton_struct){"rotate", (buttonTargetFunction)rotateEntityButton}};
contextButton_struct turretButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}, (contextButton_struct){"rotate", (buttonTargetFunction)rotateEntityButton}};
contextButton_struct cubeButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}};
contextButton_struct gridButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}};
contextButton_struct platformButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton},(contextButton_struct){"set path", (buttonTargetFunction)setPlatformTargetButton}};
contextButton_struct doorButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}, (contextButton_struct){"rotate", (buttonTargetFunction)rotateEntityButton}};
contextButton_struct lightButtonArray[]={(contextButton_struct){"delete", (buttonTargetFunction)deleteEntityButton}};

//SPECIAL INITS/UPDATES


extern editorRoom_struct editorRoom;

void cubeSpecialInit(entity_struct* e)
{
	if(!e || e->placed)return;

	entity_struct* t=createEntity(e->position, 7, e->placed);
	if(!t)return;

	t->target=e;
	e->target=t;

	if(e->placed)cubeSpecialMove(e, e->position, e->direction, false);
}

void cubeSpecialMove(entity_struct* e, vect3D op, u8 od, bool m)
{
	if(!e || !e->target || e==e->target)return;
	entity_struct* t=e->target;

	int j;
	for(j=e->position.y;j<ROOMARRAYSIZEY && !getBlock(editorRoom.blockArray,e->position.x,j,e->position.z);j++);
	j--;

	t->placed=true;
	t->position=vect(e->position.x, j, e->position.z);
	t->direction=3;
	t->blockFace=getEntityBlockFace(t, editorRoom.blockFaceList);
}

void dispenserSpecialMove(entity_struct* e, vect3D op, u8 od, bool m)
{
	if(!e || !e->target || e==e->target)return;
	entity_struct* t=e->target;

	int j;
	for(j=e->position.y;j>=0 && !getBlock(editorRoom.blockArray,e->position.x,j,e->position.z);j--);
	j++;

	t->placed=true;
	t->position=vect(e->position.x, j, e->position.z);
	t->direction=2;
	t->blockFace=getEntityBlockFace(t, editorRoom.blockFaceList);
}

void platformSpecialMove(entity_struct* e, vect3D op, u8 od, bool m)
{
	if(!e || !e->target || e==e->target || m)return;
	entity_struct* t=e->target;

	if(!platformTargetSpecialMoveCheck(t, t->position, t->direction))
	{
		t->target=NULL;
		removeEntity(t);
	}
}

bool platformTargetSpecialMoveCheck(entity_struct* e, vect3D p, u8 dir)
{
	if(!e || !e->target || e==e->target)return false;
	entity_struct* t=e->target;

	return (t->position.x==p.x && t->position.y==p.y)
		|| (t->position.x==p.x && t->position.z==p.z)
		|| (t->position.z==p.z && t->position.y==p.y);
}

void wallDoorSpecialMove(entity_struct* e, vect3D op, u8 od, bool m)
{
	if(!e || m)return;

	vect3D p=vectDifference(op,adjustVectForNormal(od, vect(0,0,0)));
	BLOCK_TYPE v=getBlock(editorRoom.blockArray,p.x,p.y,p.z);

	v=v&(~BLOCK_NOWALLS);

	setBlock(editorRoom.blockArray,p.x,p.y,p.z,v);

	p=vectDifference(e->position,adjustVectForNormal(e->direction, vect(0,0,0)));

	setBlock(editorRoom.blockArray,p.x,p.y,p.z,getBlock(editorRoom.blockArray,p.x,p.y,p.z)|BLOCK_NOWALLS);
}

bool wallDoorSpecialMoveCheck(entity_struct* e, vect3D p, u8 dir)
{
	if(!e)return false;

	int i, j, k;
	s8 v=-1;

	//ugh
	if(dir<=1)
	{
		if(!(dir%2))v=1;
		if(!(p.x>-v && p.y>1 && p.z>1 && p.x<ROOMARRAYSIZEX+v && p.y<ROOMARRAYSIZEY-1 && p.z<ROOMARRAYSIZEZ-1))return false;

		for(i=0;i<3;i++)for(j=0;j<3;j++)for(k=0;k<3;k++)if(!getBlock(editorRoom.blockArray,p.x+(i-3)*v,p.y+j-1,p.z+k-1))return false;
	}else{
		if(!(dir%2))v=1;
		if(!(p.x>1 && p.y>1 && p.z>-v && p.x<ROOMARRAYSIZEX-1 && p.y<ROOMARRAYSIZEY-1 && p.z<ROOMARRAYSIZEZ+v))return false;

		for(i=0;i<3;i++)for(j=0;j<3;j++)for(k=0;k<3;k++)if(!getBlock(editorRoom.blockArray,p.x+i-1,p.y+j-1,p.z+(k-3)*v))return false;
	}

	return true;
}

void wallDoorSpecialDraw(entity_struct* e)
{
	if(!e)return;

	bindTexture(pointTexture);
	GFX_COLOR=RGB15(31,31,31);
	glScalef32(inttof32(3),inttof32(3),inttof32(3));
	//fait main avec amour
	//animation ?
	glBegin(GL_TRIANGLES);
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,-(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,-(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),-(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),-(1<<5));


		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,-(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,-(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,-(1<<5));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,-(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,(1<<5));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,(1<<5));


		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),0,(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),(1<<5));
		GFX_VERTEX10=NORMAL_PACK((1<<5),-2*(1<<5),(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,(1<<5));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),0,(1<<5));

		GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),-(1<<5));
		GFX_TEX_COORD=TEXTURE_PACK(inttot16(32), inttot16(0));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),(1<<5));
		GFX_VERTEX10=NORMAL_PACK(-(1<<5),-2*(1<<5),(1<<5));
}

int32 getGridLength(entity_struct* e)
{
	if(!e)return 0;

	vect3D d;

	switch(e->direction)
	{
		case 0:
			d=vect(1,0,0);
			break;
		case 1:
			d=vect(-1,0,0);
			break;
		case 2:
			d=vect(0,1,0);
			break;
		case 3:
			d=vect(0,-1,0);
			break;
		case 4:
			d=vect(0,0,1);
			break;
		default:
			d=vect(0,0,-1);
	}

	vect3D p;
	int32 l=1;
	for(p=addVect(e->position,d);!getBlock(editorRoom.blockArray,p.x,p.y,p.z);p=addVect(p,d))l++;

	switch(e->direction)
	{
		case 0: case 1:
			l*=BLOCKSIZEX;
			break;
		case 2: case 3:
			l*=BLOCKSIZEY;
			break;
		case 4: default:
			l*=BLOCKSIZEZ;
	}

	return l;
}
