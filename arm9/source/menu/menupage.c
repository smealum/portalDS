#include "menu/menu_main.h"

#include <dirent.h>



/**  Structures that represents a menu button. */
typedef struct
{
	/** Displayed title of button */
	const char* string;
	/** Function to be executed when button is pressed */
	buttonTargetFunction targetFunction;
}menuButton_struct;


//extern u8 logoAlpha;


static void setupMenuPage(menuButton_struct* mp, u8 n);

static void startMenuPlayButtonFunction(sguiButton_struct* b);
static void mainMenuCreateButtonFunction(sguiButton_struct* b);
static void mainMenuPlayButtonFunction(sguiButton_struct* b);
static void playMenuCampaignButtonFunction(sguiButton_struct* b);
static void playMenuLoadLevelButtonFunction(sguiButton_struct* b);
static void playMenuBackButtonFunction(sguiButton_struct* b);
static void createMenuBackButtonFunction(sguiButton_struct* b);
static void createMenuNewLevelButtonFunction(sguiButton_struct* b);
static void createMenuLoadLevelButtonFunction(sguiButton_struct* b);
static void newLevelMenuOKButtonFunction(sguiButton_struct* b);
static void newLevelMenuBackButtonFunction(sguiButton_struct* b);
static void selectLevelMenuUpButtonFunction(sguiButton_struct* b);
static void selectLevelMenuDownButtonFunction(sguiButton_struct* b);
static void selectLevelMenuOKButtonFunction(sguiButton_struct* b);
static void selectLevelMenuBackButtonFunction(sguiButton_struct* b);
static void loadLevelMenuUpButtonFunction(sguiButton_struct* b);
static void loadLevelMenuDownButtonFunction(sguiButton_struct* b);
static void loadLevelMenuOKButtonFunction(sguiButton_struct* b);
static void loadLevelMenuBackButtonFunction(sguiButton_struct* b);


static void freeFileList(char** list, int length);
static int listFiles(char* path, char** list);

static char **testList=NULL;
static int testListCnt, testListCnt1;
static screenList_struct testScreenList;

static menuButton_struct startMenuPage[]={(menuButton_struct){"START", (buttonTargetFunction)startMenuPlayButtonFunction}};
static u8 startMenuPageLength=arrayLength(startMenuPage);
static menuButton_struct mainMenuPage[]={(menuButton_struct){"Options", NULL}, (menuButton_struct){"Create", (buttonTargetFunction)mainMenuCreateButtonFunction}, (menuButton_struct){"Play", (buttonTargetFunction)mainMenuPlayButtonFunction}};
static u8 mainMenuPageLength=arrayLength(mainMenuPage);
static menuButton_struct playMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)playMenuBackButtonFunction}, (menuButton_struct){"Select Level", playMenuLoadLevelButtonFunction}, (menuButton_struct){"Campaign", playMenuCampaignButtonFunction}};
static u8 playMenuPageLength=arrayLength(playMenuPage);
static menuButton_struct createMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)createMenuBackButtonFunction}, (menuButton_struct){"Load Level", createMenuLoadLevelButtonFunction}, (menuButton_struct){"New level", (buttonTargetFunction)createMenuNewLevelButtonFunction}};
static u8 createMenuPageLength=arrayLength(createMenuPage);
static menuButton_struct newLevelMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)newLevelMenuBackButtonFunction}, (menuButton_struct){"OK", (buttonTargetFunction)newLevelMenuOKButtonFunction}};
static u8 newLevelMenuPageLength=arrayLength(newLevelMenuPage);
static menuButton_struct selectLevelMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)selectLevelMenuBackButtonFunction}, (menuButton_struct){"OK", (buttonTargetFunction)selectLevelMenuOKButtonFunction}, (menuButton_struct){"Down", (buttonTargetFunction)selectLevelMenuDownButtonFunction}, (menuButton_struct){"Up", (buttonTargetFunction)selectLevelMenuUpButtonFunction}};
static u8 selectLevelMenuPageLength=arrayLength(selectLevelMenuPage);
static menuButton_struct loadLevelMenuPage[]={(menuButton_struct){"Back", (buttonTargetFunction)loadLevelMenuBackButtonFunction}, (menuButton_struct){"OK", (buttonTargetFunction)loadLevelMenuOKButtonFunction}, (menuButton_struct){"Down", (buttonTargetFunction)loadLevelMenuDownButtonFunction}, (menuButton_struct){"Up", (buttonTargetFunction)loadLevelMenuUpButtonFunction}};
static u8 loadLevelMenuPageLength=arrayLength(loadLevelMenuPage);

void initMenuButtons(void)
{
	initSimpleGui();
}

void setupHomeMenuPage(void)
{
	setupMenuPage(startMenuPage, startMenuPageLength);
}

static void setupMenuPage(menuButton_struct* mp, u8 n)
{
	if(!mp || !n)return;

	cleanUpSimpleButtons();

	int i;
	for(i=0;i<n;i++)
	{
		createSimpleButton(vect(0,192-(i+1)*16,0), mp[i].string, mp[i].targetFunction);
	}
}



static void startMenuPlayButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[4],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
	/* vanish logo */
	logoAlpha=0;
}



static void mainMenuPlayButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[1],48);
	setupMenuPage(playMenuPage, playMenuPageLength);
}

static void mainMenuCreateButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[0],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
}

