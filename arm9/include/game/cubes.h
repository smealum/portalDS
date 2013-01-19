#ifndef CUBES_H
#define CUBES_H

#define NUMCUBEDISPENSERS (8)

typedef struct
{
	vect3D position;
	modelInstance_struct modelInstance;
	bool companion;
	bool active, oldActive;
	OBB_struct* currentCube;
	bool used;
}cubeDispenser_struct;

void initCubes(void);
void drawCubeDispensers(void);
void updateCubeDispensers(void);
cubeDispenser_struct* createCubeDispenser(room_struct* r, vect3D pos, bool companion);

#endif