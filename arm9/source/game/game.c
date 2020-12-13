#include "game/game_main.h"

bool currentBuffer;
int mainBG;
u16 mainScreen[256*192];

bool isNextRoom;

char mapFilePath[2048];
char nextMapFilePath[2048];

char levelTitle[32];
char levelAuthor[32];

s16 levelInfoCounter;

bool testStepByStep=false;

PrintConsole bottomScreen;

// extern md2Model_struct storageCubeModel, companionCubeModel, cubeDispenserModel; //TEMP
cubeDispenser_struct* testDispenser;
bigButton_struct* testButton;
bigButton_struct* testButton2;
platform_struct* testPlatform;

u16 vblCNT, frmCNT, FPS;

void setMapFilePath(char* path)
{
	if(!path)return;

	strcpy(mapFilePath,path);
}

void setNextMapFilePath(char* path)
{
	if(!path)return;

	strcpy(nextMapFilePath,path);
}

void setLevelInfo(char* title, char* author)
{
	levelTitle[0]='\0';
	levelAuthor[0]='\0';
	if(title)strcpy(levelTitle, title); //enforce length limit
	if(author)sprintf(levelAuthor, "by %s", author); //enforce length limit
}

void endGame(void)
{
	if(!isNextRoom)changeState(&menuState);
	else {
		strcpy(mapFilePath,nextMapFilePath);
		changeState(&gameState);
	}
}

void initGame(void)
{
	lcdMainOnTop();
	int oldv=getMemFree();
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("initializing...");
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	glInit();

	vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_LCD,VRAM_D_MAIN_BG_0x06000000);
	vramSetBankH(VRAM_H_SUB_BG);
	vramSetBankI(VRAM_I_SUB_BG_0x06208000);

	glEnable(GL_TEXTURE_2D);
	// glEnable(GL_ANTIALIAS);
	glDisable(GL_ANTIALIAS);
	glEnable(GL_BLEND);
	glEnable(GL_OUTLINE);

	glSetOutlineColor(0,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(1,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(7,RGB15(31,0,0)); //TEMP?
	glSetToonTableRange(0, 15, RGB15(8,8,8)); //TEMP?
	glSetToonTableRange(16, 31, RGB15(24,24,24)); //TEMP?

	glClearColor(31,31,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);

	glViewport(0,0,255,191);

	// initVramBanks(1);
	initVramBanks(2);
	initTextures();
	initSound();

	initCamera(NULL);

	initPlayer(NULL);

	initLights();
	// Not used
	//initParticles();

	initMaterials();

	loadMaterialSlices("slices.ini");
	loadMaterials("materials.ini");
	loadControlConfiguration("config.ini");

	initElevators();
	initWallDoors();
	initTurrets();
	initBigButtons();
	initTimedButtons();
	initEnergyBalls();
	initPlatforms();
	initCubes();
	initEmancipation();
	initDoors();
	initSludge();
	initPause();

	initText();

	NOGBA("lalala");

	getPlayer()->currentRoom=&gameRoom;

	currentBuffer=false;

	getVramStatus();
	fadeIn();

	mainBG=bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(mainBG, 0);
	REG_BG0CNT=BG_PRIORITY(3);

	#ifdef DEBUG_GAME
		consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 16, 0, false, true);
		consoleSelect(&bottomScreen);
	#endif

	// glSetToonTableRange(0, 14, RGB15(16,16,16));
	// glSetToonTableRange(15, 31, RGB15(26,26,26));

	initPortals();

	//PHYSICS
	initPI9();

	strcpy(&mapFilePath[strlen(mapFilePath)-3], "map");
	newReadMap(mapFilePath, NULL, 255);

	transferRectangles(&gameRoom);
	makeGrid();
	generateRoomGrid(&gameRoom);
	gameRoom.displayList=generateRoomDisplayList(&gameRoom, vect(0,0,0), vect(0,0,0), false);

	getVramStatus();

	startPI();

	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("vs mem free : %dko (%do)",oldv/1024,oldv);

	levelInfoCounter=60;
}

bool testbool=false;
bool switchPortal=false;
//portal_struct *currentPortal,
portal_struct *previousPortal;

void postProcess(u16* scrP, u32* stackP);
bool orangeSeen, blueSeen;
extern u16** stackEnd;
u16* ppStack[192*16];

u32 prevTiming;

u32 cpuEndSlice()
{
	u32 temp=prevTiming;
	prevTiming=cpuGetTiming();
	return prevTiming-temp;
}

