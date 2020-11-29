#include "menu/menu_main.h"

u32 lightAngle=54912;
cameraState_struct tempState={(vect3D){0,0,0}, (vect3D){0,0,0}};
cameraTransition_struct testTransition;
bool tempbool=false;

mtlImg_struct* logoMain;
mtlImg_struct* logoRotate;
u32 logoAngle=0;
u8 logoAlpha;

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

	initMenuScene();

	initMenuButtons();

	//TEMP
	glLight(0, RGB15(31,31,31), cosLerp(4096), 0, sinLerp(4096));

	glMaterialf(GL_AMBIENT, RGB15(8,8,8));
	glMaterialf(GL_DIFFUSE, RGB15(24,24,24));
	glMaterialf(GL_SPECULAR, RGB15(0,0,0));
	glMaterialf(GL_EMISSION, RGB15(0,0,0));

	glSetToonTableRange(0, 15, RGB15(8,8,8)); //TEMP?
	glSetToonTableRange(16, 31, RGB15(24,24,24)); //TEMP?

	applyCameraState(&menuCamera,&cameraStates[4]);
	tempState=cameraStates[4];
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[4],64);

	setupMenuPage(startMenuPage, startMenuPageLength);

	logoMain=createTexture("logo.pcx", "menu");
	logoRotate=createTexture("rotate_logo.pcx", "menu");
	logoAlpha=31;

	glSetOutlineColor(0,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(1,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(7,RGB15(0,0,0)); //TEMP?

	NOGBA("END mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	fadeIn();
}

touchPosition currentTouchA;

void drawLogo(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrthof32(inttof32(0), inttof32(255), inttof32(191), inttof32(0), -inttof32(1), inttof32(1));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		applyMTL(logoMain);
		glScalef32(inttof32(256),inttof32(128),inttof32(1));
		glBegin(GL_QUADS);
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
			glVertex3v16(inttof32(0), inttof32(0), inttof32(0));
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(256), inttot16(0));
			glVertex3v16(inttof32(1), inttof32(0), inttof32(0));
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(256), inttot16(128));
			glVertex3v16(inttof32(1), inttof32(1), inttof32(0));
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(128));
			glVertex3v16(inttof32(0), inttof32(1), inttof32(0));
	glPopMatrix(1);

	glPushMatrix();
		glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
		applyMTL(logoRotate);
		glTranslate3f32(inttof32(90+32),inttof32(28+32),-inttof32(1)/16);
		glRotateZi(logoAngle+=32);
		glTranslate3f32(-inttof32(32),-inttof32(32),0);
		glScalef32(inttof32(64),inttof32(64),inttof32(1));
		glBegin(GL_QUADS);
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(0));
			glVertex3v16(inttof32(0), inttof32(0), inttof32(0));
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(64), inttot16(0));
			glVertex3v16(inttof32(1), inttof32(0), inttof32(0));
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(64), inttot16(64));
			glVertex3v16(inttof32(1), inttof32(1), inttof32(0));
			GFX_TEX_COORD = TEXTURE_PACK(inttot16(0), inttot16(64));
			glVertex3v16(inttof32(0), inttof32(1), inttof32(0));
	glPopMatrix(1);
}

int cpt_debug_test = 0;
void menuFrame(void)
{
	cpt_debug_test++;
	if(!d3dScreen)initProjectionMatrix(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));
	else initProjectionMatrixBottom(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));

	projectCamera(&menuCamera);
	glLoadIdentity();

	glLight(0, RGB15(31,31,31), cosLerp(lightAngle)>>3, 0, sinLerp(lightAngle)>>3);

	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);

	scanKeys();
	touchRead(&currentTouchA);

	if((keysHeld() & KEY_R) && (keysHeld() & KEY_L))changeState(&menuState);

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
	else  updateSimpleGui((float)(currentTouchA.rawx)*256.0/(4080.0), (float)(currentTouchA.rawy)*192.0/3072.0);

	//updateSimpleGui(currentTouch.px, currentTouch.py);

	// applyCameraState(&menuCamera,&tempState);
	updateCameraTransition(&menuCamera,&testTransition);

	drawMenuScene();

	switch(d3dScreen)
	{
		case true:
			drawSimpleGui();
			updateMenuScene();
			break;
		default:
			if(logoAlpha)drawLogo();
			break;
	}

	glFlush(0);
	swiWaitForVBlank();

	updateD3D();
}

void killMenu(void)
{
	fadeOut();
	freeMenuScene();
	freeState(NULL);
}

void menuVBL(void)
{

}

