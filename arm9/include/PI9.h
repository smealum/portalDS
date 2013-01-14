#ifndef PI9_H
#define PI9_H

#include "math.h"
#include "../../arm7/include/PIC.h"

#define NUMOBJECTS (16)
#define NUMAARS (150)

typedef struct
{
	int32 transformationMatrix[9];
	modelInstance_struct modelInstance;
	vect3D position, size;
	int32 mass;
	bool used;
	u8 id;
}OBB_struct;

typedef struct
{
	vect3D position, size;
	bool used;
	u16 id;
}AAR_struct;

void initPI9(void);
void startPI(void);
void pausePI(void);
void listenPI9(void);
void applyForce(u8 id, vect3D pos, vect3D v);
void createBox(vect3D pos, int32 mass, md2Model_struct* model);
void createAAR(vect3D size, vect3D pos, vect3D normal);

void drawOBBs(void);
void drawAARs(void);

#endif