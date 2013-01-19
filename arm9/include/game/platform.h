#ifndef PLATFORM_H
#define PLATFORM_H

#define NUMPLATFORMS (8)
#define PLATFORMSIZE (TILESIZE*3) //half

typedef struct
{
	vect3D position, velocity;
	vect3D origin, destination;
	bool direction; //true=orig->dest
	bool touched, oldTouched;
	bool active, backandforth;
	bool used;
}platform_struct;

void initPlatforms(void);
void drawPlatforms(void);
void updatePlatforms(void);
platform_struct* createPlatform(vect3D orig, vect3D dest, bool BAF);

#endif