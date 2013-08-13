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

extern u8 logoAlpha;

void startMenuPlayButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[4],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
	logoAlpha=0;
}

menuButton_struct startMenuPage[]={(menuButton_struct){"START", startMenuPlayButtonFunction}};
u8 startMenuPageLength=arrayLength(startMenuPage);

void mainMenuPlayButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[1],48);
	setupMenuPage(playMenuPage, playMenuPageLength);
}

void mainMenuCreateButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
}

menuButton_struct mainMenuPage[]={(menuButton_struct){"Options", NULL}, (menuButton_struct){"Create", mainMenuCreateButtonFunction}, (menuButton_struct){"Play", mainMenuPlayButtonFunction}};
u8 mainMenuPageLength=arrayLength(mainMenuPage);

void playMenuBackButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

menuButton_struct playMenuPage[]={(menuButton_struct){"Back", playMenuBackButtonFunction}, (menuButton_struct){"Select Level", NULL}, (menuButton_struct){"Campaign", NULL}};
u8 playMenuPageLength=arrayLength(playMenuPage);

void createMenuBackButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[0],64);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

void createMenuNewLevelButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[3],64);
	setupMenuPage(newLevelMenuPage, newLevelMenuPageLength);
}

menuButton_struct createMenuPage[]={(menuButton_struct){"Back", createMenuBackButtonFunction}, (menuButton_struct){"Load Level", NULL}, (menuButton_struct){"New level", createMenuNewLevelButtonFunction}};
u8 createMenuPageLength=arrayLength(createMenuPage);

void newLevelMenuBackButtonFunction(void)
{
	testTransition=startCameraTransition(&cameraStates[3],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
}

menuButton_struct newLevelMenuPage[]={(menuButton_struct){"Back", newLevelMenuBackButtonFunction}};
u8 newLevelMenuPageLength=arrayLength(newLevelMenuPage);
