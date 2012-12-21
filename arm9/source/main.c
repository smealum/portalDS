#include "common/general.h"

extern state_struct* currentState;
state_struct gameState;
state_struct editorState;
state_struct ACOTMenuState;

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
	createState(&editorState, initEditor, editorFrame, killEditor, editorVBL);
	createState(&ACOTMenuState, initACOTMenu, ACOTMenuFrame, killACOTMenu, ACOTMenuVBL);
	
	// doSPALSH();
	
	// changeState(&ACOTMenuState);
	// changeState(&editorState);
	changeState(&gameState);
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