void drawCenteredString(char* str, int32 scale, u16 y)
{
	if(!str)return;
	int l=strlen(str);

	drawString(str, RGB15(31,31,31), scale, inttof32(128)-(l*scale)*4, inttof32(y));
}

static inline void render1(void)
{
	scanKeys();

	// cpuEndSlice();
	playerControls(NULL);
	updateControls();
	// iprintf("controls : %d  \n",cpuEndSlice());

		updatePlayer(NULL);
	// iprintf("player : %d  \n",cpuEndSlice());

		updatePortals();
		updateTurrets();
		updateBigButtons();
		updateTimedButtons();
		updateEnergyDevices();
		updateEnergyBalls();
		updatePlatforms();
		updateCubeDispensers();
		updateEmancipators();
		updateEmancipationGrids();
		updateDoors();
		updateWallDoors();
	// iprintf("updates : %d  \n",cpuEndSlice());

	// if(currentPortal)GFX_CLEAR_COLOR=currentPortal->color|(31<<16);
	// else GFX_CLEAR_COLOR=0;
	u16 color=getCurrentPortalColor(getPlayer()->object->position);
	// NOGBA("col %d",color);
	// GFX_CLEAR_COLOR=color|(31<<16);
	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);

	#ifdef DEBUG_GAME
		if(fifoCheckValue32(FIFO_USER_08))iprintf("\x1b[0J");
		while(fifoCheckValue32(FIFO_USER_08)){int32 cnt=fifoGetValue32(FIFO_USER_08);iprintf("ALERT %ld      \n",cnt);NOGBA("ALERT %d      \n",cnt);}
	#else
		while(fifoCheckValue32(FIFO_USER_08)){int32 cnt=fifoGetValue32(FIFO_USER_08);NOGBA("ALERT %ld      \n",cnt);}
	#endif

	projectCamera(NULL);

	glPushMatrix();

		glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);

		renderGun(NULL);

		transformCamera(NULL);

		cpuEndSlice();
			// drawRoomsGame(128, color);
			drawRoomsGame(0, color);
			// drawCell(getCurrentCell(getPlayer()->currentRoom,getPlayerCamera()->position));
		// iprintf("room : %d  \n",cpuEndSlice());
		// Not used
		//updateParticles();
		//drawParticles();
		// iprintf("particles : %d  \n",cpuEndSlice());

			drawOBBs();
		// iprintf("OBBs : %d  \n",cpuEndSlice());
			drawBigButtons();
			drawTimedButtons();
			drawEnergyDevices();
			drawEnergyBalls();
			drawPlatforms();
			drawCubeDispensers();
			drawTurretsStuff();
			drawEmancipators();
			drawEmancipationGrids();
			drawDoors();
			drawWallDoors(NULL);
			drawSludge(&gameRoom);
		// iprintf("stuff : %d  \n",cpuEndSlice());

		drawPortal(&portal1);
		drawPortal(&portal2);

	glPopMatrix(1);

	//HUD TEST
	if(levelInfoCounter>0 && (levelTitle[0] || levelAuthor[0]))
	{
		levelInfoCounter--;
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrthof32(inttof32(0), inttof32(255), inttof32(191), inttof32(0), -inttof32(1), inttof32(1));

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();

				if(levelTitle[0])drawCenteredString(levelTitle, inttof32(17)/10, (82));
				if(levelAuthor[0])drawCenteredString(levelAuthor, inttof32(1), (100));

			glPopMatrix(1);
			glMatrixMode(GL_PROJECTION);
		glPopMatrix(1);
	}

	glFlush(0);
}

static inline void render2(void)
{
	if(!(orangeSeen||blueSeen)){previousPortal=NULL;return;}
	if((orangeSeen&&blueSeen)/*||(!orangeSeen&&!blueSeen)*/)
	{
		if(switchPortal)currentPortal=&portal1;
		else currentPortal=&portal2;
	}else if(orangeSeen)currentPortal=&portal1;
	else if(blueSeen)currentPortal=&portal2;

	previousPortal=currentPortal;

	switchPortal^=1;

	glClearColor(0,0,0,31);

	updatePortalCamera(currentPortal, NULL);
	projectCamera(&currentPortal->camera);

	glPushMatrix();

		glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);

		renderGun(NULL);

		transformCamera(&currentPortal->camera);

		// drawRoomsGame(0);
		drawPortalRoom(currentPortal);

		drawPlayer(NULL);

		drawOBBs();
		drawBigButtons();
		drawTimedButtons();
		drawEnergyDevices();
		drawEnergyBalls();
		drawPlatforms();
		drawCubeDispensers();
		drawTurretsStuff();
		drawEmancipators();
		drawEmancipationGrids();
		drawDoors();
		drawWallDoors(currentPortal);
		drawSludge(&gameRoom);

	glPopMatrix(1);

	glFlush(0);
}

