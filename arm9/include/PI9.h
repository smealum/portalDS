#ifndef PI9_H
#define PI9_H

#include "math.h"
#include "../../arm7/include/PIC.h"

#define NUMOBJECTS (8)

typedef struct
{
	int32 transformationMatrix[9];
	modelInstance_struct modelInstance;
	vect3D position, size;
	s16 groundID;
	int32 mass;
	bool used;
	u8 id;
}OBB_struct;

typedef struct
{
	vect3D position, size;
	bool touched;
	bool used;
	u16 id;
}AAR_struct;

extern AAR_struct aaRectangles[NUMAARS];

void initPI9(void);
void startPI(void);
void pausePI(void);
void listenPI9(void);
void updateOBBs(void);
void applyForce(u8 id, vect3D pos, vect3D v);
s16 createAAR(vect3D size, vect3D pos, vect3D normal);
void addPlatform(u8 id, vect3D orig, vect3D dest, bool BAF);
void changePlatform(u8 id, vect3D pos);
void resetPortalsPI(void);

void killBox(OBB_struct* o);
void resetBox(OBB_struct* o, vect3D pos);
OBB_struct* createBox(vect3D pos, int32 mass, md2Model_struct* model);
void getBoxAABB(OBB_struct* o, vect3D* s);
bool intersectAABBAAR(vect3D o1, vect3D s1, vect3D o2, vect3D sp);
bool intersectOBBPortal(portal_struct* p, OBB_struct* o);
void ejectPortalOBBs(portal_struct* p);

void drawOBBs(void);
void drawAARs(void);

#endif