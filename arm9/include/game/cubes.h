#ifndef CUBES_H
#define CUBES_H

#define NUMCUBEDISPENSERS (8)

typedef struct
{
	vect3D position;
	modelInstance_struct modelInstance;
	bool companion;
	bool active, oldActive;
	rectangle_struct* openingRectangle;
	OBB_struct* currentCube;
	bool used;
	u8 id;
}cubeDispenser_struct;

void initCubes(void);
void freeCubes(void);
void drawCubeDispensers(void);
void updateCubeDispensers(void);
void resetCubeDispenserCube(cubeDispenser_struct* cd);
cubeDispenser_struct* createCubeDispenser(room_struct* r, vect3D pos, bool companion);

#endif