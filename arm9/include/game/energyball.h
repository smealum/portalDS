#ifndef ENERGYBALL_H
#define ENERGYBALL_H



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
	rectangle_struct* surface;
	vect3D position;
	activator_struct activator;
	bool type; //true=launcher
	bool active;
	bool used;
	u8 id;
}energyDevice_struct;

typedef struct
{
	modelInstance_struct modelInstance;
	energyDevice_struct* launcher;
	vect3D position, direction;
	int32 speed;
	u16 maxLife, life;
	bool used;
	u8 id;
}energyBall_struct;

void initEnergyBalls(void);
void freeEnergyBalls(void);
energyDevice_struct* createEnergyDevice(room_struct* r, vect3D pos, deviceOrientation_type or, bool type);
void drawEnergyDevices(void);
void updateEnergyDevices(void);

energyBall_struct* createEnergyBall(energyDevice_struct* launcher, vect3D pos, vect3D dir, u16 life);
void drawEnergyBalls(void);
void updateEnergyBalls(void);

#endif