#ifndef ENERGYBALL_H
#define ENERGYBALL_H

#define NUMENERGYDEVICES (8)

typedef enum
{
	pX,
	mX,
	pY,
	mY,
	pZ,
	mZ
}deviceOrientation_type;

typedef struct
{
	modelInstance_struct modelInstance;
	deviceOrientation_type orientation;
	vect3D position;
	bool type; //true=launcher
	bool used;
}energyDevice_struct;

void initEnergyBalls(void);
energyDevice_struct* createEnergyDevice(room_struct* r, vect3D pos, deviceOrientation_type or, bool type);
void drawEnergyDevices(void);
void updateEnergyDevices(void);

#endif