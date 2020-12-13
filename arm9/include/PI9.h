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
	void* spawner;
	s16 groundID;
	int32 mass;
	s32 startAngle;
	bool inPortal;
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
void resetAllPI(void);
void listenPI9(void);
void updateOBBs(void);
void applyForce(u8 id, vect3D pos, vect3D v);
s16 createAAR(vect3D size, vect3D pos, vect3D normal);
void addPlatform(u8 id, vect3D orig, vect3D dest, bool BAF);
void changePlatform(u8 id, vect3D pos);
void resetPortalsPI(void);
void toggleAAR(s16 id);

void killBox(OBB_struct* o);
void resetBox(OBB_struct* o, vect3D pos);
OBB_struct* createBox(vect3D pos, int32 mass, md2Model_struct* model, s32 angle);
void getBoxAABB(OBB_struct* o, vect3D* s);
bool intersectAABBAAR(vect3D o1, vect3D s1, vect3D o2, vect3D sp);
bool intersectOBBPortal(portal_struct* p, OBB_struct* o);
void ejectPortalOBBs(portal_struct* p);

int32 distanceLinePoint(vect3D o, vect3D u, vect3D p);
OBB_struct* collideRayBoxes(vect3D o, vect3D u, int32 l);

void drawOBBs(void);
void drawAARs(void);

void makeGrid(void);
void setVelocity(u8 id, vect3D v);
void togglePlatform(u8 id, bool active);
void updatePortalPI(u8 id, vect3D pos, vect3D normal, vect3D plane0);
void multTMatrix(int32* m);

void drawOBB(OBB_struct* o);
#endif