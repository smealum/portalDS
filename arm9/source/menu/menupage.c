#include "menu/menu_main.h"

void initMenuButtons(void)
{
	initSimpleGui();
}

void setupMenuPage(menuButton_struct* mp, u8 n)
{
	if(!mp || !n)return;

	cleanUpSimpleButtons();

	int i;
	for(i=0;i<n;i++)
	{
		createSimpleButton(vect(0,192-(i+1)*16,0), mp[i].string, mp[i].targetFunction);
	}
}

void mainMenuPlayButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[1],48);
	setupMenuPage(playMenuPage, playMenuPageLength);
}

menuButton_struct mainMenuPage[]={(menuButton_struct){"Options", NULL}, (menuButton_struct){"Create", NULL}, (menuButton_struct){"Play", mainMenuPlayButtonFunction}};
u8 mainMenuPageLength=arrayLength(mainMenuPage);

void playMenuBackButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

menuButton_struct playMenuPage[]={(menuButton_struct){"Back", playMenuBackButtonFunction}, (menuButton_struct){"Select Level", NULL}, (menuButton_struct){"Campaign", NULL}};
u8 playMenuPageLength=arrayLength(playMenuPage);