static inline void postProcess1(void)
{
	postProcess(mainScreen,ppStack);
}

static inline void postProcess2(void)
{
	u16* p1=mainScreen;
	u16* p2=portal1.viewPoint;
	u16* p3=portal2.viewPoint;

	u16** stp;
	blueSeen=orangeSeen=false;
	int oval=0;
	for(stp=ppStack;stp<stackEnd;stp++)
	{
		int val=(int)((*stp)-p1);
		if(val>256*192*2){blueSeen=true;val-=256*192*2;if(p1[oval-1]==65504)oval--;if(p1[val]==65504)val++;if(val-oval>0)dmaCopy(&(p3[oval]), &(bgGetGfxPtr(mainBG)[oval]), (val-oval)*2);}
		else if(val>256*192){orangeSeen=true;val-=256*192;if(p1[oval-1]==33791)oval--;if(p1[val]==33791)val++;if(val-oval>0)dmaCopy(&(p2[oval]), &(bgGetGfxPtr(mainBG)[oval]), (val-oval)*2);}
		else {if(val-oval>0)dmaCopy(&(p1[oval]), &(bgGetGfxPtr(mainBG)[oval]), (val-oval)*2);}
		oval=val;
	}
}

u32 debugVal; //TEMP

void gameFrame(void)
{
	switch(currentBuffer)
	{
		case false:
			#ifdef DEBUG_GAME
				iprintf("\x1b[0;0H");
				iprintf("%d FPS   \n", FPS);
				iprintf("%d (debug)   \n", debugVal);
				iprintf("%d (free ram)   \n", getMemFree()/1024);
				iprintf("%p (portal)   \n", portal1.displayList);
				iprintf("%p (portal)   \n", portal2.displayList);
			#endif
			cpuEndSlice();
			postProcess1();
			// iprintf("postproc : %d  \n",cpuEndSlice());
			render1();

			// if(keysDown()&KEY_SELECT)testStepByStep^=1; //TEMP
			#ifdef DEBUG_GAME
				iprintf("full : %d (%d)  \n",cpuEndTiming(),testStepByStep);
			#endif
			swiWaitForVBlank();
			cpuStartTiming(0);
			prevTiming=0;
			if(previousPortal)dmaCopy(VRAM_C, previousPortal->viewPoint, 256*192*2);
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			frmCNT++;
			break;
		case true:
			// cpuStartTiming(0);
			postProcess2();
			// iprintf("frm 2 : %d  \n",cpuGetTiming());
			render2();
			listenPI9();
			updateOBBs();
			// iprintf("frm 2 : %d  \n",cpuEndTiming());
			#ifdef DEBUG_GAME
				iprintf("fake frame : %d   \n",cpuEndTiming());
			#endif
			swiWaitForVBlank();
			cpuStartTiming(0);
			prevTiming=0;
			dmaCopy(VRAM_C, mainScreen, 256*192*2);
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			break;
	}

	// if(testStepByStep){int i=0;while(!(keysUp()&KEY_TOUCH)){scanKeys();listenPI9();swiWaitForVBlank();}NOGBA("WAITED");scanKeys();scanKeys();if(keysHeld()&KEY_SELECT)testStepByStep=false;}
	// else if(keysDown()&KEY_SELECT)testStepByStep=true;

	currentBuffer^=1;
}

void killGame(void)
{
	fadeOut();
	NOGBA("KILLING IT");
	freePlayer();
	freeEnergyBalls();
	freeBigButtons();
	freeCubes();
	freeDoors();
	freeElevators();
	freeEmancipation();
	freePlatforms();
	freeTimedButtons();
	freeTurrets();
	freeWallDoors();
	freeSludge();
	freeRoom(&gameRoom);
	freePortals();
	freeState(NULL);
	freeSound();
	freePause();

	resetAllPI();

	NOGBA("END mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void gameVBL(void)
{
	vblCNT++;
	if(vblCNT>=60)
	{
		FPS=frmCNT;
		frmCNT=vblCNT=0;
	}
}
