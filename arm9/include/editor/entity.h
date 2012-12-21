#ifndef __ENTITY9__
#define __ENTITY9__

#define ENTITYCOLLECTIONNUM 128

typedef enum
{
	lightEntity,
	enemyEntity,
}entity_type;

typedef struct
{
	int32 intensity;
}lightData_struct;

typedef struct
{
	u8 type;
}enemyData_struct;

typedef struct
{
	vect3D position, origin;
	entity_type type;
	mtlImg_struct* mtl;
	bool selected;
	void* data;
	bool used;
}entity_struct;

typedef struct
{
	entity_struct entity[ENTITYCOLLECTIONNUM];
	u16 num;
}entityCollection_struct;

void initEntity(entity_struct* e);
void initEnemy(entity_struct* e, u8 type);
void initLight(entity_struct* e, int32 intensity);
void initEntityCollection(entityCollection_struct* ec);
void wipeEntityCollection(entityCollection_struct* ec);
entity_struct* createEntity(entityCollection_struct* ec, vect3D position);
entity_struct* createLight(entityCollection_struct* ec, vect3D position, int32 intensity);
entity_struct* createEnemy(entityCollection_struct* ec, vect3D position, u8 type);
void removeEntity(entityCollection_struct* ec, entity_struct* e);
void drawEntity(entity_struct* e);
void drawEntityCollection(entityCollection_struct* ec);
void renderEntityCollection(entityCollection_struct* ec);
entity_struct* collideEntityCollection(entityCollection_struct* ec, int px, int py);

#endif
