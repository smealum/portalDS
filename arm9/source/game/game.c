#include "game/game_main.h"

bool currentBuffer;
int mainBG;
u16 mainScreen[256*192];

PrintConsole bottomScreen;

extern roomEdit_struct roomEdits[NUMROOMEDITS];

// extern md2Model_struct storageCubeModel, companionCubeModel, cubeDispenserModel; //TEMP
cubeDispenser_struct* testDispenser;
bigButton_struct* testButton;
bigButton_struct* testButton2;
platform_struct* testPlatform;

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
	
	glClearColor(31,31,0,31);
	glClearPolyID(63);
	glClearDepth(0x7FFF);

	glViewport(0,0,255,191);
	
	initVramBanks(1);
	initTextures();
	
	initPathfindingGlobal();
	
	initRoomEditor();
	
	initCamera(NULL);
	
	initPlayer(NULL);
	
	initParticles();
	
	initLightMaps();
	initMaterials();
	initDoorCollection(NULL);
	
	loadMaterialSlices("slices.ini");
	loadMaterials("materials.ini");
	
	// initEnemies();
	initTurrets();
	initBigButtons();
	initEnergyBalls();
	initPlatforms();
	initCubes();
		
	readMap("lalala.map", true);
	
	currentBuffer=false;
	
	getVramStatus();
	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
	NOGBA("vs mem free : %dko (%do)",oldv/1024,oldv);
	fadeIn();
	
	mainBG=bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(mainBG, 0);
	REG_BG0CNT=BG_PRIORITY(3);
	
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 16, 0, false, true);
	consoleSelect(&bottomScreen);
	
	initPortals();
	
	//PHYSICS
	initPI9();
	
	updatePlayer(NULL);testButton=createBigButton(NULL, vect(10,0,10)); //TEMP
	testButton2=createBigButton(NULL, vect(6,0,4)); //TEMP
	testDispenser=createCubeDispenser(NULL, vect(4,0,4), true); //TEMP
	createEnergyDevice(NULL, vect(0,7,7), pX, false); //TEMP
	testPlatform=createPlatform(vect(-TILESIZE*2,TILESIZE,TILESIZE*4),vect(-TILESIZE*2,TILESIZE*4,TILESIZE*4), true); //TEMP
	// addActivatorTarget(&testButton2->activator,(void*)testDispenser,DISPENSER_TARGET);//
	addActivatorTarget(&testButton->activator,(void*)testPlatform,PLATFORM_TARGET);//
	
	transferRectangles(&roomEdits[0].data);
	makeGrid();
	
	getVramStatus();
	
	startPI();
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

static inline void render1(void)
{
	scanKeys();
	
	playerControls(NULL);
		// if(keysDown()&KEY_X)createBox(vect(TILESIZE,TILESIZE,TILESIZE),vect(-inttof32(0),HEIGHTUNIT*26,-inttof32(0)),inttof32(1));
		if(keysDown()&KEY_X)createTurret(vectMultInt(vect(-inttof32(0),HEIGHTUNIT*26,-inttof32(0)),4));
		// if(keysDown()&KEY_SELECT)createBox(vectMultInt(vect(-inttof32(0),HEIGHTUNIT*26,-inttof32(0)),4),inttof32(1),&companionCubeModel);
		if(keysDown()&KEY_SELECT)testDispenser->active=true;
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
	
	// cpuStartTiming(0);
		updatePlayer(NULL);
	// iprintf("player : %d  \n",cpuEndTiming());
	// cpuStartTiming(0);
		updatePortals();
		updateTurrets();
		updateBigButtons();
		updateEnergyDevices();
		updateEnergyBalls();
		updatePlatforms();
		updateCubeDispensers();
	// iprintf("updates : %d  \n",cpuEndTiming());
	
	// if(currentPortal)GFX_CLEAR_COLOR=currentPortal->color|(31<<16);
	// else GFX_CLEAR_COLOR=0;
	GFX_CLEAR_COLOR=0;
	
		while(fifoCheckValue32(FIFO_USER_08)){u32 cnt=fifoGetValue32(FIFO_USER_08);iprintf("ALERT %d      \n",cnt);}
	
	projectCamera(NULL);

	glPushMatrix();
		
		glScalef32(SCALEFACT,SCALEFACT,SCALEFACT);
		
		renderGun(NULL);
		
		transformCamera(NULL);
		
		// glPushMatrix();
			// glPushMatrix();
				// glTranslate3f32(-TILESIZE*5,0,0);
				// renderModelFrameInterp(0, 0, 0, &testCatcher, POLY_ALPHA(31) | POLY_CULL_NONE | POLY_FORMAT_LIGHT0, false, NULL);
			// glPopMatrix(1);
		// glPopMatrix(1);
		
		drawRoomsGame(128);
		
		updateParticles();
		drawParticles();
		
		// cpuStartTiming(0);
			drawOBBs();
		// iprintf("OBBs : %d  \n",cpuEndTiming());
		// cpuStartTiming(0);
			drawBigButtons();
			drawEnergyDevices();
			drawEnergyBalls();
			drawPlatforms();
			drawCubeDispensers();
		// iprintf("stuff : %d  \n",cpuEndTiming());
		
		drawPortal(&portal1);
		drawPortal(&portal2);
			
	glPopMatrix(1);
	
	glFlush(0);
}

