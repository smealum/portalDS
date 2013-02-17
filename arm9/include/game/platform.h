#ifndef PLATFORM_H
#define PLATFORM_H

typedef struct
{
	vect3D position, velocity;
	vect3D origin, destination;
	bool direction; //true=orig->dest
	bool touched, oldTouched;
	bool active, oldactive, backandforth;
	bool used;
	u8 id;
}platform_struct;

extern platform_struct platform[NUMPLATFORMS];

void initPlatforms(void);
void drawPlatforms(void);
void updatePlatforms(void);
platform_struct* createPlatform(room_struct* r, vect3D orig, vect3D dest, bool BAF);

#endif