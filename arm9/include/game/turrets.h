#ifndef TURRETS_H
#define TURRETS_H

#define NUMTURRETS (8)
#define TURRETMASS (inttof32(1))

typedef struct
{
	OBB_struct* OBB;
	vect3D laserOrigin, laserDestination;
	vect3D laserOrigin2, laserDestination2;
	bool laserThroughPortal;
	u8 counter;
	bool used;
}turret_struct;

void initTurrets(void);
turret_struct* createTurret(room_struct* r, vect3D position);
void updateTurrets(void);
void drawTurretsStuff(void);

#endif