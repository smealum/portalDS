#ifndef ENTITY_H
#define ENTITY_H

#define NUMENTITYTYPES (14)

enum
{
	pX_mask = 1,
	mX_mask = 1<<1,
	pY_mask = 1<<2,
	mY_mask = 1<<3,
	pZ_mask = 1<<4,
	mZ_mask = 1<<5
}directionMask_type;

struct entity_struct;

typedef void(*entityFunction)(struct entity_struct*);
typedef void(*entityMoveFunction)(struct entity_struct*, bool);
typedef bool(*entityMoveCheckFunction)(struct entity_struct*, vect3D, u8 dir);

typedef struct
{
	const char* modelName;
	const char* textureName;
	u8 possibleDirections;
	contextButton_struct* contextButtonsArray;
	u8 numButtons;
	entityFunction specialInit;
	entityMoveFunction specialMove;
	entityMoveCheckFunction specialMoveCheck;
	bool removeTarget, rotate;
	md2Model_struct model;
	u8 id;
}entityType_struct;

typedef struct entity_struct
{
	u8 direction;
	vect3D position;
	entityType_struct* type;
	blockFace_struct* blockFace;
	struct entity_struct* target;
	u8 writeID;
	bool used, placed;
}entity_struct;

extern contextButton_struct ballLauncherButtonArray[];
extern contextButton_struct ballCatcherButtonArray[];
extern contextButton_struct button1ButtonArray[];
extern contextButton_struct button2ButtonArray[];
extern contextButton_struct turretButtonArray[];
extern contextButton_struct cubeButtonArray[];
extern contextButton_struct gridButtonArray[];
extern contextButton_struct platformButtonArray[];
extern contextButton_struct doorButtonArray[];
extern contextButton_struct lightButtonArray[];

void initEntities(void);
void removeEntities(void);
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
void removeEntity(entity_struct* e);

int32 getGridLength(entity_struct* e);

#endif