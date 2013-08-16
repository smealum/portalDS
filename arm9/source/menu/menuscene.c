#include "menu/menu_main.h"

md2Model_struct GLaDOSmodel, domeModel, lairModel, cubeModel;
modelInstance_struct GLaDOSmodelInstance;

camera_struct menuCamera;

menuBox_struct menuBoxes[NUMMENUBOXES];
char menuScreenText[MENUSCREENLINES][MENUSCREENCHARS];

void initMenuBoxes(void)
{
	int i;
	for(i=0;i<NUMMENUBOXES;i++)
	{
		menuBoxes[i].used=false;
	}
}

void initMenuScene(void)
{
	initCamera(&menuCamera);

	menuCamera.position=vect(0,inttof32(1),inttof32(1));

	loadMd2Model("menu/glados.md2","glados_256.pcx",&GLaDOSmodel);
	loadMd2Model("menu/lair_dome.md2","lairdome_256.pcx",&domeModel);
	loadMd2Model("menu/lairv2.md2","gladoslair.pcx",&lairModel);
	loadMd2Model("models/cube.md2","storagecube.pcx",&cubeModel);

	initModelInstance(&GLaDOSmodelInstance,&GLaDOSmodel);
	changeAnimation(&GLaDOSmodelInstance,1,false);
}

void freeMenuScene(void)
{
	freeMd2Model(&GLaDOSmodel);
	freeMd2Model(&domeModel);
	freeMd2Model(&lairModel);
	freeMd2Model(&cubeModel);
}

vect3D boxOrigin={-9984,11392,-14016};
vect3D boxDestination={-9984,-576,-14016};

void updateMenuBox(menuBox_struct* mb)
{
	if(!mb)return;

	mb->angle.x+=mb->anglespeed.x;
	mb->angle.y+=mb->anglespeed.y;
	mb->angle.z+=mb->anglespeed.z;

	mb->progress++;

	if(mb->progress>MENUBOXSPEED)mb->used=false;
}

void updateMenuBoxes(void)
{
	int i;
	for(i=0;i<NUMMENUBOXES;i++)
	{
		if(menuBoxes[i].used)updateMenuBox(&menuBoxes[i]);
	}
}

void initMenuBox(menuBox_struct* mb)
{
	if(!mb)return;

	mb->anglespeed.x=(rand()%MENUBOXANGLESPEED)-MENUBOXANGLESPEED/2;
	mb->anglespeed.y=(rand()%MENUBOXANGLESPEED)-MENUBOXANGLESPEED/2;
	mb->anglespeed.z=(rand()%MENUBOXANGLESPEED)-MENUBOXANGLESPEED/2;

	mb->progress=0;
	mb->used=true;
}

void createMenuBox(void)
{
	int i;
	for(i=0;i<NUMMENUBOXES;i++)
	{
		if(!menuBoxes[i].used){initMenuBox(&menuBoxes[i]);return;}
	}
}

void updateMenuScene(void)
{
	updateAnimation(&GLaDOSmodelInstance);
	updateMenuBoxes();

	if(!(rand()%MENUBOXFREQUENCY))createMenuBox();
}

void drawMenuBox(menuBox_struct* mb)
{
	if(!mb)return;

	vect3D pos=vect(boxOrigin.x+((boxDestination.x-boxOrigin.x)*mb->progress)/MENUBOXSPEED,
					boxOrigin.y+((boxDestination.y-boxOrigin.y)*mb->progress)/MENUBOXSPEED,
					boxOrigin.z+((boxDestination.z-boxOrigin.z)*mb->progress)/MENUBOXSPEED);

	glPushMatrix();
		glTranslate3f32(pos.x,pos.y,pos.z);
		glScalef32(inttof32(3)/2,inttof32(3)/2,inttof32(3)/2);
		glRotateXi(mb->angle.x);
		glRotateYi(mb->angle.y);
		glRotateZi(mb->angle.z);
		renderModelFrameInterp(0, 0, 0, &cubeModel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(2), false, NULL, RGB15(31,31,31));
	glPopMatrix(1);
}

void drawMenuBoxes(void)
{
	int i;
	for(i=0;i<NUMMENUBOXES;i++)
	{
		if(menuBoxes[i].used)drawMenuBox(&menuBoxes[i]);
	}
}

vect3D textPosition={-8256,6784,-14848};
vect3D textAngle={0,-1664,0};

void drawScreenText(void)
{
	glPushMatrix();
		glTranslate3f32(textPosition.x,textPosition.y,textPosition.z);
		glRotateXi(8192*2);
		glRotateYi(textAngle.y);
		int i;
		for(i=0;i<MENUSCREENLINES;i++)drawString(menuScreenText[i], RGB15(31,31,31), inttof32(1)/96, 0, 512*i);
	glPopMatrix(1);
}

void drawMenuScene(void)
{
	glPushMatrix();
		glScalef32(inttof32(16),inttof32(16),inttof32(16));
		transformCamera(&menuCamera);

		renderModelFrameInterp(GLaDOSmodelInstance.currentFrame,GLaDOSmodelInstance.nextFrame,GLaDOSmodelInstance.interpCounter, &GLaDOSmodel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(2), false, NULL, RGB15(31,31,31));
		renderModelFrameInterp(0, 0, 0, &domeModel, POLY_ALPHA(31) | POLY_CULL_FRONT | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(0), false, NULL, RGB15(31,31,31));
		renderModelFrameInterp(0, 0, 0, &lairModel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(1), false, NULL, RGB15(31,31,31));

		drawMenuBoxes();

		drawScreenText();
	glPopMatrix(1);
}

void resetSceneScreen(void)
{
	int i; for(i=0;i<MENUSCREENLINES;i++)menuScreenText[i][0]='\0';
}

void updateScreenList(char* title, char** list, int l, int p, int c)
{
	if(!title || !list)return;

	resetSceneScreen();

	strcpy(menuScreenText[0],title);

	int i;
	for(i=p;i<min(l,p+MENUSCREENLINES-1);i++)
	{
		char tempString[MENUSCREENCHARS-2];

		if(strlen(list[i])>=MENUSCREENCHARS-2)
		{
			memcpy(tempString,list[i],MENUSCREENCHARS-1-2);
			tempString[MENUSCREENCHARS-2-1]='\0';
			tempString[MENUSCREENCHARS-2-2]='.';
			tempString[MENUSCREENCHARS-2-3]='.';
		}else strcpy(tempString, list[i]);

		if(i==c)sprintf(menuScreenText[i+1],"* %s",tempString);
		else sprintf(menuScreenText[i+1],"  %s",tempString);
	}
}
