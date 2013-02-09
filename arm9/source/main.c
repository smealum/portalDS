#include "common/general.h"

extern state_struct* currentState;
state_struct gameState;
state_struct editorState;

int main(int argc, char **argv)
{
	initHardware();
	initFilesystem(argc, argv);
	
	initAudio();
	
	createState(&gameState, initGame, gameFrame, killGame, gameVBL);
	createState(&editorState, initEditor, editorFrame, killEditor, editorVBL);
	
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
