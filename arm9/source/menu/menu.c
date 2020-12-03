#include "menu/menu_main.h"



#define MENU_LIGHTANGLE 54912

cameraTransition_struct testTransition;
u8 logoAlpha;



static mtlImg_struct* logoMain;
static mtlImg_struct* logoRotate;

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

	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[4],64);

	setupHomeMenuPage();

	logoMain=createTexture("logo.pcx", "menu");
	logoRotate=createTexture("rotate_logo.pcx", "menu");
	logoAlpha=31;

	glSetOutlineColor(0,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(1,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(7,RGB15(0,0,0)); //TEMP?

	NOGBA("END mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	fadeIn();
}

void drawLogo(void)
{
	static u32 logoAngle=0;
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

void menuFrame(void)
{
	touchPosition currentTouch;
	if(!d3dScreen)initProjectionMatrix(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));
	else initProjectionMatrixBottom(&menuCamera, 70*90, inttof32(4)/3, inttof32(2), inttof32(1000));

	projectCamera(&menuCamera);
	glLoadIdentity();

	glLight(0, RGB15(31,31,31), cosLerp(MENU_LIGHTANGLE)>>3, 0, sinLerp(MENU_LIGHTANGLE)>>3);

	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);

	scanKeys();
	touchRead(&currentTouch);

	//TEMP (updateCamera stuff)
	menuCamera.viewPosition=menuCamera.position;



	if(!(keysHeld() & KEY_TOUCH)) updateSimpleGui(-1, -1);
	else  updateSimpleGui((float)(currentTouch.rawx)*256.0/(4080.0), (float)(currentTouch.rawy)*192.0/3072.0);

	/**
	 * Moves camera
	 */
	updateCameraTransition(&menuCamera,&testTransition);

	drawMenuScene();

	if(d3dScreen)
	{
			drawSimpleGui();
			updateMenuScene();


	}
	else
	{
		if(logoAlpha)
		{
			drawLogo();
		}

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

