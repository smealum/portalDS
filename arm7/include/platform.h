#ifndef PLATFORM7_H
#define PLATFORM7_H

typedef struct
{
	vect3D position, velocity;
	vect3D origin, destination;
	bool direction; //true=orig->dest
	bool active, backandforth;
	// u16 aarID;
	AAR_struct AAR;
	bool used;
}platform_struct;

extern platform_struct platform[NUMPLATFORMS];

void initPlatforms(void);
void movePlatform(u8 id, vect3D pos);
void updatePlatforms(void);
void createPlatform(u16 id, vect3D orig, vect3D dest, bool BAF);
void collideOBBPlatforms(OBB_struct* o, vect3D* v);
void togglePlatform(u8 id, bool active);

#endif