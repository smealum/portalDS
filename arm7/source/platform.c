#include "stdafx.h"

platform_struct platform[NUMPLATFORMS];

void initPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		platform[i].used=false;
	}
}

void createPlatform(u16 id, vect3D pos)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(!platform[i].used)
		{
			createAAR(id, vect(pos.x-PLATFORMSIZE*4,pos.y,pos.z-PLATFORMSIZE*4), vect(PLATFORMSIZE*2*4,0,PLATFORMSIZE*2*4), vect(0,inttof32(1),0));
			platform[i].position=pos;
			platform[i].aarID=id;
			platform[i].used=true;
			return;
		}
	}
}

void updatePlatform(u8 id, vect3D pos)
{
	if(id>=NUMPLATFORMS)return;
	
	updateAAR(platform[id].aarID, vect(pos.x-PLATFORMSIZE*4,pos.y,pos.z-PLATFORMSIZE*4));
	platform[id].position=pos;
}

void collideSpherePlatform(vect3D* p, int32 radius, int32 sqRadius, u8 id)
{
	if(id>=NUMPLATFORMS)return;
	platform_struct* pf=&platform[id];
	
	vect3D v=vect(p->x-pf->position.x,0,p->z-pf->position.z);
	if(v.x<-PLATFORMSIZE*4)v.x=-PLATFORMSIZE*4;
	else if(v.x>PLATFORMSIZE*4)v.x=PLATFORMSIZE*4;
	if(v.z<-PLATFORMSIZE*4)v.z=-PLATFORMSIZE*4;
	else if(v.z>PLATFORMSIZE*4)v.z=PLATFORMSIZE*4;
	v=vectDifference(addVect(v,pf->position),*p);
	int32 sqd=mulf32(v.x,v.x)+mulf32(v.y,v.y)+mulf32(v.z,v.z);
	if(sqd<sqRadius)
	{
		int32 sqd=(v.x*v.x)+(v.y*v.y)+(v.z*v.z);
		u32 d=sqrtv(sqd);
		v=divideVect(vectMult(vect(v.x,v.y,v.z),-((radius<<6)-d)),d);
		*p=addVect(*p,v);
	}
}

void collideSpherePlatforms(vect3D* p, int32 radius) //TEMP, change radius with vect3D size...
{
	int32 sqr=mulf32(radius,radius);
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)collideSpherePlatform(p,radius,sqr,i);
	}
}
