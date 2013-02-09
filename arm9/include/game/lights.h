#ifndef LIGHTS_H
#define LIGHTS_H

#define NUMLIGHTS 64

typedef struct
{
	vect3D position;
	int32 intensity;
	bool used;
}light_struct;

extern light_struct lights[NUMLIGHTS];

void initLights(void);
light_struct* createLight(vect3D pos, int32 intensity);
void getClosestLights(vect3D tilepos, light_struct** ll1, light_struct** ll2, light_struct** ll3, int32* dd1, int32* dd2, int32* dd3);

#endif
