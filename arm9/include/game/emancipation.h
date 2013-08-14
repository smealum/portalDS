#ifndef EMANCIPATION_H
#define EMANCIPATION_H

#define NUMEMANCIPATIONGRIDS (16)
#define NUMEMANCIPATORS (16)

#define EMANCIPATIONGRIDHEIGHT (HEIGHTUNIT*8)
#define BLACKENINGTIME (16)
#define FADINGTIME (24)

typedef struct
{
	vect3D position, velocity, axis;
	modelInstance_struct modelInstance;
	int32 transformationMatrix[9];
	u16 counter, angle;
	bool used;
}emancipator_struct;

typedef struct
{
	vect3D position;
	int32 length;
	bool direction; //true=Z, false=X
	bool used;
}emancipationGrid_struct;

void initEmancipation(void);
void freeEmancipation(void);

void createEmancipator(modelInstance_struct* mi, vect3D pos, int32* m);
void updateEmancipators(void);
void drawEmancipators(void);

void createEmancipationGrid(room_struct* r, vect3D pos, int32 l, bool dir);
void updateEmancipationGrids(void);
void drawEmancipationGrids(void);

void getEmancipationGridAAR(emancipationGrid_struct* eg, vect3D* pos, vect3D* sp);
bool collideBoxEmancipationGrids(OBB_struct* o);

#endif
