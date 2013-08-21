#include "common/general.h"

extern state_struct* currentState;
state_struct gameState;
state_struct menuState;
state_struct editorState;

void doSPALSH()
{
	setBrightness(3,-16);
	swiWaitForVBlank();
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);
	
    vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
    vramSetBankC(VRAM_C_SUB_BG);

	int bg = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	struct gl_texture_t* spalsh=(struct gl_texture_t *)ReadPCXFile("spalsh.pcx","");
	
	dmaCopy(spalsh->texels, bgGetGfxPtr(bg), 256*192);
	dmaCopy(spalsh->palette, BG_PALETTE, 256*2);
	
	freePCX(spalsh);

	int bg_sub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0,0);
	struct gl_texture_t* spalsh_sub=(struct gl_texture_t *)ReadPCXFile("spalsh_bottom.pcx","");
	
	dmaCopy(spalsh_sub->texels, bgGetGfxPtr(bg_sub), 256*192);
	dmaCopy(spalsh_sub->palette, BG_PALETTE_SUB, 256*2);
	
	freePCX(spalsh_sub);
	
	fadeIn();
	int i;
	for(i=0;i<60;i++)swiWaitForVBlank();
	fadeOut();
}

int main(int argc, char **argv)
{
	initHardware();
	initFilesystem(argc, argv);
	
	initAudio();
	
	createState(&gameState, initGame, gameFrame, killGame, gameVBL);
	createState(&menuState, initMenu, menuFrame, killMenu, menuVBL);
	createState(&editorState, initEditor, editorFrame, killEditor, editorVBL);

	doSPALSH();
	
	//TEMP DEBUG
	scanKeys();
	scanKeys();
	scanKeys();
	scanKeys();

	if(keysHeld() & KEY_SELECT)changeState(&editorState);
	else changeState(&gameState);

	// changeState(&editorState);
	changeState(&menuState);
	applyState();

	while(1)
	{
		currentState->init();
		while(currentState->used)currentState->frame();
		currentState->kill();
		applyState();
	}

	return 0;
}
