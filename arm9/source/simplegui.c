#include "common/general.h"

#define NUMSIMPLEBUTTONS (16)
#define BUTTONMARGINX (6)
#define BUTTONMARGINY (3)

sguiButton_struct simpleButtons[NUMSIMPLEBUTTONS];

void initSimpleButtons(void)
{
	int i;
	for(i=0;i<NUMSIMPLEBUTTONS;i++)
	{
		simpleButtons[i].used=false;
		simpleButtons[i].string=NULL;
	}
}

void initSimpleGui(void)
{
	initText();
	initSimpleButtons();
}

void initSimpleButton(sguiButton_struct* b, vect3D p, const char* str)
{
	if(!b)return;

	b->position=p;
	b->string=str;
	b->size=vect(inttof32(strlen(str)*8+BUTTONMARGINX*2),inttof32(8+BUTTONMARGINY*2),0);
	b->used=true;
}

sguiButton_struct* createSimpleButton(vect3D p, const char* str)
{
	int i;
	for(i=0;i<NUMSIMPLEBUTTONS;i++)
	{
		if(!simpleButtons[i].used)
		{
			initSimpleButton(&simpleButtons[i], p, str);
			return &simpleButtons[i];
		}
	}
	return NULL;
}

void drawSimpleButton(sguiButton_struct* b)
{
	if(!b || !b->used)return;

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);

	glPushMatrix();
		glTranslate3f32(inttof32(b->position.x),inttof32(b->position.y),0);
		if(b->string)drawString(b->string, RGB15(31,31,31), inttof32(1), inttof32(BUTTONMARGINX), inttof32(BUTTONMARGINY));

		unbindMtl();
		glScalef32(b->size.x,b->size.y,inttof32(1));
		glBegin(GL_QUADS);	
			GFX_VERTEX10=NORMAL_PACK(0,(1<<6),0);
			GFX_VERTEX10=NORMAL_PACK((1<<6),(1<<6),0);
			GFX_VERTEX10=NORMAL_PACK((1<<6),0,0);
			GFX_VERTEX10=NORMAL_PACK(0,0,0);
	glPopMatrix(1);
}

void drawSimpleButtons(void)
{
	int i;
	for(i=0;i<NUMSIMPLEBUTTONS;i++)
	{
		if(simpleButtons[i].used)drawSimpleButton(&simpleButtons[i]);
	}
}

void drawSimpleGui(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof32(inttof32(0), inttof32(255), inttof32(191), inttof32(0), -inttof32(1), inttof32(1));
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	drawSimpleButtons();
}
