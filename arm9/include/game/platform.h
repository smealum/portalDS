#ifndef PLATFORM_H
#define PLATFORM_H

#define NUMPLATFORMS (8)

typedef struct
{
	vect3D position;
	bool used;
}platform_struct;

void initPlatforms(void);
void drawPlatforms(void);
void updatePlatforms(void);
platform_struct* createPlatform(vect3D pos);

#endif