static void playMenuCampaignButtonFunction(sguiButton_struct* b)
{
	setMapFilePath("maps/test1.map");
	changeState(&gameState);
}

static void playMenuLoadLevelButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[3],64);
	setupMenuPage(selectLevelMenuPage, selectLevelMenuPageLength);

	testListCnt=0;
	testListCnt1=0;

	testListCnt1=testListCnt+=listFiles("./maps", NULL);
	#ifndef FATONLY
		char str[255];
		sprintf(str,"%s/%s/maps",basePath,ROOT);
		testListCnt+=listFiles(str, NULL);
	#endif

	testList=malloc(sizeof(char*)*testListCnt);

	listFiles("./maps", testList);
	#ifndef FATONLY
		listFiles(str, &testList[testListCnt1]);
	#endif

	initScreenList(&testScreenList, "Select level", testList, testListCnt);
	updateScreenList(&testScreenList);
}

static void playMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[1],&cameraStates[0],48);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}


static void createMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[0],64);
	setupMenuPage(mainMenuPage, mainMenuPageLength);
}

static void createMenuNewLevelButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[3],64);
	setupMenuPage(newLevelMenuPage, newLevelMenuPageLength);

	resetSceneScreen();
	sprintf(menuScreenText[0],"Level name :");
	sprintf(menuScreenText[1],"  ");

	setupKeyboard(&menuScreenText[1][2], 10, 16, 16);
}

static void createMenuLoadLevelButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[2],&cameraStates[3],64);
	setupMenuPage(loadLevelMenuPage, loadLevelMenuPageLength);

	testListCnt=0;
	testListCnt1=0;

	char str[255];
	sprintf(str,"%s/%s/maps",basePath,ROOT);
	testListCnt1=testListCnt+=listFiles(str, NULL);

	testList=malloc(sizeof(char*)*testListCnt);

	listFiles(str, testList);

	initScreenList(&testScreenList, "Load level", testList, testListCnt);
	updateScreenList(&testScreenList);
}

static void newLevelMenuOKButtonFunction(sguiButton_struct* b)
{
	static char str[2048];
	sprintf(str,"%s/%s/maps/%s.map",basePath,ROOT,&menuScreenText[1][2]);

	setEditorMapFilePath(str);
	changeState(&editorState);
}

static void newLevelMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[3],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
}


static void selectLevelMenuUpButtonFunction(sguiButton_struct* b)
{
	screenListMove(&testScreenList, -1);
	updateScreenList(&testScreenList);
}

static void selectLevelMenuDownButtonFunction(sguiButton_struct* b)
{
	screenListMove(&testScreenList, 1);
	updateScreenList(&testScreenList);
}

static void selectLevelMenuOKButtonFunction(sguiButton_struct* b)
{
	static char str[2048];
	if(testScreenList.cursor<testListCnt1)sprintf(str,"./maps/%s",testScreenList.list[testScreenList.cursor]);
	else sprintf(str,"%s/%s/maps/%s",basePath,ROOT,testScreenList.list[testScreenList.cursor]);

	setMapFilePath(str);
	changeState(&gameState);
}

static void selectLevelMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[3],&cameraStates[1],64);
	setupMenuPage(playMenuPage, playMenuPageLength);
	freeFileList(testList, testListCnt);
	testList=NULL;
}


static void loadLevelMenuUpButtonFunction(sguiButton_struct* b)
{
	screenListMove(&testScreenList, -1);
	updateScreenList(&testScreenList);
}

static void loadLevelMenuDownButtonFunction(sguiButton_struct* b)
{
	screenListMove(&testScreenList, 1);
	updateScreenList(&testScreenList);
}

static void loadLevelMenuOKButtonFunction(sguiButton_struct* b)
{
	static char str[2048];
	sprintf(str,"%s/%s/maps/%s",basePath,ROOT,testScreenList.list[testScreenList.cursor]);

	setEditorMapFilePath(str);
	changeState(&editorState);
}

static void loadLevelMenuBackButtonFunction(sguiButton_struct* b)
{
	testTransition=startCameraTransition(&cameraStates[3],&cameraStates[2],64);
	setupMenuPage(createMenuPage, createMenuPageLength);
	freeFileList(testList, testListCnt);
	testList=NULL;
}



static void freeFileList(char** list, int length)
{
	if(!list)return;

	int i;
	for(i=0;i<length;i++)
	{
		if(list[i]){free(list[i]);list[i]=NULL;}
	}
	free(list);
}

static int listFiles(char* path, char** list)
{
	if(!path)return 0;

	char currentPath[255];
	getcwd(currentPath,255);

	chdir(path);

	struct dirent *ent;
	struct stat st;
	DIR* dir=opendir(".");

	int cnt=0;
	while((ent=readdir(dir)))
	{
		stat(ent->d_name,&st);
		if(!S_ISDIR(st.st_mode) && strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
		{
			//dirty .map filter
			int l=strlen(ent->d_name);
			if(l>4 && ent->d_name[l-1]=='p' && ent->d_name[l-2]=='a' && ent->d_name[l-3]=='m' && ent->d_name[l-4]=='.')
			{
				if(list)
				{
					list[cnt]=malloc(strlen(ent->d_name)+1);
					strcpy(list[cnt],ent->d_name);
				}
				cnt++;
			}
		}
	}
	closedir(dir);

	chdir(currentPath);

	return cnt;
}