#ifndef ENTITY_H
#define ENTITY_H

#define NUMENTITIES (64)

typedef struct
{
	const char* spriteName;
	mtlImg_struct* spriteTexture;
}entityType_struct;

typedef struct
{
	vect3D position;
	bool used;
}entity_struct;

void initEntities(void);
entity_struct* createEntity(vect3D pos);
void drawEntities(void);

#endif