#include "game/game_main.h"

light_struct lights[NUMLIGHTS];

void initLights(void)
{
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		lights[i].used=false;
	}
}

void initLight(light_struct* l, vect3D pos, int32 intensity)
{
	if(!l)return;

	l->intensity=intensity;
	l->position=pos;
	l->used=true;
}

light_struct* createLight(vect3D pos, int32 intensity)
{
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		if(!lights[i].used)
		{
			initLight(&lights[i],pos,intensity);
			return &lights[i];
		}
	}
	return NULL;
}

void getClosestLights(vect3D tilepos, light_struct** ll1, light_struct** ll2, light_struct** ll3, int32* dd1, int32* dd2, int32* dd3)
{
	if(!ll1 || !ll2 || !ll3 || !dd1 || !dd2 || !dd3)return;
	light_struct *l1, *l2, *l3;
	int32 d1, d2, d3;
	d1=d2=d3=inttof32(300);
	l1=l2=l3=NULL;
	int i;
	for(i=0;i<NUMLIGHTS;i++)
	{
		if(lights[i].used)
		{
			light_struct* l=&lights[i];
			int32 d=(tilepos.x-l->position.x)*(tilepos.x-l->position.x)+((tilepos.y-l->position.y)*(tilepos.y-l->position.y))/16+(tilepos.z-l->position.z)*(tilepos.z-l->position.z);
			if(d<d1){d3=d2;d2=d1;d1=d;
					l3=l2;l2=l1;l1=l;}
			else if(d<d2){d3=d2;d2=d;
					l3=l2;l2=l;}
			else if(d<d3){d3=d;
					l3=l;}
		}
	}
	*ll1=l1;
	*ll2=l2;
	*ll3=l3;
	*dd1=d1;
	*dd2=d2;
	*dd3=d3;
}
