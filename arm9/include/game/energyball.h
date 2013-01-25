#ifndef ENERGYBALL_H
#define ENERGYBALL_H

#define NUMENERGYDEVICES (8)
#define NUMENERGYBALLS (8)

typedef enum
{
	pX=0,
	mX=1,
	pY=2,
	mY=3,
	pZ=4,
	mZ=5
}deviceOrientation_type;

typedef struct
{
	modelInstance_struct modelInstance;
	deviceOrientation_type orientation;
	vect3D position;
	bool type; //true=launcher
	bool used;
	u8 id;
}energyDevice_struct;

typedef struct
{
	modelInstance_struct modelInstance;
	vect3D position, direction;
	int32 speed;
	u16 life;
	bool used;
	u8 id;
}energyBall_struct;

void initEnergyBalls(void);
energyDevice_struct* createEnergyDevice(room_struct* r, vect3D pos, deviceOrientation_type or, bool type);
void drawEnergyDevices(void);
void updateEnergyDevices(void);

energyBall_struct* createEnergyBall(vect3D pos, vect3D dir);
void drawEnergyBalls(void);
void updateEnergyBalls(void);

#endif