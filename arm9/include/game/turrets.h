#ifndef TURRETS_H
#define TURRETS_H

#define NUMTURRETS (8)
#define TURRETMASS (inttof32(1))

typedef struct
{
	OBB_struct* OBB;
	u8 counter;
	bool used;
}turret_struct;

void initTurrets(void);
turret_struct* createTurret(vect3D position);
void updateTurrets(void);

#endif