#include "game/game_main.h"

rectangleList_struct sludgeRectangleList;
mtlImg_struct* sludgeMtl;

void initSludge(void)
{
	initRectangleList(&sludgeRectangleList);

	sludgeMtl=createTexture("sludge1.pcx", "textures");

	if(!sludgeMtl)return;
	sludgeMtl->param|=(1<<18)|(1<<19); //flip repeats, see http://nocash.emubase.de/gbatek.htm#ds3dtextureattributes
}

void freeSludge(void)
{
	while(sludgeRectangleList.num)popRectangle(&sludgeRectangleList);
}

void addSludgeRectangle(rectangle_struct* rec)
{
	if(!rec)return;

	addRectangle(*rec, &sludgeRectangleList);
}

void drawSludgeRectangle(rectangle_struct* rec)
{
	if(!rec)return;

	vect3D pos=vectMultInt(rec->position,32);
	vect3D size=vectMultInt(rec->size,32);

	size=addVect(pos,size);

	glBegin(GL_QUAD);
		GFX_TEX_COORD = TEXTURE_PACK(pos.x*6, pos.z*6);
		glVertex3v16(pos.x, pos.y, pos.z);

		GFX_TEX_COORD = TEXTURE_PACK(pos.x*6, size.z*6);
		glVertex3v16(pos.x, pos.y, size.z);

		GFX_TEX_COORD = TEXTURE_PACK(size.x*6, size.z*6);
		glVertex3v16(size.x, pos.y, size.z);

		GFX_TEX_COORD = TEXTURE_PACK(size.x*6, pos.z*6);
		glVertex3v16(size.x, pos.y, pos.z);
}

int sludgeAnimationCounter=0;

void drawSludge(room_struct* r)
{
	if(!r)return;
	listCell_struct* lc=sludgeRectangleList.first;

	glPolyFmt(POLY_ALPHA(15) | POLY_CULL_BACK);
	glColor(RGB15(31,31,31));
	applyMTL(sludgeMtl);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	sludgeAnimationCounter+=48;

	glTranslate3f32(cosLerp(sludgeAnimationCounter)<<7,sinLerp(sludgeAnimationCounter)<<6,0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

		glTranslate3f32(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y);
		glTranslate3f32(-TILESIZE,0,-TILESIZE);
		glTranslate3f32(0,-SLUDGEMARGIN,0); //arbitrary, just so that the sludge is slightly lower than surrounding ground...
		glScalef32((TILESIZE*2)<<7,(HEIGHTUNIT)<<7,(TILESIZE*2)<<7);

		while(lc)
		{
			drawSludgeRectangle(&lc->data);
			lc=lc->next;
		}

	glMatrixMode(GL_TEXTURE);
	glPopMatrix(1);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix(1);
}

bool sludgeBoxCollision(rectangle_struct* rec, OBB_struct* o)
{
	if(!rec || !o)return false;
	
	vect3D p=vectDivInt(o->position,4);
	vect3D s;
	getBoxAABB(o,&s);
	vect3D pos=addVect(convertVect(rec->position),convertVect(vect(gameRoom.position.x,0,gameRoom.position.y)));
	vect3D sp=convertSize(rec->size);

	if(sp.x<0){pos.x+=sp.x;sp.x=-sp.x;}
	if(sp.z<0){pos.z+=sp.z;sp.z=-sp.z;}

	sp.x/=2; sp.z/=2;
	pos.x+=sp.x; pos.z+=sp.z;
	pos.y-=SLUDGEMARGIN;

	return intersectAABBAAR(p, s, pos, sp);
}

bool collideBoxSludge(OBB_struct* o)
{
	if(!o)return false;
	listCell_struct* lc=sludgeRectangleList.first;
	while(lc)
	{
		if(sludgeBoxCollision(&lc->data, o))return true;
		lc=lc->next;
	}
	return false;
}
