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
	entityType_struct* type;
	bool used, placed;
}entity_struct;

void initEntities(void);
entity_struct* createEntity(vect3D pos, u8 type, bool placed);
entity_struct* collideLineEntities(vect3D o, vect3D v, vect3D p1, vect3D p2, int32* d);
void changeEntityType(entity_struct* e, u8 type);
void drawEntities(void);

#endif