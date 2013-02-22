#include "game/game_main.h"

bool currentBuffer;
int mainBG;
u16 mainScreen[256*192];

bool testStepByStep=false;

PrintConsole bottomScreen;

// extern md2Model_struct storageCubeModel, companionCubeModel, cubeDispenserModel; //TEMP
cubeDispenser_struct* testDispenser;
bigButton_struct* testButton;
bigButton_struct* testButton2;
platform_struct* testPlatform;

u16 vblCNT, frmCNT, FPS;

void initGame(void)
{
	int oldv=getMemFree();
	NOGBA("mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("initializing...");
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_0_2D);
	
	glInit();
	
	vramSetPrimaryBanks(VRAM_A_TEXTURE,VRAM_B_TEXTURE,VRAM_C_LCD,VRAM_D_MAIN_BG_0x06000000);
	vramSetBankH(VRAM_H_SUB_BG);
	vramSetBankI(VRAM_I_SUB_BG_0x06208000);
	
	glEnable(GL_TEXTURE_2D);
	// glEnable(GL_ANTIALIAS);
	glEnable(GL_BLEND);
	glEnable(GL_OUTLINE);
	
	glSetOutlineColor(0,RGB15(0,0,0)); //TEMP?
	glSetOutlineColor(7,RGB15(31,0,0)); //TEMP?
	glSetToonTableRange(0, 15, RGB15(8,8,8)); //TEMP?
	glSetToonTableRange(16, 31, RGB15(24,24,24)); //TEMP?
	
	glClearColor(31,31,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);

	glViewport(0,0,255,191);
	
	initVramBanks(1);
	initTextures();
	
	initCamera(NULL);
	
	initPlayer(NULL);
	
	initLights();
	initParticles();
	
	initLightMaps();
	initMaterials();
	
	loadMaterialSlices("slices.ini");
	loadMaterials("materials.ini");
	
	initTurrets();
	initBigButtons();
	initEnergyBalls();
	initPlatforms();
	initCubes();
	initEmancipation();
	initDoors();
	
	NOGBA("lalala");

	getPlayer()->currentRoom=&gameRoom;
	
	currentBuffer=false;
	
	getVramStatus();
	fadeIn();
	
	mainBG=bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(mainBG, 0);
	REG_BG0CNT=BG_PRIORITY(3);
	
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 16, 0, false, true);
	consoleSelect(&bottomScreen);
	
	// glSetToonTableRange(0, 14, RGB15(16,16,16));
	// glSetToonTableRange(15, 31, RGB15(26,26,26));
	
	initPortals();
	
	//PHYSICS
	initPI9();

	// readMap("lalala.map", NULL);
	// newReadMap("../testedit.map", NULL);
	newReadMap("test.map", NULL);
	
	// testButton=createBigButton(NULL, vect(10,0,10)); //TEMP
	// testButton2=createBigButton(NULL, vect(6,0,4)); //TEMP
	// testDispenser=createCubeDispenser(NULL, vect(4,0,4), true); //TEMP
	// createEnergyDevice(NULL, vect(0,7,9), pX, true); //TEMP
	// createEnergyDevice(NULL, vect(20,0,9), pY, false); //TEMP
	// createPlatform(vect(TILESIZE*2,TILESIZE,TILESIZE*4),vect(TILESIZE*10,TILESIZE,TILESIZE*4), true); //TEMP
	// testPlatform=createPlatform(vect(-TILESIZE*2,TILESIZE,TILESIZE*4),vect(-TILESIZE*2,TILESIZE*4,TILESIZE*4), true); //TEMP
	// addActivatorTarget(&testButton2->activator,(void*)testDispenser,DISPENSER_TARGET); //TEMP
	// addActivatorTarget(&testButton->activator,(void*)testPlatform,PLATFORM_TARGET); //TEMP
	// createEmancipationGrid(NULL,vect(0,0,7),TILESIZE*8,false);
	
	transferRectangles(&gameRoom);
	makeGrid();
	generateRoomGrid(&gameRoom);
	
	getVramStatus();
	
	startPI();
	
	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("vs mem free : %dko (%do)",oldv/1024,oldv);
}

bool testbool=false;
bool switchPortal=false;
portal_struct *currentPortal, *previousPortal;

void postProcess(u16* scrP, u32* stackP);
bool orangeSeen, blueSeen;
extern u16** stackEnd;
u16* ppStack[192*16];

extern u8 selectID;
extern OBB_struct objects[NUMOBJECTS];

u32 prevTiming;

u32 cpuEndSlice()
{
	u32 temp=prevTiming;
	prevTiming=cpuGetTiming();
	return prevTiming-temp;
}

