#include "menu/menu_main.h"

md2Model_struct GLaDOSmodel, domeModel, lairModel;
modelInstance_struct GLaDOSmodelInstance;
camera_struct menuCamera;

u32 lightAngle=54912;
cameraState_struct tempState={(vect3D){0,0,0}, (vect3D){0,0,0}};
cameraTransition_struct testTransition;
bool tempbool=false;

void initMenu(void)
{
	lcdMainOnTop();
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_5_2D);
	
	vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_SUB_BG,VRAM_D_TEXTURE);	
	
	initD3D();

	glInit();
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ANTIALIAS);
	glEnable(GL_BLEND);
	glEnable(GL_OUTLINE);
	
	glClearPolyID(63);
	glClearDepth(0x7FFF);
	glViewport(0,0,255,191);
	
	initVramBanks(2);
	initTextures();

	initCamera(&menuCamera);

	menuCamera.position=vect(0,inttof32(1),inttof32(1));

	loadMd2Model("menu/glados.md2","glados_256.pcx",&GLaDOSmodel);
	loadMd2Model("menu/lair_dome.md2","lairdome_256.pcx",&domeModel);
	loadMd2Model("menu/lairv2.md2","gladoslair.pcx",&lairModel);

	initModelInstance(&GLaDOSmodelInstance,&GLaDOSmodel);
	changeAnimation(&GLaDOSmodelInstance,1,false);

	initMenuButtons();

	//TEMP
	glLight(0, RGB15(31,31,31), cosLerp(4096), 0, sinLerp(4096));

	glMaterialf(GL_AMBIENT, RGB15(8,8,8));
	glMaterialf(GL_DIFFUSE, RGB15(24,24,24));
	glMaterialf(GL_SPECULAR, RGB15(0,0,0));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));

	glSetToonTableRange(0, 15, RGB15(8,8,8)); //TEMP?
	glSetToonTableRange(16, 31, RGB15(24,24,24)); //TEMP?

	applyCameraState(&menuCamera,&cameraStates[0]);
	tempState=cameraStates[1];
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[0],64);

	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

touchPosition currentTouch;

void menuFrame(void)
{
	if(!d3dScreen)initProjectionMatrix(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));
	else initProjectionMatrixBottom(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));

	projectCamera(&menuCamera);
	glLoadIdentity();

	glLight(0, RGB15(31,31,31), cosLerp(lightAngle)>>3, 0, sinLerp(lightAngle)>>3);

	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);

	scanKeys();
	touchRead(&currentTouch);

	if(keysHeld() & KEY_R)tempState.position=addVect(tempState.position,vect(0,0,inttof32(1)/64));
	if(keysHeld() & KEY_L)tempState.position=addVect(tempState.position,vect(0,0,-inttof32(1)/64));
	if(keysHeld() & KEY_UP)tempState.position=addVect(tempState.position,vect(0,inttof32(1)/64,0));
	if(keysHeld() & KEY_DOWN)tempState.position=addVect(tempState.position,vect(0,-inttof32(1)/64,0));
	if(keysHeld() & KEY_RIGHT)tempState.position=addVect(tempState.position,vect(inttof32(1)/64,0,0));
	if(keysHeld() & KEY_LEFT)tempState.position=addVect(tempState.position,vect(-inttof32(1)/64,0,0));

	//TEMP (updateCamera stuff)
		menuCamera.viewPosition=menuCamera.position;

	// if(keysHeld() & KEY_A)lightAngle+=128;
	// else if(keysHeld() & KEY_B)lightAngle-=128;
	if(keysHeld() & KEY_A)tempState.angle.x+=64;
	if(keysHeld() & KEY_B)tempState.angle.x-=64;
	if(keysHeld() & KEY_X)tempState.angle.y+=64;
	if(keysHeld() & KEY_Y)tempState.angle.y-=64;
	if(keysHeld() & KEY_START)tempState.angle.z+=64;
	if(keysHeld() & KEY_SELECT)tempState.angle.z-=64;

	// if(keysUp() & KEY_TOUCH)
	// {
	// 	if(tempbool)testTransition=startCameraTransition(&cameraStates[1],&cameraStates[0],64);
	// 	else testTransition=startCameraTransition(&cameraStates[0],&cameraStates[1],64);

	// 	tempbool^=1;
	// }

	if(!(keysHeld() & KEY_TOUCH)) updateSimpleGui(-1, -1);
	else updateSimpleGui(currentTouch.px, currentTouch.py);

	// applyCameraState(&menuCamera,&tempState);
	updateCameraTransition(&menuCamera,&testTransition);
	
	glPushMatrix();
		glScalef32(inttof32(16),inttof32(16),inttof32(16));
		transformCamera(&menuCamera);

		renderModelFrameInterp(GLaDOSmodelInstance.currentFrame,GLaDOSmodelInstance.nextFrame,GLaDOSmodelInstance.interpCounter, &GLaDOSmodel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(2), false, NULL, RGB15(31,31,31));
		renderModelFrameInterp(0, 0, 0, &domeModel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(0), false, NULL, RGB15(31,31,31));
		renderModelFrameInterp(0, 0, 0, &lairModel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(1), false, NULL, RGB15(31,31,31));
	glPopMatrix(1);

	switch(d3dScreen)
	{
		case true:
			drawSimpleGui();
			break;
		default:
			break;
	}
	
	glFlush(0);
	swiWaitForVBlank();

	updateAnimation(&GLaDOSmodelInstance);

	updateD3D();
}

void killMenu(void)
{

}

void menuVBL(void)
{

}

