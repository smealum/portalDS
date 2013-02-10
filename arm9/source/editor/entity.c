#include "editor/editor_main.h"

#define NUMENTITYTYPES (11)

entityType_struct entityTypes[]={(entityType_struct){"ballcatcher_16.pcx", NULL},
								(entityType_struct){"balllauncher_16.pcx", NULL},
								(entityType_struct){"button_16.pcx", NULL},
								(entityType_struct){"pressurebttn_16.pcx", NULL},
								(entityType_struct){"turret_16.pcx", NULL},
								(entityType_struct){"companion_16.pcx", NULL},
								(entityType_struct){"storagecube_16.pcx", NULL},
								(entityType_struct){"dispenser_16.pcx", NULL},
								(entityType_struct){"grid_16.pcx", NULL},
								(entityType_struct){"platform_16.pcx", NULL},
								(entityType_struct){"door_16.pcx", NULL}};

entity_struct entity[NUMENTITIES];

void initEntityType(entityType_struct* et)
{
	if(!et)return;

	et->spriteTexture=createTexture((char*)et->spriteName, "editor/3D");
}

void initEntityTypes(void)
{
	int i;
	for(i=0;i<NUMENTITYTYPES;i++)
	{
		initEntityType(&entityTypes[i]);
	}
}

void initEntities(void)
{
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		entity[i].used=false;
	}
	initEntityTypes();
}

void initEntity(entity_struct* e, vect3D pos)
{
	if(!e)return;

	e->position=pos;
	e->type=NULL;
	e->used=true;
}

entity_struct* createEntity(vect3D pos)
{
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		if(!entity[i].used)
		{
			initEntity(&entity[i], pos);
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

extern camera_struct editorCamera;

void drawEntity(entity_struct* e)
{
	if(!e)return;
	entityType_struct* et=e->type;
	if(!et)et=&entityTypes[0];

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);

	glPushMatrix();
		editorRoomTransform();
		glTranslate3f32(inttof32(e->position.x),inttof32(e->position.y),inttof32(e->position.z));
		untransformCamera(&editorCamera);
		applyMTL(et->spriteTexture);
		GFX_BEGIN=GL_QUADS;
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(0));
			GFX_VERTEX10=NORMAL_PACK(-(1<<5),-(1<<5), 0);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(64), inttot16(0));
			GFX_VERTEX10=NORMAL_PACK( (1<<5),-(1<<5), 0);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(64), inttot16(64));
			GFX_VERTEX10=NORMAL_PACK( (1<<5), (1<<5), 0);
			GFX_TEX_COORD=TEXTURE_PACK(inttot16(0), inttot16(64));
			GFX_VERTEX10=NORMAL_PACK(-(1<<5), (1<<5), 0);
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
