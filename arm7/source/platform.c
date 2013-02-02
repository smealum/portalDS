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

void initPlatform(platform_struct* pf, vect3D orig, vect3D dest, bool BAF)
{
	if(!pf)return;
	
	pf->origin=orig;
	pf->destination=dest;
	
	pf->position=orig;
	pf->velocity=vectDivInt(normalize(vectDifference(dest,orig)),256*2);
	
	pf->direction=true;
	pf->backandforth=BAF;
	
	pf->active=false;
	
	pf->used=true;
}

void createPlatform(u16 id, vect3D orig, vect3D dest, bool BAF)
{
	int i=id;
		platform[i].AAR.position=vect(orig.x-PLATFORMSIZE*4,orig.y,orig.z-PLATFORMSIZE*4);
		platform[i].AAR.size=vect(PLATFORMSIZE*2*4,0,PLATFORMSIZE*2*4);
		platform[i].AAR.normal=vect(0,inttof32(1),0);
		platform[i].AAR.used=true;
		initPlatform(&platform[i], orig, dest, BAF);
		platform[i].used=true;
}

void movePlatform(u8 id, vect3D pos)
{
	if(id>=NUMPLATFORMS)return;
	
	// updateAAR(platform[id].aarID, vect(pos.x-PLATFORMSIZE*4,pos.y,pos.z-PLATFORMSIZE*4));
	platform[id].AAR.position=vect(pos.x-PLATFORMSIZE*4,pos.y,pos.z-PLATFORMSIZE*4);
	platform[id].position=pos;
}

void togglePlatform(u8 id, bool active)
{
	if(id>=NUMPLATFORMS)return;
	
	platform[id].active=active;
}

void updatePlatform(platform_struct* pf)
{
	if(!pf)return;
	
	if(pf->active)
	{
		switch(pf->direction)
		{
			case true:
				if(dotProduct(vectDifference(pf->position,pf->destination),pf->velocity)>0)
				{
					if(pf->backandforth)
					{
						pf->velocity=vectMultInt(pf->velocity,-1);
						pf->direction=false;
					}else{
						pf->velocity=vect(0,0,0);
						pf->active=false;
					}
				}
				break;
			default:
				if(dotProduct(vectDifference(pf->position,pf->origin),pf->velocity)>0)
				{
					pf->velocity=vectMultInt(pf->velocity,-1);
					pf->direction=true;
				}
				break;
		}
		pf->position=addVect(pf->position,pf->velocity);
	}
	pf->AAR.position=vect(pf->position.x-PLATFORMSIZE*4,pf->position.y,pf->position.z-PLATFORMSIZE*4);
}

void updatePlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)updatePlatform(&platform[i]);
	}
}

void collideOBBPlatform(OBB_struct* o, platform_struct* pf, vect3D* v)
{
	if(!o || !pf || !pf->used)return;
	
	if(AAROBBContacts(&pf->AAR, o, v, false) && pf->active)
	{
		o->position=addVect(o->position,pf->velocity);
		o->sleep=false;
		o->counter=false;
	}
}

void collideOBBPlatforms(OBB_struct* o, vect3D* v)
{
	if(!o || !v)return;
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)collideOBBPlatform(o,&platform[i],v);
	}
}
