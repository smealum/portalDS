#include "neweditor/editor.h"

editorRoom_struct testRoom;

void newEditorVBL(void)
{

}

void initNewEditor(void)
{
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
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

void newEditorFrame(void)
{
	scanKeys();
	GFX_CLEAR_COLOR=0;
	
	glOrthof32(inttof32(-128), inttof32(127),inttof32(-96), inttof32(95), inttof32(1)/10, inttof32(100));
	
	glPushMatrix();
		
		drawEditorRoom(&testRoom);
	glPopMatrix(1);
}

void killNewEditor(void)
{
	freeEditorRoom(&testRoom);
}


