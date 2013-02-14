#include "editor/editor_main.h"

#define NUMENTITYTYPES (12)

entityType_struct entityTypes[]={(entityType_struct){"editor/models/ballcatcher_ed.md2", "balllauncher.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/balllauncher_ed.md2", "balllauncher.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/button2_ed.md2", "button2.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/button1_ed.md2", "button1.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/turret_ed.md2", "turret.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/cube_ed.md2", "companion.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/cube_ed.md2", "storagecube.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/dispenser_ed.md2", "dispenser.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/grid_ed.md2", "grid.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/platform_ed.md2", "platform.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/door_ed.md2", "door.pcx"/*, NULL, 0*/},
								(entityType_struct){"editor/models/light_ed.md2", "light.pcx"/*, NULL, 0*/}};

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
	}
	initEntityTypes();
}

void initEntity(entity_struct* e, entityType_struct* et, vect3D pos, bool placed)
{
	if(!e)return;

	e->position=pos;
	e->type=et;
	e->placed=placed;
	e->blockFace=NULL;
	e->used=true;
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

bool moveEntityToBlockFace(entity_struct* e, blockFace_struct* bf)
{
	if(!e || !bf)return false;

	e->position=adjustVectForNormal(bf->direction, vect(bf->x,bf->y,bf->z));
	e->blockFace=bf;
	e->placed=true;

	return true;
}

extern camera_struct editorCamera;

void drawEntity(entity_struct* e)
{
	if(!e || !e->placed)return;
	entityType_struct* et=e->type;
	if(!et)et=&entityTypes[0];

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glPushMatrix();
		editorRoomTransform();
		glTranslate3f32(inttof32(e->position.x),inttof32(e->position.y),inttof32(e->position.z));

		if(e->blockFace)
		{
			if(e->blockFace->direction<=1)glRotateZi(-8192);
			else if(e->blockFace->direction>=4)glRotateXi(8192);
			if(e->blockFace->direction%2)glRotateXi(16384);
		}
		glTranslate3f32(0,-inttof32(1)/2,0);
		renderModelFrameInterp(0, 0, 0, &et->model, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT, false, NULL, RGB15(31,31,31));
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
