#ifndef TURRETS_H
#define TURRETS_H

#define NUMTURRETS (8)
#define TURRETMASS (inttof32(1))

typedef enum
{
	TURRET_CLOSED,
	TURRET_OPENING,
	TURRET_OPEN,
	TURRET_CLOSING
}turretState_type;

typedef struct
{
	OBB_struct* OBB;
	vect3D laserOrigin, laserDestination;
	vect3D laserOrigin2, laserDestination2;
	turretState_type state;
	bool laserThroughPortal;
	u8 counter;
	bool used;
}turret_struct;

void initTurrets(void);
turret_struct* createTurret(room_struct* r, vect3D position, u8 d);
void updateTurrets(void);
void drawTurretsStuff(void);

#endif