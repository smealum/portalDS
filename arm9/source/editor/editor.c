#include "editor/editor_main.h"

void editorVBL(void)
{

}

void initEditor(void)
{
	lcdMainOnBottom();
	videoSetMode(MODE_5_3D);
	videoSetModeSub(MODE_0_2D);
	
	vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_LCD,VRAM_D_MAIN_BG_0x06000000);	
	
	glInit();
	
	glEnable(GL_TEXTURE_2D);
	// glEnable(GL_ANTIALIAS);
	glEnable(GL_BLEND);
	glEnable(GL_OUTLINE);
	
	glClearPolyID(63);
	glClearDepth(0x7FFF);
	glViewport(0,0,255,191);
	
	initVramBanks(1);
	initTextures();
	
	initRoomEdition();
}

int cnd=0;

void editorFrame(void)
{
	scanKeys();
	GFX_CLEAR_COLOR=RGB15(27,27,27)|(31<<16);
	
	updateRoomEditor();
	drawRoomEditor();
	
	swiWaitForVBlank();
}

void killEditor(void)
{
	freeRoomEditor();
}


