#include "menu/menu_main.h"


camera_struct menuCamera;
char menuScreenText[MENUSCREENLINES][MENUSCREENCHARS];


static md2Model_struct GLaDOSmodel, domeModel, lairModel, cubeModel;
static modelInstance_struct GLaDOSmodelInstance;
static menuBox_struct menuBoxes[NUMMENUBOXES];

/**
 * Moves and rotates a 3D box.
 * \param[inout] mb box to draw
 */
static void updateMenuBox(menuBox_struct* mb);
/**
 * Moves and rotates all 3D boxes.
 * \param[inout] mb box to draw
 */
static void updateMenuBoxes(void);

/**
 * Creates a new box in the array menuBoxes,
 * in the first unused location.
 */
static void createMenuBox(void);

/**
 * Initialises a box.
 *
 * \param[in] mb box to initialise
 */
static void initMenuBox(menuBox_struct* mb);

/**
 * Draws a background 3D box
 * \param[in] mb box to draw
 */
static void drawMenuBox(const menuBox_struct* mb);
/**
 * Draws all background 3D boxes
 */
static void drawMenuBoxes(void);


static void drawScreenText(void);

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


static void updateMenuBox(menuBox_struct* mb)
{
	if(!mb)return;

	mb->angle.x+=mb->anglespeed.x;
	mb->angle.y+=mb->anglespeed.y;
	mb->angle.z+=mb->anglespeed.z;

	mb->progress++;

	if(mb->progress>MENUBOXSPEED)
	{
		// if mb->progress >= MENUBOXSPEED, the box reached its destination
		mb->used=false;
	}
}

static void updateMenuBoxes(void)
{
	int i;
	for(i=0;i<NUMMENUBOXES;i++)
	{
		if(menuBoxes[i].used)updateMenuBox(&menuBoxes[i]);
	}
}

static void initMenuBox(menuBox_struct* mb)
{
	if(!mb)return;

	mb->anglespeed.x=(rand()%MENUBOXANGLESPEED)-MENUBOXANGLESPEED/2;
	mb->anglespeed.y=(rand()%MENUBOXANGLESPEED)-MENUBOXANGLESPEED/2;
	mb->anglespeed.z=(rand()%MENUBOXANGLESPEED)-MENUBOXANGLESPEED/2;

	mb->progress=0;
	mb->used=true;
}

static void createMenuBox(void)
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


static void drawMenuBox(const menuBox_struct* mb)
{
	if(!mb)return;
	vect3D boxOrigin={-9984,11392,-14016};
	vect3D boxDestination={-9984,-576,-14016};

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

static void drawMenuBoxes(void)
{
	int i;
	for(i=0;i<NUMMENUBOXES;i++)
	{
		if(menuBoxes[i].used)drawMenuBox(&menuBoxes[i]);
	}
}


static void drawScreenText(void)
{
	vect3D textPosition={-8256,6784,-14848};
	vect3D textAngle={0,-1664,0};
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


void initScreenList(screenList_struct* sl, char* title, char** list, int l)
{
	if(!sl || !title || !list)return;

	strcpy(sl->title,title);
	sl->list=list;
	sl->length=l;
	sl->offset=0;
	sl->cursor=0;
}

void screenListMove(screenList_struct* sl, s8 move)
{
	if(!sl)return;

	sl->cursor+=move;
	if(sl->cursor<=0)sl->cursor=0;
	if(sl->cursor>=sl->length)sl->cursor=sl->length-1;

	if(sl->cursor<sl->offset || sl->cursor>=sl->offset+MENUSCREENLINES-1)
	{
		sl->offset+=move;
		if(sl->offset<=0)sl->offset=0;
		if(sl->offset>sl->length-(MENUSCREENLINES-1))sl->offset=sl->length-(MENUSCREENLINES-1);
	}
}

void updateScreenList(screenList_struct* sl)
{
	if(!sl || !sl->title || !sl->list)return;

	resetSceneScreen();

	strcpy(menuScreenText[0],sl->title);

	int i; int j=0;
	for(i=sl->offset;i<min(sl->length,sl->offset+MENUSCREENLINES-1);i++)
	{
		char tempString[MENUSCREENCHARS-2];

		if(strlen(sl->list[i])>=MENUSCREENCHARS-2)
		{
			//TODO : scrolling effect would be cool (to read the whole thing)
			memcpy(tempString,sl->list[i],MENUSCREENCHARS-1-2);
			tempString[MENUSCREENCHARS-2-1]='\0';
			tempString[MENUSCREENCHARS-2-2]='.';
			tempString[MENUSCREENCHARS-2-3]='.';
		}else strcpy(tempString, sl->list[i]);

		if(i==sl->cursor)sprintf(menuScreenText[j+1],"* %s",tempString);
		else sprintf(menuScreenText[j+1],"  %s",tempString);
		j++;
	}
}

