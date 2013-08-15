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

void startMenuPlayButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[4],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
	logoAlpha=0;
}

menuButton_struct startMenuPage[]={(menuButton_struct){"START", (buttonTargetFunction)startMenuPlayButtonFunction}};
u8 startMenuPageLength=arrayLength(startMenuPage);

void mainMenuPlayButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[1],48);
	setupMenuPage(playMenuPage, playMenuPageLength);
}

void mainMenuCreateButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
}

menuButton_struct mainMenuPage[]={(menuButton_struct){"Options", NULL}, (menuButton_struct){"Create", (buttonTargetFunction)mainMenuCreateButtonFunction}, (menuButton_struct){"Play", (buttonTargetFunction)mainMenuPlayButtonFunction}};
u8 mainMenuPageLength=arrayLength(mainMenuPage);

void playMenuCampaignButtonFunction(sguiButton_struct* b)
{
	changeState(&gameState);
}

void playMenuLoadLevelButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[3],64);
	setupMenuPage(selectLevelMenuPage, selectLevelMenuPageLength);

	resetSceneScreen();
	sprintf(menuScreenText[0],"Select level");
	sprintf(menuScreenText[1],"  - test");
	sprintf(menuScreenText[2],"  - lala");
}

void playMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

menuButton_struct playMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)playMenuBackButtonFunction}, (menuButton_struct){"Select Level", playMenuLoadLevelButtonFunction}, (menuButton_struct){"Campaign", playMenuCampaignButtonFunction}};
u8 playMenuPageLength=arrayLength(playMenuPage);

void createMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[0],64);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

void createMenuNewLevelButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[3],64);
	setupMenuPage(newLevelMenuPage, newLevelMenuPageLength);

	resetSceneScreen();
	sprintf(menuScreenText[0],"Level name :");
	sprintf(menuScreenText[1],"  ");
	
	setupKeyboard(&menuScreenText[1][2], 10, 16, 16);
}

menuButton_struct createMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)createMenuBackButtonFunction}, (menuButton_struct){"Load Level", NULL}, (menuButton_struct){"New level", (buttonTargetFunction)createMenuNewLevelButtonFunction}};
u8 createMenuPageLength=arrayLength(createMenuPage);

void newLevelMenuOKButtonFunction(sguiButton_struct* b)
{
	changeState(&editorState);
}

void newLevelMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[3],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
}

menuButton_struct newLevelMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)newLevelMenuBackButtonFunction}, (menuButton_struct){"OK", (buttonTargetFunction)newLevelMenuOKButtonFunction}};
u8 newLevelMenuPageLength=arrayLength(newLevelMenuPage);

void selectLevelMenuOKButtonFunction(sguiButton_struct* b)
{
	changeState(&gameState);
}

void selectLevelMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[3],&cameraStates[1],64);
	setupMenuPage(playMenuPage, playMenuPageLength);
}

menuButton_struct selectLevelMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)selectLevelMenuBackButtonFunction}, (menuButton_struct){"OK", (buttonTargetFunction)selectLevelMenuOKButtonFunction}};
u8 selectLevelMenuPageLength=arrayLength(selectLevelMenuPage);
