#include "common/general.h"

static sguiButton_struct simpleButtons[NUMSIMPLEBUTTONS];

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

void initSimpleButton(sguiButton_struct* b, vect3D p, const char* str, buttonTargetFunction f)
{
	if(!b)return;

	b->position=p;
	b->string=(char*)str;
	b->size=vect((strlen(str)*8+SIMPLEBUTTONMARGINX*2),(SIMPLEBUTTONSIZEY),0);
	b->targetFunction=f;
	b->active=false;
	b->used=true;

	b->mtl=NULL;
	b->mtlOffset=b->mtlSize=vect(0,0,0);
}

sguiButton_struct* createSimpleButton(vect3D p, const char* str, buttonTargetFunction f)
{
	int i;
	for(i=0;i<NUMSIMPLEBUTTONS;i++)
	{
		if(!simpleButtons[i].used)
		{
			initSimpleButton(&simpleButtons[i], p, str, f);
			return &simpleButtons[i];
		}
	}
	return NULL;
}

void simpleButtonSetImage(sguiButton_struct* b, mtlImg_struct* mtl, vect3D o, vect3D s)
{
	if(!b || !mtl)return;

	b->mtl=mtl;
	b->mtlOffset=o;
	b->mtlSize=s;
}


bool updateSimpleButton(sguiButton_struct* b, s16 x, s16 y)
{

	if(!b || !b->used)return false;

	bool ret=false;
	if(!(x==-1 && y==-1) && x>=b->position.x && x<b->position.x+b->size.x && y>=b->position.y && y<b->position.y+b->size.y)
	{
		b->active=true;
		ret=true;
	}else if(b->active && x==-1 && y==-1){
		if(b->targetFunction)b->targetFunction(b);
		b->active=false;
	}else b->active=false;

	return ret;
}

bool updateSimpleButtons(s16 x, s16 y)
{
	int i;
	bool ret=false;
	for(i=0;i<NUMSIMPLEBUTTONS;i++)
	{
		if(simpleButtons[i].used)ret=updateSimpleButton(&simpleButtons[i], x, y)||ret;
	}
	return ret;
}

bool updateSimpleGui(s16 x, s16 y)
{
	return updateSimpleButtons(x,y);
}

void drawSimpleButton(sguiButton_struct* b)
{
	if(!b || !b->used)return;

	glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);

	glPushMatrix();
		glTranslate3f32(inttof32(b->position.x),inttof32(b->position.y),0);
		if(b->string)drawString(b->string, RGB15(31,31,31), inttof32(1), inttof32(SIMPLEBUTTONMARGINX), inttof32(SIMPLEBUTTONMARGINY));

		unbindMtl();
		glTranslate3f32(0,0,-256);
		glScalef32(inttof32(b->size.x),inttof32(b->size.y),inttof32(1));

		if(b->active)GFX_COLOR=RGB15(31,0,0);
		else GFX_COLOR=RGB15(31,31,31);

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

void cleanUpSimpleButtons(void)
{
	int i;
	for(i=0;i<NUMSIMPLEBUTTONS;i++)
	{
		simpleButtons[i].used=false;
		simpleButtons[i].string=NULL;
	}
}
