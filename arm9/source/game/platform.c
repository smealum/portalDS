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

void initPlatform(platform_struct* pf, room_struct* r, vect3D orig, vect3D dest, u8 id, bool BAF)
{
	if(!pf || !r)return;
	
	orig=vect(orig.x+r->position.x, orig.y, orig.z+r->position.y);
	dest=vect(dest.x+r->position.x, dest.y, dest.z+r->position.y);

	pf->id=id;
	pf->origin=convertVect(orig);
	pf->destination=convertVect(dest);

	pf->position=pf->origin;
	pf->velocity=vectDivInt(normalize(vectDifference(pf->destination,pf->origin)),256);
	
	pf->direction=true;
	pf->touched=false;
	pf->backandforth=true;
	
	pf->oldactive=pf->active=false;
	
	addPlatform(id,vectMultInt(pf->origin,4),vectMultInt(pf->destination,4),BAF); //TEMP
	
	pf->used=true;
}

platform_struct* createPlatform(room_struct* r, vect3D orig, vect3D dest, bool BAF)
{
	if(!r)r=&gameRoom;
	int i;
	for(i=0;i<NUMPLATFORMS;i++)
	{
		if(!platform[i].used)
		{
			initPlatform(&platform[i],r,orig,dest,i,BAF);
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
		renderModelFrameInterp(0, 0, 0, &platformModel, params, false, NULL, RGB15(26,26,26));
		
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
	
	player_struct* p=getPlayer();
	
	if(pf->touched && p->object->position.y>pf->position.y+p->object->radius*2)
	{
		p->object->position=addVect(p->object->position,pf->velocity);
	}else if(pf->oldTouched){
		p->object->speed=addVect(p->object->speed,pf->velocity);
	}
	
	if(pf->oldactive!=pf->active)togglePlatform(pf->id, pf->active);
	
	pf->oldactive=pf->active;
	pf->velocity=vect(0,0,0);
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
