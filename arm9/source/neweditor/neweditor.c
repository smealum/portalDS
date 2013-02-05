#include "neweditor/editor.h"

editorRoom_struct testRoom;

void newEditorVBL(void)
{

}

void initNewEditor(void)
{
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_0_2D);
	
	glInit();
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ANTIALIAS);
	glEnable(GL_BLEND);
	glEnable(GL_OUTLINE);
	
	glClearColor(31,31,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);
	glViewport(0,0,255,191);
	
	initVramBanks(1);
	initTextures();	
	
	initBlocks();
	initEditorRoom(&testRoom);
}

int cnd=0;

void newEditorFrame(void)
{
	scanKeys();
	GFX_CLEAR_COLOR=0;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof32(inttof32(-128), inttof32(127),inttof32(-96), inttof32(95), inttof32(-100), inttof32(100));
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glPushMatrix();
		
		glScalef32(inttof32(10),inttof32(10),inttof32(10));
		glRotateYi(cnd+=64);
		glRotateXi(1024*4);
		// NOGBA("angle %d",cnd);
		drawEditorRoom(&testRoom);
	glPopMatrix(1);
	
	glFlush(0);
	
	swiWaitForVBlank();
}

void killNewEditor(void)
{
	freeEditorRoom(&testRoom);
}


