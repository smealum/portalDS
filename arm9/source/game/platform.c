#include "game/game_main.h"

#define LOGOSIZE (192)
#define LOGOHEIGHT (64)
#define PLATFORMHEIGHT (32)

platform_struct platform[NUMPLATFORMS];
mtlImg_struct* platformTexture;
md2Model_struct platformModel;

void initPlatforms(void)
{
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		platform[i].used=false;
		platform[i].id=i;
	}
	
	platformTexture=createTexture("logo.pcx","textures");
	loadMd2Model("models/platform.md2", "", &platformModel);
	generateModelDisplayLists(&platformModel, false, 0);
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
	
	u8 id=pf->id+50;
	
	glPushMatrix();
		u32 params=POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_ID(id)|POLY_TOON_HIGHLIGHT;
		setupObjectLighting(NULL, pf->position, &params);
		
		glTranslate3f32(pf->position.x,pf->position.y,pf->position.z);
		GFX_COLOR=RGB15(28,30,31);
		renderModelFrameInterp(0, 0, 0, &platformModel, params, false, NULL);
		
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(id));
		applyMTL(platformTexture);
		glBegin(GL_QUAD);
			GFX_TEX_COORD = TEXTURE_PACK(0<<4, 0<<4);
			glVertex3v16(-LOGOSIZE, LOGOHEIGHT, LOGOSIZE);
			GFX_TEX_COORD = TEXTURE_PACK(31<<4, 0<<4);
			glVertex3v16(LOGOSIZE, LOGOHEIGHT, LOGOSIZE);
			GFX_TEX_COORD = TEXTURE_PACK(31<<4, 31<<4);
			glVertex3v16(LOGOSIZE, LOGOHEIGHT, -LOGOSIZE);
			GFX_TEX_COORD = TEXTURE_PACK(0<<4, 31<<4);
			glVertex3v16(-LOGOSIZE, LOGOHEIGHT, -LOGOSIZE);
			
		glPolyFmt(POLY_ALPHA(16) | POLY_ID(31) | POLY_CULL_BACK);
		unbindMtl();
		glBegin(GL_QUAD);
			//top
			// GFX_NORMAL=NORMAL_PACK(0, inttov10(1)-1, 0);
			glVertex3v16(-PLATFORMSIZE, PLATFORMHEIGHT, PLATFORMSIZE);
			glVertex3v16(PLATFORMSIZE, PLATFORMHEIGHT, PLATFORMSIZE);
			glVertex3v16(PLATFORMSIZE, PLATFORMHEIGHT, -PLATFORMSIZE);
			glVertex3v16(-PLATFORMSIZE, PLATFORMHEIGHT, -PLATFORMSIZE);
			
			//bottom
			// GFX_NORMAL=NORMAL_PACK(0, -(inttov10(1)-1), 0);
			glVertex3v16(-PLATFORMSIZE, -PLATFORMHEIGHT, -PLATFORMSIZE);
			glVertex3v16(PLATFORMSIZE, -PLATFORMHEIGHT, -PLATFORMSIZE);
			glVertex3v16(PLATFORMSIZE, -PLATFORMHEIGHT, PLATFORMSIZE);
			glVertex3v16(-PLATFORMSIZE, -PLATFORMHEIGHT, PLATFORMSIZE);
			
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
