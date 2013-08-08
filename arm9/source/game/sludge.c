#include "game/game_main.h"

rectangleList_struct sludgeRectangleList;

void initSludge(void)
{
	initRectangleList(&sludgeRectangleList);
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

	// NOGBA("p %d %d %d",pos.x,pos.y,pos.z);
	// NOGBA("s %d %d %d",size.x,size.y,size.z);

	glBegin(GL_QUAD);
		// GFX_TEX_COORD = t[0];
		glVertex3v16(pos.x, pos.y, pos.z);

		// GFX_TEX_COORD = t[1];
		glVertex3v16(pos.x, pos.y, pos.z+size.z);

		// GFX_TEX_COORD = t[2];
		glVertex3v16(pos.x+size.x, pos.y, pos.z+size.z);

		// GFX_TEX_COORD = t[3];
		glVertex3v16(pos.x+size.x, pos.y, pos.z);
}

void drawSludge(room_struct* r)
{
	if(!r)return;
	listCell_struct* lc=sludgeRectangleList.first;

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);

	unbindMtl();
	glPushMatrix();
		glTranslate3f32(TILESIZE*2*r->position.x, 0, TILESIZE*2*r->position.y);
		glTranslate3f32(-TILESIZE,0,-TILESIZE);
		glScalef32((TILESIZE*2)<<7,(HEIGHTUNIT)<<7,(TILESIZE*2)<<7);
		while(lc)
		{
			drawSludgeRectangle(&lc->data);
			lc=lc->next;
		}
	glPopMatrix(1);
}
