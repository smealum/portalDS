#include "game/game_main.h"

platform_struct platform[NUMPLATFORMS];

void initPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		platform[i].used=false;
	}
}

void initPlatform(platform_struct* pf, vect3D orig, vect3D dest, u8 id, bool BAF)
{
	if(!pf)return;
	
	pf->id=id;
	pf->origin=orig;
	pf->destination=dest;
	
	pf->position=orig;
	pf->velocity=vectDivInt(normalize(vectDifference(dest,orig)),256);
	
	pf->direction=true;
	pf->touched=false;
	pf->backandforth=true;
	
	addPlatform(vectMultInt(orig,4)); //TEMP
	
	pf->used=true;
}

platform_struct* createPlatform(vect3D orig, vect3D dest, bool BAF)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(!platform[i].used)
		{
			initPlatform(&platform[i],orig,dest,i,BAF);
			return &platform[i];
		}
	}
	return NULL;
}

void drawPlatform(platform_struct* pf)
{
	if(!pf)return;
	
	glPushMatrix();
		glTranslate3f32(pf->position.x,pf->position.y,pf->position.z);
		//TEMP
		glBegin(GL_QUAD);
			GFX_TEX_COORD = TEXTURE_PACK(0<<6, 0<<6);
			glVertex3v16(-PLATFORMSIZE, 0, -PLATFORMSIZE);

			GFX_TEX_COORD = TEXTURE_PACK(64<<6, 0<<6);
			glVertex3v16(PLATFORMSIZE, 0, -PLATFORMSIZE);

			GFX_TEX_COORD = TEXTURE_PACK(64<<6, 64<<6);
			glVertex3v16(PLATFORMSIZE, 0, PLATFORMSIZE);

			GFX_TEX_COORD = TEXTURE_PACK(0<<6, 64<<6);
			glVertex3v16(-PLATFORMSIZE, 0, PLATFORMSIZE);
	glPopMatrix(1);
}

void drawPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)drawPlatform(&platform[i]);
	}
}

void updatePlatform(platform_struct* pf)
{
	if(!pf)return;
	
	if(pf->active)
	{
		player_struct* p=getPlayer();
		
		if(pf->touched && p->object->position.y>pf->position.y+p->object->radius*2)
		{
			p->object->position=addVect(p->object->position,pf->velocity);
		}else if(pf->oldTouched){
			p->object->speed=addVect(p->object->speed,pf->velocity);
		}
		
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
		changePlatform(pf->id,vectMultInt(pf->position,4));
	}
	
	pf->oldTouched=pf->touched;
	pf->touched=false;
}

void updatePlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(platform[i].used)updatePlatform(&platform[i]);
	}
}
