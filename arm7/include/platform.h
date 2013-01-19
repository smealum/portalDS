#ifndef PLATFORM7_H
#define PLATFORM7_H

typedef struct
{
	vect3D position;
	u16 aarID;
	bool used;
}platform_struct;

void initPlatforms(void);
void updatePlatform(u8 id, vect3D pos);
void createPlatform(u16 id, vect3D pos);
void collideSpherePlatforms(vect3D* p, int32 radius);

#endif