static inline void render2(void)
{	
	if(!(orangeSeen||blueSeen)){previousPortal=NULL;return;}
	
	if((switchPortal||!blueSeen)&&orangeSeen)currentPortal=&portal1;
	else currentPortal=&portal2;
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
		u16* scrp=NULL, oscrp=NULL;
		blueSeen=orangeSeen=false;
		for(stp=ppStack;stp<stackEnd;stp++)
		{
			scrp=(*stp)-p1;
			int val=(int)scrp;
			if(val>256*192*2){blueSeen=true;val-=256*192*2;scrp=(u16*)val;if((int)scrp-(int)oscrp>0)dmaCopy((void*)((int)p3+(int)oscrp*2), (void*)((int)bgGetGfxPtr(mainBG)+(int)oscrp*2), (int)((int)scrp-(int)oscrp)*2);}
			else if(val>256*192){orangeSeen=true;val-=256*192;scrp=(u16*)val;if((int)scrp-(int)oscrp>0)dmaCopy((void*)((int)p2+(int)oscrp*2), (void*)((int)bgGetGfxPtr(mainBG)+(int)oscrp*2), (int)((int)scrp-(int)oscrp)*2);}
			else if((int)scrp-(int)oscrp>0){dmaCopy((void*)((int)p1+(int)oscrp*2), (void*)((int)bgGetGfxPtr(mainBG)+(int)oscrp*2), (int)((int)scrp-(int)oscrp)*2);}
			oscrp=scrp;
		}
}

void gameFrame(void)
{
	int lala;
	switch(currentBuffer)
	{
		case false:
			iprintf("\x1b[0;0H");
			// cpuStartTiming(0);
			postProcess1();
			// iprintf("frm 1 : %d  \n",cpuGetTiming());
			render1();
			// iprintf("frm 1 : %d  \n",cpuEndTiming());
			swiWaitForVBlank();
			if(previousPortal)dmaCopy(VRAM_C, previousPortal->viewPoint, 256*192*2);			
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			break;
		case true:
			// cpuStartTiming(0);
			postProcess2();
			// iprintf("frm 2 : %d  \n",cpuGetTiming());
			render2();
			listenPI9();
			// iprintf("frm 2 : %d  \n",cpuEndTiming());
			swiWaitForVBlank();	
			dmaCopy(VRAM_C, mainScreen, 256*192*2);			
			setRegCapture(true, 0, 15, 2, 0, 3, 1, 0);
			break;
	}
	
	currentBuffer^=1;
}

void killGame(void)
{
	fadeOut();
	NOGBA("KILLING IT");
	wipeMapEdit();
	// freeEnemies();
	freePlayer();
	freeState(NULL);
	NOGBA("START mem free : %dko (%do)",getMemFree()/1024,getMemFree());
}

void gameVBL(void)
{

}
