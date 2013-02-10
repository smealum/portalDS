#include "editor/editor_main.h"

entityType_struct entityTypes[]={(entityType_struct){"", NULL}};

entity_struct entity[NUMENTITIES];

void initEntities(void)
{
	int i;
	for(i=0;i<NUMENTITIES;i++)
	{
		entity[i].used=false;
	}
}

void initEntity(entity_struct* e, vect3D pos)
{
	if(!e)return;

	e->position=pos;
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

void drawEntity(entity_struct* e)
{
	if(!e)return;

	glPushMatrix();
		editorRoomTransform();
		glTranslate3f32(e->position.x,e->position.y,e->position.z);
		GFX_BEGIN=GL_QUADS;
			GFX_VERTEX10=NORMAL_PACK(-(1<<5),-(1<<5), 0);
			GFX_VERTEX10=NORMAL_PACK( (1<<5),-(1<<5), 0);
			GFX_VERTEX10=NORMAL_PACK( (1<<5), (1<<5), 0);
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
