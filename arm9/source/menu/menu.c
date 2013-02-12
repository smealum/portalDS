#include "menu/menu_main.h"

md2Model_struct GLaDOSmodel, domeModel, lairModel;
modelInstance_struct GLaDOSmodelInstance;
camera_struct menuCamera;

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

	//TEMP
	glLight(0, RGB15(31,31,31), cosLerp(4096), 0, sinLerp(4096));

	glMaterialf(GL_AMBIENT, RGB15(8,8,8));
	glMaterialf(GL_DIFFUSE, RGB15(24,24,24));
	glMaterialf(GL_SPECULAR, RGB15(0,0,0));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));

	glSetToonTableRange(0, 15, RGB15(8,8,8)); //TEMP?
	glSetToonTableRange(16, 31, RGB15(24,24,24)); //TEMP?

	menuCamera.position=vect(156,8000,13000);
}

u32 lightAngle=54912;

void menuFrame(void)
{
	if(!d3dScreen)initProjectionMatrix(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));
	else initProjectionMatrixBottom(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));

	projectCamera(&menuCamera);
	glLoadIdentity();

	glLight(0, RGB15(31,31,31), cosLerp(lightAngle)>>3, 0, sinLerp(lightAngle)>>3);

	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);

	scanKeys();
	if(keysHeld() & KEY_R)moveCameraImmediate(&menuCamera, vect(0,0,inttof32(1)/64));
	if(keysHeld() & KEY_L)moveCameraImmediate(&menuCamera, vect(0,0,-inttof32(1)/64));
	
	if(keysHeld() & KEY_UP)moveCameraImmediate(&menuCamera, vect(0,inttof32(1)/64,0));
	else if(keysHeld() & KEY_DOWN)moveCameraImmediate(&menuCamera, vect(0,-inttof32(1)/64,0));
	if(keysHeld() & KEY_RIGHT)moveCameraImmediate(&menuCamera, vect(inttof32(1)/64,0,0));
	else if(keysHeld() & KEY_LEFT)moveCameraImmediate(&menuCamera, vect(-inttof32(1)/64,0,0));

	// if(keysHeld() & KEY_A)lightAngle+=128;
	// else if(keysHeld() & KEY_B)lightAngle-=128;
	if(keysHeld() & KEY_A)rotateMatrixY(menuCamera.transformationMatrix, 64, true);
	if(keysHeld() & KEY_Y)rotateMatrixY(menuCamera.transformationMatrix, -64, true);
	if(keysHeld() & KEY_X)rotateMatrixX(menuCamera.transformationMatrix, 64, false);
	if(keysHeld() & KEY_B)rotateMatrixX(menuCamera.transformationMatrix, -64, false);

	NOGBA("%d",lightAngle);
	
	glPushMatrix();
		glScalef32(inttof32(16),inttof32(16),inttof32(16));
		transformCamera(&menuCamera);

		renderModelFrameInterp(GLaDOSmodelInstance.currentFrame,GLaDOSmodelInstance.nextFrame,GLaDOSmodelInstance.interpCounter, &GLaDOSmodel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(2), false, NULL, RGB15(31,31,31));
		renderModelFrameInterp(0, 0, 0, &domeModel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(0), false, NULL, RGB15(31,31,31));
		renderModelFrameInterp(0, 0, 0, &lairModel, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0 | POLY_TOON_HIGHLIGHT | POLY_ID(1), false, NULL, RGB15(31,31,31));
	glPopMatrix(1);
	
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

