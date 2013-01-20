#ifndef PI7_H
#define PI7_H

typedef struct
{
	vect3D position;
}player_struct;

typedef struct portal_struct
{
	vect3D position, normal, plane[2];
	int32 cos, sin;
	AAR_struct guideAAR[4];
	struct portal_struct* targetPortal;
}portal_struct;

#include "PIC.h"

extern player_struct player;
extern portal_struct portal[2];

void initPI7(void);
void listenPI7(void);

#endif