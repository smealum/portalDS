#ifndef ENTITY_H
#define ENTITY_H

#define NUMENTITIES (64)

enum
{
	pX_mask = 1,
	mX_mask = 1<<1,
	pY_mask = 1<<2,
	mY_mask = 1<<3,
	pZ_mask = 1<<4,
	mZ_mask = 1<<5
}directionMask_type;

typedef struct
{
	const char* modelName;
	const char* textureName;
	u8 possibleDirections;
	md2Model_struct model;
	u8 id;
}entityType_struct;

typedef struct
{
	u8 direction;
	vect3D position;
	entityType_struct* type;
	blockFace_struct* blockFace;
	bool used, placed;
}entity_struct;

void initEntities(void);
entity_struct* createEntity(vect3D pos, u8 type, bool placed);
entity_struct* collideLineEntities(vect3D o, vect3D v, vect3D p1, vect3D p2, int32* d);
blockFace_struct* getEntityBlockFace(entity_struct* e, blockFace_struct* l);
void getEntityBlockFaces(blockFace_struct* l);
void changeEntityType(entity_struct* e, u8 type);
bool moveEntityToBlockFace(entity_struct* e, blockFace_struct* bf);
void moveEntitiesRange(vect3D o, vect3D s, vect3D u);
void getEntityBlockFacesRange(blockFace_struct* l, vect3D o, vect3D s, bool delete);
void generateLightsFromEntities(void);
void drawEntities(void);

#endif