static inline void render1(void)
{
	scanKeys();
	
	playerControls(NULL);
		// if(keysDown()&KEY_X)createBox(vect(TILESIZE,TILESIZE,TILESIZE),vect(-inttof32(0),HEIGHTUNIT*26,-inttof32(0)),inttof32(1));
		if(keysDown()&KEY_X)createTurret(NULL, vect(-inttof32(0),26,-inttof32(0)));
		// if(keysDown()&KEY_SELECT)createBox(vectMultInt(vect(-inttof32(0),HEIGHTUNIT*26,-inttof32(0)),4),inttof32(1),&companionCubeModel);
		if(keysDown()&KEY_B)applyForce(selectID, vect(-TILESIZE*4,0,0), vect(0,inttof32(150),0));
		if(keysDown()&KEY_Y){selectID++;selectID%=NUMOBJECTS;if(!objects[selectID].used)selectID=0;}
		
		if(objects[selectID].used && keysHeld()&KEY_A)
		{
			camera_struct* c=getPlayerCamera();
			// applyForce(selectID, vect(0,0,0), vect(0,inttof32(2)*5,0));
			// applyForce(selectID, vect(0,0,0), vectMultInt(normalize(vectDifference(vectMultInt(addVect(getPlayer()->object->position,vectDivInt(vect(-c->transformationMatrix[2],-c->transformationMatrix[5],-c->transformationMatrix[8]),8)),4),objects[selectID].position)),100));
			setVelocity(selectID, vectMultInt(/*normalize*/(vectDifference(vectMultInt(addVect(getPlayer()->object->position,vectDivInt(vect(-c->transformationMatrix[2],-c->transformationMatrix[5],-c->transformationMatrix[8]),4)),4),objects[selectID].position)),4));
			// updatePlayerPI(NULL);
			changeAnimation(&getPlayer()->modelInstance,2,false);
		}else if(keysUp()&KEY_A){changeAnimation(&getPlayer()->modelInstance,0,false);changeAnimation(&getPlayer()->modelInstance,1,true);}
	// iprintf("controls : %d  \n",cpuEndSlice());
	
		updatePlayer(NULL);
	// iprintf("player : %d  \n",cpuEndSlice());
	
		updatePortals();
		updateTurrets();
		updateBigButtons();
		updateEnergyDevices();
		updateEnergyBalls();
		updatePlatforms();
		updateCubeDispensers();
		updateEmancipators();
		updateEmancipationGrids();
		updateDoors();
	// iprintf("updates : %d  \n",cpuEndSlice());
	
	// if(currentPortal)GFX_CLEAR_COLOR=currentPortal->color|(31<<16);
	// else GFX_CLEAR_COLOR=0;
	u16 color=getCurrentPortalColor(getPlayer()->object->position);
	// NOGBA("col %d",color);
	// GFX_CLEAR_COLOR=color|(31<<16);
	GFX_CLEAR_COLOR=RGB15(0,0,0)|(31<<16);
	
		if(fifoCheckValue32(FIFO_USER_08))iprintf("\x1b[0J");
		while(fifoCheckValue32(FIFO_USER_08)){int32 cnt=fifoGetValue32(FIFO_USER_08);iprintf("ALERT %d      \n",cnt);NOGBA("ALERT %d      \n",cnt);}
	
	projectCamera(NULL);

	glPushMatrix();
		
		glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);
		
		renderGun(NULL);
		
		transformCamera(NULL);
		
		cpuEndSlice();
			drawRoomsGame(128, color);
			// drawCell(getCurrentCell(getPlayer()->currentRoom,getPlayerCamera()->position));
		iprintf("room : %d  \n",cpuEndSlice());
		
		updateParticles();
		drawParticles();
		// iprintf("particles : %d  \n",cpuEndSlice());
		
			drawOBBs();
		// iprintf("OBBs : %d  \n",cpuEndSlice());
			drawBigButtons();
			drawEnergyDevices();
			drawEnergyBalls();
			drawPlatforms();
			drawCubeDispensers();
			drawTurretsStuff();
			drawEmancipators();
			drawEmancipationGrids();
			drawDoors();
		// iprintf("stuff : %d  \n",cpuEndSlice());
		
		drawPortal(&portal1);
		drawPortal(&portal2);
			
	glPopMatrix(1);
	
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
		drawEnergyDevices();
		drawEnergyBalls();
		drawPlatforms();
		drawCubeDispensers();
		drawTurretsStuff();
		drawEmancipators();
		drawEmancipationGrids();
		drawDoors();

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

void gameFrame(void)
{
	int lala;
	switch(currentBuffer)
	{
		case false:
			iprintf("\x1b[0;0H");
			iprintf("%d FPS   \n", FPS);
			cpuEndSlice();
			postProcess1();
			// iprintf("postproc : %d  \n",cpuEndSlice());
			render1();

			if(keysDown()&KEY_SELECT)testStepByStep^=1; //TEMP
			iprintf("full : %d (%d)  \n",cpuEndTiming(),testStepByStep);
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
			iprintf("fake frame : %d   \n",cpuEndTiming());
			swiWaitForVBlank();
			cpuStartTiming(0);
			prevTiming=0;
			dmaCopy(VRAM_C, mainScreen, 256*192*2);			
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			break;
	}
	
	// if(testStepByStep){while(!(keysUp()&KEY_TOUCH))scanKeys();scanKeys();scanKeys();if(keysHeld()&KEY_SELECT)testStepByStep=false;}
	// else if(keysDown()&KEY_SELECT)testStepByStep=true;
	
	currentBuffer^=1;
}

void killGame(void)
{
	fadeOut();
	NOGBA("KILLING IT");
	freePlayer();
	freeState(NULL);
	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
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
