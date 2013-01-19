#ifndef ENERGYBALL_H
#define ENERGYBALL_H

#define NUMENERGYDEVICES (8)
#define NUMENERGYBALLS (8)

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

typedef struct
{
	vect3D position, direction;
	u16 life;
	bool used;
}energyBall_struct;

void initEnergyBalls(void);
energyDevice_struct* createEnergyDevice(room_struct* r, vect3D pos, deviceOrientation_type or, bool type);
void drawEnergyDevices(void);
void updateEnergyDevices(void);

energyBall_struct* createEnergyBall(vect3D pos, vect3D dir);
void drawEnergyBalls(void);
void updateEnergyBalls(void);

#endif